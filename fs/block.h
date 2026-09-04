#ifndef _BLOCK_H_
#define _BLOCK_H_

/*
 * wrapper functions around block type devices
 */

#include "common/manager.h"
#include "error.h"

/*
 * read n blocks starting from given sector
 * the buf should be large enough to be able to fil n * block_size bytes
 */
errno_t block_read(Device *device, usize sector, usize count, void *buf);

/*
 * write n blocks starting from given sector
 * the buf should be large enough to be able to fil n * block_size bytes
 */
errno_t block_write(Device *device, usize sector, usize count, const void *buf);

/*
 * get capacity of the block device
 */
usize block_capacity(Device *device);

/*
 * get sector size of the block device
 */
usize block_sector_size(Device *device);

#endif // _BLOCK_H_
