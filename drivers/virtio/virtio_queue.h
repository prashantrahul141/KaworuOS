#ifndef _VIRTIO_QUEUE_H_
#define _VIRTIO_QUEUE_H_

#include "common_defs.h"
#include "types.h"
#include "register.h"
#include "memlayout.h"

enum : u16 {
	/* This marks a buffer as continuing via the next field. */
	VIRTQ_DESC_F_NEXT = 1,
	/* This marks a buffer as device write-only (otherwise device
	   read-only). */
	VIRTQ_DESC_F_WRITE = 2,
	/* This means the buffer contains a list of buffer descriptors. */
	VIRTQ_DESC_F_INDIRECT = 4,
};

constexpr usize VIRTIO_OFFSET_QUEUE_SEL = 0x30;
constexpr usize VIRTIO_OFFSET_QUEUE_NUM_MAX = 0x34;
constexpr usize VIRTIO_OFFSET_QUEUE_NUM = 0x38;
constexpr usize VIRTIO_OFFSET_QUEUE_PFN = 0x40;
constexpr usize VIRTIO_OFFSET_QUEUE_READY = 0x44;
constexpr usize VIRTIO_OFFSET_QUEUE_NOTIFY = 0x50;

/* virtqueue Descriptor table entry */
typedef struct {
	u64 addr;
	u32 len;
	u16 flags;
	u16 next;
} PACKED VirtIOQueueDesc;

constexpr usize VIRTIO_QUEUE_ENTRY_COUNT = 16;

// Virtqueue Available Ring.
typedef struct {
	uint16_t flags;
	uint16_t index;
	uint16_t ring[VIRTIO_QUEUE_ENTRY_COUNT];
} PACKED VirtIOQueueAvail;

typedef struct {
	/* Index of start of used descriptor chain. */
	u32 id;
	/* Total length of the descriptor chain which was used (written to) */
	u32 len;
} VirtIOQueueUsedElement;

typedef struct {
	u16 flags;
	u16 index;
	VirtIOQueueUsedElement ring[VIRTIO_QUEUE_ENTRY_COUNT];
	/* Only if VIRTIO_F_EVENT_IDX */
	u16 avail_event;
} VirtIOVirtQUsed;

/* virtqueue */
typedef struct {
	VirtIOQueueDesc descs[VIRTIO_QUEUE_ENTRY_COUNT];
	VirtIOQueueAvail avail;
	ALIGNED(PAGE_SIZE) VirtIOVirtQUsed used;
	u32 queue_index;
	volatile u16 *used_index;
	u16 last_used_index;
} PACKED VirtQ;

/* new virtio queue at given index */
VirtQ *virtio_queue_create(Register base, u32 index);

/* new virtio queue at given index */
void virtio_queue_destroy(VirtQ *v);

static inline bool virtq_is_busy(VirtQ *vq)
{
	return vq->last_used_index != *vq->used_index;
}

#endif // _VIRTIO_MMIO_H_
