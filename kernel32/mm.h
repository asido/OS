#ifndef MM_ZPVRK7R1
#define MM_ZPVRK7R1

typedef unsigned int addr_t;

/* PMM */
addr_t pmm_init(unsigned int mem_kb, addr_t bitmap_loc);
int pmm_init_region(unsigned int addr, size_t size);
extern void *pmm_alloc(unsigned int bytes);

/* VMM */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end);
void free(void *ptr);
void *kalloc(size_t bytes);
void *malloc(size_t bytes);

#endif /* end of include guard: MM_ZPVRK7R1 */
