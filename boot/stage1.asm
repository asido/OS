;==============================================================================
; A complete starting point of the OS.
;
; This is going to be a messy code because we can't use any includes,
; defines or structs. Everything is 1:1 representation of the machine code
; since we need it to be compiled to pure binary without any headers and such,
; where BIOS would be able to jump to 0x0 loaders offset and start execution right
; away. This is why we are using ASM in the first place.
;
; The best part - we have 512 bytes only to do our job here and we need to 
; write FAT12 driver, load STAGE2 of bootloader and run it ;)
;
;       Author: Arvydas Sidorenko
;==============================================================================
[BITS 16]
[ORG 0x0]

START:
    jmp LOADER_MAIN


;------------------------------------------------------------------------------
;   FAT12 BPB  (BIOS Parameter Block)
;   (This is what our floppy is formatted in)
;------------------------------------------------------------------------------
OEM                     db "AxidOS  "   ; max 8 bytes
BytesPerSector:         dw 512
SectorsPerCluster:      db 1
ReservedSectors:        dw 1
FatCopies:              db 2
RootDirEntries:         dw 224  ; max dirs/files a floppy can hold
NumSectors:             dw 2880
MediaType:              db 0xf8 ; 11111000
                                ; Bit 0: Sides/Heads = 0 if it is single sided, 1 if its double sided
                                ; Bit 1: Size = 0 if it has 9 sectors per FAT, 1 if it has 8.
                                ; Bit 2: Density = 0 if it has 80 tracks, 1 if it is 40 tracks.
                                ; Bit 3: Type = 0 if its a fixed disk (hard drive), 1 if removable (floppy drive)
                                ; Bits 4 to 7 are unused - always 1.
SectorsPerFAT:          dw 9
SectorsPerTrack:        dw 18
NumberOfHeads:          dw 2
DriveNumber:            db 0


; FAT12 filesystem structure
;--------------------------------------------------
; Boot Sector
; File Allocation Table 1
; File Allocation Table 2
; Root Directory Table
; Data Region containng files and directories
;--------------------------------------------------


;------------------------------------------------------------------------------
;   DATA SECTION
;------------------------------------------------------------------------------
CurrentSector:  db 0x00
CurrentHead:    db 0x00
CurrentTrack:   db 0x00

FATSize:            dw 0x0000
FATDataSector:      dw 0x0000
Stage2ImageName:    db "STAGE2  SYS"
Stage2CurrentCluster:   dw 0x0000
Stage2FileNotFoundMsg:  db "FATAL**: Missing or corrupt STAGE2 image.",0

; Loading addresses
FatRootLoadLoc: dw 0x0200   ; (7C00:0200)
Stage2LoadLoc:  dw 0x0050   ; (0050:0000)
;------------------------------------------------------------------------------
;   END OF DATA SECTION
;------------------------------------------------------------------------------


;--------------------------------------------------
; Prints null terminated char array on the screen
; ARG0 => pointer to null terminated char array to print
;--------------------------------------------------
PRINT:
    push bp
    mov bp, sp
    push bx
    mov bx, [bp+4]
.MAIN:
    mov al, [bx]
    or al, al
    jz .PRINT_DONE
    mov ah, 0x0E    ; BIOS VGA interrupt
    int 0x10        ; perform the interrupt
    inc bx
    jmp .MAIN
.PRINT_DONE:
    leave
    ret 2


;--------------------------------------------------
; Loads FAT root
;--------------------------------------------------
LOAD_ROOT:
    push bp
    mov bp, sp
    ; calculate size of root directory
    mov ax, 32                  ; 32 byte directory entry
    mul WORD [RootDirEntries]   ; total size of directory
    div WORD [BytesPerSector]   ; sectors used by directory
    mov cx, ax

    ; calculate location of root directory
    movzx ax, BYTE [FatCopies]  ; number of FATs
    mul WORD [SectorsPerFAT]        ; sectors used by FATs
    add ax, WORD [ReservedSectors]  ; adjust for boot sector
    mov WORD [FATDataSector], ax        ; base of root directory
    add WORD [FATDataSector], cx

    ; read root directory into memory (7C00:0200)
    push WORD [FatRootLoadLoc]      ; copy root dir above boot code
    push ax             ; root dir location
    push cx             ; size of root dir
    call READ_SECTORS
    leave
    ret


;--------------------------------------------------
;   Calculates FAT size
;--------------------------------------------------
GET_FAT_SIZE:
    push bp
    mov bp, sp
    xor ax, ax
    mov al, BYTE [FatCopies]    ; number of FATs
    mul WORD [SectorsPerFAT]
    leave
    ret


;--------------------------------------------------
;   Read STAGE2 image to memory 
;--------------------------------------------------
LOAD_STAGE2:
        push bp
        mov bp, sp
        pusha
        mov ax, WORD [Stage2LoadLoc]
        mov es, ax
        xor bx, bx      ; destination for image
    LOAD_CLUSTER:
        ; convert cluster to LBA
        push WORD [Stage2CurrentCluster]
        call CLUSTER2LBA

        ; read sector
        push bx
        push ax
        movzx ax, BYTE [SectorsPerCluster]  ; sectors to read
        push ax
        call READ_SECTORS

        ; calculate next cluster
        mov ax, WORD [Stage2CurrentCluster]     ; identify current cluster
        mov cx, ax                  ; copy current cluster
        mov dx, ax                  ; copy current cluster
        shr dx, 1                   ; divide by two
        add cx, dx                  ; sum for (3/2)
        mov di, WORD [FatRootLoadLoc]   ; location of FAT in memory
        add di, cx                  ; index into FAT
        mov dx, WORD [di]           ; read two bytes from FAT
        test ax, 1
        jnz .ODD_CLUSTER
    .EVEN_CLUSTER:
        and dx, 0000111111111111b   ; take low 12 bits
        jmp .READ_DONE
    .ODD_CLUSTER:
        shr dx, 4               ; take high 12 bits
    .READ_DONE:
        mov WORD [Stage2CurrentCluster], dx     ; store new cluster
        cmp dx, 0x0FF8              ; test for end of file (0x0FF8 - 0x0FFF)
        jb LOAD_CLUSTER

        popa
        leave
        ret


;--------------------------------------------------
;   Finds a file in loaded floppy.
;   ARG0 => filename we are looking for
;   Returns the first sector of the file, 0 otherwise.
;--------------------------------------------------
FIND_FILE:
    push bp
    mov bp, sp
    push cx
    push di
    mov cx, [RootDirEntries]    ; the number of dirs a floppy can save. If we reach 0 - file doesn't exist
    mov di, WORD [FatRootLoadLoc]
.Loop:
    push cx
    mov cx, 11          ; 11 character name
    push di
    mov si, [bp+4]      ; load filename
    rep cmpsb           ; test for entry match
    pop di
    je .Found           ; if match - go out
    pop cx
    add di, 32          ; doesn't match - go to the next entry (32 bytes)
    loop .Loop
.Not_Found:
    xor ax, ax
    jmp .Return
.Found:
    mov ax, WORD [di+0x001A] ; from Root Directory Table, starting cluster address is at 26 bytes offset
.Return:
    pop di
    pop cx
    leave
    ret 2

;--------------------------------------------
; Reads sectors from drive to memory
; ARG0 => number of sectors to read
; ARG1 => starting sector
; ARG2 => buffer to read to
;--------------------------------------------
READ_SECTORS:
        push bp 
        mov bp, sp
        mov cx, [bp+4] ; ARG0
        mov bx, [bp+8] ; ARG2

    .SECTORLOOP:
        mov [bp+4], cx

        push WORD [bp+6]
        call LBA2CHS                    ; convert starting sector to CHS

        ; read using BIOS interrupt
        mov al, 0x01                    ; read one sector
        mov ah, 0x02                    ; BIOS read sector
        mov cl, BYTE [CurrentSector]    ; sector
        mov ch, BYTE [CurrentTrack]     ; track
        mov dl, BYTE [DriveNumber]      ; drive
        mov dh, BYTE [CurrentHead]      ; head
        int 0x13                        ; invoke BIOS

        add bx, WORD [BytesPerSector]   ; queue next buffer
        inc WORD [bp+6]                 ; queue next sector
        mov cx, [bp+4]
        loop .SECTORLOOP                ; read next sector

        leave
        ret 6

;----------------------------------------------
; Convert cluster to LBA
; LBA = (cluster - 2) * SectorsPerCluster + FATDataOffset
; ARG0 => cluster
;----------------------------------------------
CLUSTER2LBA:
    push bp
    mov bp, sp
    push bx

    mov ax, [bp+4]
    sub ax, 2
    xor bx, bx
    mov bl, BYTE [SectorsPerCluster]
    mul bx
    add ax, WORD [FATDataSector]

    pop bx
    leave
    ret 2

;------------------------------------------------------------------------------
; Convert LBA (Logical Block Address) to CHS (Cylinder/Head/Sector)
; ARG0 => LBA to convert
;
; absolute sector   = (logical sector / sectors per track) + 1
; absolute head     = (logical sector / sectors per track) MOD number of heads
; absolute track    = logical sector / (sectors per track * number of heads)
;------------------------------------------------------------------------------
LBA2CHS:
    push bp
    mov bp, sp
    mov ax, [bp+4]
    xor dx, dx                      ; prepare DX:AX for operation
    div WORD [SectorsPerTrack]  ; calculate
    inc dl                          ; adjust for sector 0
    mov BYTE [CurrentSector], dl
    xor dx, dx                      ; prepare DX:AX for operation
    div WORD [NumberOfHeads]    ; calculate
    mov BYTE [CurrentHead], dl
    mov BYTE [CurrentTrack], al
    leave
    ret 2


;----------------------------------------------------
; STARTING POINT OF EXECUTION
;----------------------------------------------------
LOADER_MAIN:
        ; set sement registers to our loc - 0000:7C00
        cli             ; disable interrupts
        mov ax, 0x07C0
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        ; create the stack
        mov ax, 0x0000  ; set the stack
        mov ss, ax
        mov sp, 0xFFFF
        sti             ; restore interrupts

        ; load floppy root
        call LOAD_ROOT

        ; search for loaders STAGE2 image in loaded root
        push Stage2ImageName
        call FIND_FILE
        or ax, ax
        je FAILURE
        mov WORD [Stage2CurrentCluster], ax ; save file's first cluster loc

        ; read FAT into memory 
        push WORD [FatRootLoadLoc]
        push WORD [ReservedSectors]
        call GET_FAT_SIZE
        push ax
        call READ_SECTORS

        ; and finally load the STAGE2 image 
        call LOAD_STAGE2

        ; Done! Jump to STAGE2 ^^
        push WORD [Stage2LoadLoc]   ; IP register..
        push WORD 0x0000    ; and code segment for..
        retf                ; far jump

    FAILURE:
        push Stage2FileNotFoundMsg
        call PRINT
        mov ah, 0x00
        int 0x16        ; wait keypress
        int 0x19        ; warm boot computer

; Finish filling loader up to 510 bytes and add loader signature
times 510 - ($ - $$) db 0
dw 0xAA55
