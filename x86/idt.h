/******************************************************************************
 * IDT (Interrupt Descriptor Table)
 *
 *      Author: Arvydas Sidorenko
 *****************************************************************************/

#ifndef IDT_DCKW0PDL
#define IDT_DCKW0PDL


#define IDT_MAX_INTERRUPTS 256

#define IDT_ATTR_16BIT  0x6 /* 00000110 */
#define IDT_ATTR_32BIT  0xE /* 00001110 */

#define IDT_ATTR_DPL_RING_0  0
#define IDT_ATTR_DPL_RING_1  (1 << 5) /* 00100000 */
#define IDT_ATTR_DPL_RING_2  (1 << 6) /* 01000000 */
#define IDT_ATTR_DPL_RING_3  (3 << 5) /* 01100000 */

#define IDT_ATTR_SEGMENT_PRESENT    0x80 /* 10000000 */

typedef void (*irq_handler)(void);

int reg_irq(int irq_line, irq_handler);
int install_idt();


#endif /* end of include guard: IDT_DCKW0PDL */
