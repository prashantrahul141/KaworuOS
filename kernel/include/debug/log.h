#ifndef _LOG_H_
#define _LOG_H_

#include "config.h"
#include "io/io.h"
#include "printf.h"
#include "io/console.h"

#define LEVEL_TRACE 0
#define LEVEL_DEBUG 1
#define LEVEL_INFO  2
#define LEVEL_WARN  3
#define LEVEL_ERROR 4

#if defined(CONFIG_LOGGING_LEVEL_TRACE)
	#define LEVEL LEVEL_TRACE
#elif defined(CONFIG_LOGGING_LEVEL_DEBUG)
	#define LEVEL LEVEL_DEBUG
#elif defined(CONFIG_LOGGING_LEVEL_INFO)
	#define LEVEL LEVEL_INFO
#elif defined(CONFIG_LOGGING_LEVEL_WARN)
	#define LEVEL LEVEL_WARN
#elif defined(CONFIG_LOGGING_LEVEL_ERROR)
	#define LEVEL LEVEL_ERROR
#endif

static inline void __log(const i8 *level_str, const IOColor color,
			 const i8 *file, const i8 *func, usize line,
			 const i8 *fmt, ...)
{
	i8 buffers[3][(1 << 10)];
	IOEvent events[3];
	usize wrote = vsnprintf((i8 *)buffers[0], 2048, "[ %s ]", level_str);
	events[0].bg = color;
	events[0].fg = IO_DEFAULT_COLOR_BG;
	events[0].len = wrote;
	events[0].msg = (i8 *)&buffers[0];

	wrote = vsnprintf((i8 *)buffers[1], 2048, " %s:%s:%d: ", file, func,
			  line);
	events[1].bg = IO_DEFAULT_COLOR_BG;
	events[1].fg = color;
	events[1].len = wrote;
	events[1].msg = buffers[1];

	va_list args;
	va_start(args, fmt);
	wrote = __vsnprintf((i8 *)buffers[2], 2048, fmt, args);
	events[2].bg = IO_DEFAULT_COLOR_BG;
	events[2].fg = color;
	events[2].len = wrote;
	events[2].msg = buffers[2];
	va_end(args);

	console_write_multiple(events, 3);
	console_flush();
}

#define LOG(level_str, color, fmt, ...)                                 \
	__log(#level_str, color, __FILE_NAME__, __FUNCTION__, __LINE__, \
	      fmt "\n", ##__VA_ARGS__)

static inline void __user_log(const i8 *level_str, const IOColor color,
			      const i8 *fmt, ...)
{
	i8 buffer[2048];
	usize wrote = vsnprintf(buffer, 2048, "[ %s ]", level_str);
	IOEvent ev = { .bg = color,
		       .fg = IO_DEFAULT_COLOR_BG,
		       .len = wrote,
		       .msg = buffer };
	console_write(ev);

	va_list args;
	va_start(args, fmt);
	wrote = __vsnprintf(buffer, 2048, fmt, args);
	ev = (IOEvent){ .bg = IO_DEFAULT_COLOR_BG,
			.fg = IO_DEFAULT_COLOR_FG,
			.len = wrote,
			.msg = buffer };
	console_write(ev);
	va_end(args);
	console_flush();
}

#if LEVEL > LEVEL_TRACE
	#define TRACE(fmt, ...)
#else
	#define TRACE(fmt, ...) LOG(TRACE, IO_COLOR_WHITE, fmt, ##__VA_ARGS__)
#endif

#if LEVEL > LEVEL_DEBUG
	#define DEBUG(fmt, ...)
#else
	#define DEBUG(fmt, ...) \
		LOG(DEBUG, IO_COLOR_WHITE_BRIGHT, fmt, ##__VA_ARGS__)
#endif

#if LEVEL > LEVEL_INFO
	#define INFO(fmt, ...)                                        \
		__user_log("OK", IO_COLOR_GREEN_BRIGHT, " " fmt "\n", \
			   ##__VA_ARGS__)
#else
	#define INFO(fmt, ...) \
		LOG(INFO, IO_COLOR_GREEN_BRIGHT, fmt, ##__VA_ARGS__)
#endif

#if LEVEL > LEVEL_WARN
	#define WARN(fmt, ...)                                           \
		__user_log("WARN", IO_COLOR_YELLOW_BRIGHT, " " fmt "\n", \
			   ##__VA_ARGS__)
#else
	#define WARN(fmt, ...) \
		LOG(WARN, IO_COLOR_YELLOW_BRIGHT, fmt, ##__VA_ARGS__)
#endif

/* we always print error and fatal messages */
#define ERROR(fmt, ...) LOG(ERROR, IO_COLOR_RED_BRIGHT, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) LOG(FATAL, IO_COLOR_MAGENTA_BRIGHT, fmt, ##__VA_ARGS__)

#endif // _LOG_H_
