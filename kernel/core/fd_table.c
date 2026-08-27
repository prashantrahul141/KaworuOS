#include "core/fd_table.h"
#include "debug/assert.h"
#include "debug/log.h"
#include "error.h"
#include "mm/kheap.h"
#include "string.h"
#include "fs/file.h"
#include "sync/spinlock.h"

static inline bool is_fd_state_equal(FDTable *table, usize fd,
				     FileDescriptorState state)
{
	return table->files_state[fd] == state;
}

static bool is_fd_free(FDTable *table, usize fd)
{
	return is_fd_state_equal(table, fd, FILE_DESCRIPTOR_FREE);
}

static bool is_fd_reserved(FDTable *table, usize fd)
{
	return is_fd_state_equal(table, fd, FILE_DESCRIPTOR_RESERVED);
}

static bool is_fd_installed(FDTable *table, usize fd)
{
	return is_fd_state_equal(table, fd, FILE_DESCRIPTOR_INSTALLED);
}

/*
 *
 * init fd table
 */
void fdtable_init(FDTable *fdtable)
{
	spinlock_init(&fdtable->lock, "FDTable");
	memset(&fdtable->files, 0, sizeof(fdtable->files));
	memset(&fdtable->files_state, 0, sizeof(fdtable->files_state));
	memset(&fdtable->close_on_exec, 0, sizeof(fdtable->close_on_exec));
}

/*
 * init fd table with no files
 */
FDTable *fdtable_create(void)
{
	FDTable *table = kalloc(sizeof(FDTable));
	if (IS_ERR(table)) {
		ERROR("failed to allocate for fdtable");
		return ERR_TO_PTR(-ENOMEM);
	}

	fdtable_init(table);
	return table;
}

/*
 * asserts all files closed and destroys all fdtable
 */
errno_t fdtable_destroy(FDTable *fd_table)
{
	spinlock_acquire(&fd_table->lock);
	for (usize c = 0; c < MAX_FILES_PER_FD; c++) {
		FileDescriptorState state = fd_table->files_state[c];
		ASSERT(state == FILE_DESCRIPTOR_FREE, "trying to close fd with "
						      "opened files");
	}

	spinlock_release(&fd_table->lock);
	kfree(fd_table);
	return EOK;
}

/*
 *  closes all files
 */
errno_t fdtable_close_all(FDTable *table)
{
	File *to_put[MAX_FILES_PER_FD];
	usize count = 0;

	spinlock_acquire(&table->lock);
	for (usize fd = 0; fd < MAX_FILES_PER_FD; fd++) {
		File *file = table->files[fd];
		if (is_fd_installed(table, fd)) {
			to_put[count++] = file;
		}

		table->files[fd] = nullptr;
		table->files_state[fd] = FILE_DESCRIPTOR_FREE;
		table->close_on_exec[fd] = false;
	}
	spinlock_release(&table->lock);

	/* calling fileops within lock is not a good idea */
	for (usize c = 0; c < count; c++) {
		file_put(to_put[c]);
	}

	return EOK;
}

/*
 * copies from existing fdtable
 */
FDTable *fdtable_create_from(FDTable *src)
{
	FDTable *dst = fdtable_create();
	if (IS_ERR(dst)) {
		return dst;
	}

	spinlock_acquire_scoped(&src->lock);
	for (usize fd = 0; fd < MAX_FILES_PER_FD; fd++) {
		if (is_fd_installed(src, fd)) {
			dst->files[fd] = file_get(src->files[fd]);
		} else {
			dst->files[fd] = src->files[fd];
		}
		dst->files_state[fd] = src->files_state[fd];
		dst->close_on_exec[fd] = src->close_on_exec[fd];
	}

	return dst;
}

/*
 * reserves lowest free slot which is >= min_fd
 */
errno_t fdtable_reserve(FDTable *table, usize min_fd, usize *out_fd)
{
	spinlock_acquire_scoped(&table->lock);
	if (min_fd >= MAX_FILES_PER_FD) {
		return -EINVAL;
	}

	for (usize fd = min_fd; fd < MAX_FILES_PER_FD; fd++) {
		if (is_fd_free(table, fd)) {
			table->files_state[fd] = FILE_DESCRIPTOR_RESERVED;
			table->files[fd] = nullptr;
			*out_fd = fd;
			return EOK;
		}
	}

	return -EMFILE;
}

/*
 * install a file to a previously reserved slot
 */
errno_t fdtable_install_at(FDTable *table, File *file, usize fd)
{
	spinlock_acquire_scoped(&table->lock);
	if (fd >= MAX_FILES_PER_FD) {
		return -EINVAL;
	}

	if (!is_fd_reserved(table, fd)) {
		return -EBADF;
	}

	table->files[fd] = file;
	table->files_state[fd] = FILE_DESCRIPTOR_INSTALLED;

	return EOK;
}

/*
 * add a new file to file table
 */
errno_t fdtable_add(FDTable *table, File *file, usize *out_fd)
{
	errno_t err = fdtable_reserve(table, 0, out_fd);
	if (EOK != err) {
		return err;
	}

	return fdtable_install_at(table, file, *out_fd);
}

/*
 * open a file which is already present in fd table
 */
File *fdtable_get_file(FDTable *table, usize fd)
{
	spinlock_acquire_scoped(&table->lock);
	if (fd >= MAX_FILES_PER_FD || !is_fd_installed(table, fd)) {
		return ERR_TO_PTR(-EBADF);
	}

	File *file = table->files[fd];
	return file_get(file);
}

/*
 * close a file which is already present in fd table
 */
errno_t fdtable_close_file(FDTable *table, usize fd)
{
	spinlock_acquire(&table->lock);
	if (fd >= MAX_FILES_PER_FD || !is_fd_installed(table, fd)) {
		spinlock_release(&table->lock);
		return -EBADF;
	}

	File *file = table->files[fd];

	table->files[fd] = nullptr;
	table->files_state[fd] = FILE_DESCRIPTOR_FREE;
	table->close_on_exec[fd] = false;

	spinlock_release(&table->lock);

	file_put(file);
	return EOK;
}

static errno_t fdtable_set_close_on_exec_impl(FDTable *table, usize fd,
					      bool close_on_exec)
{
	if (fd >= MAX_FILES_PER_FD || !is_fd_installed(table, fd)) {
		return -EBADF;
	}

	table->close_on_exec[fd] = close_on_exec;
	return EOK;
}

/*
 * set close_on_exec for a fd
 */
errno_t fdtable_set_close_on_exec(FDTable *table, usize fd, bool close_on_exec)
{
	spinlock_acquire_scoped(&table->lock);
	return fdtable_set_close_on_exec_impl(table, fd, close_on_exec);
}

/*
 * copies fd from src_fd to out_fd
 */
errno_t fdtable_dup(FDTable *table, usize src_fd, usize *out_fd)
{
	spinlock_acquire_scoped(&table->lock);

	/* verify if src exsts */
	if (src_fd >= MAX_FILES_PER_FD || !is_fd_installed(table, src_fd)) {
		return -EBADF;
	}

	File *src = table->files[src_fd];

	usize dst_fd = 0;
	for (; dst_fd < MAX_FILES_PER_FD; dst_fd++) {
		if (is_fd_free(table, dst_fd)) {
			break;
		}
	}

	if (dst_fd == MAX_FILES_PER_FD) {
		return -EMFILE;
	}

	table->files[dst_fd] = file_get(src);
	table->files_state[dst_fd] = FILE_DESCRIPTOR_INSTALLED;
	table->close_on_exec[dst_fd] = false;

	*out_fd = dst_fd;
	return EOK;
}

/*
 * copies fd from src_fd to dst_fd
 */
errno_t fdtable_dup_at(FDTable *table, usize src_fd, usize dst_fd,
		       bool close_on_exec)
{
	File *src;
	File *old = nullptr;

	spinlock_acquire(&table->lock);

	/* verify if src exists */
	if (src_fd >= MAX_FILES_PER_FD || !is_fd_installed(table, src_fd)) {
		spinlock_release(&table->lock);
		return -EBADF;
	}

	if (src_fd == dst_fd) {
		errno_t err = fdtable_set_close_on_exec_impl(table, src_fd,
							     close_on_exec);
		spinlock_release(&table->lock);
		return err;
	}

	/* verify dst fd */
	if (dst_fd >= MAX_FILES_PER_FD) {
		spinlock_release(&table->lock);
		return -EBADF;
	}

	src = table->files[src_fd];

	/* dup2/dup3 replace an already open dst */
	if (is_fd_installed(table, dst_fd)) {
		old = table->files[dst_fd];
		table->files[dst_fd] = nullptr;
		table->files_state[dst_fd] = FILE_DESCRIPTOR_FREE;
		table->close_on_exec[dst_fd] = false;
	}

	table->files[dst_fd] = file_get(src);
	table->files_state[dst_fd] = FILE_DESCRIPTOR_INSTALLED;
	table->close_on_exec[dst_fd] = close_on_exec;

	spinlock_release(&table->lock);

	/* drop the replaced file's reference outside the lock */
	if (nullptr != old) {
		file_put(old);
	}

	return EOK;
}
