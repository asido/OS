#!/bin/bash
if [[ $1 = '-d' ]]; then
	sudo qemu-system-i386 -fda FLOPPY.IMG -m 200 -S -gdb tcp::1234 -serial stdio
else
	sudo qemu-system-i386 -fda FLOPPY.IMG -m 256
fi
