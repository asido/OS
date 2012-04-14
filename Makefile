#==============================================================================
#	The top build script
#		
#		Author: Arvydas Sidorenko
#==============================================================================

# Check arch
ARCH	= $(shell uname -m)
ifneq ($(ARCH),x86_64)
ifneq ($(ARCH),i686)
$(error $(ARCH) architecture is not supported)
endif
endif

# Kernel executable name
PROGRAM			= KERNEL

# Current working directory
PWD				= $(shell pwd)

# Assembler
export AS		= nasm
export ASFLAGS	=

# C compiler
export CC		= gcc

# Don't use -O2 optimization, it tripple faults the CPU !
export CFLAGS	=	-c	\
					-Wall	\
					-g		\
					-gstabs	\
					-Wextra	\
					-fno-builtin	\
					-nodefaultlibs	\
					-nostartfiles	\
					-nostdlib	\
					-m32	\
					-mtune=pentium

# libraries
export CLIB		= 	-I $(PWD)	\
					-I $(PWD)/libc	\
					-I $(PWD)/kernel32	\
					-I $(PWD)/fs	\
					-I $(PWD)/drivers/floppy	\
					-I $(PWD)/drivers/keyboard

# if 'werror=y' flag is specified, include -Werror flag for C compiler
ifeq ($(werror),y)
	CFLAGS += -Werror
endif

# Linker
LD				= ld
LDFLAGS			= -T linker.ld -m elf_i386

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS = -rR --no-print-directory


# Default rule
default:
	cd boot; make
	cd libc; make
	cd x86; make
	cd kernel32; make
	cd fs; make
	cd apps; make
	cd drivers/keyboard; make
	cd drivers/floppy; make
	$(LD) $(LDFLAGS) -o $(PROGRAM)	libc/*.o	\
									drivers/keyboard/*.o	\
									drivers/floppy/*.o	\
									x86/*.o		\
									kernel32/*.o	\
									apps/*.o	\
									fs/*.o
	./floppy.sh

# Full rule, which first cleans all the build files and then does the build from scratch
all: clean default

# Does the cleanup
clean:
	@find . \( -name '*.o' -o -name '*.SYS' -o -name '*.bin' \) -print -exec rm -f '{}' \;
	@rm -f $(PROGRAM)

# Print help
help:
	@echo 'Make options:'
	@echo '    all		- Build all targets marked with [*]'
	@echo '  * default	- Default rule, which just builds the kernel executable'
	@echo '  * clean	- Removes generated files'
	@echo '    help	- Prints this help text'
	@echo '    force=y	- By default C compiler uses -Werror flag, unless this flag is set'
