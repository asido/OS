;==============================================================================
;   Memory related routines used in 16-bit mode
;
;       Author: Arvydas Sidorenko
;==============================================================================

%ifndef __MEM16_INC_
%define __MEM16_INC_

[bits 16]

;------------------------------------------
;   Enables A20 CPU line
;       to be able to reference up to 4GB of memory
;------------------------------------------
ENABLE_A20:
    push bp
    mov bp, sp
    mov ax, 0x2401
    int 0x15
    leave
    ret


;------------------------------------------
;   Returns available memory count:
;       ax = count of KB up to 15MB
;       bx = count of 64KB blocks in >15MB section
;------------------------------------------
GET_AVAIL_MEM_KB:
    push bp
    mov bp, sp
    push cx
    push dx
    xor cx, cx
    xor dx, dx
    mov ax, 0xE801
    int 0x15    
    jc  .ERROR
    cmp ah, 0x86        ; BIOS error: unsupported function
    je  .ERROR
    cmp ah, 0x80        ; BIOS error: invalid command
    je  .ERROR
    jcxz .MEM_DONE      ; some BIOS store it in CX/DX. test if CX is 0
    mov ax, cx
    mov bx, dx
    jmp .MEM_DONE
.ERROR:
    mov ax, -1
.MEM_DONE:
    pop dx
    pop cx
    leave
    ret


;--------------------------------------------------
;   Puts CPU into 32-bit mode.
;--------------------------------------------------
ENABLE_PMODE:
    push bp
    mov bp, sp

    cli             ; clear interrupts
    mov eax, cr0    ; set bit 0 in CR0 - enters pmode
    or eax, 1
    mov cr0, eax

    leave
    ret

%endif
