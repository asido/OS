/******************************************************************************
 *       Virtual filesystem.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef VFS_SWF400KO
#define VFS_SWF400KO

#include <libc.h>

#define MAX_MOUNTNAME_SIZE 16
#define MAX_FILENAME_LENGTH 12

/*
 * Storage device type used when mounting.
 */
enum storage_dev_type
{
    STORAGE_DEVICE_FLOPPY,
    STORAGE_DEVICE_HDD /* <-- hopefully someday will be used ^^ */
};

enum seek_type
{
    SEEK_BEGIN,
    SEEK_OFFSET,
    SEEK_END
};

enum file_type
{
    FOLDER_TYPE,
    FILE_TYPE,
    DEVICE_TYPE
};

typedef char *dev_read_func_t(char *buf, size_t offset, size_t b_cnt);
typedef int dev_write_func_t(/* TODO: NOT IMPLEMENTED */);

struct dev_driver
{
	dev_read_func_t *read;
    dev_write_func_t *write;
};

typedef char *fs_read_func_t(char *buf, size_t b_cnt);
typedef int fs_write_func_t(/* TODO: NOT IMPLEMENTED */);
typedef char **fs_ls_func_t(struct fs_driver *fs_drv, const char *dir);

struct fs_driver
{
    fs_read_func_t *read;
    fs_write_func_t *write;
	fs_ls_func_t *ls;
	struct dev_driver *dev_driver;
};

/*
 * Definition of FILE type, which is defined as a standard
 * and is a must have in all systems.
 */
typedef struct _file
{
	char *filename;
    /* Current offset into the file.
     * The next read or write is going to start from here */
    unsigned int offset;
    /* Permissions and all whatever i'll think of
     * will go here */
    unsigned int flags;
    enum file_type file_type;
    /* File size */
    size_t size;
    /* Functions implemented in by the device driver
     * which will be called to perform specific 
     * operations on the file. */
    struct mount_point *mount_pt;
} FILE;

enum FileAttributes
{
	READ_ONLY	= 0x01,
	HIDDEN		= 0x02,
	SYSTEM		= 0x04,
	VOLUME_LABEL= 0x08,
	SUBDIRECTORY= 0x10,
	ARCHIVE		= 0x20
};

struct fileinfo
{
	char *filename;
	unsigned int flags;
	size_t size; /* in bytes */
};

/* Common I/O functions */
int mount(enum storage_dev_type dev_type, char *filename);
int unmount(char *filename);
FILE *open(char *filename);
void close(FILE *hndl);
int read(FILE *hndl, void *buf, size_t nbytes);

/* Returns char array with files in directory separated by '\0'.
 * Double '\0' means the end of array. */
struct fileinfo **get_file_list(const char *dir);

struct fileinfo *alloc_fileinfo();
void dealoc_fileinfo(struct fileinfo *inf);

#endif /* end of include guard: VFS_SWF400KO */
