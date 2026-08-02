# Version
export VERSION_MAJOR = 0
export VERSION_MINOR = 3

CONFIG_FREE_TARGETS := help menuconfig defconfig debugconfig clean cleanall cleandebug cleanconfig
# Require .config for everything else
ifeq ($(filter $(MAKECMDGOALS),$(CONFIG_FREE_TARGETS)),)
  ifeq ($(wildcard .config),)
    $(error .config was not found, run `make menuconfig` or `make defconfig`. Run `make help` for more info)
  endif
  include .config
endif

NAME = kaworu

# Point to the uefi firmware for qemu (not the directory, the firmware file itself)
# UEFI_FIRMWARE :=

ELF = kernel/$(NAME).elf
ISO = $(NAME).iso

# qemu flags for virt
# WARN: dont forget to update in release run scripts
QEMU_MACHINE := virt,acpi=off
QEMU_FLAGS := -cpu cortex-a72 \
			-m $(CONFIG_PHYSICAL_MEMORY_MB)M \
			-device ramfb \
			-device qemu-xhci \
			-device usb-kbd \
			-device usb-tablet \
			-serial stdio \
			-drive if=pflash,unit=0,format=raw,file=$(UEFI_FIRMWARE),readonly=on \
			-cdrom $(ISO) \
			-smp $(CONFIG_CPU_COUNT)

ifeq ($(CONFIG_ENABLE_SEMIHOSTING),y)
	 QEMU_FLAGS += -semihosting
endif

GDB_FLAGS = -ex "target remote :1234" -ex "set scheduler-locking step" -ex "b start" -ex "c"

all: build

# configuration -------------------------------
export MENUCONFIG_STYLE = monochrome
.PHONY: menuconfig
menuconfig: _menuconfig syncconfig ## Configure the kernel using ncurses tui
_menuconfig:
	@printf "\tMENU\n"
	@menuconfig

.PHONY: defconfig
defconfig: _defconfig syncconfig ## Use default kernel config
_defconfig:
	@printf "\tCOPY configs/defaultconfig\n"
	@defconfig configs/defaultconfig > /dev/null

.PHONY: debugconfig
debugconfig: _debugconfig syncconfig ## Use debug kernel config
_debugconfig:
	@printf "\tCOPY configs/debugconfig\n"
	@defconfig configs/debugconfig> /dev/null

.PHONY: syncconfig
syncconfig:
	@printf "\tSYNC\n"
	@genconfig

# building ---------------------
build: $(ISO) ## Build kernel iso

$(ISO): kernel | setup-iso-dir
	@printf "\tXORRISO %s\n" $(ISO)
	@xorriso -as mkisofs -R -r -J \
		-hfsplus -apm-block-size 2048 \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso -o $(ISO) > /dev/null 2>&1
	@printf "\niso file: %s\n" $(ISO)

setup-iso-dir: iso/boot/$(NAME) iso/boot/limine/limine.conf iso/boot/limine/limine-uefi-cd.bin iso/EFI/BOOT/BOOTAA64.EFI

# copy files
iso/.dirs:
	@printf "\tMKDIR iso\n"
	@mkdir -p iso/boot/limine iso/EFI/BOOT
	@touch $@
iso/boot/$(NAME): $(ELF) | iso/.dirs
	@printf "\tCOPY %s\n" $@
	@cp $< $@
iso/boot/limine/limine.conf: limine.conf | iso/.dirs
	@printf "\tCOPY %s\n" $@
	@cp $< $@
iso/boot/limine/limine-uefi-cd.bin: thirdparty/limine-binaries/limine-uefi-cd.bin | iso/.dirs
	@printf "\tCOPY %s\n" $@
	@cp $< $@
iso/EFI/BOOT/BOOTAA64.EFI: thirdparty/limine-binaries/BOOTAA64.EFI | iso/.dirs
	@printf "\tCOPY %s\n" $@
	@cp $< $@

.PHONY: kernel
kernel:
	@make -C kernel/

# running ---------------------
.PHONY: run
run: $(ISO) ## Run the kernel inside qemu
	qemu-system-aarch64 -M $(QEMU_MACHINE) $(QEMU_FLAGS)

# cleaning -------------------
.PHONY: cleanall
cleanall: clean cleanconfig cleandebug ## Clean all build, config and debug files

.PHONY: clean
clean: cleandebug ## Clean only iso and build files
	rm -f $(ISO)
	rm -rf iso
	$(MAKE) -C kernel/ clean

.PHONY: cleanconfig
cleanconfig: ## Clean only config files
	rm -f config.h .config

.PHONY: cleandebug
cleandebug:
	rm -rf stripped.elf
	rm -rf *.objdump

# hacking ------------------
.PHONY: clang-tidy
clang-tidy: $(ELF)  ## Run clang-tidy on the entire source
	run-clang-tidy -source-filter ".*\.(c|h)" -quiet -allow-no-checks

.PHONY: gdb
gdb: ## Run gdb debugger
	gdb $(ELF) $(GDB_FLAGS)

.PHONY: pwndbg
pwndbg: ## Run pwndbg debugger
	pwndbg $(ELF) $(GDB_FLAGS)

.PHONY: stripped
stripped: $(ELF)
	llvm-strip --strip-debug $(ELF) -o stripped.elf

.PHONY: objdump
objdump: $(ELF) stripped
	llvm-objdump --disassemble-all --line-numbers --full-contents stripped.elf > dump.objdump

.PHONY: rund
rund: $(ISO)
	qemu-system-aarch64 -M $(QEMU_MACHINE) $(QEMU_FLAGS) -s -S

.PHONY: qemu_dump_dts
qemu_dump_dts:
	qemu-system-aarch64 -machine $(QEMU_MACHINE),dumpdtb=virt.dtb $(QEMU_FLAGS)
	dtc -I dtb -O dts -o virt.dts virt.dtb

.PHONY: help
help: # Show this help
	@sed -nE 's/^([[:alnum:]_.-]+):.*##[[:space:]]*(.*)/\1\t\2/p' $(MAKEFILE_LIST) | column -ts $$'\t'

# create releases
.PHONY: release_kernel
rkf = kaworu-kernel-$(VERSION_MAJOR).$(VERSION_MINOR)
release_kernel: $(ISO)
	mkdir -p $(rkf)/
	cp $(ISO) $(rkf)/
	cp ./meta/releases/kernel/* $(rkf)/
	tar -czvf $(rkf).tar.gz $(rkf)/
	rm -rf $(rkf)

.PHONY: release_full
rff = kaworu-full-$(VERSION_MAJOR).$(VERSION_MINOR)
release_full: $(ISO)
	mkdir -p $(rff)/
	cp $(ISO) $(rff)/
	cp ./meta/releases/full/* $(rff)/
	cp $(UEFI_FIRMWARE) $(rff)/
	tar -czvf $(rff).tar.gz $(rff)/
	rm -rf $(rff)

-include $(DEPS)
