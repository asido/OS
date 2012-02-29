;==============================================================================
; STAGE2 of bootloader. Finally not 512 bytes restriction and we can use
; much more feature rich NASM, including %include, %define keywords.
; 
; The goal here is to prepare the CPU to go into 32-bit and protected
; memory mode. Then jump to STAGE3, which will execute the kernel.
;
;       Author: Arvydas Sidorenko
;==============================================================================
[BITS 16]

; According memory map 0x500 - 0x7BFF is free
[ORG 0x500] ; (0x50:0)

jmp STAGE2_MAIN

;------------------------------------
; Preprocessor directives
;------------------------------------
%include "stdio16.asm"
%include "gdt.asm"
%include "mem16.asm"
%include "fat12.asm"

;------------------------------------
; DATA SECTION
;------------------------------------
%define KERNEL_PMODE_BASE 0xC0000000; kernel load location in protected mode
%define KERNEL_RMODE_BASE 0x7E00    ; kernel load location in real mode
KernelImgName:  db "KERNEL     "    ; MUST be 11 bytes

; uninitialized data
KernelImgSizeLow: dd 0
KernelImgSizeHigh: dd 0
MemKBLow: dd 0
MemKBHigh: dd 0

FailureMsg: db 0x0D, 0x0A, "*** FATAL: KERNEL is corrupt or not in the FLOPPY.IMG.", 0x0D, 0x0A, 0x0A, 0x00
;------------------------------------
; END OF DATA SECTION
;------------------------------------


STAGE2_MAIN:
    ; setup segments and stack
    cli             ; clear interrupts
    xor ax, ax      ; null segments
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7DFF  ; stack begins at stage1 bootloader overwritting what there is
    sti             ; enable interrupts

    ; install GDT
    call GDT_INSTALL

    ; enable A20 for 4GB memory access
    call ENABLE_A20

    ; Get memory size
    call GET_AVAIL_MEM_KB
    cmp ax, -1
    je .Error16
    ; save the results for when we enter 32-bit mode
    mov WORD [MemKBLow], ax
    mov WORD [MemKBHigh], bx

    ; Load the kernel while it's easy in 16-bit mode ^^  (will move after)
    push KERNEL_RMODE_BASE      ; memory offset
    push 0                      ; memory segment
    push KernelImgName          ; filename
    call LOAD_FILE
    mov WORD [KernelImgSizeLow], ax
    mov WORD [KernelImgSizeHigh], bx
    or ax, ax
    jnz PMode
    or bx, bx
    jnz PMode

.Error16:
    push FailureMsg
    call PRINT16
    mov ah, 0
    int 0x16                    ; await keypress
    int 0x19                    ; warm boot computer
    cli                         ; if you get here, your PC is fucked up
    hlt

PMode:
    ; And finally enable 32-bit mode ^^
    call ENABLE_PMODE
    jmp GDT_CODE_DESC:STAGE3    ; far jump to fix CS

    ;*****************************************************************************************
    ; NOTE: Do NOT re-enable interrupts! Doing so will tripple fault the CPU!
    ; Fix this in STAGE3
    ;*****************************************************************************************

;-------------------------------------------------------------------
;   ENTRY POINT FOR STAGE 3
;-------------------------------------------------------------------
[bits 32]   ; Finally 32 bit world!

%include "mem32.asm"
%include "elf32.asm"

; uninitialized data
MemorySize: dd 0
KernelImgSize:  dd 0


STAGE3:
    ; set segment registers
    cli
    mov ax, GDT_DATA_DESC   ; set data segments to data selectors (0x10)
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov esp, 0x90000    ; stack begins from 0x90000

    ; map PDE and enable paging
    call ENABLE_PAGING

    ; calculate memory size in Kb from in 16-bit returned BIOS
    push DWORD [MemKBHigh]
    push DWORD [MemKBLow]
    call MEM16_TO_MEM32_SIZE
    mov DWORD [MemorySize], eax

    ; calculate kernel size in sectors returned from in 16-bit FAT12 driver
    push DWORD [KernelImgSizeHigh]
    push DWORD [KernelImgSizeLow]
    call KRNL_SIZE_TO_INT32
    mov DWORD [KernelImgSize], eax

    ; load the kernel
    push KERNEL_RMODE_BASE
    call LOAD_ELF

    ; and execute it!
    cli
    push DWORD KERNEL_PMODE_BASE
    push DWORD [KernelImgSize]
    push DWORD [MemorySize]
    call eax
    add esp, 0x4


; Sloppy 32-bit error handler ^^
.Error32:
    ; TODO: 32-bit PRINT function
    cli         ; clear interrupts to prevent triple faults
    hlt         ; halt the system
    jmp .Error32
