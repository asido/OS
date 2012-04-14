/******************************************************************************
 *		FAT12 filesystem driver
 *****************************************************************************/

#include "fat12.h"

static char *fs_read(FILE *file)
{
	return NULL;
}

struct fs_driver *fat12_init_fs(struct fs_driver *driver)
{
	if (!driver)
		return driver;

	driver->read = fs_read;
	driver->write = NULL; /* TODO */

	return driver;
}
