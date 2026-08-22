#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "aarch64/exception.h"
#include "common_defs.h"

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

#define __SC_DECL(t, a) t a

#define __SC_ARG1(t, a) ((t)frame->x0)
#define __SC_ARG2(t, a) ((t)frame->x1)
#define __SC_ARG3(t, a) ((t)frame->x2)
#define __SC_ARG4(t, a) ((t)frame->x3)
#define __SC_ARG5(t, a) ((t)frame->x4)
#define __SC_ARG6(t, a) ((t)frame->x5)

#define __SC_ARGS0(...)

#define __SC_ARGS1(t1, a1) __SC_ARG1(t1, a1)

#define __SC_ARGS2(t1, a1, t2, a2) __SC_ARG1(t1, a1), __SC_ARG2(t2, a2)

#define __SC_ARGS3(t1, a1, t2, a2, t3, a3) \
	__SC_ARG1(t1, a1), __SC_ARG2(t2, a2), __SC_ARG3(t3, a3)

#define __SC_ARGS4(t1, a1, t2, a2, t3, a3, t4, a4)               \
	__SC_ARG1(t1, a1), __SC_ARG2(t2, a2), __SC_ARG3(t3, a3), \
		__SC_ARG4(t4, a4)

#define __SC_ARGS5(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)       \
	__SC_ARG1(t1, a1), __SC_ARG2(t2, a2), __SC_ARG3(t3, a3), \
		__SC_ARG4(t4, a4), __SC_ARG5(t5, a5)

#define __SC_ARGS6(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
	__SC_ARG1(t1, a1), __SC_ARG2(t2, a2), __SC_ARG3(t3, a3),   \
		__SC_ARG4(t4, a4), __SC_ARG5(t5, a5), __SC_ARG6(t6, a6)

#define SYSCALL_DEFINE0(name, frame_name)                                      \
	static SyscallReturn __sys_##name(const ExceptionFrame *(frame_name)); \
	SyscallReturn sys_##name(const ExceptionFrame *frame)                  \
	{                                                                      \
		return __sys_##name(frame);                                    \
	}                                                                      \
	static SyscallReturn __sys_##name(const ExceptionFrame *(frame_name))

#define SYSCALL_DEFINE_N(name, frame_name, n, ...)                             \
	static SyscallReturn __sys_##name(__MAP(n, __SC_DECL, __VA_ARGS__),    \
					  const ExceptionFrame *(frame_name)); \
	SyscallReturn sys_##name(const ExceptionFrame *(frame_name))           \
	{                                                                      \
		return __sys_##name(__SC_ARGS##n(__VA_ARGS__), (frame_name));  \
	}                                                                      \
	static SyscallReturn __sys_##name(__MAP(n, __SC_DECL, __VA_ARGS__),    \
					  const ExceptionFrame *(frame_name))

#define SYSCALL_DEFINE1(name, frame_name, t1, a1) \
	SYSCALL_DEFINE_N(name, frame_name, 1, t1, a1)

#define SYSCALL_DEFINE2(name, frame_name, t1, a1, t2, a2) \
	SYSCALL_DEFINE_N(name, frame_name, 2, t1, a1, t2, a2)

#define SYSCALL_DEFINE3(name, frame_name, t1, a1, t2, a2, t3, a3) \
	SYSCALL_DEFINE_N(name, frame_name, 3, t1, a1, t2, a2, t3, a3)

#define SYSCALL_DEFINE4(name, frame_name, t1, a1, t2, a2, t3, a3, t4, a4) \
	SYSCALL_DEFINE_N(name, frame_name, 4, t1, a1, t2, a2, t3, a3, t4, a4)

#define SYSCALL_DEFINE5(name, frame_name, t1, a1, t2, a2, t3, a3, t4, a4, t5, \
			a5)                                                   \
	SYSCALL_DEFINE_N(name, frame_name, 5, t1, a1, t2, a2, t3, a3, t4, a4, \
			 t5, a5)

#define SYSCALL_DEFINE6(name, frame_name, t1, a1, t2, a2, t3, a3, t4, a4, t5, \
			a5, t6, a6)                                           \
	SYSCALL_DEFINE_N(name, frame_name, 6, t1, a1, t2, a2, t3, a3, t4, a4, \
			 t5, a5, t6, a6)

#endif // _SYSCALL_H_]
