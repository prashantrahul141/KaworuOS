#include "virtio_blk.h"
#include "boot/fdt.h"
#include "common/manager.h"
#include "error.h"
#include "irq/irq_controller.h"
#include "memlayout.h"
#include "mm/kheap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "register.h"
#include "sync/completion.h"
#include "virtio/virtio_mmio.h"
#include "virtio/virtio_queue.h"
#include "string.h"

static void virtio_blk_flush_notify_host(Device *device, u16 desc_index)
{
	VirtIOBlkDriverData *data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);
	VirtQ *vq = pmm_phys_to_virt((usize)data->virtq_phy);
	vq->avail.ring[vq->avail.index % VIRTIO_QUEUE_ENTRY_COUNT] = desc_index;
	vq->avail.index++;

	dmb(BARRIER_ISH);

	reg_write32(&data->base_addr, VIRTIO_OFFSET_QUEUE_NOTIFY,
		    vq->queue_index);
	vq->last_used_index++;
}

static void virtio_blk_submit_and_wait(Device *device)
{
	VirtIOBlkDriverData *data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);
	completion_reset(&data->irq_completion);
	virtio_blk_flush_notify_host(device, 0);
	completion_wait(&data->irq_completion);
	dmb(BARRIER_ISH);
}

static void virtio_blk_add_requests(const VirtIOBlkDriverData *data,
				    VirtIOBlkRequest *blk_req_va, usize sector,
				    bool writing)
{
	blk_req_va->sector = sector;
	blk_req_va->type = writing ? VIRTIO_BLOCK_TYPE_OUT :
				     VIRTIO_BLOCK_TYPE_IN;

	VirtQ *vq = pmm_phys_to_virt((usize)data->virtq_phy);
	vq->descs[0].addr = data->request_addr_phy;
	vq->descs[0].len = sizeof(u32) * 2 + sizeof(u64);
	vq->descs[0].flags = VIRTQ_DESC_F_NEXT;
	vq->descs[0].next = 1;

	vq->descs[1].addr =
		data->request_addr_phy + offsetof(VirtIOBlkRequest, data);
	vq->descs[1].len = SECTOR_SIZE;
	vq->descs[1].flags = VIRTQ_DESC_F_NEXT |
			     (writing ? 1 : VIRTQ_DESC_F_WRITE);
	vq->descs[1].next = 2;

	vq->descs[2].addr =
		data->request_addr_phy + offsetof(VirtIOBlkRequest, status);
	vq->descs[2].len = sizeof(uint8_t);
	vq->descs[2].flags = VIRTQ_DESC_F_WRITE;
}

static void virtio_blk_add_write_requests(const VirtIOBlkDriverData *data,
					  VirtIOBlkRequest *blk_req_va,
					  usize sector)
{
	virtio_blk_add_requests(data, blk_req_va, sector, true);
}

static void virtio_blk_add_read_requests(const VirtIOBlkDriverData *data,
					 VirtIOBlkRequest *blk_req_va,
					 usize sector)
{
	virtio_blk_add_requests(data, blk_req_va, sector, false);
}

static errno_t write(Device *device, usize sector, const void *buf)
{
	DEBUG("virtio-blk: write sector = %d", sector);
	VirtIOBlkDriverData *data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);

	VirtIOBlkRequest *blk_req = pmm_phys_to_virt(data->request_addr_phy);

	memcpy(blk_req->data, buf, SECTOR_SIZE);

	virtio_blk_add_write_requests(data, blk_req, sector);

	virtio_blk_submit_and_wait(device);

	if (0 != blk_req->status) {
		ERROR("failed to write sector=%d status=%d", sector,
		      blk_req->status);
		return -ENODEV;
	}

	return EOK;
}

static errno_t read(Device *device, usize sector, void *buf)
{
	DEBUG("virtio-blk: read sector = %d", sector);
	VirtIOBlkDriverData *data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);

	VirtIOBlkRequest *blk_req = pmm_phys_to_virt(data->request_addr_phy);

	virtio_blk_add_read_requests(data, blk_req, sector);

	virtio_blk_submit_and_wait(device);

	if (0 != blk_req->status) {
		ERROR("failed to read sector=%d status=%d", sector,
		      blk_req->status);
		return -ENODEV;
	}

	memcpy(buf, blk_req->data, SECTOR_SIZE);

	return EOK;
}

static usize sector_size(Device *device)
{
	UNUSED_ARG(device);
	return SECTOR_SIZE_BITS;
}

static usize capacity(Device *device)
{
	VirtIOBlkDriverData *data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);

	return data->capacity;
}

static const BlockOps virtio_block_ops = { .write = write,
					   .read = read,
					   .sector_size = sector_size,
					   .capacity = capacity };

static void virtio_blk_irq_handler(void *_data)
{
	Device *device = _data;
	VirtIOBlkDriverData *driver_data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);

	u32 isr = reg_read32(&driver_data->base_addr,
			     VIRTIO_OFFSET_INTERRUPT_STATUS);
	if (isr & 1) {
		reg_write32(&driver_data->base_addr,
			    VIRTIO_OFFSET_INTERRUPT_ACK, 1);
		completion_signal(&driver_data->irq_completion);
	}
}

errno_t virtio_blk_probe(Device *device, Register base, u32 index)
{
	DEBUG("probing virtio block device");
	/* verify magic and version */
	if (VIRTIO_MAGIC != reg_read32(&base, VIRTIO_OFFSET_MAGIC)) {
		ERROR("virtio magic mismatch");
		return -EINVAL;
	}

	if (VIRTIO_VERSION != reg_read32(&base, VIRTIO_OFFSET_VERSION)) {
		ERROR("virtio version mismatch");
		return -EINVAL;
	}

	VirtIOBlkDriverData *data = kalloc(sizeof(VirtIOBlkDriverData));
	if (IS_ERR(data)) {
		ERROR("failed to allocate for virtioblkdriverdata");
		return -ENOMEM;
	}

	data->base_addr = base;

	completion_init(&data->irq_completion, "virtio-blk");

	/* reset device */
	reg_write32(&base, VIRTIO_OFFSET_DEVICE_STATUS, 0);

	/* ack */
	u32 device_status = reg_read32(&base, VIRTIO_OFFSET_DEVICE_STATUS);
	device_status |= VIRTIO_STATUS_ACK;

	/* set driver bit */
	device_status |= VIRTIO_STATUS_DRIVER;
	reg_write32(&base, VIRTIO_OFFSET_DEVICE_STATUS, device_status);

	/* set page size */
	reg_write32(&base, VIRTIO_OFFSET_PAGE_SIZE, PAGE_SIZE);

	/* init queue */
	data->virtq_phy = virtio_queue_create(data->base_addr, index);
	if (IS_ERR(data->virtq_phy)) {
		ERROR("failed creating virtio queue");
		return PTR_TO_ERR(data->virtq_phy);
	}

	/* set driver:OK */
	reg_write32(&base, VIRTIO_OFFSET_DEVICE_STATUS,
		    VIRTIO_STATUS_DRIVER_OK);

	/* get capacity */
	data->capacity = reg_read32(&base, VIRTIO_OFFSET_DEVICE_CONFIG + 0) *
			 SECTOR_SIZE;

	/* memory to hold requests */
	data->request_addr_phy = pmm_alloc();
	if (IS_ERR((void *)data->request_addr_phy)) {
		ERROR("failed to allocate request buffer");
		return -ENOMEM;
	}

	/* request irq */
	FDTInterrupt fdt_interrupt;
	fdt_get_interrupt_cells(device->fdt_node_offset, &fdt_interrupt, 1);
	u32 irq_base = (0 == fdt_interrupt.cells[0]) ? 32 : 16;
	data->irq = fdt_interrupt.cells[1] + irq_base;
	request_irq(data->irq, virtio_blk_irq_handler, device);

	device->driver_data = data;
	device->name = "virtio-blk";
	device->block_ops = &virtio_block_ops;

	return EOK;
}

void virtio_blk_remove(const Device *device)
{
	VirtIOBlkDriverData *data =
		ACCESS_DRIVER_DATA(VirtIOBlkDriverData, device);
	virtio_queue_destroy(data->virtq_phy);
	pmm_free(data->request_addr_phy);
	vm_mmio_unmap(data->base_addr.address, data->base_addr.size);
	kfree(data);
}
