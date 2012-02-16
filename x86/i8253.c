/******************************************************************************
 *		Intel 8253 PIT controller - Programmable Interval Timer
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include "cpu.h"
#include "i8253.h"
#include "i8259.h"


/* interrupt jiffy */
unsigned long long pit_jiffy = 0;

/*
 * PIT IRQ0 interrupt handler
 */
void x86_i8253_irq_do_handle()
{
	cursor_save();
	goto_xy(0, 0);
	pit_jiffy++;
	printf("%d", pit_jiffy);
	cursor_load();

	irq_done(IRQ0_VECTOR);
}

static inline void i8253_set_frequency()
{
	outportb(PIT_PORT_PIT_0, PIT_FREQ & 0xFF);
	outportb(PIT_PORT_PIT_0, (PIT_FREQ >> 8) & 0xFF);
}

int i8253_init()
{
	outportb(PIT_PORT_MODE, PIT_CTRL_BCD_DEC |
	                        PIT_CTRL_MODE_SQR_WAVE |
	                        PIT_CTRL_RL_LEAST_MOST_SIG |
	                        PIT_CTRL_SELECT_0);
	i8253_set_frequency();
	return 0;
}
