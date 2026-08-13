#ifndef _ARCH64_H_
#define _ARCH64_H_

#include "common_defs.h"
#include "types.h"

/* max virtual address */
constexpr usize MAX_VA = (1L << 9 << 9 << 9);

constexpr usize DAIF_I = (1 << 7);
constexpr usize DAIF_F = (1 << 6);

/* mmu enabled */
constexpr usize SCTLR_M = (1 << 0);
/* alignment check for address translation lookup */
constexpr usize SCTLR_A = (1 << 1);
/* cacheability */
constexpr usize SCTLR_C = (1 << 2);
/* stage 1 instruction cacheability */
constexpr usize SCTLR_I = (1 << 12);

/* barrier all stores */
#define BARRIER_ISHST "ISHST"
/* all operations in the inner shareable */
#define BARRIER_ISH   "ISH"
/* fullsystem operation: this is default */
#define BARRIER_ALL   "SY"

/* break */
#define brk(imm) asm volatile("brk #" #imm)

/* udf */
#define udf(imm) asm volatile("udf #" #imm)

/* Data sync barrier */
#define dsb(_type) asm volatile("dsb " _type)
#define dmb(_type) asm volatile("dsb " _type)
#define isb()	   asm volatile("isb")

/* read mpidr_el1 */
static inline u64 r_mpidr_el1(void)
{
	u64 mpidr;
	asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));
	return mpidr;
}

/* read daif */
static inline u64 r_daif(void)
{
	u64 daif;
	asm volatile("mrs %0, daif" : "=r"(daif));
	return daif;
}

/* are device interrupts enabled? */
static inline bool r_intrd_enabled(void)
{
	u64 daif = r_daif();
	u64 masked = (daif & (DAIF_F | DAIF_I));
	return masked == 0;
}

/* disable device interrupts */
static inline void w_intrd_disable(void)
{
	asm volatile("msr daifset, #0b0011");
}

/* enable device interrupts */
static inline void w_intrd_enable(void)
{
	asm volatile("msr daifclr, #0b0011");
}

/* disable all interrupts */
static inline void w_intrall_disable(void)
{
	asm volatile("msr daifset, #0b1111");
}

/* halt */
#define hlt(imm) asm volatile("hlt #" #imm)

/* wfe */
#define wfe() asm volatile("wfe")

/* wfi */
#define wfi() asm volatile("wfi")

/* read sctlr register */
static inline u64 r_sctlr(void)
{
	u64 read;
	asm volatile("mrs %0, SCTLR_EL1" : "=r"(read));
	return read;
}

/* write to sctlr register */
static inline void w_sctlr(u64 value)
{
	asm volatile("msr SCTLR_EL1, %0" : : "r"(value));
}

/* enable mmu */
static inline void enable_mmu(void)
{
	u64 value = r_sctlr() | SCTLR_M;
	w_sctlr(value);
	isb();
}

/* read tcr_el1 */
static inline u64 r_tcr_el1(void)
{
	u64 value;
	asm volatile("mrs %0, TCR_EL1" : "=r"(value));
	return value;
}

/* write to tcr_el1 */
static inline void w_tcr_el1(u64 value)
{
	asm volatile("msr TCR_EL1, %0" : : "r"(value));
}

/* read mair_el1 */
static inline u64 r_mair_el1(void)
{
	u64 value;
	asm volatile("mrs %0, MAIR_EL1" : "=r"(value));
	return value;
}

/* write mair_el1 */
static inline void w_mair_el1(u64 value)
{
	asm volatile("msr MAIR_EL1, %0" : : "r"(value));
}

/* write ttbr0_el1: user process's virtual address space base addr */
static inline void w_ttbr0_el1(usize value)
{
	asm volatile("msr TTBR0_EL1, %0" : : "r"(value));
}

/* write ttbr1_el1: kernel's virtual address space base addr */
static inline void w_ttbr1_el1(usize value)
{
	asm volatile("msr TTBR1_EL1, %0" : : "r"(value));
}

/* flush tlb entries */
static inline void tlb_flush(void)
{
	dsb(BARRIER_ISHST);
	asm volatile("tlbi vmalle1");
	dsb(BARRIER_ISH);
}

static inline u64 r_cntfrq_el0(void)
{
	u64 value;
	asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(value));
	return value;
}

static inline u32 r_counter_timer_freq(void)
{
	return EXTRACT_BITS(r_cntfrq_el0(), 31, 0);
}

static inline u64 r_cntpct_el0(void)
{
	u64 value;
	asm volatile("mrs %0, CNTPCT_EL0" : "=r"(value));
	return value;
}

static inline u64 r_counter_timer_physical_count(void)
{
	return r_cntpct_el0();
}

static inline void w_cntp_tval_el0(u64 value)
{
	asm volatile("msr CNTP_TVAL_EL0, %0" : : "r"(value));
}

static inline u64 r_cntp_ctl_el0(void)
{
	u64 value;
	asm volatile("mrs %0, CNTP_CTL_EL0" : "=r"(value));
	return value;
}

static inline void w_cntp_ctl_el0(u64 value)
{
	asm volatile("msr CNTP_CTL_EL0, %0" : : "r"(value));
}

static inline bool r_counter_is_enabled(void)
{
	return r_cntp_ctl_el0() & BIT(0);
}

static inline void w_counter_enable(bool value)
{
	u64 old = r_cntp_ctl_el0();
	u64 new = value ? old | BIT(0) : old & ~BIT(0);
	w_cntp_ctl_el0(new);
}

static inline void w_cntp_cval_el0(u64 value)
{
	asm volatile("msr CNTP_CVAL_EL0, %0" : : "r"(value));
}

static inline void w_tpidr_el1(u64 value)
{
	asm volatile("msr TPIDR_EL1, %0" : : "r"(value));
}

static inline u64 r_tpidr_el1(void)
{
	u64 value;
	asm volatile("mrs %0, TPIDR_EL1" : "=r"(value));
	return value;
}

static inline void cpu_relax(void)
{
	asm volatile("yield");
}

#endif // _ARCH64_H_
