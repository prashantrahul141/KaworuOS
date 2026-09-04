# Version
export VERSION_MAJOR = 0
export VERSION_MINOR = 6

CONFIG_FREE_TARGETS := help menuconfig defconfig debugconfig clean cleanall cleandebug cleanconfig
# Require .config for everything else
ifeq ($(filter $(MAKECMDGOALS),$(CONFIG_FREE_TARGETS)),)
  ifeq ($(wildcard .config),)
    $(error .config was not found, run `make menuconfig` or `make defconfig`. Run `make help` for more info)
  endif
  include .config
endif

NAME = kaworu

# kernel compilation is done by meson
BUILD_DIR = build
CROSS_FILE = toolchain/aarch64-clang.ini
ELF = $(BUILD_DIR)/$(NAME).elf
ISO = $(NAME).iso

# qemu flags for virt
# dont forget to update in release/run scripts
QEMU_BLOCK_FILE_SIZE_MB = 128
QEMU_BLOCK_FILE = "block"
QEMU_MACHINE := virt,acpi=off
QEMU_FLAGS := -cpu cortex-a72 \
			-m $(CONFIG_QEMU_PHYSICAL_MEMORY_MB)M \
			-device ramfb \
			-device qemu-xhci \
			-device usb-kbd \
			-device usb-tablet \
			-drive if=pflash,unit=0,format=raw,file=$(UEFI_FIRMWARE),readonly=on \
			-drive id=drive0,file=$(QEMU_BLOCK_FILE),format=raw,if=none \
			-device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
			-cdrom $(ISO) \
			-smp $(CONFIG_QEMU_CPU_COUNT)

ifeq ($(CONFIG_QEMU_WITH_DISPLAY),y)
	 QEMU_FLAGS += -serial stdio
else
	 QEMU_FLAGS += -display none -nographic
endif

ifeq ($(CONFIG_ENABLE_SEMIHOSTING),y)
	 QEMU_FLAGS += -semihosting
endif

GDB_FLAGS = -ex "target remote :1234" -ex "set scheduler-locking step" -ex "b start" -ex "c"

all: build

# configuration -------------------------------
.PHONY: menuconfig
menuconfig: ## Configure the kernel using ncurses tui
	@printf "\tMENU\n"
	@MENUCONFIG_STYLE=monochrome scripts/configure.py $(CURDIR) $(CURDIR)/$(BUILD_DIR) $(CURDIR)/$(CROSS_FILE) menuconfig ""

.PHONY: defconfig
defconfig: ## Use default kernel config
	@printf "\tCONFIG configs/defaultconfig\n"
	@scripts/configure.py $(CURDIR) $(CURDIR)/$(BUILD_DIR) $(CURDIR)/$(CROSS_FILE) defconfig configs/defaultconfig

.PHONY: debugconfig
debugconfig: ## Use debug kernel config
	@printf "\tCONFIG configs/debugconfig\n"
	@scripts/configure.py $(CURDIR) $(CURDIR)/$(BUILD_DIR) $(CURDIR)/$(CROSS_FILE) defconfig configs/debugconfig

# building ---------------------
.PHONY: kernel
kernel: ## Build the kernel elf (runs meson compile)
	@printf "\tMESON\n"
	@meson compile -C $(BUILD_DIR)

build: $(ISO) ## Build kernel iso

$(ISO): iso/boot/$(NAME).elf \
        iso/boot/limine/limine.conf \
        iso/boot/limine/limine-uefi-cd.bin \
        iso/EFI/BOOT/BOOTAA64.EFI
	@printf "\tXORRISO %s\n" $(ISO)
	@xorriso -as mkisofs -R -r -J \
		-hfsplus -apm-block-size 2048 \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso -o $(ISO) > /dev/null 2>&1
	@printf "\niso file: %s\n" $(ISO)

iso/boot/$(NAME).elf: kernel | iso/boot/limine
	@printf "\tCOPY %s\n"
	@cp $(ELF) $@

iso/boot/limine/limine.conf: limine.conf | iso/boot/limine
	@printf "\tCOPY %s\n" $@
	@cp $< $@

iso/boot/limine/limine-uefi-cd.bin: thirdparty/limine-binaries/limine-uefi-cd.bin | iso/boot/limine
	@printf "\tCOPY %s\n" $@
	@cp $< $@

iso/EFI/BOOT/BOOTAA64.EFI: thirdparty/limine-binaries/BOOTAA64.EFI | iso/EFI/BOOT
	@printf "\tCOPY %s\n" $@
	@cp $< $@

iso/boot/limine:
	@mkdir -p $@

iso/EFI/BOOT:
	@mkdir -p $@

# running ---------------------
.PHONY: run
run: $(ISO) $(QEMU_BLOCK_FILE) ## Run the kernel inside qemu
	qemu-system-aarch64 -M $(QEMU_MACHINE) $(QEMU_FLAGS)

.PHONY: rund
rund: $(ISO) ## Run the kernel inside qemu with a gdb stub
	qemu-system-aarch64 -M $(QEMU_MACHINE) $(QEMU_FLAGS) -s -S

.PHONY: qemu_dump_dts
qemu_dump_dts: ## Dump the qemu virt device tree
	qemu-system-aarch64 -machine $(QEMU_MACHINE),dumpdtb=virt.dtb $(QEMU_FLAGS)
	dtc -I dtb -O dts -o virt.dts virt.dtb

$(QEMU_BLOCK_FILE):
	@printf "\tDD %sMB > %s\n" $(QEMU_BLOCK_FILE_SIZE_MB) $(QEMU_BLOCK_FILE)
	@dd count=$(QEMU_BLOCK_FILE_SIZE_MB) bs=1M if=/dev/zero of=$@

# hacking ------------------
.PHONY: clang-tidy
clang-tidy: kernel ## Run clang-tidy on the entire source
	run-clang-tidy -source-filter ".*\.(c|h)" -quiet -allow-no-checks

.PHONY: gdb
gdb: kernel ## Run gdb debugger
	gdb $(ELF) $(GDB_FLAGS)

.PHONY: pwndbg
pwndbg: kernel ## Run pwndbg debugger
	pwndbg $(ELF) $(GDB_FLAGS)

.PHONY: stripped
stripped: kernel ## Strip debug symbols into stripped.elf
	llvm-strip --strip-debug $(ELF) -o stripped.elf

.PHONY: objdump
objdump: stripped ## Disassemble the stripped elf into dump.objdump
	llvm-objdump --disassemble-all --line-numbers --full-contents stripped.elf > dump.objdump

# cleaning -------------------
.PHONY: cleanall
cleanall: clean ## Clean all build, config and debug files
	rm -f config.h .config $(BUILD_DIR)/.prev.config.h
	rm -f stripped.elf dump.objdump

.PHONY: clean
clean: ## Clean only build and iso files
	rm -f $(ISO)
	rm -rf iso
	rm -rf build/

# create releases
.PHONY: release_kernel
rkf = $(NAME)os-kernel-$(VERSION_MAJOR).$(VERSION_MINOR)
release_kernel: $(ISO)
	mkdir -p $(rkf)/
	cp $(ISO) $(rkf)/
	cp ./meta/releases/kernel/* $(rkf)/
	tar -czvf $(rkf).tar.gz $(rkf)/
	rm -rf $(rkf)

.PHONY: release_full
rff = $(NAME)os-full-$(VERSION_MAJOR).$(VERSION_MINOR)
release_full: $(ISO)
	mkdir -p $(rff)/
	cp $(ISO) $(rff)/
	cp ./meta/releases/full/* $(rff)/
	cp $(UEFI_FIRMWARE) $(rff)/
	tar -czvf $(rff).tar.gz $(rff)/
	rm -rf $(rff)

.PHONY: help
help: ## Show this help
	@sed -nE 's/^([[:alnum:]_.-]+):.*##[[:space:]]*(.*)/\1\t\2/p' $(MAKEFILE_LIST) | column -ts $$'\t'
