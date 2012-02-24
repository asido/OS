/******************************************************************************
 *      Linked list implementation
 *
 *  NOTE: struct llist_t should be the first member of the structure.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef LINKLIST_OUJCFBOR
#define LINKLIST_OUJCFBOR

#include "mm.h"

struct llist_t {
    struct llist_t *begin;
    struct llist_t *next;
    struct llist_t *prev;
};

/*
 * Initializes a new linked list.
 */
#define llist_init(ptr, ll_field)  \
    do {    \
        /* since the list is empty, everything points to itself */  \
        (ptr)->ll_field.begin = ((struct llist_t *) (ptr));    \
        (ptr)->ll_field.next = ((struct llist_t *) (ptr));     \
        (ptr)->ll_field.prev = ((struct llist_t *) (ptr));     \
    } while (0);

/*
 * Adds `data` before `ll_memb` in the linked list.
 */
#define llist_add_before(ll_memb, data, ll_field)  \
    do {    \
        /* set new members ll */   \
        (data)->ll_field.begin = (ll_memb)->ll_field.begin;     \
        (data)->ll_field.prev = (ll_memb)->ll_field.prev;       \
        (data)->ll_field.next = &(ll_memb)->ll_field;       \
        /* merge it before a given `ll_memb` */     \
        (ll_memb)->ll_field.prev->next = &(data)->ll_field;    \
        (ll_memb)->ll_field.prev = &(data)->ll_field;  \
    } while (0);

#endif /* end of include guard: LINKLIST_OUJCFBOR */
