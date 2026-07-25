#include "string.h"
#include <limits.h>

void *memmove(void *dest, const void *src, size_t len)
{
	char *d = dest;
	const char *s = src;
	if (d < s) {
		while (len--) {
			*d++ = *s++;
		}
	} else {
		const i8 *lasts = s + (len - 1);
		char *lastd = d + (len - 1);
		while (len--) {
			*lastd-- = *lasts--;
		}
	}
	return dest;
}
void *memchr(const void *src, i32 c, size_t n)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
	// NOLINTNEXTLINE
	u8 *s = (u8 *)src;
#pragma clang diagnostic pop

	c = (u8)c;
	for (; n && *s != c; s++, n--)
		;
	return n ? s : nullptr;
}

void *memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = m;
	c = (unsigned char)c;
	while (n--) {
		if (s[n] == c) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
			// NOLINTNEXTLINE
			return (void *)(s + n);
#pragma clang diagnostic pop
		}
	}
	return nullptr;
}

i8 *strrchr(const i8 *s, i32 c)
{
	return memrchr(s, c, strlen(s) + 1);
}

void *memset(void *s, u8 byte, usize count)
{
	u8 *ptr = s;
	while (count-- > 0)
		*ptr++ = byte;
	return s;
}

void *memcpy(void *dest, const void *src, usize size)
{
	u8 *dest_ptr = dest;
	const u8 *src_ptr = src;
	while (size-- > 0)
		*dest_ptr++ = *src_ptr++;
	return dest;
}

i32 memcmp(const void *s1, const void *s2, usize n)
{
	const u8 *p1 = s1;
	const u8 *p2 = s2;
	for (; n--; ++p1, ++p2) {
		int diff = *p1 - *p2;
		if (diff) {
			return diff;
		}
	}

	return 0;
}

usize strlen(const i8 *s)
{
	const i8 *p = s;
	while (0 != *p++)
		;
	return (usize)(p - s - 1);
}

i32 strncmp(const i8 *a, const i8 *b, u32 n)
{
	while (*a && (*a == *b) && --n > 0) {
		++a;
		++b;
	}

	return (u8)*a - (u8)*b;
}

i32 strcmp(const i8 *a, const i8 *b)
{
	while (*a && (*a == *b)) {
		++a;
		++b;
	}

	return (u8)*a - (u8)*b;
}

usize strnlen(const i8 *s, size_t n)
{
	const i8 *p = memchr(s, 0, n);
	return p ? (usize)(p - s) : n;
}

i8 *strchrnul(const i8 *s, i32 c)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
	// NOLINTNEXTLINE
	i8 *_s = (i8 *)s;
#pragma clang diagnostic pop
	c = (u8)c;
	if (!c) {
		return (i8 *)_s + strlen(_s);
	}

	for (; *s && *(u8 *)_s != c; _s++)
		;
	return (i8 *)_s;
}

i8 *strchr(const i8 *s, i32 c)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
	// NOLINTNEXTLINE
	i8 *s_ = (i8 *)s;
#pragma clang diagnostic pop
	char *r = strchrnul(s_, c);
	return *(u8 *)r == (u8)c ? r : nullptr;
}
