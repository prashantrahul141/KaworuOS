#ifndef _PANIC_H_
#define _PANIC_H_

#include "log.h"
#include "aarch64/aarch64.h"
#include "common_defs.h"

#define panic(fmt, ...)                    \
	do {                               \
		FATAL(fmt, ##__VA_ARGS__); \
		brk(0);                    \
		w_intrall_disable();       \
		wfi();                     \
		UNREACHABLE();             \
	} while (0)

#endif // _PANIC_H_
