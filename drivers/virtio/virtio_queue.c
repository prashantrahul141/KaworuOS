#include "virtio_queue.h"
#include "debug/assert.h"
#include "debug/log.h"
#include "error.h"
#include "memlayout.h"
#include "mm/pmm.h"
#include "register.h"

/* new virtio queue at given index */
VirtQ *virtio_queue_create(Register base, u32 index)
{
	ASSERT(round_up(sizeof(VirtQ), PAGE_SIZE) / PAGE_SIZE == (usize)2,
	       "virtio struct size is not 2 page sizes");

	/*
	 * TODO: according to spec this should be continous memory pages but
	 * since our freelist allocator cannot gurantee that i have added this
	 * check for now.
	 * Have to come back to this later. maybe having a
	 * pmm_alloc_cont(PAGE_COUNT) could work? idk
	 *
	 */
	usize virtq_paddr = pmm_alloc();
	if (IS_ERR((void *)virtq_paddr)) {
		ERROR("Failed to do first allocation for virtio queue");
		return ERR_TO_PTR(-ENOMEM);
	}

	usize virtq_paddr2 = pmm_alloc();
	if (IS_ERR((void *)virtq_paddr2)) {
		ERROR("Failed to do second allocation for virtio queue");
		pmm_free(virtq_paddr);
		return ERR_TO_PTR(-ENOMEM);
	}

	if (virtq_paddr - virtq_paddr2 != PAGE_SIZE) {
		ERROR("TODO: ofcourse pmm alloc cannot gurantee continous "
		      "pages");
		pmm_free(virtq_paddr);
		pmm_free(virtq_paddr2);
		return ERR_TO_PTR(-ENOMEM);
	}

	VirtQ *queue_phy = (VirtQ *)virtq_paddr2;
	VirtQ *queue = pmm_phys_to_virt((usize)queue_phy);

	queue->queue_index = index;
	queue->used_index = &queue->used.index;

	/* select this queue */
	reg_write32(&base, VIRTIO_OFFSET_QUEUE_SEL, index);

	/* specify queue size, number of desciptor */
	reg_write32(&base, VIRTIO_OFFSET_QUEUE_NUM, VIRTIO_QUEUE_ENTRY_COUNT);

	/* write page number count */
	reg_write32(&base, VIRTIO_OFFSET_QUEUE_PFN, (u32)(virtq_paddr2 >> 12));

	return queue_phy;
}

/* new virtio queue at given index */
void virtio_queue_destroy(VirtQ *v)
{
	pmm_free((usize)v);
	pmm_free((usize)v + PAGE_SIZE);
}
