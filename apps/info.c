#include <libc.h>
#include <mm.h>

int info_main(int argc, const char *argv[])
{

    printf("Total memory:      %d\n", get_total_mem_b());
    printf("Available memory:  %d\n", get_free_mem_b());
    printf("Used memory:       %d\n", get_used_mem_b());
    printf("Kernel size:       %d\n", get_krnl_size());

    return 0;
}
