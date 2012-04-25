/******************************************************************************
 *  Kernel entry file
 *
 *      Author: Arvydas Sidorenko
 *****************************************************************************/

#include <libc.h>
#include <x86/cpu.h>
#include <x86/i8259.h>
#include <x86/cmos.h>
#include <fs/vfs.h>
#include "mm.h"
#include "time.h"
#include "shell.h"
#include "linklist.h"

static int screen_init()
{
    set_color(VID_CLR_BLACK, VID_CLR_LIGHT_BLUE);
    clear_screen();
    goto_xy(30, 12);
	printf("Loading AxidOS...");

    return 0;
}

static void os_loop()
{
    while (1)
    {
        __asm__ __volatile__ ("sti\n"
                              "hlt\n"
                            : : : "memory");
    }
}

struct boot_info *binfo;

/* Kernel entry point */
int kmain(struct boot_info bi)
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

    binfo = &bi;

    /* Architecture specific initialization */
    if (screen_init())
        kernel_panic("screen init error");
    if (x86_init())
        kernel_panic("x86 init error");

    /* init PMM */
    addr_t pmm_tbl_loc = binfo->krnl_loc + KB_TO_BYTE(binfo->krnl_size);
    addr_t pmm_end = pmm_init(binfo->mem_size, pmm_tbl_loc);
    /* first 5MB reserved in boot loader */
    int mem_avail_begin = MB_TO_BYTE(5);
    int mem_avail_end = KB_TO_BYTE(binfo->mem_size);
    pmm_init_region(mem_avail_begin, mem_avail_end - mem_avail_begin);
    set_krnl_size(binfo->krnl_size * 512);

    /* init VMM */
    if (vmm_init(binfo->mem_size, pmm_end))
        kernel_panic("VMM init error");

    /* Driver initialization */
    if (cmos_init())
        kernel_panic("CMOS init error");
    if (floppy_init())
        kernel_warning("Floppy initialization failure.");
    if (kbrd_init())
        kernel_warning("Keyboard initialization failure.");

	if (mount(STORAGE_DEVICE_FLOPPY, "floppy"))
		kernel_warning("Floppy mount failure");

    clock_init();
    if (shell_init("[axidos]$ "))
        kernel_panic("Shell init error");

    os_loop();

    return 0;
}
