#ifndef _IO_H_
#define _IO_H_

#include "types.h"

enum {
	_IO_COLOR_BLACK = 0,
	_IO_COLOR_RED = 1,
	_IO_COLOR_GREEN = 2,
	_IO_COLOR_YELLOW = 3,
	_IO_COLOR_BLUE = 4,
	_IO_COLOR_MAGENTA = 5,
	_IO_COLOR_CYAN = 6,
	_IO_COLOR_WHITE = 7,
};

typedef struct {
	u8 color;
	bool bright;
} IOColor;

constexpr IOColor IO_COLOR_BLACK = { .color = _IO_COLOR_BLACK,
				     .bright = false };
constexpr IOColor IO_COLOR_BLACK_BRIGHT = { .color = _IO_COLOR_BLACK,
					    .bright = true };

constexpr IOColor IO_COLOR_RED = { .color = _IO_COLOR_RED, .bright = false };
constexpr IOColor IO_COLOR_RED_BRIGHT = { .color = _IO_COLOR_RED,
					  .bright = true };

constexpr IOColor IO_COLOR_GREEN = { .color = _IO_COLOR_GREEN,
				     .bright = false };
constexpr IOColor IO_COLOR_GREEN_BRIGHT = { .color = _IO_COLOR_GREEN,
					    .bright = true };

constexpr IOColor IO_COLOR_YELLOW = { .color = _IO_COLOR_YELLOW,
				      .bright = false };
constexpr IOColor IO_COLOR_YELLOW_BRIGHT = { .color = _IO_COLOR_YELLOW,
					     .bright = true };

constexpr IOColor IO_COLOR_BLUE = { .color = _IO_COLOR_BLUE, .bright = false };
constexpr IOColor IO_COLOR_BLUE_BRIGHT = { .color = _IO_COLOR_BLUE,
					   .bright = true };

constexpr IOColor IO_COLOR_MAGENTA = { .color = _IO_COLOR_MAGENTA,
				       .bright = false };
constexpr IOColor IO_COLOR_MAGENTA_BRIGHT = { .color = _IO_COLOR_MAGENTA,
					      .bright = true };

constexpr IOColor IO_COLOR_CYAN = { .color = _IO_COLOR_CYAN, .bright = false };
constexpr IOColor IO_COLOR_CYAN_BRIGHT = { .color = _IO_COLOR_CYAN,
					   .bright = true };

constexpr IOColor IO_COLOR_WHITE = { .color = _IO_COLOR_WHITE,
				     .bright = false };
constexpr IOColor IO_COLOR_WHITE_BRIGHT = { .color = _IO_COLOR_WHITE,
					    .bright = true };

constexpr IOColor IO_DEFAULT_COLOR_FG = IO_COLOR_WHITE_BRIGHT;
constexpr IOColor IO_DEFAULT_COLOR_BG = IO_COLOR_BLACK;

typedef struct {
	IOColor fg;
	IOColor bg;
	const i8 *msg;
	usize len;
} IOEvent;

#endif // _IO_H_
