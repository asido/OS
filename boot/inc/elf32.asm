;==============================================================================
;   ELF executable related routines
;       
;       Author: Arvydas Sidorenko
;==============================================================================

ELFSignature:    db 0x7F,'ELF'  ; ELF signature

;------------------------------------------------
;   Validates ELF
;       ARG0 => pointer to ELF's base in memory
;       Returns 0 if all good, -1 if error found.
;------------------------------------------------
VALIDATE_ELF:
    push ebp
    mov ebp, esp
    push esi
    push edi
    xor eax, eax

    ; kernel image signature
    mov esi, DWORD [ebp+0x8]
    mov edi, ELFSignature
    cmpsd
    jne .Error
    ; check data encoding
    add esi, 5
    cmp esi, 0
    jne .Success
.Error:
    mov eax, -1
.Success:
    pop edi
    pop esi
    leave
    ret 4


;------------------------------------------------
;   Returns executable entry point
;       ARG0 => pointer to ELF's base in memory.
;------------------------------------------------
GET_ELF_ENTRY:
    push ebp
    mov ebp, esp

    mov eax, DWORD [ebp+0x8]
    add eax, 0x18   ; ELF header holds the EP
    mov eax, [eax]

    leave
    ret 4


;------------------------------------------------
;   Loads ELF sector into memory
;       ARG0 => pointer to ELF file
;       ARG1 => pointer to the sector header
;------------------------------------------------
_LOAD_SECTOR:
    push ebp
    mov ebp, esp
    pusha
    
    mov ebx, DWORD [ebp+0xC] ; get the entry structure
    mov esi, DWORD [ebx+0x4] ; offset to sector data in file
    add esi, DWORD [ebp+0x8]   ; add the offset to the file base
    mov edi, DWORD [ebx+12] ; PA (destination)
    mov ecx, DWORD [ebx+16] ; size in file
    mov ebx, DWORD [ebx+20] ; size in memory
    sub ebx, ecx    ; if memor size is bigger, we will need to NULL out the
                    ; part which overflowed (.BSS section or something)
    ; perform the copy
    cld
    rep movsb

    or ebx, ebx
    jz .Done
    ; memory section is bigger then it was in file, do the NULL out
    mov ecx, ebx
    mov ebx, DWORD [ebp+0xC]
    mov edi, DWORD [ebx+12] ; PA (destination)
    push ecx
    mov ecx, DWORD [ebx+16] ; size in file
    add edi, ecx
    pop ecx
    xor eax, eax
    cld
    rep stosb

.Done:
    popa
    leave
    ret 4


;------------------------------------------------
;   Loads ELF for execution.
;       ARG0 => pointer to ELF file in memory.
;       Returns it's entry point or -1 on error
;------------------------------------------------
LOAD_ELF:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    push edi

    ; first validate the ELF
    push DWORD [ebp+0x8]
    call VALIDATE_ELF
    or eax, eax
    jnz .Error

    mov eax, DWORD [ebp+0x8]
    add eax, 28 ; header offset
    push eax
    add eax, 14 ; entry size
    movzx edx, WORD [eax]
    add eax, 2
    movzx ecx, WORD [eax] ; number of entries
    pop eax
    mov eax, [eax] ; the header offset in file
    add eax, DWORD [ebp+0x8]
    ; current state: EAX = first entry ptr, ECX = entry count, EDX = entry size
.Loop:
    mov ebx, DWORD [eax]
    or ebx, ebx ; if header type is NULL_TYPE - skip it
    jz .Loop
    push eax
    push DWORD [ebp+0x8]
    call _LOAD_SECTOR
    add eax, edx    ; shift to another entry
    dec ecx     ; decrement header count
    jnz .Loop
    jmp .Success

.Error:
    mov eax, -1
    jmp .Done
.Success:
    push DWORD [ebp+0x8]
    call GET_ELF_ENTRY
.Done:
    pop edi
    pop edx
    pop ecx
    pop ebx
    leave
    ret 4
