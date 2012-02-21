/******************************************************************************
 * IDT (Interrupt Descriptor Table)
 *
 * ----------------------------------------------------------------------------
 * IDT manual:
 * ----------------------------------------------------------------------------
 * Bits 0...15:
 *      Interrupt / Trap Gate: Offset address Bits 0-15 of IR
 *      Task Gate: not used.
 * Bits 16...31:
 *      Interrupt / Trap Gate: Segment Selector (Useually 0x10)
 *      Task Gate: TSS Selector
 * Bits 31...35: Not used
 * Bits 36...38:
 *      Interrupt / Trap Gate: Reserved. Must be 0.
 *      Task Gate: Not used.
 * Bits 39...41:
 *      Interrupt Gate: Of the format 0D110, where D determins size
 *          01110 - 32 bit descriptor
 *          00110 - 16 bit descriptor
 *      Task Gate: Must be 00101
 *      Trap Gate: Of the format 0D111, where D determins size
 *          01111 - 32 bit descriptor
 *          00111 - 16 bit descriptor
 * Bits 42...44: Descriptor Privedlge Level (DPL)
 *      00: Ring 0
 *      01: Ring 1
 *      10: Ring 2
 *      11: Ring 3
 * Bit 45: Segment is present (1: Present, 0:Not present)
 * Bits 46...62:
 *      Interrupt / Trap Gate: Bits 16...31 of IR address
 *      Task Gate: Not used
 * ----------------------------------------------------------------------------
 *
 *          Author: Arvydas Sidorenko (not the table above ^^)
 *****************************************************************************/

#include <libc.h>
#include "idt.h"


struct idt_t {
    unsigned short offset_low;
    unsigned short segment_selector;
    unsigned char reserved;
    unsigned char attr;
    unsigned short offset_high;
} __attribute__((__packed__));

/* This will go into LIDT instruction */
struct idt_ptr {
    unsigned short limit;
    unsigned int addr;
} __attribute__((__packed__));

static struct idt_t _idt[IDT_MAX_INTERRUPTS];

static struct idt_ptr _idt_ptr = {
    .limit = (sizeof(struct idt_t) * IDT_MAX_INTERRUPTS) - 1,
    .addr = (int) _idt
};


/*
 * Registers interrupt handler.
 */
int reg_irq(int irq_line, irq_handler hndl)
{
    if (irq_line < 0 || irq_line > IDT_MAX_INTERRUPTS)
        return -1;
    
    _idt[irq_line].offset_low = ((int) hndl) & 0xFFFF;
    _idt[irq_line].offset_high = (((int) hndl) >> 16) & 0xFFFF;
    _idt[irq_line].segment_selector = 0x8;
    _idt[irq_line].reserved = 0;
    _idt[irq_line].attr = IDT_ATTR_32BIT | IDT_ATTR_DPL_RING_0 | IDT_ATTR_SEGMENT_PRESENT;
    
    return 0;
}

/*
 * Installs IDT.
 */
int install_idt()
{
    __asm__ __volatile__("lidtl %0" : : "m" (_idt_ptr));
    return 0;
}
