/******************************************************************************
 *      Time
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include "time.h"

inline struct time_t *get_time(struct time_t *t)
{
    int time = rtc_get_time();

    t->hour = RTC_TO_HOUR(time);
    t->min = RTC_TO_MIN(time);
    t->sec = RTC_TO_SEC(time);

    return t;
}

inline void update_clock()
{
    struct time_t t;

    get_time(&t);
    cursor_save();
    goto_xy(72, 0);
    printf("%d:%d:%d", t.hour, t.min, t.sec);
    cursor_load();
}
