/******************************************************************************
 *      Direct Memory Address, Intel 8237A controller.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include <mm.h>
#include <error.h>
#include "dma.h"

#define DMA_TOTAL_CHANNELS 7
#define MASTER_MAX_COUNT (KB_TO_BYTE(128))
#define SLAVE_MAX_COUNT (KB_TO_BYTE(64))

/*
 * DMA ports
 */
/* Master 0-3 channels */
#define DMA0_STATUS_REG 0x08    /* read-only: status register */
#define DMA0_CMD_REG 0x08       /* write-only: command register */
#define DMA0_REQ_REG 0x09       /* write-only: request register */
#define DMA0_MASK_REG 0x0A      /* write-only: single channel mask register */
#define DMA0_MODE_REG 0x0B      /* write-only: mode register */
#define DMA0_FLIPFLOP_REG 0x0C  /* write-only: flip-flop reset register */
#define DMA0_INTERM_REG 0x0D    /* read-only: intermediate register */
#define DMA0_MASTER_CLR_REG 0x0D /* write-only: master reset register */
#define DMA0_MASK_CLR_REG 0x0E  /* write-only: mask reset register */
#define DMA0_MASK_ALL_REG 0x0F      /* write-only: multichannel mask register */
/* Slave 4-7 channels (4 is reserved for connection with slave) */
#define DMA1_STATUS_REG 0xD0    /* read-only: status register */
#define DMA1_CMD_REG 0xD0       /* write-only: command register */
#define DMA1_REQ_REG 0xD2       /* write-only: request register */
#define DMA1_MASK_REG 0xD4      /* write-only: single channel mask register */
#define DMA1_MODE_REG 0xD6      /* write-only: mode register */
#define DMA1_FLIPFLOP_REG 0xD8  /* write-only: flip-flop reset register */
#define DMA1_INTERM_REG 0xDA    /* read-only: intermediate register */
#define DMA1_MASTER_CLR_REG 0xDA /* write-only: master reset register */
#define DMA1_MASK_CLR_REG 0xDC  /* write-only: mask reset register */
#define DMA1_MASK_ALL_REG 0xDE      /* write-only: multichannel mask register */

/*
 * DMA address registers
 */
#define DMA_ADDR0_REG 0x00
#define DMA_ADDR1_REG 0x02
#define DMA_ADDR2_REG 0x04
#define DMA_ADDR3_REG 0x06
#define DMA_ADDR4_REG 0xC0
#define DMA_ADDR5_REG 0xC4
#define DMA_ADDR6_REG 0xC8
#define DMA_ADDR7_REG 0xCC

/*
 * DMA counter registers
 */
#define DMA_CNT0_REG 0x01
#define DMA_CNT1_REG 0x03
#define DMA_CNT2_REG 0x05
#define DMA_CNT3_REG 0x07
#define DMA_CNT4_REG 0xC2
#define DMA_CNT5_REG 0xC6
#define DMA_CNT6_REG 0xCA
#define DMA_CNT7_REG 0xCE

/*
 * DMA extended page address registers
 */
#define DMA_PAGE0_REG 0x87
#define DMA_PAGE1_REG 0x83
#define DMA_PAGE2_REG 0x81
#define DMA_PAGE3_REG 0x82
/* Channel 4 is used to cascade the DMAs - nothing should ever write to it
 * but for the sake of completness, its port is 0x8F */
#define DMA_PAGE5_REG 0x8B
#define DMA_PAGE6_REG 0x89
#define DMA_PAGE7_REG 0x8A

/*
 * DMA mode masks
 */
enum dma_mode_mask
{
    DMA_MODE_MASK_SEL = 0x03,       /* 00000011 */
    DMA_MODE_MASK_TRA = 0x0C,       /* 00001100 */
    DMA_MODE_MASK_AUTO_INIT = 0x10, /* 00010000 */
    DMA_MODE_MASK_DOWN = 0x20,      /* 00100000 */
    DMA_MODE_MASK_MODE = 0xC0       /* 11000000 */
};

/*
 * DMA modes
 */
enum dma_mode
{
    /* Channel selection */
    DMA_MODE_CHAN0 = 0x00,
    DMA_MODE_CHAN1 = 0x01,
    DMA_MODE_CHAN2 = 0x02,
    DMA_MODE_CHAN3 = 0x03,
    /* Tranfser type */
    DMA_MODE_WRITE = 0x04,
    DMA_MODE_READ = 0x08,
    /* Automatic re-initialize after tranfser completes */
    DMA_MODE_AUTO_INIT = 0x10,
    /* Reverses the memory order of the data. Memory is accessed from
     * hight to low addresses (the address is decremented between
     * each transfer). */
    DMA_MODE_DOWN = 0x20,
    /* DMA operating modes */
    DMA_MODE_ON_DEMAND = 0x00,
    DMA_MODE_SINGLE = 0x40,
    DMA_MODE_BLOCK = 0x80,
    DMA_MODE_CASCADE = 0xC0
};

static const unsigned int DMA_BUFFER_START = 0x1000;

#define channel_to_dma(CHAN)  \
    ((CHAN) <= 3 && (CHAN) >= 0 ? (DMA_MASTER) : (DMA_SLAVE))

/*
 * Initializes provided struct dma_t with the correct ports
 * for a given channel.
 */
int dma_struct_init(struct dma_t *dma, unsigned char channel)
{
    if (!dma)
        return -EBADARG;
    if (channel > DMA_TOTAL_CHANNELS)
        return -EBADARG;

    dma->channel = channel;
    dma->_dma = channel_to_dma(channel);
    if (dma->_dma == DMA_MASTER)
    {
        dma->max_buf_sz = MASTER_MAX_COUNT;
        dma->buf = DMA_BUFFER_START + (dma->channel * dma->max_buf_sz);
        dma->cmd_reg = DMA0_CMD_REG;
        dma->flipflop_reg = DMA0_FLIPFLOP_REG;
        dma->interm_reg = DMA0_INTERM_REG;
        dma->mask_clr_reg = DMA0_MASK_CLR_REG;
        dma->mask_reg = DMA0_MASK_REG;
        dma->master_clr_reg = DMA0_MASTER_CLR_REG;
        dma->mode_reg = DMA0_MODE_REG;
        dma->req_reg = DMA0_REQ_REG;
        dma->status_reg = DMA0_STATUS_REG;
    }
    else if (dma->_dma == DMA_SLAVE)
    {
        dma->max_buf_sz = SLAVE_MAX_COUNT;
        dma->buf = DMA_BUFFER_START + (dma->channel * dma->max_buf_sz);
        dma->cmd_reg = DMA1_CMD_REG;
        dma->flipflop_reg = DMA1_FLIPFLOP_REG;
        dma->interm_reg = DMA1_INTERM_REG;
        dma->mask_clr_reg = DMA1_MASK_CLR_REG;
        dma->mask_reg = DMA1_MASK_REG;
        dma->master_clr_reg = DMA1_MASTER_CLR_REG;
        dma->mode_reg = DMA1_MODE_REG;
        dma->req_reg = DMA1_REQ_REG;
        dma->status_reg = DMA1_STATUS_REG;
    }

    switch (dma->channel) {
    #define CHANNEL_REGS(CHAN)  \
        case CHAN: {    \
            dma->addr_reg = DMA_ADDR##CHAN##_REG;   \
            dma->cnt_reg = DMA_CNT##CHAN##_REG;     \
            dma->page_reg = DMA_PAGE##CHAN##_REG;   \
            break;  \
        }

        CHANNEL_REGS(0);
        CHANNEL_REGS(1);
        CHANNEL_REGS(2);
        CHANNEL_REGS(3);
        /* Ignore case 4 - cascade channel */
        CHANNEL_REGS(5);
        CHANNEL_REGS(6);
        CHANNEL_REGS(7);
    #undef CHANNEL_REGS
    default:
        return -EFAULT;
    };

    return 0;
}

/*
 * Enables DMA channel.
 */
static inline void dma_enable_channel(struct dma_t *dma)
{
    if (!dma)
        return;

    /* The first two bits is channel selector.
     * The third bit 0 - enables, 1 - disables it */
    outportb(dma->mask_reg, dma->channel & 3);
}

/*
 * Disables DMA channel.
 * This should be done before every try to re-program it.
 */
static inline void dma_disable_channel(struct dma_t *dma)
{
    if (!dma)
        return;

    outportb(dma->mask_reg, dma->channel | 4);
}

/*
 * Clears DMA flip-flop.
 * DMA is working with 16 bit data on an 8 bit data bus,
 * so flip-flop tells which, high or low bits, we refer to.
 */
static inline void dma_clear_ff(struct dma_t *dma)
{
    if (!dma)
        return;

    outportb(dma->flipflop_reg, 0xFF);
}

static int dma_set_page(struct dma_t *dma, addr_t addr)
{
    /* DMA can reference up to 16MB physical memory */
    if (addr >= MB_TO_BYTE(16))
        return -ESIZE;

    dma_clear_ff(dma);
    outportb(dma->page_reg, (addr >> 16) & 0xFF);
}

static int dma_set_addr(struct dma_t *dma, addr_t addr)
{
    /* DMA can reference up to 16MB physical memory */
    if (addr >= MB_TO_BYTE(16))
        return -ESIZE;

    dma_clear_ff(dma);
    outportb(dma->addr_reg, addr & 0xFF);
    outportb(dma->addr_reg, (addr >> 8) & 0xFF);
    dma_set_page(dma, addr);

    return 0;
}

static int dma_set_cnt(struct dma_t *dma, size_t cnt)
{
    if (cnt < 1)
        return -ESIZE;
    if (cnt >= dma->max_buf_sz)
        return -ESIZE;

    /* count always will be 1 more than requested.
     * setting 100 will make tranfer 101 - normalize it */
    dma->buf_sz = cnt - 1;

    dma_clear_ff(dma);
    outportb(dma->cnt_reg, dma->buf_sz & 0xFF);
    outportb(dma->cnt_reg, (dma->buf_sz >> 8) & 0xFF);
}

/*
 * Initializes specific DMA channel.
 * channel - specifies which DMA channel should be selected for the operation.
 * cnt     - specifies the size of data a channel will be dealing with.
 */
int dma_reg_channel(struct dma_t *dma, size_t cnt)
{
    if (!dma)
        return -EBADARG;

    /* Keep the channel disabled while re-programming it */
    dma_disable_channel(dma);
    dma_set_addr(dma, dma->buf);
    dma_set_cnt(dma, cnt);
    dma_enable_channel(dma);
}

/*
 * Initializes DMA microcontrllers.
 */
int dma_init()
{
    int i;
    struct dma_t dma;

    /* Disable all channels on startup */
    for (i = 0; i < DMA_TOTAL_CHANNELS; i++)
    {
        /* skip 4th channels - it is used for cascade */
        if (i == 4)
            continue;

        if (dma_struct_init(&dma, i))
            return -EFAULT;
        dma_disable_channel(&dma);
    }

    return 0;
}

/*
 * Prepare a DMA channel for read operation.
 */
void dma_set_read(struct dma_t *dma)
{
    dma_disable_channel(dma);
    outportb(dma->mode_reg, DMA_MODE_SINGLE | DMA_MODE_AUTO_INIT |
                            DMA_MODE_CHAN2 | DMA_MODE_WRITE);
    dma_enable_channel(dma);
}

/*
 * Prepare DMA channel for write to memory operation.
 */
void dma_set_write(struct dma_t *dma)
{
    dma_disable_channel(dma);
    outportb(dma->mode_reg, DMA_MODE_SINGLE | DMA_MODE_AUTO_INIT |
                            DMA_MODE_CHAN2 | DMA_MODE_READ);
    dma_enable_channel(dma);
}
