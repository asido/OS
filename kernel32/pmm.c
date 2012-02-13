/******************************************************************************
 *		Physical Memory Manager
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>

#define BLOCK_SIZE 4096 /* same size as VMM block size */
#define BITMAP_BIT_CNT CHAR_BIT

/* conversion macros */
#define MEM_TO_BLOCK_IDX(b)	\
	((b) / BLOCK_SIZE)
#define MEM_TO_BLOCK_OFFSET(b)	\
	((b) % BLOCK_SIZE )
#define SIZE_KB_TO_BLOCKS(kb)	\
	((kb) * 1024 / BLOCK_SIZE)
#define SIZE_B_TO_BLOCKS(b)	\
	((b) / BLOCK_SIZE)

#define BLOCK_IDX_TO_BITMAP_IDX(b_idx)	\
	((b_idx) / BITMAP_BIT_CNT)
#define BLOCK_IDX_TO_BIT_OFFSET(b_idx)	\
	((b_idx) % BITMAP_BIT_CNT)

#define BLOCK_TO_MEM(idx)	\
	((idx) * BLOCK_SIZE)

#define UNSET_BIT(val, bit)	\
	((val) & (~(1 << (bit))))
#define SET_BIT(val, bit)	\
	((val) | (1 << (bit)))
#define GET_BIT(val, bit)	\
	(((val) & (1 << (bit))) >> (bit))
#define IS_BIT_FREE(val, bit)	\
	(GET_BIT(val, bit) == 0)

enum ALIGN {
	ALIGN_LOW,
	ALIGN_HIGH
};

struct pmm_t {
	unsigned int block_cnt;
	unsigned int blocks_free;
};

static struct pmm_t pmm;
static unsigned char mem_bitmap[UINT_MAX / BLOCK_SIZE / sizeof(char) / BITMAP_BIT_CNT];

/*
 * Initializes PMM.
 * Returns 0 on success.
 */
int pmm_init(unsigned int mem_kb)
{
	/* on init set all memory as reservered */
	memset(mem_bitmap, 0xFF, sizeof(mem_bitmap));
	pmm.block_cnt = mem_kb * 1024 / BLOCK_SIZE;
	pmm.blocks_free = 0;

	return 0;
}

static void set_bit(size_t idx)
{
	size_t map_idx, bit_idx;

	map_idx = BLOCK_IDX_TO_BITMAP_IDX(idx);
	bit_idx = BLOCK_IDX_TO_BIT_OFFSET(idx);

	mem_bitmap[map_idx] = SET_BIT(mem_bitmap[map_idx], bit_idx);
}

static void unset_bit(size_t idx)
{
	size_t map_idx, bit_idx;

	map_idx = BLOCK_IDX_TO_BITMAP_IDX(idx);
	bit_idx = BLOCK_IDX_TO_BIT_OFFSET(idx);

	mem_bitmap[map_idx] = UNSET_BIT(mem_bitmap[map_idx], bit_idx);
}

inline static int is_map_idx_full(unsigned int idx)
{
	return mem_bitmap[idx] == 0xFF;
}

static unsigned int get_free_bit(char val)
{
	int i;

	for (i = 0; i < BITMAP_BIT_CNT; i++)
		if (IS_BIT_FREE(val, i))
			return i;
	return -1;
}

static unsigned int find_free_block()
{
	size_t i;

	if (!pmm.blocks_free)
		return -1;

	for (i = 0; i < pmm.block_cnt; i++)
		if (!is_map_idx_full(i))
		{
			int free_bit = get_free_bit(mem_bitmap[i]);
			if (free_bit < 0)
				return -1;
			return (i * BITMAP_BIT_CNT) + free_bit;
		}

	return -2;
}

/*
 * Allocated `size` of blocks starting from `start`
 * Returns negative on error, allocated size on success.
 */
unsigned int pmm_alloc()
{
	int idx;

	if (!pmm.blocks_free)
		return -1;

	idx = find_free_block();
	if (idx <= 0)
		return -2;

	set_bit(idx);
	pmm.blocks_free--;

	return BLOCK_TO_MEM(idx);
}

int pmm_dealloc(unsigned int addr)
{
	size_t idx = MEM_TO_BLOCK_IDX(addr);
	if (idx > pmm.block_cnt)
		return -1;
	unset_bit(idx);
	pmm.blocks_free++;
	return 0;
}

/*
 * Registers a chunk of memory as usable
 * NOTE: BLOCK_SIZE aligned
 * NOTE2: everything what exceeds physical memory address is silently ignored.
 */
int pmm_init_region(unsigned int addr, size_t size)
{
	size_t i, block_cnt;
	unsigned int block_idx;
		
	block_idx = MEM_TO_BLOCK_IDX(addr);
	block_cnt = SIZE_B_TO_BLOCKS(size);

	for (i = 0;
         (i < block_cnt) || (block_idx + i < pmm.block_cnt);
         i++, addr += BLOCK_SIZE)
		pmm_dealloc(addr);
	return i;
}


