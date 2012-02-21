/******************************************************************************
 *  Intel 8259A PIC - Programmable Interrupt Controller
 *
 *      Author: Arvydas Sidorenko
 *****************************************************************************/

#ifndef I8259_5UIR4IFN
#define I8259_5UIR4IFN

/* IRQ mappings */  /* in 8086 must be 4 bits aligned */
#define IRQ0_VECTOR     (0x20 & ~7) /* 0x0-0x1F is reserved by Intel */
#define IRQ1_VECTOR     (IRQ0_VECTOR + 1)
#define IRQ2_VECTOR     (IRQ0_VECTOR + 2)
#define IRQ3_VECTOR     (IRQ0_VECTOR + 3)
#define IRQ4_VECTOR     (IRQ0_VECTOR + 4)
#define IRQ5_VECTOR     (IRQ0_VECTOR + 5)
#define IRQ6_VECTOR     (IRQ0_VECTOR + 6)
#define IRQ7_VECTOR     (IRQ0_VECTOR + 7)
#define IRQ8_VECTOR     (IRQ0_VECTOR + 8)
#define IRQ9_VECTOR     (IRQ0_VECTOR + 9)
#define IRQ10_VECTOR    (IRQ0_VECTOR + 10)
#define IRQ11_VECTOR    (IRQ0_VECTOR + 11)
#define IRQ12_VECTOR    (IRQ0_VECTOR + 12)
#define IRQ13_VECTOR    (IRQ0_VECTOR + 13)
#define IRQ14_VECTOR    (IRQ0_VECTOR + 14)
#define IRQ15_VECTOR    (IRQ0_VECTOR + 15)

#define IS_PIC1_LINE(irq_line)  \
        ((irq_line >= IRQ0_VECTOR) && (irq_line <= IRQ7_VECTOR))
#define IS_PIC2_LINE(irq_line)  \
        ((irq_line >= IRQ8_VECTOR) && (irq_line <= IRQ15_VECTOR))
#define IS_PIC_LINE(irq_line)   \
        (IS_PIC1_LINE(irq_line) || IS_PIC2_LINE(irq_line))

int i8259_init();
int irq_done(int irq);
inline int irq_disable();
inline int irq_enable();

#endif /* end of include guard: I8259_5UIR4IFN */
