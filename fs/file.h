#ifndef _FILE_H_
#define _FILE_H_

#include "sync/refcount.h"
#include "types.h"

typedef struct FileOps FileOps;
typedef struct File File;

typedef enum {
	FILE_MODE_READ = (1 << 0),
	FILE_MODE_WRITE = (1 << 1),
	FILE_MODE_APPEND = (1 << 2),
	FILE_MODE_NONBLOCK = (1 << 3),
} FileModes;

/* whence values for file_lseek */
typedef enum : u8 {
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2,
} LSeekWhence;

struct FileOps {
	/*
	 * write buf[buf_count] to file, return number of bytes written or
	 * -errno
	 */
	i64 (*write)(File *file, const void *buf, usize buf_count);
	/*
	 * read into buf[buf_count] from file, return number of bytes read or
	 * -errno
	 */
	i64 (*read)(File *file, void *buf, usize buf_count);
	/*
	 * move cursor, return new absolute offset or -errno
	 */
	i64 (*lseek)(File *file, i64 offset, LSeekWhence whence);
	/*
	 * ioctl calls
	 */
	i64 (*ioctl)(File *file, u64 request, void *arg);
	/*
	 * closes file
	 */
	errno_t (*close)(File *file);
};

struct File {
	const FileOps *ops;
	u32 mode;
	i64 offset;
	RefCount rc;
	void *private_data;
};

/*
 * initialise file
 */
void file_init(File *file, const FileOps *ops, u32 file_mode,
	       void *private_data);

/*
 * create a new file
 */
File *file_create(const FileOps *ops, u32 file_modes, void *private_data);

/*
 * get a file, increases its reference count
 */
File *file_get(File *file);

/*
 * give back a file, decreases its reference count
 */
i64 file_put(File *file);

/* generic dispatch */
i64 file_read(File *file, void *buf, usize count);
i64 file_write(File *file, const void *buf, usize count);
i64 file_lseek(File *file, i64 offset, LSeekWhence whence);

static inline bool file_mode_is_readable(File *file)
{
	return (file->mode & FILE_MODE_READ) != 0;
}

static inline bool file_mode_is_writable(File *file)
{
	return (file->mode & FILE_MODE_WRITE) != 0;
}

static inline bool file_mode_is_appending(File *file)
{
	return (file->mode & FILE_MODE_APPEND) != 0;
}

static inline bool file_mode_is_nonblocking(File *file)
{
	return (file->mode & FILE_MODE_NONBLOCK) != 0;
}

#endif // _FILE_H_
