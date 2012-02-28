/******************************************************************************
 *      x86 CPU exception handlers.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include "cpu.h"

extern void kernel_panic(char *msg);

/*
 * Divide by zero exception handler.
 * DIV and IDIV instructions can cause it.
 * IRQ: 0
 */
void x86_divide_except()
{
    kernel_panic("attempt to divide by zero");
}

/*
 * Occurs during various breakpoint traps and faults.
 * IRQ: 1
 */
void x86_single_step_debug_except()
{
    kernel_panic("single step debug trap");
}

/*
 * Occurs during nonmaskable hardware interrupt.
 * IRQ: 2
 */
void x86_nonmask_except()
{
    kernel_panic("nonmaskable hardware interrupt");
}

/*
 * Occurs when CPU encounters INT 3 instruction.
 * IRQ: 3
 */
void x86_breakpoint_except()
{
    kernel_panic("breakpoint INT 3 instruction");
}

/*
 * Occurs when CPU encounters INT0 instruction while OF flag is set.
 * IRQ: 4
 */
void x86_overflow_except()
{
    kernel_panic("overflow fault");
}

/*
 * Occurs when BOUND instructions operand exceeds specified limit.
 * IRQ: 5
 */
void x86_bound_except()
{
    kernel_panic("BOUND instruction fault");
}

/*
 * Invalid opcode exception.
 * IRQ: 6
 */
void x86_invalid_opcode_except()
{
    kernel_panic("invalid opcode");
}

/*
 * Occurs during one of the two conditions:
 *      - CPU reaches ESC instruction while EM (emulate) bit of CR0 is set.
 *      - CPU reaches WAIT or ESC instruction and both MP (monitor coprocessor)
 *      and TS (task switched) bits of CR0 are set.
 * IRQ: 7
 */
void x86_busy_coproc_except()
{
    kernel_panic("busy co-CPU fault");
}

/*
 * Occurs when CPU detects an exception while trying to invoke prior
 * exception handler.
 * IRQ: 8
 */
void x86_double_fault_except()
{
    kernel_panic("double fault");
}

/*
 * Occurs when a page or segment violation is detected while transferring the
 * middle portion of a coprocessor operand to the NPX.
 * IRQ: 9
 * NOTE: 386 or earlier only.
 */
void x86_coproc_overrun_except()
{
    kernel_panic("co-CPU overrun fault");
}

/*
 * Occurs if during a task switch the new TSS is invalid.
 * IRQ: 10
 */
void x86_invalid_tss_except()
{
    kernel_panic("invalid TSS");
}

/*
 * Occurs when CPU detects that the present bit of a descriptor is zero.
 * IRQ: 11
 */
void x86_no_segment_except()
{
    kernel_panic("no segment fault");
}

/*
 * Occurs during one of two conditions:
 *      - Limit violation in any operation that refers to
 *      SS (stack segment register).
 *      - When attempting to load SS with a descriptor which is marked
 *      as not-present but is otherwise valid.
 * IRQ: 12
 */
void x86_stack_except()
{
    kernel_panic("stack fault");
}

/*
 * Occurs during all the rest of protection violations
 * IRQ: 13
 */
void x86_gpf_except()
{
    kernel_panic("GPF");
}

/*
 * Page translation exception.
 * IRQ: 14
 */
void x86_page_fault_except()
{
    __asm__ __volatile__("movl %%cr2, %%eax" : : : "eax");
    
    kernel_panic("page fault ");
}

/* IRQ 15 is reserved */

/*
 * Occurs when CPU detects a signal from the coCPU on the ERROR# input pin.
 * IRQ: 16
 */
void x86_coproc_except()
{
    kernel_panic("internal co-CPU fault");
}

/* IRQ 17-31 are reserved */
