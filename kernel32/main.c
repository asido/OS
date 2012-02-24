/******************************************************************************
 *  Kernel entry file
 *
 *      Author: Arvydas Sidorenko
 *****************************************************************************/

#include <libc.h>
#include <x86/cpu.h>
#include <x86/i8259.h>
#include <x86/cmos.h>
#include "mm.h"
#include "time.h"
#include "shell.h"
#include "linklist.h"

extern inline void kernel_panic(char *msg);

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

    if (screen_init())
        kernel_panic("screen init error");
    if (x86_init())
        kernel_panic("x86 init error");

    /* init PMM */
    addr_t pmm_tbl_loc = binfo.krnl_loc + KB_TO_BYTE(binfo.krnl_size);
    addr_t pmm_end = pmm_init(binfo.mem_size, pmm_tbl_loc);
    int mem_avail_begin = MB_TO_BYTE(5); /* first 5MB reserved in boot loader */
    int mem_avail_end = KB_TO_BYTE(binfo.mem_size);
    pmm_init_region(mem_avail_begin, mem_avail_end - mem_avail_begin);

    /* init VMM */
    if (vmm_init(binfo.mem_size, pmm_end))
        kernel_panic("VMM init error");

    if (cmos_init())
        kernel_panic("CMOS init error");

    if (shell_init(" $ "))
        kernel_panic("Shell init error");

    goto_xy(10,10);
    printf("Memory size: %dKb\n", binfo.mem_size);
    goto_xy(10,11);
    printf("Kernel size: %dKb\n", binfo.krnl_size);
    goto_xy(10,12);
    printf("Kernel loc: 0x%x\n", binfo.krnl_loc);

    struct test_t {
        struct llist_t ll;
        int a;
        int b;
        int c;
    };

    struct test_t test1;
    struct test_t test2;
    struct test_t test3;
    struct test_t test4;
    llist_init(&test1, ll);   
    llist_add_before(&test1, &test2, ll);
    llist_add_before(&test1, &test3, ll);
    llist_add_before(&test1, &test4, ll);

    for (;;)
    {
    }

    return 0;
}
