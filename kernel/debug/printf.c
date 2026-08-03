#include "debug/printf.h"
#include "io/console.h"
#include <stdarg.h>
#include "types.h"

static u8 digits[] = "0123456789ABCDEF";
constexpr usize PRINT_BUFFER_SIZE = 2048;

/* static function declarations */
static void _print_int(u8 buf[static 32], i32 *size, i64 x, u8 base, bool sign);
static void print_int(i8 *buffer, usize *write_count, i64 x, u8 base,
		      bool sign);
static void print_double(i8 *buffer, usize *write_count, f64 f, i32 precision);
static void print_string(i8 *buffer, usize *write_count, const u8 *s);

// NOLINTBEGIN(clang-analyzer-valist.Uninitialized,
// clang-analyzer-valist.Uninitialized)
void __attribute__((format(printf, 1, 2))) printf(const i8 *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void vprintf(const i8 *fmt, va_list ap)
{
	i8 printf_buffer[PRINT_BUFFER_SIZE];
	usize wrote = __vsnprintf(printf_buffer, PRINT_BUFFER_SIZE, fmt, ap);
	IOEvent e = { .msg = printf_buffer,
		      .len = wrote,
		      .bg = IO_DEFAULT_COLOR_BG,
		      .fg = IO_DEFAULT_COLOR_FG };
	console_write(e);
}

usize vsnprintf(i8 *buffer, usize buf_size, const i8 *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	usize wrote = __vsnprintf(buffer, buf_size, fmt, ap);
	va_end(ap);
	return wrote;
}

usize __vsnprintf(i8 *buffer, usize buf_size, const i8 *fmt, va_list ap)
{
	bool state_format_specifier = false;
	usize write_count = 0;
	for (usize i = 0; fmt[i] != 0; i++) {
		if (write_count > buf_size) {
			return 0;
		}

		u8 ch = fmt[i] & 0xFF;
		if (!state_format_specifier) {
			/* not in format specifier mode */
			if ('%' == ch) {
				state_format_specifier = true;
			} else {
				buffer[write_count++] = ch;
				if (ch == '\n') {
					buffer[write_count++] = '\r';
				}
			}
		} else {
			/* in format specifier mode */
			if ('d' == ch) {
				print_int(buffer, &write_count, va_arg(ap, i32),
					  10, true);
			} else if ('x' == ch) {
				print_int(buffer, &write_count, va_arg(ap, i64),
					  16, false);
			} else if ('f' == ch) {
				print_double(buffer, &write_count,
					     va_arg(ap, f64), 12);
			} else if ('p' == ch) {
				buffer[write_count++] = '0';
				buffer[write_count++] = 'x';
				print_int(buffer, &write_count, va_arg(ap, i64),
					  16, false);
			} else if ('s' == ch) {
				u8 *s = va_arg(ap, u8 *);
				print_string(buffer, &write_count, s);
			} else if ('c' == ch) {
				buffer[write_count++] = (u8)va_arg(ap, i32);
			} else if ('b' == ch) {
				bool v = (bool)va_arg(ap, i32);
				print_string(buffer, &write_count,
					     (const u8 *)(v ? "true" :
							      "false"));
			} else if ('%' == ch) {
				buffer[write_count++] = '%';
			} else {
				buffer[write_count++] = '%';
				buffer[write_count++] = ch;
			}
			state_format_specifier = false;
		}
	}
	return write_count;
}

// NOLINTEND(clang-analyzer-valist.Uninitialized,
// clang-analyzer-valist.Uninitialized)

static void _print_int(u8 buf[static 32], i32 *size, i64 x, u8 base, bool sign)
{
	u64 xx = 0;
	bool is_negative = false;
	if (sign && x < 0) {
		is_negative = true;
		xx = (u64)-x;
	} else {
		xx = (u64)x;
	}

	i32 i = 0;
	do {
		buf[i++] = digits[xx % base];
	} while ((xx /= base) != 0);

	if (is_negative) {
		buf[i++] = '-';
	}
	*size = i;
}

static void print_int(i8 *buffer, usize *write_count, i64 x, u8 base, bool sign)
{
	u8 buf[32] = { 0 };
	i32 i;
	_print_int(buf, &i, x, base, sign);
	while (--i >= 0) {
		buffer[(*write_count)++] = buf[i];
	}
}

static void print_double(i8 *buffer, usize *write_count, f64 f, i32 precision)
{
	bool is_negative = false;
	if (f < 0) {
		is_negative = true;
		f = -f;
	}

	u64 integer = (u64)f;
	double fraction = f - (f64)integer;
	u8 buf_integer[32] = { 0 };
	i32 integer_buf_size;
	_print_int(buf_integer, &integer_buf_size, (i32)integer, 10, false);

	u8 buf_float[16] = { 0 };
	i32 float_buf_size = 0;
	for (; float_buf_size < precision; float_buf_size++) {
		fraction *= 10.0;
		i32 digit = (i32)fraction;
		buf_float[float_buf_size++] = '0' + (u8)digit;
		fraction -= digit;
	}

	if (is_negative) {
		buffer[(*write_count)++] = '-';
	}

	while (--integer_buf_size >= 0) {
		buffer[*write_count++] = buf_integer[integer_buf_size];
	}

	buffer[(*write_count)++] = '.';

	i32 i = 0;
	while (i < float_buf_size) {
		buffer[(*write_count)++] = buf_float[i++];
	}
}

static void print_string(i8 *buffer, usize *write_count, const u8 *s)
{
	while (*s != 0) {
		buffer[(*write_count)++] = *s;
		if (*s == '\n') {
			buffer[*write_count++] = '\r';
		}
		s++;
	}
}
