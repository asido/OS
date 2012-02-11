/******************************************************************************
 *		Intel 8253 PIT controller - Programmable Interval Timer
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include "i8253.h"
#include "i8259.h"

/* TODO: PIT driver */

/* PIT jiffy */
unsigned long long pit_jiffy = 0;

/*
 * PIT interrupt handler
 */
void x86_i8253_irq_do_handle()
{
	goto_xy(0, 0);
	pit_jiffy++;
	printf("%d", pit_jiffy);

	irq_done(IRQ0_VECTOR);
}
