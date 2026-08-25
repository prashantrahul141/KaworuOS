#include "debug/log.h"
#include "debug/printf.h"
#include "config.h"
#include "io/io.h"
#include "io/console.h"
#include <stdatomic.h>

_Atomic u64 dynamic_log_level = LEVEL;
constexpr usize individual_buffer_sizes = (1 << 10);
constexpr usize level_buffer_size = 20;

void log_set_level(usize level)
{
	atomic_store(&dynamic_log_level, level);
}

usize log_get_level(void)
{
	return (usize)atomic_load(&dynamic_log_level);
}

// NOLINTBEGIN(clang-analyzer-valist.Uninitialized,
// clang-analyzer-valist.Uninitialized)
void __log(const i8 *level_str, u64 level, const IOColor color, const i8 *file,
	   const i8 *func, usize line, const i8 *fmt, ...)
{
	if (level < atomic_load(&dynamic_log_level)) {
		return;
	}

	i8 level_buffer[level_buffer_size];
	IOEvent events[3];
	usize wrote = vsnprintf(level_buffer, sizeof(level_buffer), "[ %s ]",
				level_str);
	events[0].bg = color;
	events[0].fg = IO_DEFAULT_COLOR_BG;
	events[0].len = wrote;
	events[0].msg = level_buffer;

	i8 buffers[2][individual_buffer_sizes];
	wrote = vsnprintf((i8 *)buffers[0], individual_buffer_sizes,
			  " %s:%s:%d: ", file, func, line);
	events[1].bg = IO_DEFAULT_COLOR_BG;
	events[1].fg = color;
	events[1].len = wrote;
	events[1].msg = buffers[0];

	va_list args;
	va_start(args, fmt);
	wrote = __vsnprintf((i8 *)buffers[1], individual_buffer_sizes, fmt,
			    args);
	events[2].bg = IO_DEFAULT_COLOR_BG;
	events[2].fg = color;
	events[2].len = wrote;
	events[2].msg = buffers[1];
	va_end(args);

	console_write_multiple(events, 3);
	console_flush();
}

// NOLINTEND(clang-analyzer-valist.Uninitialized,
// clang-analyzer-valist.Uninitialized)

void __user_log(const i8 *level_str, const IOColor color, const i8 *fmt, ...)
{
	IOEvent events[2];
	i8 level_buffer[level_buffer_size];
	usize wrote = vsnprintf(level_buffer, sizeof(level_buffer), "[ %s ]",
				level_str);
	events[0].bg = color;
	events[0].fg = IO_DEFAULT_COLOR_BG;
	events[0].len = wrote;
	events[0].msg = level_buffer;

	i8 buffer[individual_buffer_sizes];
	va_list args;
	va_start(args, fmt);
	wrote = __vsnprintf(buffer, individual_buffer_sizes, fmt, args);
	va_end(args);
	events[1].bg = IO_DEFAULT_COLOR_BG;
	events[1].fg = IO_DEFAULT_COLOR_FG;
	events[1].len = wrote;
	events[1].msg = buffer;

	console_write_multiple(events, 2);
	console_flush();
}
