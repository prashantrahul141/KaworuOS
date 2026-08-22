#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "aarch64/exception.h"

/* helper to check if exception is a syscall */
static inline bool is_syscall(ExceptionFrame *frame)
{
	return EXTRACT_BITS(frame->ESR_EL1, 31, 26) == 0b010101;
}

/*
 * dispatch syscall
 * returns if true if rescheduling is required
 */
bool syscall_dispatch(ExceptionFrame *frame);

#define SYSCALL_DECLARE(name) \
	SyscallReturn sys_##name(const ExceptionFrame *frame)

#define SYSCALL_DEFINE_N(name, n, args, frame_args)           \
	static SyscallReturn __sys_##name args;               \
	SyscallReturn sys_##name(const ExceptionFrame *frame) \
	{                                                     \
		(void)frame;                                  \
		return __sys_##name frame_args;               \
	}                                                     \
	static SyscallReturn __sys_##name args

#define SYSCALL_DEFINE0(name) SYSCALL_DEFINE_N(name, 0, (void), ())

#define SYSCALL_DEFINE1(name, t1, a1) \
	SYSCALL_DEFINE_N(name, 1, (t1 a1), ((t1)frame->x0))

#define SYSCALL_DEFINE2(name, t1, a1, t2, a2)     \
	SYSCALL_DEFINE_N(name, 2, (t1 a1, t2 a2), \
			 ((t1)frame->x0, (t2)frame->x1))

#define SYSCALL_DEFINE3(name, t1, a1, t2, a2, t3, a3)    \
	SYSCALL_DEFINE_N(name, 3, (t1 a1, t2 a2, t3 a3), \
			 ((t1)frame->x0, (t2)frame->x1), (t3)frame->x2)

#define SYSCALL_DEFINE4(name, t1, a1, t2, a2, t3, a3, t4, a4)           \
	SYSCALL_DEFINE_N(name, 4, (t1 a1, t2 a2, t3 a3, t4 a4),         \
			 ((t1)frame->x0, (t2)frame->x1), (t3)frame->x2, \
			 (t4)frame->x3)

#define SYSCALL_DEFINE5(name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)   \
	SYSCALL_DEFINE_N(name, 5, (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5),  \
			 ((t1)frame->x0, (t2)frame->x1), (t3)frame->x2, \
			 (t4)frame->x3, (t5)frame->x4)

#define SYSCALL_DEFINE6(name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
	SYSCALL_DEFINE_N(name, 6, (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6), \
			 ((t1)frame->x0, (t2)frame->x1), (t3)frame->x2,       \
			 (t4)frame->x3, (t5)frame->x4, (t6)frame->x5)

#endif // _SYSCALL_H_
