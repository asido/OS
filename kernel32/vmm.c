/******************************************************************************
 *		Virtual Memory Manager
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include "mm.h"

extern void *pmm_alloc(unsigned int bytes);
inline void kernel_panic(char *msg);

#define CR0_ENABLE_PAGING 0x80000000
#define BIOS_PTE_PA 0x9D000
#define BIOS_PTE_VA BIOS_PTE_PA
#define KRNL_VA_BEGIN 0xC0000000
#define KRNL_PA_BEGIN 0x100000
#define KRNL_PDE_PA 0x9C000

#define USR_PDE_BASE 0xC0400000

#define PAGE_SIZE 4096
#define ENTRY_SIZE 4
#define TABLE_ENTRIES 1024
#define DIR_ENTRIES 1024
#define TABLE_SIZE ((ENTRY_SIZE) * (TABLE_ENTRIES))
#define DIR_SIZE ((TABLE_SIZE) * (DIR_ENTRIES))

#define PTE_FLAG_PRESENT	0x1			/* 000000000001 */
#define PTE_FLAG_RW			0x2			/* 000000000010 */
#define PTE_FLAG_SUPERVISOR	0x4			/* 000000000100 */
					 /* RESERVED BY INTEL: 000000011000 */
/* Following flags are set by CPU */
#define PTE_FLAG_PAGE_ACCESSED	0x10	/* 000000100000 */
#define PTE_FLAG_PAGE_DIRTY		0x20	/* 000001000000 */
					 /* RESERVED BY INTEL: 000110000000 */
/* Flags free to use by OS */
#define PTE_FLAG_FREE_USE_0		0x100	/* 001000000000 */
#define PTE_FLAG_FREE_USE_1		0x200	/* 010000000000 */
#define PTE_FLAG_FREE_USE_2		0x400	/* 100000000000 */
										/* Remaining 12-31 bits: */
#define PTE_FRAME_ADDR		0xfffff000	/* 11111111111111111111000000000000 */

struct pde_t {
	addr_t ptes[DIR_ENTRIES];
};

struct pte_t {
	addr_t entries[TABLE_ENTRIES];
};

/* set on vmm_init */
static addr_t KRNL_PTE_VA = 0;
static addr_t KRNL_PTE_PA = 0;

static struct pde_t *_cur_pde;
static int _pde_count;

/* XXX: below macros works on 0xC0000000 - 0xC0400000 VA range only! */
#define KRNL_VA_TO_PA(va)	\
	((va) - (KRNL_VA_BEGIN) + (KRNL_PA_BEGIN))
#define KRNL_PA_TO_VA(pa)	\
	((pa) + (KRNL_VA_BEGIN) - (KRNL_PA_BEGIN))

/* Hardcoded values to not to kill performance */
#define VA_TO_PDE_IDX(mem)	\
	((mem) >> 22)
#define VA_TO_PTE_IDX(mem)	\
	(((mem) << 10) >> 22)

#define PAGE_ALIGN(addr)	\
	((addr) + ((PAGE_SIZE) - ((addr) % (PAGE_SIZE))))

#define entry_add_frame(entry_ptr, p_addr)	\
	do {	\
		*(entry_ptr) = ((p_addr) & (PTE_FRAME_ADDR));	\
	} while (0);

#define entry_add_flag(entry_ptr, flag)	\
	do {	\
		*(entry_ptr) |= flag;	\
	} while (0);

#define entry_rm_flag(entry_ptr, flag)	\
	do {	\
		*(entry_ptr) &= (~flag);	\
	} while (0);


static addr_t *alloc_page(/* addr_t va */)
{
	addr_t *addr = (addr_t *) pmm_alloc(PAGE_SIZE);
	if (!addr)
		kernel_panic("Out of memory");

	/* TODO */
	return addr;
}

static void vmm_load_pde(struct pde_t *pde)
{
	__asm__ __volatile__("movl %0, %%eax \n"
	                     "movl %%eax, %%cr3 \n"
					:
					: "m" (pde)
					: "eax");
}

/*
 * Returns VA to the PTE of provided address
 */
inline static addr_t va_to_pde_mem(addr_t va, addr_t pde)
{
	int pde_idx, pte_idx;

	pde_idx = VA_TO_PDE_IDX(va);
	pte_idx = VA_TO_PTE_IDX(va);
	
	return (addr_t) &((struct pde_t *)pde)->ptes[pde_idx] + pte_idx;
}

/*
 * Maps p_addr to a va in PDE
 */
static int vmm_map_table(addr_t table_addr, addr_t va)
{
	addr_t entry;
	int pde_idx;

	pde_idx = VA_TO_PDE_IDX(va);
	entry_add_frame(&entry, table_addr);
	entry_add_flag(&entry, PTE_FLAG_PRESENT);
	entry_add_flag(&entry, PTE_FLAG_RW);
	_cur_pde->ptes[pde_idx] = entry;

	return 0;
}

/*
 * Initializes VMM by creating PDE with mapped kernel's and BIOS space.
 */
int vmm_init(addr_t krnl_pte_va)
{
	unsigned int offset;
	addr_t addr;

	KRNL_PTE_VA = PAGE_ALIGN(krnl_pte_va);
	KRNL_PTE_PA = KRNL_VA_TO_PA(KRNL_PTE_VA);

	/* Use PDE created in bootloader */
	_cur_pde = (struct pde_t *) KRNL_PDE_PA;
	_pde_count = 0;

	/* clear kernels PTs */
	memset((void *) KRNL_PTE_VA, 0, MB_TO_BYTE(1));

	/* Copy bootloader's prepared 768th table */
	/* offset since we keep last quarter of kernel tables just above loaded kernel */
	offset = DIR_ENTRIES * ENTRY_SIZE * 0.75;
	addr = va_to_pde_mem(KRNL_VA_BEGIN, KRNL_PTE_VA - offset);
	memcpy((void *) addr, (void *) 0x9E000, TABLE_ENTRIES * INT_BYTE);
	vmm_map_table(KRNL_PTE_PA + offset, KRNL_VA_BEGIN);
	vmm_load_pde(_cur_pde);

	return 0;
}

void vmm_enable_paging()
{
	__asm__ __volatile__("mov %%cr0, %%eax \n"
	                     "or %0, %%eax \n"
	                     "mov %%eax, %%cr0 \n"
	                :
					: "n" (CR0_ENABLE_PAGING)
					: "eax");
}

void vmm_disable_paging()
{
	__asm__ __volatile__("mov %%cr0, %%eax \n"
	                     "and %0, %%eax \n"
	                     "mov %%eax, %%cr0 \n"
	                :
					: "n" (~CR0_ENABLE_PAGING)
					: "eax");
}
