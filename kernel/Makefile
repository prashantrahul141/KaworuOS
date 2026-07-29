-include ../.config

CC := clang
LD := ld.lld

NAME = kaworu

LINKER_SCRIPT := kernel.ld
OUT := $(NAME).elf

TARGET := aarch64-none-elf
C_FLAGS := -MD -std=gnu23 -ffreestanding -nostdlib
WARNING_FLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wimplicit-int-conversion -Wshorten-64-to-32 -Wpointer-to-int-cast -Wint-to-pointer-cast -Wshadow -Wundef -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wswitch-enum -Wswitch-default -Wnull-dereference -Wunreachable-code -Wvla -Walloca -Wwrite-strings -Wformat=2 -Wcast-function-type -Wconditional-uninitialized -Wcomma -Wextra-semi-stmt -Wno-language-extension-token -Wno-gnu-empty-struct -Wno-gnu-zero-variadic-macro-arguments
COMMON_FLAGS = -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)

# debugging?
ifeq ($(CONFIG_DEBUG_SYMBOLS),y)
	COMMON_FLAGS += -ggdb3
endif

# optimize?
ifeq ($(CONFIG_OPTIMIZATIONS_LOW),y)
	COMMON_FLAGS += -O1
else ifeq ($(CONFIG_OPTIMIZATIONS_MEDIUM),y)
	COMMON_FLAGS += -O2 -flto=thin
else ifeq ($(CONFIG_OPTIMIZATIONS_HIGH),y)
	COMMON_FLAGS += -O2 -flto=full
else ifeq ($(CONFIG_OPTIMIZATIONS_MAX),y)
	COMMON_FLAGS += -O3 -flto=full
else ifeq ($(CONFIG_OPTIMIZATIONS_NONE),y)
	COMMON_FLAGS += -O0
endif

K = .
L = ../lib
U = ../user
D = ../drivers
TH = ../thirdparty
# files are in the order of what `fd` returns
ASM_SOURCES = $K/arch/aarch64/exception.S \
			  $K/arch/aarch64/start.S
ASM_OBJS = $(ASM_SOURCES:=.o)

C_SOURCES = $D/framebuffer/framebuffer.c \
			$D/irqchip/arm-gicv2.c \
			$D/common/manager.c \
			$D/uart/pl011.c \
			$K/arch/aarch64/exception.c \
			$K/boot/fdt.c \
			$K/boot/limine_responses.c \
			$K/core/cpu.c \
			$K/core/irq_controller.c \
			$K/debug/printf.c \
			$K/init.c \
			$K/io/console.c \
			$K/mm/kheap.c \
			$K/mm/kmem.c \
			$K/mm/paging.c \
			$K/mm/vmm.c \
			$K/mm/vmm_region.c \
			$K/sync/spinlock.c \
			$L/allocator/arena.c \
			$L/allocator/bitmap.c \
			$L/allocator/freelist.c \
			$L/ds/ringbuffer.c \
			$L/error.c \
			$L/stdlib.c \
			$L/string.c


_LIBFDT_SRCS = fdt.c fdt_ro.c fdt_wip.c fdt_sw.c fdt_rw.c fdt_strerror.c fdt_empty_tree.c \
	fdt_addresses.c fdt_overlay.c fdt_check.c
LIBFDT_SRCS := $(addprefix $(TH)/libfdt/, $(_LIBFDT_SRCS))
EXTERNAL_C_SOURCES = $(TH)/Flanterm/src/flanterm.c \
				$(TH)/Flanterm/src/flanterm_backends/fb.c \
				$(LIBFDT_SRCS)

C_OBJS = $(C_SOURCES:=.o)
C_EXTERNAL_OBJS = $(EXTERNAL_C_SOURCES:=.e.o)

OBJS = $(ASM_OBJS) $(C_OBJS) $(C_EXTERNAL_OBJS)

KI = $K/include
DI = $D/include
LI = $L/include
LIM_INC := $(TH)/limine-protocol/include/
FLAN_INC := $(TH)/Flanterm/src/
LIBFDT_INC := $(TH)/libfdt/
INCLUDES_DIR = 	-I../  \
                -I$(D)/ \
                -I$(DI)/ \
				-I. \
				-I$(KI)/ \
				-I$(KI)/arch \
				-I$(LI)/ \
				-I$(LIM_INC) \
				-I$(FLAN_INC) \
				-I$(LIBFDT_INC)

DEPS := $(C_OBJS:.o=.d) $(C_EXTERNAL_OBJS:.o=.d)

# default target which runs when none is specified
all: build

# building ---------------------
build: $(OUT) ## Build the kernel

$(OUT): $(OBJS) $(LINKER_SCRIPT)
	@printf "\tLD %s\n" $@
	@$(LD) -m aarch64elf -T $(LINKER_SCRIPT) $(OBJS) -o $(OUT)

%.S.o: %.S
	@printf "\tCC %s\n" $(shell basename $@)
	@$(CC) --target=$(TARGET) $(INCLUDES_DIR) $(COMMON_FLAGS) $(WARNING_FLAGS) -c $< -o $@

%.c.o: %.c
	@printf "\tCC %s\n" $(shell basename $@)
	@$(CC) --target=$(TARGET) $(C_FLAGS) $(INCLUDES_DIR) $(COMMON_FLAGS) $(WARNING_FLAGS) -c $< -o $@

# external c files compilation
%.c.e.o: %.c
	@printf "\tCC thirdparty/%s\n" $(shell basename $@)
	@$(CC) --target=$(TARGET) $(C_FLAGS) $(INCLUDES_DIR) $(COMMON_FLAGS) -c $< -o $@

# cleaning -------------------
.PHONY: clean
clean: ## Clean only build files
	rm -f $(DEPS)
	rm -f $(OBJS)
	rm -f $(OUT)
	rm -f $(C_EXTERNAL_OBJS)

-include $(DEPS)
