/******************************************************************************
 *       Callback subsystem.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef CALLBACK_1JKP8FFQ
#define CALLBACK_1JKP8FFQ

#include <time.h>
#include <linklist.h>

enum cb_type {
    CALLBACK_ONE_SHOT,   /* fires once and gets deleted from the list */
    CALLBACK_REPEAT      /* keeps firing until manually removed */
};

typedef void cb_func_t(void *);

struct callback_t {
    struct llist_t ll;
    enum cb_type type;
    milis_t reg_time;
    milis_t delay;
    cb_func_t *callback;
};

int register_callback(enum cb_type type,
        struct time_t *delay, cb_func_t *callback);
int remove_callback(struct callback_t *cb);
void check_callbacks();

#endif /* end of include guard: CALLBACK_1JKP8FFQ */
