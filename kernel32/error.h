/******************************************************************************
 *       Error routines.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef ERROR_M6JXQGC2
#define ERROR_M6JXQGC2

/* Error codes */
#define ENOMEM 1    /* out of memory */
#define EBADADDR 2  /* bad address */
#define EBADARG 3   /* bad argument */
#define EFAULT 4    /* unexpected behaviour */
#define ESIZE 5     /* entity too large/small */

extern int error;

#define kernel_debug(msg)   \
    _kernel_debug(__FILE__, __LINE__, msg)

inline void kernel_panic(char *msg);
void kernel_warning(char *msg);
void _kernel_debug(const char *file, unsigned int line, char *msg);

#endif /* end of include guard: ERROR_M6JXQGC2 */
