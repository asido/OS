/******************************************************************************
 *	Kernel entry file
 *
 *		Author: Arvydas Sidorenko
 *****************************************************************************/

#include <libc.h>
#include <x86/cpu.h>
#include <x86/i8259.h>

extern int pmm_init(unsigned int mem_kb);
extern int pmm_init_region(unsigned int addr, size_t size);
extern unsigned int pmm_alloc();
extern int pmm_dealloc(unsigned int addr);

static char* logo =
"\
    ___              ____   ____\n\
   /   |_  _  __    / __ \\/ ___/\n\
  / /| |\\\\//||--\\\\ / / / /\\__ \\\n\
 / /_| | \\\\ |||_||/ /_/ /___/ /\n\
/_/  |_|//\\\\||__//\\____/\\____/\n";

struct boot_info {
	unsigned int mem_size;
	unsigned int krnl_size;
	unsigned int krnl_loc;
} __attribute__((__packed__));

static int screen_init()
{
	set_color(VID_CLR_LIGHT_BLUE, VID_CLR_WHITE);
	clear_screen();
	goto_xy(0, 0);
	puts(logo);
	return 0;
}

/* Kernel entry point */
int kmain(struct boot_info binfo)
{
	/* set segment values */
	__asm__ __volatile__("movw $0x10, %%ax \n"
						 "movw %%ax, %%ds \n"
						 "movw %%ax, %%es \n"
						 "movw %%ax, %%fs \n"
						 "movw %%ax, %%gs \n"
						 /* stack at the top of the kernel's PTE */
						 "movl $0xC0400000, %%eax \n"
						 "movl %%eax, %%esp \n"
						: : : "eax");

	screen_init();
	x86_init();

	pmm_init(binfo.mem_size);
	/* bootloader mappings:  0-1MB - BIOS , 1-5MB - Kernel */
	int mem_avail_begin = MB_TO_BYTE(5);
	int mem_avail_end = KB_TO_BYTE(binfo.mem_size);
	pmm_init_region(mem_avail_begin, mem_avail_end - mem_avail_begin);
	unsigned int alloc0 = pmm_alloc(15000);
	unsigned int alloc1 = pmm_alloc(4096);
	alloc1 = pmm_alloc(4097);
	unsigned int alloc2 = pmm_alloc(0);

	/* int c = 5 / 0; */

	goto_xy(10,10);
	printf("Memory size: %dKb\n", binfo.mem_size);
	goto_xy(10,11);
	printf("Kernel size: %dKb\n", binfo.krnl_size);
	goto_xy(10,12);
	printf("Kernel loc: 0x%x\n", binfo.krnl_loc);
	goto_xy(10,13);
	printf("Allocated: %d\n", alloc0);
	goto_xy(10,14);
	printf("Allocated: %d\n", alloc1);
	goto_xy(10,15);
	printf("Allocated: %d\n", alloc2);

	for (;;);
	return 0;
}
