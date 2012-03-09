#ifndef FLOPPY_KT71TBTQ
#define FLOPPY_KT71TBTQ

int floppy_init();
void *floppy_read(void *buf, addr_t dev_loc, size_t cnt);

#endif /* end of include guard: FLOPPY_KT71TBTQ */
