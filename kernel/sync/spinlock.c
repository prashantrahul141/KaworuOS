#include "sync/spinlock.h"
#include "aarch64/aarch64.h"
#include "irq/irq_controller.h"
#include "config.h"
#include "core/cpu.h"
#include "debug/panic.h"

/*
 * If the lock is held and by this cpu.
 */
inline static bool holding(const SpinLock *sp)
{
	return sp->locked && sp->cpu == this_cpu();
}

void spinlock_init(SpinLock *sp, const i8 *name)
{
	sp->locked = false;
	sp->name = name;
	sp->cpu = nullptr;
}

void spinlock_acquire(SpinLock *sp)
{
	/* push to stack */
	irq_push_intr();

	if (holding(sp)) {
		panic("failed to acquire lock, already holding it.\n\tcpuid = "
		      "%d\n\tname = %s\n",
		      cpu_get_cpuid(), sp->name);
	}

	/*
	 * force fencing here so that it is safe to acquire lock
	 */
	dmb(BARRIER_ISH);

	/*
	 * lock using atomic operation.
	 *
	 * __atomic_exchange_n is a clang compile builtin, which writes value,
	 * to given pointer and returns old value in a "single" instruction. For
	 * me it seems to be use ldaxrb and stlxrb on debug builds.
	 */
	while (0 != __atomic_exchange_n(&sp->locked, true, __ATOMIC_ACQUIRE)) {
		cpu_relax();
	}

	/* update which cpui has the lock */
	sp->cpu = this_cpu();
}

void spinlock_release(SpinLock *sp)
{
	if (!holding(sp)) {
		panic("failed to release lock, not holding it.\n\tcpuid = "
		      "%d\n\tname = %s\n",
		      cpu_get_cpuid(), sp->name);
	}

	sp->cpu = nullptr;

	/*
	 * release using atomic operation.
	 * __atomic_store_n writes the value in the given memory atomically.
	 * For me it seems to be using stlrb on debug builds.
	 */
	__atomic_store_n(&sp->locked, false, __ATOMIC_RELEASE);

	/*
	 * force fencing here so that it is safe to release the lock
	 */
	dmb(BARRIER_ISH);

	/* pop from stack */
	irq_pop_intr();
}
