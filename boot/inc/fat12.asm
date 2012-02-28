;==============================================================================
;   FAT12 driver.
;
;       Author: Arvydas Sidorenko
;==============================================================================
; FAT12 filesystem structure
;--------------------------------------------------
; Boot Sector
; File Allocation Table 1
; File Allocation Table 2
; Root Directory Table
; Data Region containng files and directories
;--------------------------------------------------

BytesPerSector:     dw 512
SectorsPerCluster:  db 1
ReservedSectors:    dw 1
FatCopies:          db 2
RootDirEntries:     dw 224  ; max dirs/files a floppy can hold
RootDirSize:        dw 0xE
NumSectors:         dw 2880
SectorsPerFAT:      dw 9
SectorsPerTrack:    dw 18
NumberOfHeads:      dw 2
LastClusterVal:     dw 0x0FF8
DriveNumber:        db 0

; In total we need 0x4000 bytes of memory to operate. Will use 0x98000 - 0x9C000.
TableLoadSegment:   dw 0x9800   ; just before our PDE (hope nothing will overlap bc of miscalculations)
RootLoadOffset:     dw 0x0      ; 0x1C00 for RootDirEntries (RootDirEntries * 32)
FATLoadOffset:      dw 0x1C00   ; 0x2400 for FAT tables (FatCopies * SectorsPerFAT * BytesPerSector)

; Used when converting LBA (Logical Block Address) to CHS (Cylinder/Head/Sector)
_CurHead:   db 0
_CurTrack:  db 0
_CurSector: db 0


;----------------------------------------------------------
;   Loads a file into memory
;       ARG0 => filename
;       ARG1 => memory segment
;       ARG2 => memory offset
;       Returns file size in AX/BX or 0 if on not found/error
;           BX holds the count of AX roll-overs.
;----------------------------------------------------------
LOAD_FILE:
    push bp
    mov bp, sp
    push si

    ; load root dir
    call _LOAD_ROOT
    or ax, ax
    je .Error

    ; get file's FAT index
    push WORD [bp+4]
    call _FIND_FILE
    cmp ax, -1
    je .Error
    mov si, ax

    ; load FAT
    call _LOAD_FATS
    or ax, ax
    je .Error

    ; and finally load the actual file
    push WORD [bp+8]    ; memory offset
    push WORD [bp+6]    ; memory segment
    push si             ; FAT index
    call _LOAD_FILE
    or ax, ax
    jne .Success
    or bx, bx
    jne .Success
    
.Error:
    xor ax, ax
    xor bx, bx
.Success:
    pop si
    leave
    ret 6


;----------------------------------------------------------
;   Loads Root Directory
;       Returns loaded sector count
;----------------------------------------------------------
_LOAD_ROOT:
    push bp
    mov bp, sp
    push bx
    push dx

    ; find location of root dir
    ; skip bootsector
    mov ax, 1
    ; skip FATs
    xor dx, dx
    movzx bx, BYTE [FatCopies]
    imul bx, WORD [SectorsPerFAT]
    add ax, bx

    push ax
    call _SET_CHS

    ; and finally load it
    push WORD [RootDirSize]     ; amount of sectors to load
    push WORD [RootLoadOffset]
    push WORD [TableLoadSegment]
    call _LOAD_SECTORS
    cmp al, BYTE [RootDirSize]
    je .Success
.Error:
    xor ax, ax
.Success:
    pop dx
    pop bx
    leave
    ret


;----------------------------------------------------------
;   Loads FATs
;       Returns loaded sector count
;----------------------------------------------------------
_LOAD_FATS:
    push bp
    mov bp, sp
    push cx

    ; they are just after the boot sector
    push 1
    call _SET_CHS

    ; calculate sector count
    movzx ax, BYTE [FatCopies]
    mul WORD [SectorsPerFAT]
    mov cx, ax  ; save it for later

    push ax
    push WORD [FATLoadOffset]
    push WORD [TableLoadSegment]
    call _LOAD_SECTORS
    cmp ax, cx
    je .Success
.Error:
    xor ax, ax
.Success:
    pop cx
    leave
    ret


;----------------------------------------------------------
;   Loads a file into provided memory address
;       ARG0 => FAT index
;       ARG1 => memory segment
;       ARG2 => memory offset
;       Returns a number of loaded sectors
;----------------------------------------------------------
_LOAD_FILE:
    push bp
    mov bp, sp
    push cx
    push dx
    push si
    push di
    xor bx, bx  ; higher counter
    xor cx, cx  ; lower counter
    mov dx, WORD [bp+4]
    jmp .LoopFirst

    ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ; FAT entry value meanings:
    ;   0x00        - unused
    ;   0xFF0-0xFF6 - reserved cluster
    ;   0xFF7       - bad cluster
    ;   0xFF8-0xFFF - last cluster in a file
    ;   (anything else) - the next cluster of file
    ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.LoopMain:
    ; check if last loaded cluster was the last one.
    cmp si, 0xFF8
    jb .ContinueNextLoad
    cmp si, 0xFFF
    jbe .Return
.ContinueNextLoad:
    mov dx, si
.LoopFirst:
    push dx
    call _GET_INDEX_VALUE
    ; if FAT entry == 0x0, unusable - skip the sector
    or ax, ax
    je .LoopMain
    ; if FAT entry >= 0xFF0
    cmp ax, 0xFF0
    je .Error
    mov si, ax
    inc cx
    ; increment BX if we got 0xFFFF overflow
    jno .NoOverflow
    inc bx
.NoOverflow:
    push dx
    call _LOGICAL_TO_PHYSICAL
    ; set CHS values
    push ax
    call _SET_CHS
    ; and load the sector
    push 1
    push WORD [bp+8]
    push WORD [bp+6]
    call _LOAD_SECTORS
    or ax, ax
    je .Error
    ; increment offset by 1 sector
    mov di , WORD [BytesPerSector]
    add WORD [bp+8], di
    ; check for offset overflow
    jnc .LoopMain
    add WORD [bp+6], 0x1000
    jmp .LoopMain
    
.Error:
    xor bx, bx
    xor cx, cx
.Return:
    mov ax, cx
    pop di
    pop si
    pop dx
    pop cx
    leave
    ret



;----------------------------------------------------------
;   Logical sector to physical
;       ARG0 => logical sector number
;----------------------------------------------------------
_LOGICAL_TO_PHYSICAL:
    push bp
    mov bp, sp
    mov ax, WORD [bp+4]
    add ax, 31
    leave
    ret 2


;----------------------------------------------------------
;   Returns FAT index value.
;       ARG0 => FAT index
;----------------------------------------------------------
_GET_INDEX_VALUE:
    push bp
    mov bp, sp
    push dx
    push bx
    push si
    mov si, WORD [FATLoadOffset]
    push es
    mov es, WORD [TableLoadSegment]

    ; get the starting byte and if odd/even
    mov ax, WORD [bp+4]
    mov bx, 3
    imul ax, bx
    xor dx, dx
    mov bx, 2
    div bx
    or dx, dx
    je .Even
    ; FAT entry = 12 bits (one and a half byte)
.Odd:
    add si, ax
    mov ax, WORD [es:si]
    shr ax, 4
    jmp .Return
.Even:
    add si, ax
    mov ax, WORD [es:si]
    and ax, 0xFFF   ; 0000111111111111

.Return:
    pop es
    pop si
    pop bx
    pop dx
    leave
    ret 2


;----------------------------------------------------------
;   Finds a file
;       ARG0 => filename
;       Returns FAT index or -1 if not found.
;----------------------------------------------------------
_FIND_FILE:
    push bp
    mov bp, sp
    push cx
    push si
    push di
    push es

    mov es, WORD [TableLoadSegment]
    mov cx, WORD [RootDirEntries]
    mov di, WORD [RootLoadOffset]
    cld
.Loop:
    mov si, WORD [bp+4]
    push cx
    mov cx, 11
    repe cmpsb
    or cx, cx
    je .Found
    mov si, 21
    add si, cx
    add di, si
    pop cx
    loop .Loop
    jmp .NotFound
.Found:
    pop cx
    mov ax, WORD [es:di+15] ; 26th byte is file start cluster index (11 already shifted by repe cmpsb)
    jmp .Success
.NotFound:
    mov ax, -1
.Success:
    pop es
    pop di
    pop si
    pop cx
    leave
    ret 2


;----------------------------------------------------------
;   Loads provided count of sectors from floppy to provided address.
;       ARG0 => memory segment
;       ARG1 => memory offset
;       ARG2 => number of sectors to read.
;----------------------------------------------------------
_LOAD_SECTORS:
    push bp
    mov bp, sp
    push bx
    push cx
    push dx

    mov ah, 0x2     ; BIOS read sectors
    mov al, BYTE [bp+0x8]   ; ARG2
    mov ch, BYTE [_CurTrack]
    mov cl, BYTE [_CurSector]
    mov dh, BYTE [_CurHead]
    mov dl, BYTE [DriveNumber]
    mov bx, WORD [bp+6]
    push es
    mov es, WORD [bp+4]
    int 0x13
    pop es
    jc .Error
    movzx ax, al
    jmp .Success
.Error:
    xor ax, ax
.Success:
    pop dx
    pop cx
    pop bx
    leave
    ret 6


;----------------------------------------------------------
;   Sets the _CurHead, _CurTrack, _CurSector to point to privded LBA
;       ARG0 => LBA (Logical Block Address)
;           head    = LBA / SectorsPerTrack % NumberOfHeads
;           track   = LBA / (SectorsPerTrack * NumberOfHeads)
;           sector  = (LBA % SectorsPerTrack) + 1
;----------------------------------------------------------
_SET_CHS:
    push bp
    mov bp, sp
    push ax
    push bx
    mov bx, WORD [bp+4] ; take the arg

    ; calculate head
    xor dx, dx
    mov ax, bx
    div WORD [SectorsPerTrack]
    xor dx, dx
    div WORD [NumberOfHeads]
    mov WORD [_CurHead], dx

    ; calculate track
    push cx
    mov ax, WORD [SectorsPerTrack]
    mul WORD [NumberOfHeads]
    mov cx, ax
    xor dx, dx
    mov ax, bx
    div cx
    mov WORD [_CurTrack], ax
    pop cx

    ; calculate sector
    mov ax, bx
    xor dx, dx
    div WORD [SectorsPerTrack]
    inc dx
    mov WORD [_CurSector], dx

    pop bx
    pop ax
    leave
    ret
