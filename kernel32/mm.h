#ifndef MM_ZPVRK7R1
#define MM_ZPVRK7R1

typedef unsigned int addr_t;

extern void *pmm_alloc(unsigned int bytes);
int vmm_init(size_t mem_kb, addr_t krnl_bin_end);

#endif /* end of include guard: MM_ZPVRK7R1 */
