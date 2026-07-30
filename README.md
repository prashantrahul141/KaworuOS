# Kaworu

A WIP Unix-like AArch64 kernel

## Building

If you use nix, `nix develop` sets up everything you will need in order to
build and run the kernel under Qemu.

Building requires the following packages:

```sh
clang
lld

xorriso # to create iso image

qemu # to run
gnumake # ofcourse
python313Packages.kconfiglib

# llvm # not required, but contains useful tools like objdump, objcopy
# gdb # for debugging
# bear # if you want to generate compile_commands.json
```

Then generate a config using:

```sh
# Use the default config
make defconfig
# Or configure the kernel yourself and save it
make menuconfig
```

Then build the kernel

```sh
make
```

## Running

I have yet to test this on actual hardware, I have only tested this on Qemu
under the `virt` platform.

Since AArch64 only supports UEFI and not bios, you will need to provide UEFI firmware for Qemu.
You can just download the `full-release` from the [releases page](https://github.com/prashantrahul141/kaworu/releases) as it contains
both a built ISO Image and firmware you need to emulate under Qemu.

You can also download the firmware from the releases
page of:
https://github.com/osdev0/edk2-ovmf-stable-bins/releases/

Then set `UEFI_FIRMWARE` env pointing directly to the firmware.

This is done automatically if you're using nix.

run under Qemu:

```sh
make run
```

## To-Do

```
- [x] Startup
    - [x] Exception vectors
    - [x] Setup system registers
    - [x] Setup boot cpu stack
    - [x] Enable interrupts

- [x] Spinlock

- [x] Uart driver
    - [x] init pl011
    - [x] `uart_putchar`/`uart_getchar`

- [x] Framebuffer
    - [x] Set up Flanterm

- [x] Console
    - [x] Generic console

- [x] Printf implementation
    - [x] `printf`, `vprintf`
    - [x] `panic`

- [x] Logging
    - [x] `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL` conditional logging macros

- [ ] Exception handling
    - [x] Pretty exception dump
    - [ ] Decode ESR_EL1, FAR_EL1, ELR_EL1

- [x] Physical page allocator
    - [x] `kmem_alloc`/`kmem_free`

- [x] MMU
    - [x] page table setup
    - [x] map kernel
    - [x] map MMIO

- [x] Ranged bitmap allocator

- [ ] Virtual Memory
    - [x] kernel virtual memory manager
    - [x] `vm_alloc`/`vm_free` and `vm_map`/`vm_unmap`
    - [ ] Guard pages for kernel stacks

- [x] Kernel heap
    - [x] `kalloc`/`kfree`

- [x] Device driver manager

- [x] Interrupts
    - [x] GIC distributor init and cpu interface init
    - [x] IRQ framework

- [x] Timer
    - [x] Arm generic timer init
    - [x] Periodic timer interrupt

- [ ] Semi hosting
    - [ ] Semi hosting subsystem

- [ ] Scheduler
    - [ ] Concept of task, task states
    - [ ] Run queue
    - [ ] Context switch

- [ ] Kernel Threads
    - [ ] `kthread_create`/`kthread_exit`
    - [ ] Idle thread
    - [ ] Scheduler (maybe round robin?)

- [ ] Userspace
    - [ ] EL0 entry support
    - [ ] User stack
    - [ ] User address space

- [ ] Syscalls
    - [ ] SVC handler
    - [ ] Syscall table
    - [ ] Basic ones like: read, write, exit, yield

- [ ] FS
    - [ ] Decide on a simple filesystem to implement.
```

## Thirdparty

[Limine](https://github.com/limine-bootloader/limine) - Modern, secure, portable, multi protocol bootloader and boot manager.\
[Flanterm](https://github.com/Mintsuki/Flanterm) - Fast and reasonably complete (frame buffer) terminal emulator.\
[libfdt](https://elixir.bootlin.com/u-boot/v2026.07/source/lib/libfdt) - The device tree library


## Resources

https://wiki.osdev.org \
https://krinkinmu.github.io                                 # thank you @krinkinmu \
https://kernel-internals.org \
https://df.lth.se/~getz/ARM/A64/ \
https://github.com/Elite-zx/Tiny-OS                         # thank you @Elite-zx \
https://github.com/mit-pdos/xv6-riscv \
https://github.com/arryndoestech/adt_os                     # thank you @arryndoestech  \
https://xv6-guide.github.io/xv6-riscv-book \
https://github.com/gmarino2048/64bit-os-tutorial            # thank you @gmarino2048  \
https://lowenware.com/blog/aarch64-mmu-programming/ \
https://developer.arm.com/documentation/ddi0487/latest \
https://developer.arm.com/documentation/ddi0601/latest \
https://cs140e.sergio.bz/docs/ARMv8-Reference-Manual.pdf
