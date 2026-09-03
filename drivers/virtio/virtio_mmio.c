#include "virtio_mmio.h"
#include "boot/fdt.h"
#include "common/manager.h"
#include "debug/log.h"
#include "error.h"
#include "virtio_blk.h"
#include "mm/vmm.h"
#include "register.h"

static u32 virtio_devices_index = 0;

/* qemu exposes multiple virtio-mmio nodes with the same backing.
 * so only probe the first one and return error for otheres
 */
static bool virtio_mmio_device_claimed[VIRTIO_MMIO_DEVICE_ID_SCSI_HOST + 1] = {
	0
};

/*
 * initialize pl011
 */
static errno_t virtio_mmio_probe(Device *device)
{
	DEBUG("probing virtio-mmio");
	Register reg_pa;
	if (!fdt_get_reg(device->fdt_node_offset, &reg_pa, 1)) {
		return -ENODEV;
	}

	Register reg_va = {
		.address = vm_mmio_map((usize)reg_pa.address, reg_pa.size),
		.size = reg_pa.size,
	};
	if (IS_ERR(reg_va.address)) {
		WARN("virtio mapping failed");
		return -ENOMEM;
	}

	/* which type of virtio device */
	const VirtDeviceID device_id =
		get_device_id(reg_read32(&reg_va, VIRTIO_OFFSET_DEVICEID));
	if (VIRTIO_MMIO_DEVICE_ID_UNKNOWN == device_id) {
		ERROR("unknown virtio device");
		vm_mmio_unmap(reg_va.address, reg_va.size);
		return -ENODEV;
	}

	/* this type of device is already configured, skipping */
	if (virtio_mmio_device_claimed[device_id]) {
		WARN("duplicate virtio device id = %d, skipping", device_id);
		vm_mmio_unmap(reg_va.address, reg_va.size);
		return -ENODEV;
	}

	virtio_mmio_device_claimed[device_id] = true;

	/* set device type accordingly */
	device->class = device_id_to_device_class(device_id);

	/* per type init function handles filling device properties and driver
	 * data */
	errno_t ret = EOK;
	switch (device->class) {
	case DEVICE_BLOCK: {
		log_set_level(LEVEL_DEBUG);
		ret = virtio_blk_probe(device, reg_va, virtio_devices_index++);
		break;
	}
	case DEVICE_NETWORK:
	case DEVICE_VIRTIO_MMIO_DEVICE:
	case DEVICE_ENTROPY:
	case DEVICE_TIMER:
	case DEVICE_IRQCHIP:
	case DEVICE_CONSOLE:
	case DEVICE_FRAMEBUFFER:
	case DEVICE_UNKNOWN:
	default:
		ERROR("driver not available for this virtio device");
		vm_mmio_unmap(reg_va.address, reg_va.size);
		return -ENODEV;
	}

	if (ret != EOK) {
		ERROR("failed per virtio type init");
		vm_mmio_unmap(reg_va.address, reg_va.size);
		return ret;
	}

	return EOK;
}

static errno_t virtio_mmio_remove(Device *device)
{
	DEBUG("removing virtio_mmio");

	switch (device->class) {
	case DEVICE_BLOCK: {
		virtio_blk_remove(device);
	}
	case DEVICE_NETWORK:
	case DEVICE_VIRTIO_MMIO_DEVICE:
	case DEVICE_ENTROPY:
	case DEVICE_TIMER:
	case DEVICE_IRQCHIP:
	case DEVICE_CONSOLE:
	case DEVICE_FRAMEBUFFER:
	case DEVICE_UNKNOWN:
	default:
		ERROR("removing device which is not supported?");
		break;
	}
	return EOK;
}

static const i8 *virtio_mmio_compat[] = {
	"virtio,mmio",
	nullptr,
};

static const Driver virtio_mmio_driver = { .name = "virtio_mmio",
					   .probe = virtio_mmio_probe,
					   .remove = virtio_mmio_remove,
					   .compatible = virtio_mmio_compat,
					   .device_class =
						   DEVICE_VIRTIO_MMIO_DEVICE };

REGISTER_DEVICE_DRIVER(virtio_mmio_driver);
