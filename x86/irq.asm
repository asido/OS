;******************************************************************************
;		Interrupt handle wrappers
;
;			Author: Arvydas Sidorenko
;******************************************************************************

global x86_i8253_irq_handle
extern x86_i8253_irq_do_handle

section .text
align 4

x86_i8253_irq_handle:
	pushad
	cli
	call x86_i8253_irq_do_handle
	sti
	popad
	iret
