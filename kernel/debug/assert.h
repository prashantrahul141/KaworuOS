#ifndef _ASSERT_H_
#define _ASSERT_H_

#include "panic.h"

/* exp - should be something we need to make sure is true */
#define ASSERT(exp, fmt, ...) \
	if (!(exp))           \
	panic("assertion failed: %s : " fmt, #exp, ##__VA_ARGS__)

#endif // _ASSERT_H_
