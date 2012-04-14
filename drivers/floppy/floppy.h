#ifndef FLOPPY_KT71TBTQ
#define FLOPPY_KT71TBTQ

#include <mm.h>

int floppy_init();
void *floppy_read(void *buf, addr_t dev_loc, size_t cnt);
struct dev_driver *floppy_init_driver(struct dev_driver *driver);

#endif /* end of include guard: FLOPPY_KT71TBTQ */
