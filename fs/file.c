#include "fs/file.h"
#include "debug/assert.h"
#include "mm/kheap.h"
#include "sync/refcount.h"

/*
 * initialise file
 */
void file_init(File *file, const FileOps *ops, u32 file_mode,
	       void *private_data)
{
	ASSERT(ops != nullptr, "files ops is null");
	file->ops = ops;
	file->offset = 0;
	file->private_data = private_data;
	file->mode = file_mode;
	refcount_init(&file->rc);
}

/*
 * create a new file
 */
File *file_create(const FileOps *ops, u32 file_modes, void *private_data)
{
	File *file = kalloc(sizeof(File));
	if (IS_ERR(file)) {
		return file;
	}

	file_init(file, ops, file_modes, private_data);
	return file;
}

static void file_release(RefCount *rc)
{
	File *file = container_of(rc, File, rc);
	if (nullptr != file->ops && nullptr != file->ops->close) {
		file->ops->close(file);
	}
	kfree(file);
}

/*
 * give back a file, decreases its reference count
 * return
 *  - +ve = number of refs left after decrement
 *  - -ve = error
 *  - 0   = file cleaned
 */
i64 file_put(File *file)
{
	return refcount_put(&file->rc, file_release);
}

/*
 * get a file, increases its reference count
 */
File *file_get(File *file)
{
	refcount_get(&file->rc);
	return file;
}

/*
 * generic dispatach functions
 */

i64 file_read(File *file, void *buf, usize count)
{
	if (!file_mode_is_readable(file)) {
		return -EBADF;
	}

	if (nullptr == file->ops || nullptr == file->ops->read) {
		return -ENOSYS;
	}

	i64 ret = file->ops->read(file, buf, count);
	if (ret > 0) {
		file->offset += ret;
	}

	return ret;
}

i64 file_write(File *file, const void *buf, usize count)
{
	if (!file_mode_is_writable(file)) {
		return -EBADF;
	}

	if (nullptr == file->ops || nullptr == file->ops->write) {
		return -ENOSYS;
	}

	i64 ret = file->ops->write(file, buf, count);
	if (ret > 0) {
		file->offset += ret;
	}

	return ret;
}

i64 file_lseek(File *file, i64 offset, LSeekWhence whence)
{
	if (whence > SEEK_END) {
		return -EINVAL;
	}

	if (nullptr == file->ops || nullptr == file->ops->lseek) {
		return -ESPIPE;
	}

	i64 ret = file->ops->lseek(file, offset, whence);
	if (ret >= 0) {
		file->offset = ret;
	}

	return ret;
}
