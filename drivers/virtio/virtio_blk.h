#ifndef _VIRT_IO_BLK_H_
#define _VIRT_IO_BLK_H_

#include "common/manager.h"
#include "common_defs.h"
#include "error.h"
#include "register.h"
#include "types.h"
#include "virtio/virtio_queue.h"

constexpr usize SECTOR_SIZE = 512;
constexpr usize VIRTIO_BLOCK_TYPE_IN = 0;
constexpr usize VIRTIO_BLOCK_TYPE_OUT = 1;

/* virtio blk requests */
typedef struct {
	u32 type;
	u32 reserved;
	u64 sector;
	u8 data[SECTOR_SIZE];
	u8 status;
} PACKED VirtIOBlkRequest;

typedef struct {
	u32 irq;
	Register base_addr;
	VirtQ *virtq_phy;
	usize request_addr_phy;
	usize capacity;
} VirtIOBlkDriverData;

errno_t virtio_blk_probe(Device *device, Register base, u32 index);

void virtio_blk_remove(const Device *device);

#endif // _VIRT_IO_BLK_H_
