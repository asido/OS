/******************************************************************************
 *      Linked list implementation
 *
 *      The list is round - last member has pointer to the first
 *      and the first has a pointer to the last.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef LINKLIST_OUJCFBOR
#define LINKLIST_OUJCFBOR

struct llist_t {
    struct llist_t *next;
    struct llist_t *prev;
};

/*
 * Initializes a new linked list.
 */
#define llist_init(ptr, ll_field)  \
    do {    \
        /* since the list is empty, everything points to itself */  \
        (ptr)->ll_field.next = ((struct llist_t *) (ptr));     \
        (ptr)->ll_field.prev = ((struct llist_t *) (ptr));     \
    } while (0)

/*
 * Adds `data` before `ll_memb` in the linked list.
 */
#define llist_add_before(ll_memb, data, ll_field)  \
    do {    \
        /* set new members ll */   \
        (data)->ll_field.prev = (ll_memb)->ll_field.prev;       \
        (data)->ll_field.next = &(ll_memb)->ll_field;       \
        /* merge it before a given `ll_memb` */     \
        (ll_memb)->ll_field.prev->next = &(data)->ll_field;    \
        (ll_memb)->ll_field.prev = &(data)->ll_field;  \
    } while (0)

/*
 * Retrieves the next element.
 */
#define llist_next(ll_memb, ll_field) \
    ((typeof(ll_memb)) (ll_memb)->ll_field.next)

/*
 * Retrieves previous element.
 */
#define llist_prev(ll_memb, ll_field) \
    ((typeof(ll_memb)) (ll_memb)->ll_field.prev)

/*
 * Returns true if `list` has 1 member only.
 */
#define llist_is_empty(list, ll_field)  \
    (!(list)->ll_field.next || !(list)->ll_field.prev)

/*
 * Deletes a member from the list.
 */
#define llist_delete(memb, ll_field)    \
    do {    \
        (memb)->ll_field.prev->next = (memb)->ll_field.next;     \
        (memb)->ll_field.next->prev = (memb)->ll_field.prev;     \
        (memb)->ll_field.next = NULL;    \
        (memb)->ll_field.prev = NULL;    \
    } while (0)

/*
 * Returns true if `memb` bellongs to any list.
 */
#define llist_is_in_list(memb, ll_field)    \
    ((memb)->ll_field.next != NULL && (memb)->ll_field.prev != NULL)

/*
 * Macro to loop through whole linked list.
 *  list    - starting point
 *  entry   - struct pointer which will hold each looped entry
 *  counter - counter which gets incremented with every loop
 *  ll_field- name of linked list variable
 */
#define llist_foreach(list, entry, counter, ll_field)   \
    for ((entry) = ((typeof((entry))) (list)), counter = 0;  \
         (entry) != (list) || counter == 0; \
         (entry) = ((typeof((entry))) (entry)->ll_field.next), counter++)

#endif /* end of include guard: LINKLIST_OUJCFBOR */
