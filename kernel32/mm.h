#ifndef MM_ZPVRK7R1
#define MM_ZPVRK7R1

#include <libc.h>

struct boot_info {
    unsigned int mem_size;
    unsigned int krnl_size;
    unsigned int krnl_loc;
} __attribute__((__packed__));

/* PMM */
addr_t pmm_init(unsigned int mem_kb, addr_t bitmap_loc);
int pmm_init_region(unsigned int addr, size_t size);
extern void *pmm_alloc(unsigned int bytes);
size_t get_total_mem_b();
size_t get_free_mem_b();
size_t get_used_mem_b();
size_t get_krnl_size();

/* VMM */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end);
void free(void *ptr);
void *kalloc(size_t bytes);
void *malloc(size_t bytes);

#endif /* end of include guard: MM_ZPVRK7R1 */
