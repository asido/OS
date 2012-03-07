/******************************************************************************
 *       Error routines.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <time.h>
#include <x86/cpu.h>

int error = 0;

/*
 * Internal kernel error handler.
 * NOTE: CPU is halted after it's call.
 */
inline void kernel_panic(char *msg)
{
    clear_screen();
    goto_xy(0, 0);

    puts("**** FATAL KERNEL PANIC ****");
    if (msg)
        printf("\nReason: %s\n", msg);
    puts("");

    x86_dump_registers();
    puts("\nCPU IS HALTED\n");
    x86_cpu_halt();
}

/*
 * Warning or note to the user which goes away after
 * specific period of time.
 */
void kernel_warning(char *msg)
{
    printf("*** WARNING: %s", msg);
    msdelay(2000); /* 2s delay before continueing */
}

void _kernel_debug(const char *file, unsigned int line, char *msg)
{
    printf("[FILE] %s | [LINE] %d : %s\n", file, line, msg);
    msdelay(2000); /* 2s delay before continueing */
}
