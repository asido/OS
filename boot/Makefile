#==============================================================================
#	Bootloader build script
#
#		Author: Arvydas Sidorenko
#==============================================================================

# Include folder for ASM files
ASINC = -i inc/ -g

# Default rule
default:
	$(AS) $(ASFLAGS) -f bin $(ASINC) -o STAGE1.SYS		stage1.asm
	$(AS) $(ASFLAGS) -f bin $(ASINC) -o STAGE2.SYS		stage2.asm
