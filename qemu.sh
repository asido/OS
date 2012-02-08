#!/bin/bash
if [[ $1 = '-d' ]]; then
	sudo qemu-system-i386 -fda FLOPPY.IMG -m 128 -S -gdb tcp::1234 -serial stdio
else
	sudo qemu-system-i386 -fda FLOPPY.IMG -m 128
fi
