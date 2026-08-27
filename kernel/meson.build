foreach file : [
    # arch
    'arch/aarch64/exception.c',
    # boot
    'boot/fdt.c',
    'boot/limine_responses.c',
    'boot/memmap.c',
    # core
    'core/cpu.c',
    'core/fd_table.c',
    'core/process.c',
    'core/process_manager.c',
    'core/task.c',
    'core/task_manager.c',
    'core/timer.c',
    # debug
    'debug/log.c',
    'debug/printf.c',
    'debug/semihosting.c',
    # exec
    'exec/elf.c',
    # io
    'io/console.c',
    # irq
    'irq/irq_controller.c',
    # mm
    'mm/address_space.c',
    'mm/kheap.c',
    'mm/paging.c',
    'mm/pmm.c',
    'mm/uaccess.c',
    'mm/vmm.c',
    # sched
    'sched/scheduler.c',
    'sched/scheduler_policy.c',
    # sync
    'sync/mutex.c',
    'sync/refcount.c',
    'sync/run_queue.c',
    'sync/semaphore.c',
    'sync/spinlock.c',
    'sync/wait_queue.c',
    # syscall
    'syscall/sys_io.c',
    'syscall/sys_process.c',
    'syscall/sys_sched.c',
    'syscall/syscall.c',
    'syscall/syscall_table.c',
]
    kernel_c_sources += files(project_root / 'kernel' / file)
endforeach

foreach file : [
    'arch/aarch64/context_switch.S',
    'arch/aarch64/exception.S',
    'arch/aarch64/secondary_entry.S',
    'arch/aarch64/start.S',
    'arch/aarch64/user_entry_trampoline.S',
]
    kernel_asm_sources += files(project_root / 'kernel' / file)
endforeach
