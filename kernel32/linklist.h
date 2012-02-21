/******************************************************************************
 *      Linked list implementation
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef LINKLIST_OUJCFBOR
#define LINKLIST_OUJCFBOR

#include "mm.h"

struct llist_t {
    llist_t *begin;
    llist_t *next;
    llist_t *prev;
};

inline void llist_init(llist_t *list)
{
}
    

#endif /* end of include guard: LINKLIST_OUJCFBOR */
