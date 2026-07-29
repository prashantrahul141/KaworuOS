#include "common/manager.h"
#include "boot/fdt.h"
#include "debug/log.h"
#include "error.h"
#include "memlayout.h"
#include "config.h"
#include "string.h"
#include "mm/kheap.h"
#include "sync/spinlock.h"

typedef struct {
	Device *device_list;
	SpinLock lock;
} DriverManager;

static DriverManager dmanager = {};

static Driver *find_compat_driver(const i8 *compat);
static void insert_device(Device *device);

/*
 * Initialize driver manager
 */
void dmanager_init(void)
{
	INFO("Initializing driver manager");
	spinlock_init(&dmanager.lock, "driver manager");

	INFO("Discovering compatible devices");
	spinlock_acquire(&dmanager.lock);
	/* for each fdt node */
	for (i32 offset = fdt_traverse_next_node(-1); offset >= 0;
	     offset = fdt_traverse_next_node(offset)) {
		TRACE("traverse offset = %d", offset);

		/* get compatiblity for this fdt node */
		i32 len;
		const i8 *compat = fdt_get_compat(offset, &len);
		if (nullptr == compat) {
			continue;
		}

		/* find compatible driver for this node */
		Driver *driver = find_compat_driver(compat);
		if (IS_ERR(driver)) {
			continue;
		}

		/* create and prepopulate device */
		Device *device = kalloc(sizeof(Device));
		device->state = DEVICE_DISCOVERED;
		device->name = driver->name;
		device->fdt_node_offset = offset;
		device->driver = driver;

		INFO("Discovered device = %s with driver = %s", device->name,
		     driver->name);
		insert_device(device);
	}

	spinlock_release(&dmanager.lock);
}

errno_t dmanager_ready_device(Device *device)
{
	if (DEVICE_READY == device->state) {
		return EOK;
	}

	switch (device->driver->device_class) {
	case DEVICE_FRAMEBUFFER:
	case DEVICE_UART:
	case DEVICE_IRQCHIP:
	case DEVICE_TIMER: {
		errno_t ret = device->driver->probe(device);
		if (EOK == ret) {
			device->state = DEVICE_READY;
		}
		return ret;
	}
	case DEVICE_UNKNOWN:
	default: {
		return -ENOENT;
	}
	}
}

Device *dmanager_get_by_class_and_ready(DeviceClass class)
{
	Device *device = dmanager_get_by_class(class);
	if (IS_ERR(device)) {
		return device;
	}

	errno_t ret = dmanager_ready_device(device);
	if (EOK != ret) {
		return ERR_TO_PTR(ret);
	}

	return device;
}

Device *dmanager_get_by_class(DeviceClass class)
{
	DEBUG("get by class = %d", class);
	spinlock_acquire(&dmanager.lock);

	Device *curr = dmanager.device_list;
	while (nullptr != curr) {
		if (curr->driver->device_class == class) {
			spinlock_release(&dmanager.lock);
			return curr;
		}
		curr = curr->next;
	}

	spinlock_release(&dmanager.lock);
	return ERR_TO_PTR(-ENOENT);
}

static Driver *find_compat_driver(const i8 *req_compat)
{
	TRACE("finding compat driver, required = %s", req_compat);
	/* for each driver */
	for (Driver **_driver = (Driver **)__KERNEL_TEXT_DRIVERS_START;
	     _driver < (Driver **)__KERNEL_TEXT_DRIVERS_END; _driver++) {
		Driver *driver = *(_driver);

		/* match against each compat of this driver */
		const i8 *const *compat = driver->compatible;
		while (nullptr != *compat) {
			/* if found matching, return */
			if (0 == strcmp(*compat, req_compat)) {
				return driver;
			}
			compat++;
		}
	}
	return ERR_TO_PTR(-ENOENT);
}

static void insert_device(Device *device)
{
	device->next = dmanager.device_list;
	dmanager.device_list = device;
}
