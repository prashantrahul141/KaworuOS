#ifndef _PRINT_H_
#define _PRINT_H_

#include "types.h"
#include <stdarg.h>
/*
 * very dump printf implementation.
 * Supports:
 *   %d -> integers
 *   %f -> floats
 *   %b -> booleans
 *   %p -> address
 *   %x -> integer as hex
 *   %s -> string
 *   %c -> character
 */
void printf(const i8 *fmt, ...);

void vprintf(const i8 *fmt, va_list arg);

usize __vsnprintf(i8 *buffer, usize buf_size, const i8 *fmt, va_list ap);

usize vsnprintf(i8 *buffer, usize buf_size, const i8 *fmt, ...);

#endif // _PRINT_H_
