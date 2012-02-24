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

inline void kernel_panic(char *msg);

#endif /* end of include guard: ERROR_M6JXQGC2 */
