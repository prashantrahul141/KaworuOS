#include "types.h"
#include "syscall/syscall_table.h"

void _start(void);

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

static UNUSED void sys_wait4(i64 *status)
{
	register i64 *x0 asm("x0") = status;
	register u64 x8 asm("x8") = SYS_WAIT4;
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

static UNUSED void sys_call(usize id)
{
	register u64 x8 asm("x8") = id;
	asm volatile("svc #0" : : "r"(x8) : "memory");
}

void _start(void)
{
	i8 msg1[13];
	msg1[0] = 0x68;
	msg1[1] = 0x65;
	msg1[2] = 0x6C;
	msg1[3] = 0x6C;
	msg1[4] = 0x6F;
	msg1[5] = 0x20;
	msg1[6] = 0x79;
	msg1[7] = 0x61;
	msg1[8] = 0x61;
	msg1[9] = 0x72;
	msg1[10] = 0x61;
	msg1[11] = 0x6E;
	msg1[12] = 0x0A;

	i8 msg2[14];
	msg2[0] = 0x62;
	msg2[1] = 0x72;
	msg2[2] = 0x76;
	msg2[3] = 0x74;
	msg2[4] = 0x61;
	msg2[5] = 0x6c;
	msg2[6] = 0x20;
	msg2[7] = 0x79;
	msg2[8] = 0x61;
	msg2[9] = 0x61;
	msg2[10] = 0x72;
	msg2[11] = 0x61;
	msg2[12] = 0x6E;
	msg2[13] = 0x0A;

	i64 ret = sys_fork();
	if (ret == 0) {
		// child
		for (usize i = 0; i < 10; i++) {
			// 2 == stderr
			sys_write(2, msg2, sizeof(msg2));
		}
	} else {
		// parent
		for (usize i = 0; i < 10; i++) {
			// 1 == stdout
			sys_write(1, msg1, sizeof(msg1));
		}
	}

	for (;;) {
	}
}
