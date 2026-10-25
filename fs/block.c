#include "fs/block.h"
#include "common/manager.h"
#include "error.h"
#include "debug/assert.h"

/* validate given sector count */
static errno_t verify_sector_range(usize sector, usize count, usize capacity,
				   usize sector_size)
{
	if (0 == capacity || 0 == sector_size) {
		ERROR("capacity (%d) or sector_size (%d) is 0", capacity,
		      sector_size);
		return -ENODEV;
	}

	/* validate given sector count */
	if (sector >= capacity / sector_size) {
		WARN("sector (%d) bigger than capacity (%d) / sector_size (%d) "
		     "= %d",
		     sector, capacity, sector_size, capacity / sector_size);
		return -EINVAL;
	}

	if (sector + count >= capacity / sector_size) {
		WARN("sector (%d) + sector_count (%d) bigger than capacity "
		     "(%d) "
		     "/ sector_size (%d) = %d",
		     sector, count, capacity, sector_size,
		     capacity / sector_size);
		return -EINVAL;
	}

	return EOK;
}

static errno_t internal_block_read(Device *device, usize sector, usize count,
				   void *buf, const bool block)
{
	ASSERT(device->class == DEVICE_BLOCK, "block_read was given a non "
					      "block device");

	if (0 == count) {
		return EOK;
	}

	usize capacity = device->block_ops->capacity(device);
	usize sector_size = device->block_ops->sector_size(device);

	errno_t ret = verify_sector_range(sector, count, capacity, sector_size);
	if (EOK != ret) {
		return ret;
	}

	for (usize sect = sector; sect < sector + count; sect++) {
		usize offseted_buf =
			(usize)buf + ((sect - sector) * sector_size);
		if (block) {
			device->block_ops->read_sync(device, sect,
						     (void *)offseted_buf);
		} else {
			device->block_ops->read(device, sect,
						(void *)offseted_buf);
		}
	}

	return EOK;
}

/*
 * read n blocks starting from given sector
 * the buf should be large enough to be able to fil n * block_size bytes
 */
errno_t block_read_sync(Device *device, usize sector, usize count, void *buf)
{
	return internal_block_read(device, sector, count, buf, true);
}

/*
 * read n blocks starting from given sector
 * the buf should be large enough to be able to fil n * block_size bytes
 */
errno_t block_read(Device *device, usize sector, usize count, void *buf)
{
	return internal_block_read(device, sector, count, buf, false);
}

static errno_t internal_block_write(Device *device, usize sector, usize count,
				    const void *buf, const bool block)
{
	ASSERT(device->class == DEVICE_BLOCK, "block_read was given a non "
					      "block device");

	DEBUG("device = %s, sector = %d, count = %d, block = %b", device->name,
	      sector, count, block);

	if (0 == count) {
		return EOK;
	}

	usize capacity = device->block_ops->capacity(device);
	usize sector_size = device->block_ops->sector_size(device);

	errno_t ret = verify_sector_range(sector, count, capacity, sector_size);
	if (EOK != ret) {
		return ret;
	}

	for (usize sect = sector; sect < sector + count; sect++) {
		usize offseted_buf = (usize)buf + (sect * sector_size);
		if (block) {
			device->block_ops->write_sync(device, sect,
						      (void *)offseted_buf);
		} else {
			device->block_ops->write(device, sect,
						 (void *)offseted_buf);
		}
	}

	return EOK;
}

/*
 * write n blocks starting from given sector
 * the buf should be large enough to be able to fil n * block_size bytes
 * waits until write is finished
 */
errno_t block_write_sync(Device *device, usize sector, usize count,
			 const void *buf)
{
	return internal_block_write(device, sector, count, buf, true);
}

/*
 * write n blocks starting from given sector
 * the buf should be large enough to be able to fil n * block_size bytes
 */
errno_t block_write(Device *device, usize sector, usize count, const void *buf)
{
	return internal_block_write(device, sector, count, buf, false);
}

/*
 * get capacity of the block device
 */
usize block_capacity(Device *device)
{
	ASSERT(device->class == DEVICE_BLOCK, "block_read was given a non "
					      "block device");
	return device->block_ops->capacity(device);
}

/*
 * get sector size of the block device
 */
usize block_sector_size(Device *device)
{
	ASSERT(device->class == DEVICE_BLOCK, "block_read was given a non "
					      "block device");
	return device->block_ops->sector_size(device);
}
