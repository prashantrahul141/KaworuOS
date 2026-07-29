#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "error.h"
#include "common/manager.h"
#include "types.h"
#include "io.h"

typedef struct ConsoleBackend ConsoleBackend;

struct ConsoleBackend {
	Device *device;
	ConsoleBackend *next;
};

/*
 * initialize given console device
 */
errno_t console_init(bool buffering);

/*
 * deinitialize a console device
 */
void console_deinit();

/*
 * register a device to console
 */
void console_register(Device *device, bool set_default);

/* register a consolebackend directly */
void console_register_backend(ConsoleBackend *backend, bool set_default);

/*
 * unregister a device from console
 */
bool console_unregister(const Device *backend);

/*
 * enable/disable console buffering
 */
void console_set_buffering(bool buffering);

/*
 * write to console
 */
errno_t console_write(const IOEvent event);

/*
 * flush all backend
 */
errno_t console_flush();

/*
 * read into console
 */
errno_t console_receive_char(i8 c);

/*
 * write a single char with default properties
 */
errno_t console_write_char(const i8 c);

/*
 * read from the default console device
 */
bool console_read(u8 *out);

#endif
