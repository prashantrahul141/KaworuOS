#ifndef _C_TYPE_H_
#define _C_TYPE_H_

#include "types.h"

static inline i32 isdigit(i32 c)
{
	return (u32)c - '0' < 10;
}

static inline i32 isalpha(i32 c)
{
	return ((u32)c | 32) - 'a' < 26;
}

static inline i32 isupper(i32 c)
{
	return (u32)c - 'A' < 26;
}

static inline i32 isspace(i32 c)

{
	return c == ' ' || (unsigned)c - '\t' < 5;
}

#endif // _C_TYPE_H_
