/******************************************************************************
 *       Virtual filesystem.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <linklist.h>
#include <error.h>
#include <floppy.h>
#include "vfs.h"

/*
 * VFS is a root (/) directory. It holds only the folders
 * which correspond to mount point.
 * Example: a mounted floppy drive is going to be mounted as:
 * /floppy
 * To reach a file named KERNEL in it, you have to refer to:
 * /floppy/KERNEL
 * VFS exist in memory only and is created on boot.
 */



/*
 * Each mounted device in the VFS will have it's own mount_point
 * structure.
 */
struct mount_point
{
	struct llist_t ll;
    /* Filename */
    char *name;
    /* A struct with storage device driver provided routines
     * to read and manipulate data on it. */
    struct fs_driver *fs_driver;
};

struct vfs
{
    struct mount_point *mount_pts; /* linked list of mounted devices */
    size_t dir_count;   /* currently mounted dir count */
};

static struct vfs _vfs;

int vfs_init()
{
    _vfs.mount_pts = NULL;
    _vfs.dir_count = 0;

    return 0;
}

/*
 * Mounts a given device to VFS.
 */
int mount(enum storage_dev_type dev_type, char *filename)
{
    struct mount_point *mount;

    if (!filename)
        return -EBADARG;

    /* Create a new mount_point */
    mount = (struct mount_point *) kalloc(sizeof(struct mount_point));
    if (!mount)
        return -ENOMEM;
	mount->fs_driver = (struct fs_driver *) kalloc(sizeof(struct fs_driver));
	if (!mount->fs_driver)
		return -ENOMEM;
	mount->fs_driver->dev_driver = (struct dev_driver *) kalloc(sizeof(struct dev_driver));
	if (!mount->fs_driver->dev_driver)
		return -ENOMEM;
	mount->name = (char *) kalloc(MAX_MOUNTNAME_SIZE);
	strcpy(mount->name, filename);

    /* Initialize the storage driver functions */
    switch (dev_type) {
    case STORAGE_DEVICE_FLOPPY:
        floppy_init_driver(mount->fs_driver->dev_driver);
		fat12_init_fs(mount->fs_driver);
        break;
    case STORAGE_DEVICE_HDD:
        printf("HDD storage devices are not implemented yet");
        break;
    default:
        return -EBADARG;
    }

    /* Add the mount point the the VFS directory pool */
    if (_vfs.mount_pts)
        llist_add_before(_vfs.mount_pts, mount, ll);
    else
	{
		llist_init(mount, ll);
        _vfs.mount_pts = mount;
	}
	_vfs.dir_count++;

    return 0;
}

/*
 * Unmounts a device which was mounted as a given filename.
 */
int unmount(char *filename)
{
    return 0;
}

/*
 * Opens a file and return FILE handle of it.
 */
FILE *open(char *filename)
{
    if (!filename)
        return NULL;

    return NULL;
}

/*
 * Free the memory used by FILE handle.
 */
void close(FILE *hndl)
{

}

/*
 * Seek to a specific place in the file.
 * If the result will end outside the scope of file,
 * the operation will be discarded and error value returned.
 * On success 0 is returned.
 */
int seek(enum seek_type type, size_t offset)
{
	return -1;
}

int read(FILE *hndl, void *buf, size_t nbytes)
{
	return -1;
}

char **get_mounts()
{
	int idx;
	struct mount_point *mount_point;
	char **file_list = (char **) kalloc(sizeof(char *) * 128);
	file_list[0] = '\0';

	if (_vfs.dir_count == 0)
		return file_list;

	llist_foreach(_vfs.mount_pts, mount_point, idx, ll)
		file_list[idx] = mount_point->name;
	file_list[idx+1] = '\0';

	return file_list;
}

char *get_file_list(const char *dir)
{
	/* Check if root director is requested */
	if (!dir || strcmp(dir, "/") == 0)
		return get_mounts();

	return NULL;
}
