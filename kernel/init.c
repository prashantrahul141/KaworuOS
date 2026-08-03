#include "init.h"

void kernel_main(void)
{
	limine_responses_save();
	cpu_cache_current_cpu();
	console_init(true);

#ifdef CONFIG_ENABLE_SEMIHOSTING
	semihosting_init();
#endif

	printf_init();
	printf("booting...\n");
	framebuffer_init();
	kmem_init();
	vm_init();
	kheap_init();
	fdt_init();
	dmanager_init();
	irq_controller_init();

#ifndef CONFIG_ENABLE_SEMIHOSTING
	Device *uart_device = dmanager_get_by_class_and_ready(DEVICE_UART);
	if (!IS_ERR(uart_device)) {
		console_register(uart_device);
	}
#endif

	timer_init();
	wake_secondary_cpus();

	printf("\nHello from kaworu\n\n");
	console_flush();

	printf_deinit();
	console_deinit();
}
