#include "core/semihosting.h"
#include "io/console.h"
#include "types.h"
#include "config.h"

#include "common/manager.h"
#include "common_defs.h"
#include "core/semihosting.h"
#include "io/console.h"

static void sh_write_event(Device *backend, const IOEvent *event)
{
	UNUSED_ARG(backend);
	/* buffer inside IOEvent may not be null terminated */
	for (usize i = 0; i < event->len; i++) {
		sh_writec(event->msg[i]);
	}
}

static void sh_flush(Device *backend)
{
	UNUSED_ARG(backend);
}

static const ConsoleOps sh_console_ops = {
	.write = sh_write_event,
	.flush = sh_flush,
};

static errno_t sh_output_probe(Device *device)
{
	UNUSED_ARG(device);
	return EOK;
}

static errno_t sh_output_remove(Device *device)
{
	UNUSED_ARG(device);
	return EOK;
}

static const Driver sh_output_driver = { .name = "semihosting",
					 .device_class = DEVICE_SEMIHOSTING,
					 .probe = sh_output_probe,
					 .remove = sh_output_remove };

Device sh_console_device = { .console_ops = &sh_console_ops,
			     .driver_data = nullptr,
			     .name = "semihosting",
			     .driver = &sh_output_driver };

ConsoleBackend sh_console_backend = { .device = &sh_console_device };

void semihosting_init(void)
{
	console_register_backend(&sh_console_backend);
}

usize sh_call(SemiHostingOperations op, void *params)
{
	register usize x0 asm("x0") = op;
	register void *x1 asm("x1") = params;
	asm volatile("hlt #0xf000" : "+r"(x0) : "r"(x1) : "memory");
	return x0;
}

void sh_exit(u64 code)
{
	sh_call(SYS_EXIT, &code);
}

void sh_write0(u8 *str)
{
	sh_call(SYS_WRITE0, str);
}

void sh_writec(u8 ch)
{
	sh_call(SYS_WRITEC, &ch);
}
