/******************************************************************************
 *	Kernel entry file
 *
 *		Author: Arvydas Sidorenko
 *****************************************************************************/

#include <libc.h>

static char* logo =
"\
    ___              ____   ____\n\
   /   |_  _  __    / __ \\/ ___/\n\
  / /| |\\\\//||--\\\\ / / / /\\__ \\\n\
 / /_| | \\\\ |||_||/ /_/ /___/ /\n\
/_/  |_|//\\\\||__//\\____/\\____/\n";

struct boot_info {
	int mem_size;
	int krnl_size;
};

static int init_screen()
{
	set_color(VID_CLR_LIGHT_BLUE, VID_CLR_WHITE);
	clear_screen();
	goto_xy(0, 0);
	puts(logo);
	return 0;
}

static int init_hal(struct boot_info *binfo)
{
	return binfo->mem_size;
}

/* Kernel entry point */
int kmain(struct boot_info binfo)
{
	/* asm(".intel_syntax noprefix\n"); */
	/* asm("cli\n"); */
	/* asm("mov eax, 0x10\n");	 offset to 0x10 in GDT - data selector  */
	/* asm("mov ds, ax\n"); */
	/* asm("mov es, ax\n"); */
	/* asm("mov fs, ax\n"); */
	/* asm("mov gs, ax\n"); */

	init_screen();
	init_hal(&binfo);

	goto_xy(10,10);
	printf("memsize: %d\n", binfo.mem_size);

	for (;;);
	return 0;
}
