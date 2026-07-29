#include "boot/fdt.h"
#include "common/manager.h"
#include "common_defs.h"
#include "debug/log.h"
#include "mm/kheap.h"
#include "mm/vmm.h"
#include "string.h"

constexpr u32 GIC_SPI_INT_BASE = 32;

constexpr u32 GICC_CTLR_ENABLEGRP0 = BIT(0);
constexpr u32 GICC_CTLR_ENABLEGRP1 = BIT(1);
constexpr u32 GICC_CTLR_ENABLE_MASK =
	(GICC_CTLR_ENABLEGRP0 | GICC_CTLR_ENABLEGRP1);

enum {
	GICD_CTLR = 0x0,
	GICD_TYPER = 0x4,
	GICD_IIDR = 0x08,
	GICD_IGROUPRn = 0x80,
	GICD_ISENABLERn = 0x100,
	GICD_ICENABLERn = 0x180,
	GICD_ICACTIVERn = 0x380,
	GICD_IPRIORITYRn = 0x400,
	GICD_ITARGETSRn = 0x800,
	GICD_ICFGRn = 0xc00,
};

enum {
	GICC_CTLR = 0x0,
	GICC_PMR = 0x4,
	GICC_IAR = 0xC,
	GICC_EOIR = 0x10,
};

typedef struct {
	Register dist;
	Register cpu;
	usize interrupts_count;
} GicV2Data;

static void arm_gic_irq_enable(const Device *device, u32 irq)
{
	DEBUG("enabling irq = %d", irq);
	GicV2Data *data = ACCESS_DRIVER_DATA(GicV2Data, device);
	usize int_grp = irq / 32;
	usize int_off = irq % 32;
	reg_write32(&data->dist, (GICD_ISENABLERn + int_grp * 4),
		    (1 << int_off));
}

static void arm_gic_irq_disable(const Device *device, u32 irq)
{
	DEBUG("disabling irq = %d", irq);
	GicV2Data *data = ACCESS_DRIVER_DATA(GicV2Data, device);
	usize int_grp = irq / 32;
	usize int_off = irq % 32;
	reg_write32(&data->dist, (GICD_ICENABLERn + int_grp * 4),
		    (1 << int_off));
}

static u32 arm_gic_irq_get_active(const Device *device)
{
	GicV2Data *data = ACCESS_DRIVER_DATA(GicV2Data, device);
	return reg_read32(&data->cpu, GICC_IAR);
}

static void arm_gic_irq_eoi(const Device *device, u32 irq)
{
	GicV2Data *data = ACCESS_DRIVER_DATA(GicV2Data, device);
	reg_write32(&data->cpu, GICC_EOIR, irq);
}

static u32 _interrupts_count(const Register *dist)
{
	u32 typer = reg_read32(dist, GICD_TYPER) & 0x1f;
	return (typer + 1) * 32;
}

static u32 interrupts_count(const Device *device)
{
	GicV2Data *data = ACCESS_DRIVER_DATA(GicV2Data, device);
	return _interrupts_count(&data->dist);
}

IrqChipOps gic_ops = { .enable = arm_gic_irq_enable,
		       .disable = arm_gic_irq_disable,
		       .get_active = arm_gic_irq_get_active,
		       .signal_eoi = arm_gic_irq_eoi,
		       .interrupts_count = interrupts_count };

static void armgicv2_probe_dist(const Register *dist)
{
	u32 irqs = _interrupts_count(dist);

	/* disable forwarding */
	reg_write32(dist, GICD_CTLR, 0);

	usize cpu_count = fdt_cpu_count();
	u8 cpu_mask = 0;
	for (usize i = 0; i < cpu_count; i++) {
		cpu_mask |= BIT(i);
	}

	u32 reg_val = (u32)(cpu_mask | (cpu_mask << 8) | (cpu_mask << 16) |
			    (cpu_mask << 24));
	for (u32 i = GIC_SPI_INT_BASE; i < irqs; i += 4) {
		reg_write32(dist, GICD_ITARGETSRn + i, reg_val);
	}

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (u32 i = GIC_SPI_INT_BASE; i < irqs; i += 16) {
		reg_write32(dist, GICD_ICFGRn + i / 4, 0);
	}

	/*  Set priority on all global interrupts.   */
	for (u32 i = GIC_SPI_INT_BASE; i < irqs; i += 4) {
		reg_write32(dist, GICD_IPRIORITYRn + i, 0);
	}

	/* Set all interrupts to group 0 */
	for (u32 i = GIC_SPI_INT_BASE; i < irqs; i += 32) {
		reg_write32(dist, GICD_IGROUPRn + i / 8, 0);
	}

	/*
	 * Disable all interrupts.  Leave the PPI and SGIs alone
	 * as these enables are banked registers.
	 */
	for (u32 i = GIC_SPI_INT_BASE; i < irqs; i += 32) {
		reg_write32(dist, GICD_ICENABLERn + i / 8, 0xffffffff);
	}

	/* enable forwarding */
	reg_write32(dist, GICD_CTLR, 1);
}

static void armgicv2_probe_cpu(const Register *dist, const Register *cpu)
{
	reg_write32(dist, GICD_ICENABLERn, 0xffff0000);
	reg_write32(dist, GICD_ISENABLERn, 0x0000ffff);

	/*
	 * Set priority on PPI and SGI interrupts
	 */
	for (u32 i = 0; i < 32; i += 4) {
		reg_write32(dist, GICD_IPRIORITYRn + i, 0xa0a0a0a0);
	}

	reg_write32(cpu, GICC_PMR, 0xf0);

	/*
	 * Enable interrupts and signal them using the IRQ signal.
	 */
	u32 val = reg_read32(cpu, GICC_CTLR);
	val |= GICC_CTLR_ENABLE_MASK;
	reg_write32(cpu, GICC_CTLR, val);
}

static errno_t armgicv2_probe(Device *device)
{
	DEBUG("probing arm-gicv2");

	GicV2Data *gic_data = kalloc(sizeof(GicV2Data));
	if (IS_ERR(device->driver_data)) {
		WARN("failed allocation");
		return -ENOMEM;
	}

	Register reg[2];
	if (!fdt_get_reg(device->fdt_node_offset, (Register *)&reg, 2)) {
		kfree(gic_data);
		return -ENODEV;
	}

	memcpy(&gic_data->dist, &reg[0], sizeof(Register));
	memcpy(&gic_data->cpu, &reg[1], sizeof(Register));

	gic_data->dist.address =
		vm_mmio_map((usize)gic_data->dist.address, gic_data->dist.size);
	if (IS_ERR(gic_data->dist.address)) {
		kfree(gic_data);
		WARN("vm mmio map failed for distributor");
		return -ENOMEM;
	}

	gic_data->cpu.address =
		vm_mmio_map((usize)gic_data->cpu.address, gic_data->cpu.size);
	if (IS_ERR(gic_data->cpu.address)) {
		vm_mmio_unmap(gic_data->dist.address, reg[0].size);
		kfree(gic_data);
		WARN("vm mmio map failed for distributor");
		return -ENOMEM;
	}

	/* program distributor */
	armgicv2_probe_dist(&gic_data->dist);

	/* program cpu interface */
	armgicv2_probe_cpu(&gic_data->dist, &gic_data->cpu);

	gic_data->interrupts_count = _interrupts_count(&gic_data->dist);
	DEBUG("interrupts_count = %d", gic_data->interrupts_count);

	device->driver_data = gic_data;
	device->name = "gic";
	device->irq_chip_ops = &gic_ops;
	return EOK;
}

static errno_t armgicv2_remove(Device *device)
{
	GicV2Data *gic_data = device->driver_data;
	vm_mmio_unmap(gic_data->dist.address, gic_data->dist.size);
	vm_mmio_unmap(gic_data->cpu.address, gic_data->cpu.size);
	kfree(device->driver_data);
	return EOK;
}

static const i8 *armgicv2_compat[] = {
	"arm,gic-200",
	"arm,cortex-a15-gic",
	nullptr,
};

static const Driver armgicv2_driver = { .name = "arm-gicv2",
					.probe = armgicv2_probe,
					.remove = armgicv2_remove,
					.compatible = armgicv2_compat,
					.device_class = DEVICE_IRQCHIP };

REGISTER_DEVICE_DRIVER(armgicv2_driver);
