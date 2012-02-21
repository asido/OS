;==============================================================================
;   Text printing related stuff.
;
;       Author: Arvydas Sidorenko
;==============================================================================

%ifndef __STDIO16_INC_
%define __STDIO16_INC_

;------------------------------------------
; Prints a string
; ARG0 => 0 terminated string
;------------------------------------------
PRINT16:
    push bp
    mov bp, sp
    push bx
    mov bx, [bp+4]
.Main:
    mov al, [bx]
    or al, al
    jz .PrintDone
    mov ah, 0x0E    ; BIOS VGA interrupt
    int 0x10        ; perform the interrupt
    inc bx
    jmp .Main
.PrintDone:
    leave
    ret 2

%endif
