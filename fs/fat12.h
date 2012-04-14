/******************************************************************************
 *		FAT12 filesystem driver
 *****************************************************************************/

#ifndef FAT12_WNVMY8O8
#define FAT12_WNVMY8O8

#include "vfs.h"

FILE *fs_open(FILE *hndl);
void fs_close(FILE *hndl);

struct fs_driver *fat12_init_fs(struct fs_driver *driver);

#endif /* end of include guard: FAT12_WNVMY8O8 */
