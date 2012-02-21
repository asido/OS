;==============================================================================
;   Memory related routines used in 32-bit mode
;
;       Author: Arvydas Sidorenko
;==============================================================================

%ifndef __MEM32_INC_
%define __MEM32_INC_

[bits 32]

; Page map addresses. Going to use the most top of ~600Kb available free to use at 0x7E00 - 0x9FBFF.
; The lower part will be used by FAT filesystem driver and the kernel itself.
%define  PDE_ADDR       0x9C000 ; page directory entry address. NOTE: MUST be 4Kb align
%define  PTE_0_ADDR     0x9D000 ; 0th/1024 page table
%define  PTE_KRNL_ADDR  0x9E000 ; 768th/1024 page table, bc kernel will be in 3GB/4GB
; NOTE: just above these addresses FAT12 driver is going to load Root & FAT tables
%define  PTE_COUNT      1024    ; each page table has 1024 entries
; PTE: bit 0 - present | bit 1 - read/write | bit 2 - user/supervisor level
%define  PTE_FLAG 3

;------------------------------------------------------------------------------
;   Initializes paging.
;       Enables 1 page table for the moment to get 4MB of memory,
;       the rest will be done in kernel
;------------------------------------------------------------------------------
ENABLE_PAGING:
    push ebp
    mov ebp, esp
    pusha

    ; initialize the first page table
    mov edi, PTE_0_ADDR
    mov ecx, PTE_COUNT
    shr ecx, 2      ; just 1st MB since kernel space is 1-5MB
    xor eax, eax    ; the physical address
    or eax, PTE_FLAG
.PTE_0_LOOP:
    mov [edi], eax
    add edi, 4          ; move to the next PTE in the table
    lea eax, [eax+4096] ; move to the next memory area
    loop .PTE_0_LOOP

    ; initialize kernels page table
    mov edi, PTE_KRNL_ADDR
    mov ecx, PTE_COUNT
    mov eax, 0x100000   ; the 3rd gig
    or eax, PTE_FLAG
.PTE_KRNL_LOOP:
    mov [edi], eax
    lea edi, [edi+4]    ; move to the next PTE in the table
    lea eax, [eax+4096] ; move to the next memory area
    loop .PTE_KRNL_LOOP

    ; map the 0th PTE in PDE
    mov edi, PDE_ADDR
    mov eax, PTE_0_ADDR
    or eax, PTE_FLAG
    mov [edi], eax
    ; map kernels PTE in PDE
    lea edi, [edi+768*4]
    mov eax, PTE_KRNL_ADDR
    or eax, PTE_FLAG
    mov [edi], eax

    ; register the PDE
    mov eax, PDE_ADDR
    mov cr3, eax

    ; and finally enable paging
    mov eax, cr0
    or eax, 0x80000000  ; 31th bit
    mov cr0, eax

    popa
    leave
    ret


;------------------------------------------------------------------------------
;   Copies ARG2 bytes of memory from ARG0 to ARG1
;       ARG0 => source
;       ARG1 => destination
;       ARG2 => size in bytes
;------------------------------------------------------------------------------
MEM_COPY:
    push ebp
    mov ebp, esp

    mov esi, DWORD [ebp+0x8]    ; source
    mov edi, DWORD [ebp+0xC]    ; destination
    mov ecx, DWORD [ebp+0x10]   ; size
    cld
    rep movsb

    leave
    ret 0xC


;------------------------------------------------------------------------------
;   Converts previously returned available memory size in mem16.inc to
;   32-bit integer in Kb.
;       Same as returned in AX/BX from the MEM16 call
;       ARG0 => available Kb in the first 15Mb
;       ARG1 => available memory after 15Mb in 64K chunks
;------------------------------------------------------------------------------
MEM16_TO_MEM32_SIZE:
    push ebp
    mov ebp, esp

    mov eax, DWORD [ebp+0xC]
    imul eax, 64
    add eax, DWORD [ebp+0x8]

    leave
    ret 8


;------------------------------------------------------------------------------
;   Converts previously returned kernel size in sectors
;       ARG0 => lower 2 bytes
;       ARG1 => higher 2 bytes
;------------------------------------------------------------------------------
KRNL_SIZE_TO_INT32:
    push ebp
    mov ebp, esp
    push edx

    ; put the bits properly
    mov eax, DWORD [ebp+0x8]
    mov edx, DWORD [ebp+0xC]
    shl edx, 16
    or eax, edx

    pop edx
    leave
    ret 8



%endif
