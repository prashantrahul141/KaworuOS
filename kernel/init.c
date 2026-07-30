#include "init.h"

void kernel_main(void)
{
	limine_responses_save();
	console_init(true);
	printf_init();
	printf("booting...\n");
	framebuffer_init();
	kmem_init();
	vm_init();
	kheap_init();
	fdt_init();
	dmanager_init();
	irq_controller_init();
	Device *uart_device = dmanager_get_by_class_and_ready(DEVICE_UART);
	if (!IS_ERR(uart_device)) {
		console_register(uart_device, true);
	}
	timer_init();

	printf("\nHello from kaworu\n\n");
	console_flush();

	printf_deinit();
	console_deinit();
}
