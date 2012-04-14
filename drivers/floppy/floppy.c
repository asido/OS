/******************************************************************************
 *      Floppy driver, Intel 82077AA controller.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <error.h>
#include <time.h>
#include <x86/dma.h>
#include <x86/i8259.h>
#include <x86/cmos.h>
#include <x86/cpu.h>
#include <fs/vfs.h>

/*
 * All floppy control registers.
 * Some of them are not usefull here, but kept just for
 * the sake of completeness.
 */
enum FlpReg
{
    STATUS_REG_A =      0x3F0,  /* read-only */
    STATUS_REG_B =      0x3F1,  /* read-only */
    /* Digital Output Register contains the drive select and
     * motor enable bits, a reset bit and a DMA gate bit */
    DOR_REG =           0x3F2,  /* r/w */
    /* For tape support */
    TAPE_DRIVE_REG =    0x3F3,  /* right... r/w */
    MAIN_STATUS_REG =   0x3F4,  /* read-only */
    DATARATE_SELECT_REG=0x3F4,  /* write-only */
    DATA_REG =          0x3F5,  /* r/w. (FIFO) */
    DIGITAL_INPUT_REG = 0x3F7,  /* read-only */
    CTRL_REG =          0x3F7   /* write-only */
};

/*
 * Commands for writing to DATA_REG (fifo).
 * All comman parameter information and disk data
 * tranfers go through this register.
 */
enum data_cmd
{
    CMD_READ_TRACK =        0x02,  /* generates IRQ6 */
    CMD_SET_PARAM =         0x03,  /* set drive parameters */
    CMD_DRIVE_STATUS =      0x04,
    CMD_WRITE_DATA =        0x05,  /* write data to disk */
    CMD_READ_DATA =         0x06,  /* read data from disk */
    CMD_RECALIBRATE =       0x07,  /* seek to cylinder 0 */
    CMD_SENSE_INTERRUPT =   0x08,  /* ack IRQ6, get status of last cmd */
    CMD_WRITE_DELETED_DATA= 0x09,
    CMD_READ_ID =           0x0A,  /* generatess IRQ6 */
    CMD_READ_DELETED_DATA = 0x0C,
    CMD_FORMAT_TRACK =      0x0D,
    CMD_SEEK =              0x0F,  /* seek both heads to cylinder X */
    CMD_VERSION =           0x10,  /* used on init */
    CMD_SCAN_EQUAL =        0x11,
    CMD_PERPENDICULAR_MODE= 0x12,  /* used on init */
    CMD_CONFIGURE =         0x13,  /* set controller parameters */
    CMD_LOCK =              0x14,  /* protect controller parameters from reset */
    CMD_VERIFY =            0x16,
    CMD_SCAN_LOW_OR_EQUAL = 0x19,
    CMD_SCAN_HIGH_OR_EQUAL= 0x1D,

    /* When read FIFO register, this value indicates that
     * an invalid command was given on the previous write */
    CMD_ERROR = 0x80
};

/*
 * DATA_READ modes.
 */
enum data_read_mode
{
    READ_MODE_SKIP_DELETED_DATA = 0x20,
    READ_MODE_DOUBLE_DENSITY = 0x40, 
    READ_MODE_MULTITRACK = 0x80
};

/*
 * Commands for writing to DOR_REG.
 */
enum dor_cmd
{
    /* Device selection */
    DOR_SEL_0 =     0x00,   /* 00000000 */
    DOR_SEL_1 =     0x01,   /* 00000001 */
    DOR_SEL_2 =     0x02,   /* 00000010 */
    DOR_SEL_3 =     0x03,   /* 00000011 */
    /* Clears the core circuits of 82077AA */
    DOR_RESET =     0x04,   /* 00000100 */
    /* Set floppy to DMA mode */
    DOR_DMA_GATE =  0x08,   /* 00001000 */
    /* Another device selection data.
     * It has to match with DOR_SEL_X */
    DOR_MOTOR_0 =   0x10,   /* 00010000 */
    DOR_MOTOR_1 =   0x20,   /* 00100000 */
    DOR_MOTOR_2 =   0x40,   /* 01000000 */
    DOR_MOTOR_3 =   0x80    /* 10000000 */
};

enum motor_delay
{
    WAIT_MOTOR_SPIN,
    NO_WAIT_MOTOR_SPIN
};

/*
 * Commands for writing to DATARATE_SELECT_REG.
 */
enum dsr_cmd
{
    /* Data tranfer rates */
    DSR_RATE_250KBPS = 0x02,
    DSR_RATE_300KBPS = 0x01,
    DSR_RATE_500KBPS = 0x00,
    DSR_RATE_1MBPS = 0x03,
    /* Shuts the chip off. One of the *_RESET
     * functionality will turn it ON again */
    DSR_PWR_DOWN =  0x40,
    /* This reset is the same as DOR reset except that this
     * reset is self clearing. */
    DSR_RESET =     0x80
};

/*
 * Commands for READING from MAIN_STATUS_REG.
 * XXX: This is read-only register!
 */
enum msr_cmd
{
    /* Is set when drive is in seek or recalibrate states. */
    MSR_BUSY_0 = 0x01,
    MSR_BUSY_1 = 0x02,
    MSR_BUSY_2 = 0x04,
    MSE_BUSY_3 = 0x08,
    /* If set, the CMD execution is in progress */
    MSR_BUSY = 0x10,
    /* If MSR_CAN_TRANSFER is set, indicates the required
     * data transfer direction - 1 for read, 0 for write. */
    MSR_DIR = 0x40,
    /* Indicates that host can tranfser data.
     * If not set - access is not permited. */
    MSR_CAN_TRANSFER = 0x80
};

/*
 * Floppy storage info.
 */
#define SECTOR_SIZE 512
#define TOTAL_SECTORS 2880
#define SECTORS_PER_TRACK 18
#define HEAD_COUNT 2
#define TRACK_COUNT 80
#define GAP_SIZE 0x1B

#define FLOPPY_DMA 2
#define FLOPPY_IRQ 6

#define CAN_TRANSFER_RETRIES 1000
#define RECALIBRATE_RETRIES 80

struct chs_t {
    unsigned int head;
    unsigned int cylinder;
    unsigned int sector;
};

struct floppy_t {
    /* This is the variable flp_wait_irq() is spinning on.
     * When a cmd is given to the controller, it will IRQ6
     * on complete, making the variable ON and the variable */
    int irq_received;
    struct dma_t dma;
    unsigned int drive_nr;
    /* Drive specific DOR and MSR registers */
    enum dor_cmd dor_select_reg;
    enum dor_cmd dor_motor_reg;
    enum msr_cmd msr_busy_bit;
    unsigned char cur_dor;
};

struct floppy_t flp = {
    .irq_received = 0,
    .drive_nr = 0
};

/*
 * Busy loop while we are waiting for the controller to finish
 * what we asked for.
 */
static void flp_wait_irq()
{
    while (!flp.irq_received)
        ;
    flp.irq_received = 0;
}

/*
 * Disables the floppy controller.
 */
static void ctrl_disable()
{
    flp.cur_dor = 0x00;
    outportb(DOR_REG, flp.cur_dor);
}

/*
 * Enables floppy controller.
 */
static void ctrl_enable()
{
    flp.cur_dor = flp.dor_select_reg | DOR_RESET | DOR_DMA_GATE;
    flp.irq_received = 0;
    outportb(DOR_REG, flp.cur_dor);
    flp_wait_irq();
}

/*
 * Turns the motor ON.
 * NOTE: on real hardware this requires delay for the motor
 * to get some speed up.
 */
static void set_motor_on(enum motor_delay delay)
{
    flp.cur_dor |= flp.dor_motor_reg;
    outportb(DOR_REG, flp.cur_dor);

    if (delay == WAIT_MOTOR_SPIN)
        msdelay(300);
}

/*
 * Turns the motor OFF.
 */
static void set_motor_off(enum motor_delay delay)
{
    flp.cur_dor &= ~flp.dor_motor_reg;
    outportb(DOR_REG, flp.cur_dor);

    if (delay == WAIT_MOTOR_SPIN)
        msdelay(2000);
}

/*
 * Sets the speed for data tranfers.
 * On real hardware this should be chosen carefully,
 * just what the hardware is capable to perform.
 */
static void set_transfer_rate(enum dsr_cmd drate)
{
    outportb(DATARATE_SELECT_REG, drate);
}

/*
 * Returns MSR register.
 */
static unsigned char get_flp_status()
{
    return inportb(MAIN_STATUS_REG);
}

/*
 * Returns TRUE if the controller is able to
 * perform our cmd.
 */
static int can_tranfser()
{
    return get_flp_status() & MSR_CAN_TRANSFER;
}

/*
 * Returns true of the controller expects read command.
 */
static int cmd_should_read()
{
    return get_flp_status() & MSR_DIR;
}

/*
 * Returns true of the controller expects write command.
 */
static int cmd_should_write()
{
    return (get_flp_status() & MSR_DIR) == 0;
}

/*
 * Writes data to FIFO register.
 */
static void flp_send_cmd(unsigned char cmd)
{
    int i;

    if (cmd_should_read())
        kernel_warning("floppy should read while write is requested");

    for (i = 0; i < CAN_TRANSFER_RETRIES; i++)
        if (can_tranfser())
        {
            outportb(DATA_REG, cmd & 0xFF);
            break;
        }
        else if (i == CAN_TRANSFER_RETRIES)
            kernel_warning("floppy write cmd reached maximum retries");
}

/*
 * Reads the data from FIFO register.
 */
static int flp_read_cmd()
{
    int i;

    if (cmd_should_write())
        kernel_warning("floppy should write while read is requested");

    for (i = 0; i < CAN_TRANSFER_RETRIES; i++)
        if (can_tranfser())
            return inportb(DATA_REG) & 0xFF;
        else if (i == CAN_TRANSFER_RETRIES)
            kernel_warning("floppy read cmd reached maximum retries");

    return -1;
}

static void flp_recalibrate()
{
    unsigned char st3;

retry:
    flp_send_cmd(CMD_RECALIBRATE);
    flp_send_cmd(flp.drive_nr);

    /* Poll busy register to know when it's done */
    while (get_flp_status() & flp.msr_busy_bit)
        ;

    flp_send_cmd(CMD_SENSE_INTERRUPT);
    st3 = flp_read_cmd();
    flp_read_cmd();
    if ((0x20 | flp.drive_nr) != st3)
        goto retry;
}

/*
 * Initializes the drive itself by specifing mechanical settings.
 */
static void drive_init()
{
    unsigned char data;
    unsigned char step_time, load_time, unload_time;
    step_time = 3;
    load_time = 16;
    unload_time = 240;

    /* Set 500Kbps for 1.44M floppy */
    set_transfer_rate(DSR_RATE_500KBPS);

    flp_send_cmd(CMD_SET_PARAM);
    /*
     * Arguments:
     *  S S S S H H H H - S = Step Rate H = Head Unload Time
     *  H H H H H H H NDM - H = Head Load Time NDM = 0 (DMA Mode) or 1 (DMA Mode)
     */
    data = (step_time & 0x0F) << 4 | (unload_time & 0x0F);
    flp_send_cmd(data);
    data = (load_time << 1) | 1; /* 1st bit enables DMA */
    flp_send_cmd(data);
}

/*
 * Turns the controller ON.
 */
static void ctrl_reset()
{
    flp.irq_received = 0;
    outportb(DATARATE_SELECT_REG, 0x80);
    flp_wait_irq();
    drive_init();
}

/*
 * Checks if enchanted version of floppy controller is present.
 * If not, better to skip floppy susbsystem at all.
 */
static int flp_valid()
{
    flp_send_cmd(CMD_VERSION);
    return flp_read_cmd() == 0x90;
}

/*
 * Recalibrates the drive to point to a given CHS.
 */
static int seek_chs(struct chs_t *chs)
{
    flp_send_cmd(CMD_SEEK);
    flp_send_cmd((chs->head << 2) | flp.drive_nr);
    flp_send_cmd(chs->cylinder);

    /* On real hardware this might take up to 3 seconds */
    flp_wait_irq();
    
    /* Clear BUSY drive flag */
    flp_send_cmd(CMD_SENSE_INTERRUPT);
    flp_read_cmd();
    flp_read_cmd();

    return 0;
}

/*
 * Head     = LBA / SECTORS_PER_TRACK % HEAD_COUNT
 * Cylinder = LBA / (SECTORS_PER_TRACK * HEAD_COUNT)
 * Sector    = (LBA % SECTORS_PER_TRACK) + 1
 */
#define lba_to_chs(struct_chs, lba)     \
    do {    \
        (struct_chs)->head = lba / SECTORS_PER_TRACK % HEAD_COUNT;  \
        (struct_chs)->cylinder = lba / (SECTORS_PER_TRACK * HEAD_COUNT);    \
        (struct_chs)->sector = (lba % SECTORS_PER_TRACK) + 1;    \
    } while (0);

/*
 * Calculates CHS out of a given offset.
 */
static struct chs_t *offset_to_chs(addr_t offset, struct chs_t *chs)
{
    unsigned int lba = offset / SECTOR_SIZE;

    lba_to_chs(chs, lba);
    return chs;
}

/*
 * Performs the actual read from the device.
 * The drive must already be seeked to the correct place.
 * Data is read by DMA and returns a pointer to it's buffer.
 */
static void *do_read_sector(struct chs_t *chs)
{
    size_t i;

    /* Prepare DMA to do it's job */
    dma_set_read(&flp.dma);

    flp.irq_received = 0;
    flp_send_cmd(CMD_READ_DATA | READ_MODE_SKIP_DELETED_DATA |
                 READ_MODE_DOUBLE_DENSITY | READ_MODE_MULTITRACK);
    flp_send_cmd(flp.drive_nr);
    flp_send_cmd(chs->cylinder);
    flp_send_cmd(chs->head);
    flp_send_cmd(chs->sector);
    flp_send_cmd(2);
    flp_send_cmd(2);
    flp_send_cmd(27);
    flp_send_cmd(0xFF);

    flp_wait_irq();

    /* READ_DATA command returns 7 values
     * which we don't really care. */
    for (i = 0; i < 7; i++)
        flp_read_cmd();

    flp_send_cmd(CMD_SENSE_INTERRUPT);
    flp_read_cmd();
    flp_read_cmd();

    return (void *) flp.dma.buf;
}

/*
 * Reads `cnt` amount of data bytes from `dev_loc` in floppy
 * to provided `buf`.
 * The caller is responsible of allocating and de-allocating the buffer.
 */
void *floppy_read(void *buf, addr_t dev_loc, size_t cnt)
{
    struct chs_t chs;
    size_t read; /* keeps track of already read byte count */
    size_t step, offset;

    if (!buf || !cnt)
        return NULL;

    set_motor_on(WAIT_MOTOR_SPIN);

    for (read = step = 0; read < cnt; read += step, dev_loc += step, buf += step)
    {
        offset_to_chs(dev_loc, &chs);
        if (seek_chs(&chs))
        {
            kernel_warning("floppy seek_chs failure");
            return NULL;
        }

        do_read_sector(&chs);
        offset = dev_loc % SECTOR_SIZE;
        step = MIN(SECTOR_SIZE - offset, cnt);
        memcpy(buf, (void *) (flp.dma.buf + offset), step);
    }

    set_motor_off(WAIT_MOTOR_SPIN);

    return buf;
}

/*
 * Initializes the floppy drive.
 */
int floppy_init()
{
    int flp_cmos;

    /* Before anything, validate the floppy controller */
    if (!flp_valid())
    {
        kernel_warning("No enchanted floppy controller detected. "
                       "Floppy initialization aborted.");
        return -1;
    }

    /* Check the drive */
    flp_cmos = cmos_get_flp_status();
    if (!flp_cmos)
    {
        kernel_warning("No floppy drive detected");
        return -1;
    }
    else
    {
        /* Determine the drive number */
        if (CMOS_DISKETTE_TYPE_DRIVE0(flp_cmos) == CMOS_DISKETTE_1M44)
        {
            flp.drive_nr = 0;
            flp.dor_select_reg = DOR_SEL_0;
            flp.dor_motor_reg = DOR_MOTOR_0;
            flp.msr_busy_bit = MSR_BUSY_0;
        }
        else if (CMOS_DISKETTE_TYPE_DRIVE1(flp_cmos) == CMOS_DISKETTE_1M44)
        {
            flp.drive_nr = 1;
            flp.dor_select_reg = DOR_SEL_1;
            flp.dor_motor_reg = DOR_MOTOR_1;
            flp.msr_busy_bit = MSR_BUSY_1;
        }
        else
        {
            kernel_warning("No suitable floppy drive detected");
            return -1;
        }
    }

    dma_struct_init(&flp.dma, 2);
    dma_reg_channel(&flp.dma, SECTORS_PER_TRACK * 512);
    
    ctrl_disable();
    ctrl_enable();
    ctrl_reset();
    set_motor_on(WAIT_MOTOR_SPIN);
    flp_recalibrate();
    set_motor_off(NO_WAIT_MOTOR_SPIN);

    return 0;
}

struct dev_driver *floppy_init_driver(struct dev_driver *driver)
{
    if (!driver)
        return driver;

    driver->read = floppy_read;
    driver->write = NULL; /* TODO */

    return driver;
}

void x86_floppy_irq_do_handle()
{
    flp.irq_received = 1;
    irq_done(IRQ6_VECTOR);
}
