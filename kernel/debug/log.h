#ifndef _LOG_H_
#define _LOG_H_

#include "config.h"
#include "debug/printf.h"
#include "io/console.h"
#include "io/io.h"
#include <stdatomic.h>

#define LEVEL_TRACE 0
#define LEVEL_DEBUG 1
#define LEVEL_INFO  2
#define LEVEL_WARN  3
#define LEVEL_ERROR 4
#define LEVEL_FATAL 5

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

void __log(const i8 *level_str, u64 level, const IOColor color, const i8 *file,
	   const i8 *func, usize line, const i8 *fmt, ...);

void __user_log(const i8 *level_str, const IOColor color, const i8 *fmt, ...);

void log_set_level(usize level);
usize log_get_level(void);

#define LOG(level_str, level, color, fmt, ...)                                 \
	__log(#level_str, level, color, __FILE_NAME__, __FUNCTION__, __LINE__, \
	      fmt "\n", ##__VA_ARGS__)

#define TRACE(fmt, ...) \
	LOG(TRACE, LEVEL_TRACE, IO_COLOR_WHITE, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) \
	LOG(DEBUG, LEVEL_DEBUG, IO_COLOR_WHITE_BRIGHT, fmt, ##__VA_ARGS__)

#if LEVEL > LEVEL_INFO
	#define INFO(fmt, ...)                                        \
		__user_log("OK", IO_COLOR_GREEN_BRIGHT, " " fmt "\n", \
			   ##__VA_ARGS__)
#else
	#define INFO(fmt, ...) \
		LOG(INFO, LEVEL_INFO, IO_COLOR_GREEN_BRIGHT, fmt, ##__VA_ARGS__)
#endif

#if LEVEL > LEVEL_WARN
	#define WARN(fmt, ...)                                           \
		__user_log("WARN", IO_COLOR_YELLOW_BRIGHT, " " fmt "\n", \
			   ##__VA_ARGS__)
#else
	#define WARN(fmt, ...)                                     \
		LOG(WARN, LEVEL_WARN, IO_COLOR_YELLOW_BRIGHT, fmt, \
		    ##__VA_ARGS__)
#endif

/* we always print error and fatal messages */
#define ERROR(fmt, ...) \
	LOG(ERROR, LEVEL_ERROR, IO_COLOR_RED_BRIGHT, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) \
	LOG(FATAL, LEVEL_FATAL, IO_COLOR_MAGENTA_BRIGHT, fmt, ##__VA_ARGS__)

#endif // _LOG_H_
