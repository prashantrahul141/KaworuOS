#include "syscall/syscall.h"
#include "syscall/syscall_table.h"

SYSCALL_DECLARE(fork);

SYSCALL_DECLARE(exit);

SYSCALL_DECLARE(getpid);

SYSCALL_DECLARE(getppid);

SYSCALL_DECLARE(wait4);
