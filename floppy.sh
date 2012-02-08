#!/bin/bash

set -e

FLOPPY_MOUNT_DIR=/mnt/hd
BOOTABLE_FLOPPY_IMG=FLOPPY.IMG
BOOTLOADER=boot/STAGE1.SYS
STAGE2=boot/STAGE2.SYS
KERNEL=KERNEL

# declare color constants used for the output
COLOR_GREEN=$(tput setaf 2)		# green
COLOR_RED=$(tput setaf 1)		# red
COLOR_RESET=$(tput sgr0)		# reset

# delete floppy image if it exist, or mkfs.msdos will fail
if [ -e ${BOOTABLE_FLOPPY_IMG} ]; then
	rm -f ${BOOTABLE_FLOPPY_IMG}
fi

# create bootable floppy image
echo "${COLOR_GREEN}Creating bootable floppy image...${COLOR_RESET}"
mkfs.msdos -C $BOOTABLE_FLOPPY_IMG 1440
dd if=${BOOTLOADER} of=$BOOTABLE_FLOPPY_IMG bs=512 count=1 conv=notrunc	
sudo mount -o loop $BOOTABLE_FLOPPY_IMG $FLOPPY_MOUNT_DIR
sudo cp ${STAGE2} ${FLOPPY_MOUNT_DIR}/STAGE2.SYS
sudo cp ${KERNEL} ${FLOPPY_MOUNT_DIR}/${KERNEL}
sudo umount ${FLOPPY_MOUNT_DIR}
echo "${COLOR_GREEN}'${BOOTABLE_FLOPPY_IMG}' has been created!${COLOR_RESET}"
