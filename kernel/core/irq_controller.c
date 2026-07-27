#include "core/irq_controller.h"
#include "debug/log.h"
#include "common/manager.h"
#include "debug/panic.h"
#include "sync/spinlock.h"

constexpr usize MAX_IRQ = 1024;

typedef struct {
	SpinLock lock;
	irq_desc irq_table[MAX_IRQ];
	Device *device;
	usize interrupts_count;
} IrqController;

static IrqController irq_controller = {};

void irq_controller_init(void)
{
	INFO("Initializing irq controller");
	spinlock_init(&irq_controller.lock, "irq controller");
	Device *irq_device = dmanager_get_by_class(DEVICE_IRQCHIP);
	if (IS_ERR(irq_device)) {
		panic("no irq chip found");
	}

	if (EOK != dmanager_ready_device(irq_device)) {
		panic("failed to ready irq device");
	}

	irq_controller.device = irq_device;
	irq_controller.interrupts_count =
		irq_device->irq_chip_ops->interrupts_count(irq_device);
}

void irq_dispatcher(void)
{
	u32 active = irq_controller.device->irq_chip_ops->get_active(
		irq_controller.device);

	if (active > MAX_IRQ || active > irq_controller.interrupts_count) {
		ERROR("active irq (%d) more than device interrupts_count (%d) "
		      "or MAX_IRQ (%d)",
		      active, irq_controller.interrupts_count, MAX_IRQ);
		return;
	}

	irq_handler_t handler = irq_controller.irq_table[active].handler;
	void *data = irq_controller.irq_table[active].data;
	if (nullptr != handler) {
		handler(data);
	}
	irq_controller.device->irq_chip_ops->signal_eoi(irq_controller.device,
							active);
}

errno_t request_irq(u32 irq, irq_handler_t handler, void *data)
{
	DEBUG("requesting irq = %d, data = %p", irq, data);
	spinlock_acquire(&irq_controller.lock);
	irq_controller.irq_table[irq].handler = handler;
	irq_controller.irq_table[irq].data = data;
	irq_controller.device->irq_chip_ops->enable(irq_controller.device, irq);
	spinlock_release(&irq_controller.lock);
	return EOK;
}
