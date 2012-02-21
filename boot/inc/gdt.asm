;==============================================================================
;   GDT (Global Descriptor Table)
;
;       Author: Arvydas Sidorenko
;==============================================================================

;******************************************************************************
;                       GDT MANUAL
;******************************************************************************
; GDT is an 8 byte QWORD value that describes properties for the descriptor.
; They are of the format:
;
;    Bits 0-15: Bits 0-15 of the Segment Limit
;    Bits 16-39: Bits 0-23 of the Base Address
;    Bit 40: Access bit (Used with Virtual Memory)
;    Bits 41-43: Descriptor Type
;        Bit 43: Executable segment
;            0: Data Segment
;            1: Code Segment
;        Bit 42: Expansion direction (Data segments), conforming (Code Segments)
;        Bit 41: Readable and Writable
;            0: Read only (Data Segments); Execute only (Code Segments)
;            1: Read and write (Data Segments); Read and Execute (Code Segments)
;    Bit 44: Descriptor Bit
;        0: System Descriptor
;        1: Code or Data Descriptor
;    Bits 45-46: Descriptor Privilege Level
;        0: (Ring 0) Highest
;        3: (Ring 3) Lowest
;    Bit 47 Segment is in memory (Used with Virtual Memory)
;    Bits 48-51: Bits 16-19 of the segment limit
;    Bits 52: Reserved for OS use (we can do whatever we want here)
;    Bit 53: Reserved-Should be zero
;    Bit 54: Segment type
;        0: 16 bit
;        1: 32 bit
;    Bit 55: Granularity
;        0: None
;        1: Limit gets multiplied by 4K
;    Bits 56-63: Bits 24-32 of the base address
;******************************************************************************

%ifndef __GDT_INC_
%define __GDT_INC_

[bits 16]

;------------------------------------------
;   DATA SECTION
;------------------------------------------
%define GDT_NULL_DESC 0x0
%define GDT_CODE_DESC 0x8
%define GDT_DATA_DESC 0x10

GDT_START: 
    ; First 8 bytes aren't used by the CPU afaik
    .NullDesc:
        dw 0
        dw 0
        dw 0
        dw 0

    ; Setting all code section (0B - 4GB) to be readable/executable
    .CodeDesc:
        dw 0xFFFF   ; Bits 0-15
        dw 0x0000   ; Bits 16-31
        db 0x00     ; Bits 32-39
        db 10011010b; Bits 40-47
        db 11001111b; Bits 48-55
        db 0x00     ; Bits 56-63

    ; Same as CodeDesc only the descriptor type bit is set to `data` instead
    .DataDesc:
        dw 0xFFFF   ; Bits 0-15
        dw 0x0000   ; Bits 16-31
        db 0x00     ; Bits 32-39
        db 10010010b; Bits 40-47
        db 11001111b; Bits 48-55
        db 0x00     ; Bits 56-63
GDT_END:

; LGDT instruction takes 6 bytes (in 32-bit mode)
LGDT_VAL: 
    dw 0    ; size in bytes
    dd 0    ; memory address (3 bytes) and data access rights (1 byte)
;------------------------------------------
;   END OF DATA SECTION
;------------------------------------------


GDT_INSTALL:
    push bp
    mov bp, sp

    ; calculate GDT size - 1 (2 bytes)
    mov ax, WORD GDT_END
    sub ax, GDT_START
    dec ax
    mov WORD [LGDT_VAL], ax

    ; calcluate GDT location (3 bytes)
    mov ax, GDT_START
    mov WORD [LGDT_VAL+2], ax
    mov BYTE [LGDT_VAL+4], 0x00 ; our loc not exceeding 2 bytes
    ; remaining 
    mov BYTE [LGDT_VAL+5], 0x93 ; data cacess rights (can be ignored actually)

    cli ; disable interrupts
    lgdt [LGDT_VAL] ; load GDT
    sti ; enable interrupts

    leave
    ret

%endif
