/******************************************************************************
 *		Kernel panic routines.
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <x86/cpu.h>

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
