#==============================================================================
#	x86 arch build script
#
#		Author: Arvydas Sidorenko
#==============================================================================

# What is going to be compiled
SOURCE	:= $(shell find `pwd` -type f -name "*.c" -print)
# The object files to be produced
OBJ		:= $(SOURCE:.c=.o)

# Default rule
all: $(OBJ) irq

# Explanation:
# 	$@	<= means the rule
# 	$<	<= means dependency
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ $(CLIB)

irq:
	$(AS) $(ASFLAGS) -f elf -g -o irq.o irq.asm
