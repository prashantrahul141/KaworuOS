#include "boot/fdt.h"
#include "boot/limine_responses.h"
#include "common_defs.h"
#include "debug/panic.h"
#include "error.h"
#include "limine.h"
#include "libfdt.h"
#include "memlayout.h"
#include "mm/kheap.h"
#include "sync/spinlock.h"

typedef struct {
	void *fdt;
	usize cpu_count;
} FDT;

constexpr usize CELL_UNIT_SIZE = 4;

static FDT fdt = { .cpu_count = 0 };

void fdt_init(void)
{
	INFO("Initializing flat device tree parser");
	struct limine_dtb_response *dtb_response = limine_device_tree();
	if (nullptr == dtb_response || nullptr == dtb_response->dtb_ptr) {
		panic("dtb is null");
	}

	void *dtb_ptr = dtb_response->dtb_ptr;

	i32 ret = fdt_check_header(dtb_ptr);
	if (0 != ret) {
		panic("fdt's header is corrupted: %s", fdt_strerror(ret));
	}

	usize total_size = fdt_totalsize(dtb_ptr);
	DEBUG("fdt total_sizep = %p, pages = %d", total_size,
	      total_size / PAGE_SIZE);
	ret = fdt_check_full(dtb_ptr, total_size);
	if (0 != ret) {
		panic("fdt's body is corrupted: %s", fdt_strerror(ret));
	}

	usize alloc_size = round_up(total_size, PAGE_SIZE);
	fdt.fdt = kalloc(alloc_size * 3);
	if (IS_ERR(fdt.fdt)) {
		panic("out of memory");
	}

	DEBUG("fullsize = %d", total_size);
	memcpy(fdt.fdt, dtb_ptr, total_size);

	/* save cpu found */
	i32 offset = fdt_path_offset(fdt.fdt, "/cpus");
	if (offset < 0) {
		panic("failed searching for cpu count");
	}

	i32 node;
	fdt_for_each_subnode(node, fdt.fdt, offset)
	{
		i32 namelen;
		const char *name = fdt_get_name(fdt.fdt, node, &namelen);
		if (0 == strncmp(name, "cpu@", 4)) {
			fdt.cpu_count++;
		}
	}
	DEBUG("cpu counted = %d", fdt.cpu_count);
}

static const void *fdt_query_prop_value(i32 node_offset, const i8 *prop,
					i32 *len)
{
	return fdt_getprop(fdt.fdt, node_offset, prop, len);
}
bool fdt_get_reg_for_compat(const i8 *compat, Reg *reg, u32 reg_count)
{
	i32 node = fdt_query_compat(compat);
	if (node < 0) {
		return false;
	}
	return fdt_get_reg(node, reg, reg_count);
}

i32 fdt_query_compat(const i8 *compat)
{
	int node = -EINVAL;
	while ((node = fdt_next_node(fdt.fdt, node, nullptr)) >= 0) {
		if (fdt_node_check_compatible(fdt.fdt, node, compat) == 0) {
			return node;
		}
	}

	return -EINVAL;
}

i32 fdt_traverse_next_node(i32 offset)
{
	return fdt_next_node(fdt.fdt, offset, nullptr);
}

const i8 *fdt_get_compat(i32 offset, i32 *len)
{
	return fdt_query_prop_value(offset, "compatible", len);
}

bool fdt_get_reg(i32 node, Reg *reg, u32 reg_count)
{
	TRACE("get reg = %d", node);
	i32 parent = fdt_parent_offset(fdt.fdt, node);

	i32 len;

	const fdt32_t *ac_prop =
		fdt_getprop(fdt.fdt, parent, "#address-cells", &len);
	const fdt32_t *sc_prop =
		fdt_getprop(fdt.fdt, parent, "#size-cells", &len);

	u32 ac = ac_prop ? fdt32_to_cpu(*ac_prop) : 2;
	u32 sc = sc_prop ? fdt32_to_cpu(*sc_prop) : 2;

	const fdt32_t *reg_ = fdt_query_prop_value(node, "reg", &len);
	if (len <= 0 || reg == nullptr) {
		ERROR("len = %p (zero?), reg = %p (null?)", len, reg);
		return false;
	}

	/* verify given count and found count of regs */
	u32 reg_cells_count = (ac + sc);
	u32 actual = (u32)len / reg_cells_count / CELL_UNIT_SIZE;
	if (reg_count != actual) {
		ERROR("Reg count = %d, actual = %d not equal", reg_count,
		      actual);
		return false;
	}

	for (usize i_reg = 0; i_reg < reg_count; i_reg++) {
		u64 address = 0;
		u64 size = 0;
		usize offset = i_reg * reg_cells_count;

		for (u32 i = 0; i < ac; i++) {
			address = (address << 32) |
				  fdt32_to_cpu(reg_[offset + i]);
		}

		for (u32 i = 0; i < sc; i++) {
			size = (size << 32) |
			       fdt32_to_cpu(reg_[offset + ac + i]);
		}

		reg[i_reg].address = (void *)address;
		reg[i_reg].size = size;
	}
	return true;
}

usize fdt_cpu_count(void)
{
	return fdt.cpu_count;
}
