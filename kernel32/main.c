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
} __attribute((__packed__))__;

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
	/* clear interrupts until we establish the handlers */
	__asm__ __volatile__("cli": : :"memory");
	/* set segment values */
	__asm__ __volatile__("movw $0x10, %%ax \n"
						 "movw %%ax, %%ds \n"
						 "movw %%ax, %%es \n"
						 "movw %%ax, %%fs \n"
						 "movw %%ax, %%gs \n"
						: : : "ax");

	init_screen();
	init_hal(&binfo);

	goto_xy(10,10);
	printf("Memory size: %dKb\n", binfo.mem_size);
	goto_xy(10,11);
	printf("Kernel size: %dKb\n", binfo.krnl_size);

	for (;;);
	return 0;
}
