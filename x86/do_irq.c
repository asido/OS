/******************************************************************************
 *		Interrupt handlers
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include "i8253.h"
#include "i8259.h"
#include "irq.h"


/*
 * PIT handler
 * TODO: dummy handler for the moment
 */
void x86_i8253_irq_do_handle()
{
	goto_xy(0, 0);
	pit_jiffy++;
	printf("%d", pit_jiffy);

	irq_done(IRQ0_VECTOR);
}
