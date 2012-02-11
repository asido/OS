ENTRY (kmain)

SECTIONS
{
    . = 0xC0001000;

    .text :{
        *(.text)
    }

    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }

    .bss ALIGN (0x1000) : {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }
}
