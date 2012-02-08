#==============================================================================
#	Keyboard driver build script
#		
#		Author: Arvydas Sidorenko
#==============================================================================

CFLAGS	+= -nostartfiles
SOURCE	:= $(shell find `pwd` -type f -name "*.c" -print)
OBJ		:= $(SOURCE:.c=.o)

# Default rule
default: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ $(CLIB)
