;==============================================================================
;	ELF executable related routines
;		
;		Author: Arvydas Sidorenko
;==============================================================================

ELFSignature:	 db 0x7F,'ELF'	; ELF signature

;------------------------------------------------
;	Validates ELF
;		ARG0 => pointer to ELF's base in memory
;		Returns 0 if all good, -1 if error found.
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
;	Returns executable entry point
;		ARG0 => pointer to ELF's base in memory.
;------------------------------------------------
GET_ELF_ENTRY:
	push ebp
	mov ebp, esp

	mov eax, DWORD [ebp+0x8]
	add eax, 0x18
	mov eax, [eax]

	leave
	ret 4
