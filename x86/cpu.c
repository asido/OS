/******************************************************************************
 *      x86 CPU related stuff.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include "i8259.h"
#include "i8253.h"
#include "idt.h"
#include "cpu.h"
#include "dma.h"

/* CPU exception handlers defined in irq.asm */
extern void x86_divide_handle();
extern void x86_single_step_debug_handle();
extern void x86_nonmask_handle();
extern void x86_breakpoint_handle();
extern void x86_overflow_handle();
extern void x86_bound_handle();
extern void x86_invalid_opcode_handle();
extern void x86_busy_coproc_handle();
extern void x86_double_fault_handle();
extern void x86_coproc_overrun_handle();
extern void x86_invalid_tss_handle();
extern void x86_no_segment_handle();
extern void x86_stack_handle();
extern void x86_gpf_handle();
extern void x86_page_fault_handle();
extern void x86_coproc_handle();

/* PIC interrupt handlers */
extern void x86_i8253_irq_handle();
extern void x86_kbr_irq_handle();
extern void x86_floppy_irq_handle();

extern int kbrd_init();

static int _dump_registers(struct x86_reg_t *regs);
static inline int x86_get_seg_regs(struct x86_seg_reg_t *buf);
static inline int x86_get_registers(struct x86_reg_t *buf);
static inline int x86_get_gp_regs(struct x86_reg_t *buf);
static inline int x86_get_seg_regs(struct x86_seg_reg_t *buf);

static const struct x86_seg_reg_t  null_seg_regs = {
    .cs = 0,
    .ds = 0,
    .ss = 0,
    .es = 0,
    .fs = 0,
    .gs = 0
};

static const struct x86_reg_t null_regs = {
    .eax = 0,
    .ebx = 0,
    .ecx = 0,
    .edx = 0,
    .edi = 0,
    .esi = 0,
    .ebp = 0,
    .esp = 0,
    .eip = 0,
    .seg_reg = NULL//&null_seg_regs
};


static inline int x86_get_registers(struct x86_reg_t *buf)
{
    /* TODO: BROKEN! Need all this done in pure ASM */
    x86_get_gp_regs(buf);
    x86_get_seg_regs(buf->seg_reg);

    return 0;
}

static inline int x86_get_gp_regs(struct x86_reg_t *buf)
{
    /* TODO: BROKEN! Need all this done in pure ASM */
    __asm__ __volatile__("movl %%eax, %0\n"
                         "movl %%ebx, %1\n"
                         "movl %%ecx, %2\n"
                         "movl %%edx, %3\n"
                         "movl %%edi, %4\n"
                         "movl %%esi, %5\n"
            : "=m" (buf->eax), "=m" (buf->ebx), "=m" (buf->ecx), "=m" (buf->edx), "=m" (buf->edi), "=m" (buf->esi)
            :
            : "memory");

    return 0;
}

static inline int x86_get_seg_regs(struct x86_seg_reg_t *buf)
{
    /* TODO: BROKEN! Need all this done in pure ASM */
    __asm__ __volatile__("movw %%ax, %0\n"
                         "movw %%ds, %1\n"
                         "movw %%es, %2\n"
                         "movw %%fs, %3\n"
                         "movw %%gs, %4\n"
                         "movw %%ss, %5\n"
            : "=m" (buf->cs), "=m" (buf->ds), "=m" (buf->es), "=m" (buf->fs), "=m" (buf->gs), "=m" (buf->ss)
            :
            : "memory");

    return 0;
}

/*
 * Dumps register values.
 */
inline int x86_dump_registers()
{
    struct x86_reg_t regs = null_regs;
    /* TODO: kinda lousy and pointless implementation since the actual
     *       dumping will trash the registers. Will figure something out later. */
    x86_get_registers(&regs);
    _dump_registers(&regs);

    return 0;
}

static int _dump_registers(struct x86_reg_t *regs)
{
    /* TODO: */
    puts("WARNING: REGISTER VALUES ARE FALSE (TODO)");
    printf("EAX = 0x%x \t SS = 0x%x\n"
           "EBX = 0x%x \t ES = 0x%x\n"
           "ECX = 0x%x \t FS = 0x%x\n"
           "EDX = 0x%x \t GS = 0x%x\n"
           "ESI = 0x%x \t CS = 0x%x\n"
           "EDI = 0x%x \t DS = 0x%x\n"
           "EBP = 0x%x \n"
           "ESP = 0x%x \n"
           "EIP = 0x%x \n",
        regs->eax, (int) regs->seg_reg->ss,
        regs->ebx, (int) regs->seg_reg->es,
        regs->ecx, (int) regs->seg_reg->fs,
        regs->edx, (int) regs->seg_reg->gs,
        regs->esi, (int) regs->seg_reg->cs,
        regs->edi, (int) regs->seg_reg->ds,
        regs->ebp,
        regs->esp,
        regs->eip);

    return 0;
}

unsigned char inportb(unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

/*
 * Hals the CPU.
 */
inline void x86_cpu_halt()
{
    irq_disable();
    __asm__ __volatile__("cli\n"
                         "hlt\n"
                    : : : "memory");
}

/*
 * Initializes CPU 0-32 handlers
 */
static int reg_cpu_handlers()
{
    if (reg_irq(X86_DIVIDE_IRQ, x86_divide_handle))
        return -1;
    if (reg_irq(X86_SINGLE_STEP_DEBUG_IRQ, x86_single_step_debug_handle))
        return -1;
    if (reg_irq(X86_NONMASK_IRQ, x86_nonmask_handle))
        return -1;
    if (reg_irq(X86_BREAKPOINT_IRQ, x86_breakpoint_handle))
        return -1;
    if (reg_irq(X86_OVERFLOW_IRQ, x86_overflow_handle))
        return -1;
    if (reg_irq(X86_BOUND_IRQ, x86_bound_handle))
        return -1;
    if (reg_irq(X86_INVALID_OPCODE_IRQ, x86_invalid_opcode_handle))
        return -1;
    if (reg_irq(X86_BUSY_COPROC_IRQ, x86_busy_coproc_handle))
        return -1;
    if (reg_irq(X86_DOUBLE_FAULT_IRQ, x86_double_fault_handle))
        return -1;
    if (reg_irq(X86_COPROC_OVERRUN_IRQ, x86_coproc_overrun_handle))
        return -1;
    if (reg_irq(X86_INVALID_TSS_IRQ, x86_invalid_tss_handle))
        return -1;
    if (reg_irq(X86_NO_SEGMENT_IRQ, x86_no_segment_handle))
        return -1;
    if (reg_irq(X86_STACK_IRQ, x86_stack_handle))
        return -1;
    if (reg_irq(X86_GPF_IRQ, x86_gpf_handle))
        return -1;
    if (reg_irq(X86_PAGE_FAULT_IRQ, x86_page_fault_handle))
        return -1;
    if (reg_irq(X86_COPROC_IRQ, x86_coproc_handle))
        return -1;

    return 0;
}

static int reg_pic_handlers()
{
    if (reg_irq(IRQ0_VECTOR, x86_i8253_irq_handle))
        return -1;
    if (reg_irq(IRQ1_VECTOR, x86_kbr_irq_handle))
        return -1;
    if (reg_irq(IRQ6_VECTOR, x86_floppy_irq_handle))
        return -1;

    return 0;
}

/*
 * Top CPU initialization routine.
 */
int x86_init()
{
    /* disable interrupts until handlers are in place */
    irq_disable();
    /* initialize PIC controller */
    if (i8259_init())
    {
        kernel_warning("Intel 8259 PIC controller failure");
        return -1;
    }
    /* initialize PIT controller */
    if (i8253_init())
    {
        kernel_warning("Intel 8253 PIT controller failure");
        return -1;
    }
    /* register interrupt handlers */
    if (reg_cpu_handlers())
    {
        kernel_warning("CPU handler registration failure");
        return -1;
    }
    if (reg_pic_handlers())
    {
        kernel_warning("PIC handler registration failure");
        return -1;
    }
    /* install IDT */
    if (install_idt())
    {
        kernel_warning("IDT initialization failure");
        return -1;
    }
    /* handlers are set - enable interrupts */
    irq_enable();
    
    /* initialize DMA */
    if (dma_init())
    {
        kernel_warning("DMA initialization failure");
        return -1;
    }

    return 0;
}

