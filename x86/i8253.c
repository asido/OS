/******************************************************************************
 *      Intel 8253 PIT controller - Programmable Interval Timer
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <time.h>
#include <callback.h>
#include "cpu.h"
#include "i8253.h"
#include "i8259.h"


/* interrupt jiffy */
unsigned long long pit_jiffy = 0;

/*
 * 64 bit mod operation.
 * The OS runs in 32 bit mode and pit jiffy is 64 bits in size,
 * so the hardware can't do that natively.
 */
static unsigned int pit_mod(unsigned long long jiffy, unsigned int mod_val)
{
    int val = 0;

    __asm__ ("movl %1, %%eax\n"
             "xor %%edx, %%edx\n"
             "div %2\n"
             "mov %%dx, %0\n"
         : "=m" (val)
         : "m" (jiffy), "r" (mod_val)
         : "eax", "edx");

    return val;
}

/*
 * PIT IRQ0 interrupt handler
 */
void x86_i8253_irq_do_handle()
{
    pit_jiffy++;
    update_clock_pit(PIT_HZ);
    check_callbacks();

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
