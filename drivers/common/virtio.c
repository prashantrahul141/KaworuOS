#include "virtio.h"
#include "common/manager.h"
#include "debug/log.h"

void virtio_init(void)
{
	INFO("Initializing virtio devices");
	usize count = 0;
	for (;;) {
		Device *device = dmanager_get_by_class_skip(
			DEVICE_VIRTIO_MMIO_DEVICE, count);
		if (IS_ERR(device)) {
			return;
		}

		errno_t err = dmanager_ready_device(device);
		if (EOK != err) {
			WARN("failed to initialize virtio device: %s",
			     device->name);
		}

		count++;
	}
}
