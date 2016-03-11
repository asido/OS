===============================================================================
                                 Axid OS
===============================================================================

![alt](http://i.imgur.com/zbmy5lp.png)
```text
-------------------------------------------------------------------------------
        FOLDER STRUCTER
-------------------------------------------------------------------------------
boot        : all bootloader files
boot/inc    : utilities used by STAGE2 bootloader (STAGE1 can't use includes)
drivers     : device drivers
kernel32    : the kernel itself
x86         : x86 architecture specific stuff
libc        : custom libc implementations
resources   : (if you have it) docs and manuals for my personal use.


-------------------------------------------------------------------------------
        BUILD
-------------------------------------------------------------------------------
To build, just execute `make`.
Now, it will work fine on Linux, but not on BSD. My written makefiles has 
GNU'cism, which are not considered standard and not implemented in BSD make.
You can try `gmake` though, which will build, but when I tried, the GCC in the
end still failed when tried to compile my libc - even though I had specified
`-nostdlib` flag, it still included BSD libc making the compiler confused.
For Windows, you can try CYGWIN. Might work, might not. Haven't tried it.

Dependencies:
-------------------------------------------------------------------------------
dosfstools  : this is required to make virtual floppy image used to boot the OS
nasm        : the boot loader is written in Netwide Assembler (NASM)
gcc         : seriously, if anyone needs explanation what this does, go play
                ping pong or smth, because you are in a wrong place ;)
coreutils   : mount, dd, cp...

FLOPPY.IMG is a bootable floppy image. Put it in x86 emulators virtual floppy drive or
move to a real floppy disk if you have one (dd if=FLOPPY.IMG of=/dev/fda)


-------------------------------------------------------------------------------
        DEBUGGING
-------------------------------------------------------------------------------
You can launch the OS in debug mode from the very first instruction in boot loader.
I use QEMU + GDB for that:
    Run QEMU:
        qemu -fda FLOPPY.IMG -S -gdb tcp::1234 -serial stdio
    Then in another terminal attach GDB (type in gdb shell):
        (gdb) target remote localhost:1234
    Put some breakpoints or whatever and start qemu execution:
        (gdb) c

Furthermore, here are some handy settings for GDB:
    For debugging 16-bit code, don't forget to set 16-bit disassembling:
        (gdb) set architecture i8086
    Set disassembling syntax to Intel (default is AT&T):
        (gdb) set disassembly-flavor intel
    Show current executing instruction after every stop:
        For v7.0 and above:
        (gdb) set disassemble-next-line on
        For the rest (works for all, but not so nice output):
        (gdb) display/i $pc
    You can also try to set nice disassembling TUI to see if it works for you. For me it's too bugy:
        (gdb) layout asm

NOTE: to have debug info available when debuggin the kernel, add the file in GDB:
            add-symbol-file KERNEL 0x00101000

NOTE2: check for .gdbinit in root folder. It might have all that it in.
That file gets executed if you run GDB from dir with it.


Another option could be using Bochs debugger. There is also a very nice application for it
called Peter-Bochs debugger. I really advice you to check it out if you aren't comfortable
with command line GDB.


-------------------------------------------------------------------------------
        LICENSE
-------------------------------------------------------------------------------
Ehh? None, do whatever you want with it ^^
Just mention my name in case you use something of my ;)
```
