#ifndef _SYSCALL_TABLE_H_
#define _SYSCALL_TABLE_H_

#include "aarch64/exception.h"
#include "types.h"

typedef struct {
	i64 ret;
	bool should_resched;
} SyscallReturn;

typedef SyscallReturn (*syscall_function_type)(const ExceptionFrame *frame);

syscall_function_type syscall_table_retrieve(usize syscall_id);

#endif // _SYSCALL_TABLE_H_
