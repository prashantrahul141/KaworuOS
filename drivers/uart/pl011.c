#include "pl011.h"
#include "boot/fdt.h"
#include "core/irq_controller.h"
#include "io/console.h"
#include "memlayout.h"
#include "register.h"
#include "error.h"
#include "mm/kheap.h"
#include "mm/vmm.h"
#include "common/manager.h"
#include "types.h"

typedef struct {
	Register base_addr;
	u32 irq;
} Pl011DriverData;

constexpr usize BAUD_RATE = 115200;
constexpr usize BASE_CLOCK = 24000000;

constexpr usize UARTDR = 0x000; // write register
constexpr usize UARTFR = 0x018; // pooling register
constexpr usize UARTFR_BUSY = (1 << 3); // bit in UARTFR, if transmission is
					// busy
constexpr usize UARTFR_RXFE = (1 << 4); // recieve fifo is empty
constexpr usize UARTIBRD = 0x024; // speed 1
constexpr usize UARTFBRD = 0x028; // speed 2
constexpr usize UARTLCR_H = 0x02C; // line control register
constexpr usize UARTLCR_H_FEN = (1 << 4); // fifo
constexpr usize UARTCR = 0x030; // control register
constexpr usize UARTCR_UARTEN = (1 << 0); // enable/disable uart
constexpr usize UARTCR_TEX = (1 << 8); // recieve enable
constexpr usize UARTCR_REX = (1 << 9); // transmit enable
constexpr usize UARTIMSC = 0x038; // control interrupt
constexpr usize UARTICR = 0x044; // interrupt clear register
constexpr usize UARTDMACR = 0x048; // control dma

static void wait_tx_complete(const Device *device)
{
	while ((reg_read32(
			&ACCESS_DRIVER_DATA(Pl011DriverData, device)->base_addr,
			UARTFR) &
		UARTFR_BUSY) != 0)
		;
}

static void wait_rx_ready(const Device *device)
{
	while ((reg_read32(
			&ACCESS_DRIVER_DATA(Pl011DriverData, device)->base_addr,
			UARTFR) &
		UARTFR_RXFE) != 0)
		;
}

static void uart_putchar(Device *device, i8 c)
{
	wait_tx_complete(device);
	reg_write32(&ACCESS_DRIVER_DATA(Pl011DriverData, device)->base_addr,
		    UARTDR, c);
}

static void pl011_write(Device *device, const IOEvent *event)
{
	usize len = 0;
	while (len < event->len) {
		uart_putchar(device, event->msg[len++]);
	}
}

static u8 pl011_read(Device *device)
{
	wait_rx_ready(device);
	return reg_read8(
		&ACCESS_DRIVER_DATA(Pl011DriverData, device)->base_addr,
		UARTDR);
}

static void pl011_flush(Device *device)
{
	wait_tx_complete(device);
}

static void calculate_divisor(u64 base_clock, u32 baud_rate, u32 *ibrd,
			      u32 *fbrd)
{
	const u32 div = 4 * (u32)base_clock / baud_rate;
	*fbrd = div & 0x3f;
	*ibrd = (div >> 6) & 0xffff;
}

static const ConsoleOps pl011_ops = {
	.write = pl011_write,
	.read = pl011_read,
	.flush = pl011_flush,
};

static void recieve_irq_handler(void *data)
{
	Device *device = data;
	Pl011DriverData *device_data =
		ACCESS_DRIVER_DATA(Pl011DriverData, device);
	u8 read = reg_read8(&device_data->base_addr, UARTDR);
	console_receive_char(read);
	reg_write8(&device_data->base_addr, UARTICR, 0);
}

/*
 * initialize pl011
 */
errno_t pl011_probe(Device *device)
{
	DEBUG("probing pl011");
	Register reg;
	if (!fdt_get_reg(device->fdt_node_offset, &reg, 1)) {
		return -ENODEV;
	}

	Pl011DriverData *driver_data = kalloc(sizeof(Pl011DriverData));
	if (IS_ERR(driver_data)) {
		WARN("failed to allocate mem for uart");
		return -ENOMEM;
	}

	driver_data->base_addr.address =
		vm_mmio_map((usize)reg.address, reg.size);
	driver_data->base_addr.size = reg.size;
	if (IS_ERR(driver_data->base_addr.address)) {
		WARN("mapping failed");
		kfree(driver_data);
		return -ENOMEM;
	}

	device->driver_data = driver_data;

	/* disable uart */
	u32 cr = reg_read32(&driver_data->base_addr, UARTCR);
	reg_write32(&driver_data->base_addr, UARTCR,
		    (cr & (u32)~UARTCR_UARTEN));

	/* wait for current tranmission to complete */
	wait_tx_complete(device);

	/* flush fifo */

	u32 lcr = reg_read32(&driver_data->base_addr, UARTLCR_H) |
		  UARTLCR_H_FEN;
	reg_write32(&driver_data->base_addr, UARTLCR_H, lcr);

	/* configure uart */

	/* setting baud rate */
	u32 ibrd, fbrd;
	calculate_divisor(BASE_CLOCK, BAUD_RATE, &ibrd, &fbrd);
	reg_write32(&driver_data->base_addr, UARTIBRD, ibrd);
	reg_write32(&driver_data->base_addr, UARTFBRD, fbrd);

	/* clear previous interrupts */
	reg_write32(&driver_data->base_addr, UARTICR, 0x7ff);

	/* mask all interrupts except rx */
	/* 0000 0000 0001 0000 */
	reg_write32(&driver_data->base_addr, UARTIMSC, (1 << 4));

	/* disable dma */
	reg_write32(&driver_data->base_addr, UARTDMACR, 0x0);

	/* set both tx, rx enable and enable uart. */
	cr = reg_read32(&driver_data->base_addr, UARTCR);
	reg_write32(&driver_data->base_addr, UARTCR,
		    cr | UARTCR_UARTEN | UARTCR_TEX | UARTCR_REX);

	/* request irq */
	FDTInterrupt fdt_interrupt;
	fdt_get_interrupt_cells(device->fdt_node_offset, &fdt_interrupt, 1);
	driver_data->irq = fdt_interrupt.cells[1] + 32;
	request_irq(driver_data->irq, recieve_irq_handler, device);

	/* populate device driver */
	device->driver_data = driver_data;
	device->console_ops = &pl011_ops;
	device->name = "uart";

	return EOK;
}

errno_t pl011_remove(Device *device)
{
	DEBUG("removing pl011");
	Pl011DriverData *data =
		(void *)ACCESS_DRIVER_DATA(Pl011DriverData, device);
	wait_tx_complete(device);
	reject_irq(data->irq);
	vm_mmio_unmap(data->base_addr.address, PAGE_SIZE);
	kfree(device->driver_data);
	return EOK;
}

static const i8 *pl011_compat[] = {
	"arm,pl011",
	nullptr,
};

static const Driver pl011_driver = { .name = "pl011",
				     .probe = pl011_probe,
				     .remove = pl011_remove,
				     .compatible = pl011_compat,
				     .device_class = DEVICE_UART };

REGISTER_DEVICE_DRIVER(pl011_driver);
