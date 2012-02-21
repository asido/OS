/******************************************************************************
 *  Intel 8259A PIC - Programmable Interrupt Controller
 *
 *      Author: Arvydas Sidorenko
 *****************************************************************************/
/* TODO: probably will need some PIC masking or something */

#include "cpu.h"
#include "i8259.h"
#include "idt.h"

/* PIC registers */
#define i8259_MASTER_CMD_PORT 0x20
#define i8259_MASTER_DATA_PORT 0x21
#define i8259_SLAVE_CMD_PORT 0xA0
#define i8259_SLAVE_DATA_PORT 0xA1

/* PIC data values */
#define i8259_INIT_DATA 0x11
#define i8259_EOI_DATA 0x20
/* Cascade data */
#define i8259_CASCADE_IR 0x2
#define i8259_MASTER_CASCADE_IR_DATA    (0x1 << 0x2)
#define i8259_SLAVE_CASCADE_IR_DATA     i8259_CASCADE_IR
/* ICW4 */
#define i8259_ICW4_8086_MODE 0x1
#define i8259_ICW4_AUTO_EOI 0x2
#define i8259_ICW4_BUF_SLAVE_MODE 0x8
#define i8259_ICW4_BUF_MASTER_MODE 0xC
#define i8259_ICW4_NESTED_MODE 0x10


/*
 * Initializes the controller to be able to interrupt the CPU.
 */
int i8259_init()
{
    /* ICW1: put controllers to init state */
    outportb(i8259_MASTER_CMD_PORT, i8259_INIT_DATA);
    outportb(i8259_SLAVE_CMD_PORT, i8259_INIT_DATA);

    /* ICW2: map the IRQs */
    outportb(i8259_MASTER_DATA_PORT, IRQ0_VECTOR);
    outportb(i8259_SLAVE_DATA_PORT, IRQ8_VECTOR);

    /* ICW3: register cascading - slave i8259 */
    outportb(i8259_MASTER_DATA_PORT, i8259_MASTER_CASCADE_IR_DATA);
    outportb(i8259_SLAVE_DATA_PORT, i8259_SLAVE_CASCADE_IR_DATA);
    
    /* ICW4: set in 8086 mode */
    outportb(i8259_MASTER_DATA_PORT, i8259_ICW4_8086_MODE);
    outportb(i8259_SLAVE_DATA_PORT, i8259_ICW4_8086_MODE);

    return 0;
}

/*
 * Informs the PIC that interrupt is done
 * and it can continue accepting the new ones.
 */
inline int irq_done(int irq_line)
{
    if (!IS_PIC_LINE(irq_line))
        return -1;

    /* master PIC needs to be informed in all cases */
    outportb(i8259_MASTER_CMD_PORT, i8259_EOI_DATA);
    /* but for slave, only if irq line belongs to it */
    if (IS_PIC2_LINE(irq_line))
        outportb(i8259_SLAVE_CMD_PORT, i8259_EOI_DATA);

    return 0;
}

/*
 * Enables interrupts
 */
inline int irq_enable()
{
    __asm__ __volatile__("sti": : :"memory");

    return 0;
}

/*
 * Disables interrupts
 */
inline int irq_disable()
{
    __asm__ __volatile__("cli": : :"memory");

    return 0;
}
