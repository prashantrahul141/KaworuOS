#ifndef _FD_TABLE_H_
#define _FD_TABLE_H_

#include "fs/file.h"
#include "sync/spinlock.h"

constexpr usize MAX_FILES_PER_FD = 64;

typedef enum : u8 {
	FILE_DESCRIPTOR_FREE,
	FILE_DESCRIPTOR_RESERVED,
	FILE_DESCRIPTOR_INSTALLED,
} FileDescriptorState;

typedef struct {
	SpinLock lock;
	File *files[MAX_FILES_PER_FD];
	FileDescriptorState files_state[MAX_FILES_PER_FD];
	bool close_on_exec[MAX_FILES_PER_FD];
} FDTable;

/*
 *
 * init fd table
 */
void fdtable_init(FDTable *fdtable);

/*
 * init fd table with no files
 */
FDTable *fdtable_create(void);

/*
 * copies from existing fdtable
 */
FDTable *fdtable_create_from(FDTable *src);

/*
 *  closes all files
 */
errno_t fdtable_close_all(FDTable *fd_table);

/*
 * asserts all files closed and destroys all fdtable
 */
errno_t fdtable_destroy(FDTable *fd_table);

/*
 * reserves lowest free slot which is >= min_fd
 */
errno_t fdtable_reserve(FDTable *table, usize min_fd, usize *out_fd);

/*
 * install a file to a previously reserved slot
 */
errno_t fdtable_install_at(FDTable *table, File *file, usize fd);

/*
 * add a new file to file table, wrapper around reserve and install_at
 */
errno_t fdtable_add(FDTable *table, File *file, usize *out_fd);

/*
 * look up an fd, taking a ref
 * the caller must file_put
 */
File *fdtable_get_file(FDTable *table, usize fd);

/*
 * close an fd: drop te table's ref and free the slot
 */
errno_t fdtable_close_file(FDTable *table, usize fd);

/*
 * copies fd from src_fd to a new fd which is saved in out_fd
 */
errno_t fdtable_dup(FDTable *table, usize src_fd, usize *out_fd);

/*
 * copies fd from src_fd to dst_fd
 */
errno_t fdtable_dup_at(FDTable *table, usize src_fd, usize dst_fd,
		       bool close_on_exec);

/*
 * set close_on_exec for a fd
 */
errno_t fdtable_set_close_on_exec(FDTable *table, usize fd, bool close_on_exec);

/*
 * adds posix stdout, stderr to a fdtable
 */
errno_t fdtable_attach_std_files(FDTable *table);

#endif // _FD_TABLE_H_
