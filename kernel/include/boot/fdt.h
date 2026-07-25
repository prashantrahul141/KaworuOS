#ifndef _FDT_H_
#define _FDT_H_

#include "types.h"
#include "reg.h"

/*
 * checks and saves flat device tree given by the bootloader limine
 */
void fdt_init(void);

/* find reg with compat */
bool fdt_get_reg_for_compat(const i8 *compat, Reg *reg, u32 reg_count);

/*
 * Query for a node using compatiblity, returns offset, negative if not found
 */
i32 fdt_query_compat(const i8 *compat);

/* get compat for a node */
const i8 *fdt_get_compat(i32 offset, i32 *len);

/*
 * traverse node
 */
i32 fdt_traverse_next_node(i32 offset);

/*
 * Get register from a node
 */
bool fdt_get_reg(i32 node, Reg *reg, u32 reg_count);

/* cpu count */
usize fdt_cpu_count(void);

#endif // _FDT_H_
