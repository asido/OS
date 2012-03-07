/******************************************************************************
 *      Direct Memory Address, Intel 8237A controller.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef DMA_KPFGLEYD
#define DMA_KPFGLEYD

#include <mm.h>

/* 
 * Unique identifier for each DMA
 */
enum dma
{
    DMA_MASTER,
    DMA_SLAVE
};

struct dma_t
{
    enum dma _dma;
    unsigned char channel;
    addr_t buf;
    addr_t buf_sz;
    size_t max_buf_sz;
    unsigned char status_reg;
    unsigned char addr_reg;
    unsigned char page_reg;
    unsigned char cnt_reg;
    unsigned char cmd_reg;
    unsigned char req_reg;
    unsigned char mask_reg;
    unsigned char mode_reg;
    unsigned char flipflop_reg;
    unsigned char interm_reg;
    unsigned char master_clr_reg;
    unsigned char mask_clr_reg;
};

int dma_struct_init(struct dma_t *dma, unsigned char channel);
int dma_reg_channel(struct dma_t *dma, size_t cnt);
int dma_init();
void dma_set_write(struct dma_t *dma);
void dma_set_read(struct dma_t *dma);

#endif /* end of include guard: DMA_KPFGLEYD */
