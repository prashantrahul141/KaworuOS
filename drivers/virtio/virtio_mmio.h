#ifndef _VIRTIO_MMIO_H_
#define _VIRTIO_MMIO_H_

#include "common/manager.h"
#include "memlayout.h"
#include "register.h"

constexpr u32 VIRTIO_MAGIC = 0x74726976;
constexpr u32 VIRTIO_VERSION = 1;
constexpr u32 VIRTIO_STATUS_ACK = (1 << 0);
constexpr u32 VIRTIO_STATUS_DRIVER = (1 << 1);
constexpr u32 VIRTIO_STATUS_DRIVER_OK = (1 << 2);

constexpr usize VIRTIO_OFFSET_MAGIC = 0x0;
constexpr usize VIRTIO_OFFSET_VERSION = 0x4;
constexpr usize VIRTIO_OFFSET_DEVICEID = 0x8;
constexpr usize VIRTIO_OFFSET_PAGE_SIZE = 0x28;
constexpr usize VIRTIO_OFFSET_DEVICE_STATUS = 0x70;
constexpr usize VIRTIO_OFFSET_DEVICE_CONFIG = 0x100;
constexpr usize VIRTIO_OFFSET_INTERRUPT_STATUS = 0x60;
constexpr usize VIRTIO_OFFSET_INTERRUPT_ACK = 0x64;

typedef enum {
	VIRTIO_MMIO_DEVICE_ID_UNKNOWN = 0,
	VIRTIO_MMIO_DEVICE_ID_NETWORK = 1,
	VIRTIO_MMIO_DEVICE_ID_BLOCK = 2,
	VIRTIO_MMIO_DEVICE_ID_CONSOLE = 3,
	VIRTIO_MMIO_DEVICE_ID_ENTROPY = 4,
	VIRTIO_MMIO_DEVICE_ID_MEMORY_BALLOON = 5,
	VIRTIO_MMIO_DEVICE_ID_SCSI_HOST = 5,
} VirtDeviceID;

static inline VirtDeviceID get_device_id(u32 id)
{
	if (id > VIRTIO_MMIO_DEVICE_ID_SCSI_HOST || 0 == id) {
		return VIRTIO_MMIO_DEVICE_ID_UNKNOWN;
	}

	return (VirtDeviceID)id;
}

static inline DeviceClass device_id_to_device_class(VirtDeviceID id)
{
	switch (id) {
	case VIRTIO_MMIO_DEVICE_ID_NETWORK:
		return DEVICE_NETWORK;
		break;
	case VIRTIO_MMIO_DEVICE_ID_BLOCK:
		return DEVICE_BLOCK;
		break;
	case VIRTIO_MMIO_DEVICE_ID_CONSOLE:
		return DEVICE_CONSOLE;
		break;
	case VIRTIO_MMIO_DEVICE_ID_ENTROPY:
		return DEVICE_ENTROPY;
		break;
	case VIRTIO_MMIO_DEVICE_ID_MEMORY_BALLOON:
	case VIRTIO_MMIO_DEVICE_ID_UNKNOWN:
	default:
		return DEVICE_UNKNOWN;
	}

	return DEVICE_UNKNOWN;
}

#endif // _VIRTIO_MMIO_H_
