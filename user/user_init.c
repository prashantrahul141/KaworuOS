#include "types.h"
#include "syscall/syscall_table.h"

void user_init_c();

static UNUSED void sys_write(u64 fd, const i8 *buf, u64 buf_count)
{
	register u64 x0 asm("x0") = fd;
	register const i8 *x1 asm("x1") = buf;
	register u64 x2 asm("x2") = buf_count;
	register u64 x8 asm("x8") = SYS_WRITE;
	asm volatile("svc #0"
		     : "+r"(x0)
		     : "r"(x1), "r"(x2), "r"(x8)
		     : "memory");
}

static UNUSED void sys_exit(i64 status)
{
	register i64 x0 asm("x0") = status;
	register u64 x8 asm("x8") = SYS_EXIT;
	asm volatile("svc #0" : : "r"(x0), "r"(x8) : "memory");
}

static UNUSED i64 sys_getpid(void)
{
	register i64 x0 asm("x0") = 0;
	register u64 x8 asm("x8") = SYS_GETPID;
	asm volatile("svc #0" : "=r"(x0) : "r"(x8) : "memory");
	return x0;
}

static UNUSED i64 sys_getppid(void)
{
	register i64 x0 asm("x0") = 0;
	register u64 x8 asm("x8") = SYS_GETPPID;
	asm volatile("svc #0" : "=r"(x0) : "r"(x8) : "memory");
	return x0;
}

static UNUSED i64 sys_fork(void)
{
	register i64 x0 asm("x0") = 0;
	register u64 x8 asm("x8") = SYS_FORK;
	asm volatile("svc #0" : "=r"(x0) : "r"(x8) : "memory");
	return x0;
}

void user_init_c()
{
	i8 msg[13];
	msg[0] = 0x68;
	msg[1] = 0x65;
	msg[2] = 0x6C;
	msg[3] = 0x6C;
	msg[4] = 0x6F;
	msg[5] = 0x20;
	msg[6] = 0x79;
	msg[7] = 0x61;
	msg[8] = 0x61;
	msg[9] = 0x72;
	msg[10] = 0x61;
	msg[11] = 0x6E;
	msg[12] = 0x0A;

	i64 pid = sys_fork();
	for (;;) {
		i8 a = (i8)pid + 65;
		sys_write(1, &a, 1);
	}
}
