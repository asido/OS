/******************************************************************************
 *		FAT12 filesystem driver
 *****************************************************************************/

#include <linklist.h>
#include <error.h>
#include "fat12.h"
#include "vfs.h"

#define ROOT_DIR_OFFSET 0x2600
#define ROOT_DIR_SIZE 14 * 512 /* 14 sectors */
#define MAX_FILES_IN_DIR 224
#define FAT12_MAX_FILENAME_LENGTH 11

struct fat12_bootsector 
{
	/* The bootsector holds quite a lot of info, but we will have just the parts
	 * which interest us when working with the driver itself */

	size_t fat_cnt;
	size_t max_root_dir_cnt;
	size_t max_sector_cnt;
	size_t sectors_in_fat;
};

/* On every mount a strcture is created */
struct fat12_mount
{
	struct llist_t ll;
	/* FS driver is just to have some compare value for get_mount_point() */
	struct fs_driver *fs_drv;
	struct fat12_bootsector bootsec;
	/* Multi-dimensional array of all FAT data. FAT count is in `bootsec` */
	char **fat_data;
	/* Root directory data */
	char rootdir_data[ROOT_DIR_SIZE];
	size_t open_file_cnt;
};

struct rootdir_item
{
	char filename[11];
	unsigned char flags;
	short reserved;
	unsigned short create_time;
	unsigned short create_date;
	unsigned short last_access_date;
	unsigned short ignore_in_fat12;
	unsigned short last_write_time;
	unsigned short last_write_date;
	unsigned short fat_idx;
	size_t filesize;
};

struct fat12_mount *fat12_mounts;

static struct fat12_mount *get_mount_point(struct fs_driver *drv)
{
	struct fat12_mount *mount;
	size_t idx;

	llist_foreach(fat12_mounts, mount, idx, ll)
	{
		if (mount->fs_drv == drv)
			return mount;
	}

	return NULL;
}

static int init_bootsec(struct dev_driver *dev, struct fat12_bootsector *bootsec)
{
	char data[512];

	dev->read(data, 0, 512);
	if (!data)
		return -1;

	bootsec->fat_cnt = data[16];
	bootsec->max_root_dir_cnt = ((data[18] << 8) & 0xFF00) | (data[17] & 0xFF);
	bootsec->max_sector_cnt = ((data[20] << 8) & 0xFF00) | (data[19] & 0xFF);
	bootsec->sectors_in_fat = ((data[23] << 8) & 0xFF00) | (data[22] & 0xFF);

	return 0;
}

static char **init_fats(struct dev_driver *dev, struct fat12_bootsector *bootsec)
{
	int i;
	char **fat_buf;

	if (!dev || !bootsec)
	{
		error = -EBADARG;
		return NULL;
	}

	if (bootsec->fat_cnt == 0)
		return NULL;
	
	fat_buf = (char **) kalloc(sizeof(char **) * bootsec->fat_cnt);

	for (i = 0; i < bootsec->fat_cnt; i++)
	{
		fat_buf[i] = (char *) kalloc(bootsec->sectors_in_fat * 512);
		if (!fat_buf[i])
		{
			for (i--; i >= 0; i--)
				free(fat_buf[i]);
			error = -ENOMEM;
			return NULL;
		}

		dev->read(fat_buf[i], 1 + (i * bootsec->sectors_in_fat), 512 * bootsec->sectors_in_fat);
	}

	return fat_buf;
}

static char *init_rootdir(struct dev_driver *dev, struct fat12_mount *mount)
{
	if (!dev || !mount)
	{
		error = -EBADARG;
		return NULL;
	}
	
	dev->read(mount->rootdir_data, ROOT_DIR_OFFSET, ROOT_DIR_SIZE);

	return mount->rootdir_data;
}


static char *fs_read(FILE *file)
{
	return NULL;
}

static struct fileinfo *rootdir_to_fileinfo(struct rootdir_item *rootdir_itm, struct fileinfo *inf)
{
	size_t i, cur_idx;

	if (!rootdir_itm || !inf)
	{
		error = -EBADARG;
		return NULL;
	}

	/* Set filename */
	for (i = 0; rootdir_itm->filename[i] != ' '; i++)
		inf->filename[i] = rootdir_itm->filename[i];
	cur_idx = i;
	inf->filename[cur_idx++] = '.';
	while (rootdir_itm->filename[i] == ' ' && i < FAT12_MAX_FILENAME_LENGTH)
		i++;
	for (; i < FAT12_MAX_FILENAME_LENGTH && rootdir_itm->filename[i] != ' '; i++, cur_idx++)
		inf->filename[cur_idx] = rootdir_itm->filename[i];
	if (inf->filename[cur_idx-1] == '.')
		inf->filename[cur_idx-1] = '\0';
	else
		inf->filename[cur_idx] = '\0';

	/* Set file size */
	inf->size = rootdir_itm->filesize;

	/* Set file flags */
	inf->flags = rootdir_itm->flags;

	return inf;
}

static struct fileinfo **fs_ls(struct fs_driver *fs_drv, const char *dir)
{
	struct fileinfo **files;
	struct fat12_mount *mount;
	struct rootdir_item *rootdir_itm;
	size_t file_cnt, i, j;

	if (!fs_drv || !dir)
		return NULL;

	mount = get_mount_point(fs_drv);
	if (!mount)
	{
		printf("FAT12 ERROR: given fat12 driver is not mounted");
		error = -EFAULT;
		return NULL;
	}

	/* TODO: hierarchical `ls` */

	/* To get the files in filesystem we just need to loop through the
	 * root directory table */

	/* Get file count in dir */
	for (file_cnt = 0; mount->rootdir_data[file_cnt * sizeof(struct rootdir_item)]; file_cnt++)
		;
	
	
	if (file_cnt == 0)
		return NULL;

	files = (struct fileinfo **) kalloc(sizeof(struct fileinfo *) * (file_cnt + 1));
	if (!files)
	{
		error = -ENOMEM;
		return NULL;
	}
	for (i = 0; i < file_cnt; i++)
	{
		rootdir_itm = &(((struct rootdir_item *) mount->rootdir_data)[i]);
		if (!(rootdir_itm->flags & HIDDEN))
		{
            files[i] = alloc_fileinfo();
			if (!files[i])
			{
				error = -ENOMEM;
				return NULL;
			}

			if (!rootdir_to_fileinfo(rootdir_itm, files[i]))
			{
				printf("FAT12 ERROR: failure filling fileinfo from rootdir");
				return NULL;
			}
		}
	}
	files[i+1] = NULL;

	return files;
}

struct fs_driver *fat12_init_fs(struct fs_driver *driver)
{
	struct fat12_mount *mount = (struct fat12_mount *) kalloc(sizeof(struct fat12_mount));
	if (!mount)
	{
		error = -ENOMEM;
		return NULL;
	}
	mount->open_file_cnt = 0;
	mount->fs_drv = driver;

	if (!driver)
	{
		error = -EBADARG;
		goto fail_return;
	}
	if (!driver->dev_driver)
	{
		printf("WARNING: FAT12 receivd uninitialised device driver");
		error = -EBADARG;
		goto fail_return;
	}

	if (init_bootsec(driver->dev_driver, &mount->bootsec))
	{
		printf("Error initialising FAT12 bootsector");
		goto fail_return;
	}

	if (init_fats(driver->dev_driver, mount))
	{
		printf("Error initialising FAT data");
		goto fail_return;
	}

	if (!init_rootdir(driver->dev_driver, mount))
	{
		printf("Error initialising FAT12 root directory");
		goto fail_return;
	}

	if (fat12_mounts)
		llist_add_before(fat12_mounts, mount, ll);
	else
	{
		llist_init(mount, ll);
		fat12_mounts = mount;
	}

	driver->read = fs_read;
	driver->write = NULL; /* TODO */
	driver->ls = fs_ls;

	return driver;

fail_return:
	free(mount);
	return NULL;
}
