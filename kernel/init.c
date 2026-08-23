#include "init.h"
#include "memlayout.h"
#include "mm/pmm.h"
#include <string.h>

extern symbol user_init;
extern symbol user_init_c;

void kernel_main(void)
{
	limine_responses_save();
	memmap_save_init();
	cpu_cache_current_cpu();
	console_init(false);

#ifdef CONFIG_ENABLE_SEMIHOSTING
	semihosting_init();
#endif

	printf("booting...\n");
	framebuffer_init();
	pmm_init();
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
	task_manager_init();
	wake_secondary_cpus_and_wait();
	proc_manager_init();
	syscall_build_table();

	printf("\nHello from kaworu\n\n");
	console_flush();

	usize p = pmm_alloc();
	memcpy(pmm_phys_to_virt(p), user_init_c, PAGE_SIZE);
	proc_manager_create_exec("init", p, PAGE_SIZE, USER_PROGRAM_START_VM);

	scheduler_init();

	panic("schedule returned");
}
