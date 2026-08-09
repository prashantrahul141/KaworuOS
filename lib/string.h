#ifndef _STRINGS_H_
#define _STRINGS_H_

#include "types.h"
#include "common_defs.h"

/*
 * Fills first count bytes of the memory pointed by s with byte.
 * returns the original pointer.
 */
void *memset(void *s, u8 byte, usize count) NONNULL(1);

/*
 * copies size bytes from src to destination. returns original dest pointer.
 */
void *memcpy(void *dest, const void *src, usize size) NONNULL(1, 2);

/*
 * compares two memory area, returns zero if they are equal, otherwize returns
 * positive if s1 is larger than s2 and vice versa.
 *
 * also returns zero if size = 0.
 */
i32 memcmp(const void *s1, const void *s2, usize n);

/*
 *  string compare with max len
 */
i32 strncmp(const i8 *a, const i8 *b, u32 n);

/*
 * string compare
 */
i32 strcmp(const i8 *a, const i8 *b);

/*
 * Size of string
 */
usize strlen(const i8 *s);

/*
 * size of fixed size string
 */
usize strnlen(const i8 *s, usize maxlen);

/*
 * copies memory area
 */
void *memmove(void *dest, const void *src, usize len);

/* scans first n bytes of src for character c
 */
void *memchr(const void *src, i32 c, usize n);

/*
 * scan memory for a character, reversed
 */
void *memrchr(const void *m, i32 c, usize n);

/*
 * locate character in string, reversed
 */
i8 *strrchr(const i8 *s, i32 c);

/*
 * string search character or NULL
 */
i8 *strchrnul(const i8 *s, i32 c);

/* locate character in string
 */
i8 *strchr(const i8 *s, i32 c);

#endif
