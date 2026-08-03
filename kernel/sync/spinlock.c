#include "sync/spinlock.h"
#include "aarch64/aarch64.h"
#include "config.h"
#include "core/cpu.h"
#include "debug/panic.h"

inline static bool holding(const SpinLock *sp);
static void push_intr(void);
static void pop_intr(void);

void spinlock_init(SpinLock *sp, const i8 *name)
{
	sp->locked = false;
	sp->name = name;
	sp->cpu = nullptr;
}

void spinlock_acquire(SpinLock *sp)
{
	/* push to stack */
	push_intr();

#ifdef CONFIG_DEBUG_CHECKS
	if (holding(sp)) {
		panic("failed to acquire lock, already holding it.\n\tcpuid = "
		      "%d\n\tname = %s\n",
		      cpu_get_cpuid(), sp->name);
	}
#endif

	/*
	 * force fencing here so that it is safe to acquire lock
	 * uses dmb	ish
	 */
	__sync_synchronize();

	/*
	 * lock using atomic operation.
	 *
	 * __atomic_exchange_n is a clang compile builtin, which writes value,
	 * to given pointer and returns old value in a "single" instruction. For
	 * me it seems to be use ldaxrb and stlxrb on debug builds.
	 */
	while (0 != __atomic_exchange_n(&sp->locked, true, __ATOMIC_ACQUIRE))
		;

	/* update which cpui has the lock */
	sp->cpu = this_cpu();
}

void spinlock_release(SpinLock *sp)
{
#ifdef CONFIG_DEBUG_CHECKS
	if (!holding(sp)) {
		panic("failed to release lock, not holding it.\n\tcpuid = "
		      "%d\n\tname = %s\n",
		      cpu_get_cpuid(), sp->name);
	}
#endif

	sp->cpu = nullptr;

	/*
	 * release using atomic operation.
	 * __atomic_store_n writes the value in the given memory atomically.
	 * For me it seems to be using stlrb on debug builds.
	 */
	__atomic_store_n(&sp->locked, false, __ATOMIC_RELEASE);

	/*
	 * force fencing here so that it is safe to release the lock
	 * uses dmb	ish
	 */
	__sync_synchronize();

	/* pop from stack */
	pop_intr();
}

/*
 * If the lock is held and by this cpu.
 */
inline static bool holding(const SpinLock *sp)
{
	return sp->locked && sp->cpu == this_cpu();
}

/* very similar to w_intrd_disable, w_intrd_enable but works like a stack. */
static void push_intr(void)
{
	bool enabled_previously = r_intrd_enabled();
	w_intrd_disable();

	Cpu *t_cpu = this_cpu();

	/* first push_intr */
	if (0 == t_cpu->count) {
		t_cpu->intrd_was_enabled = enabled_previously;
	}
	/* increment */
	t_cpu->count += 1;
}

static void pop_intr(void)
{
	Cpu *t_cpu = this_cpu();
	/* interrupts are already enabled? */
	if (r_intrd_enabled()) {
		panic("interrupts are already enabled\n\tcpuid = %d\n",
		      t_cpu->cpuid);
	}

	if (t_cpu->count < 1) {
		panic("underflow");
	}

	t_cpu->count -= 1;
	/* if we are the last pop_off && interrupts were enabled previously */
	if (0 == t_cpu->count && t_cpu->intrd_was_enabled) {
		w_intrd_enable();
	}
}
