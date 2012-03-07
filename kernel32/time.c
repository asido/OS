/******************************************************************************
 *      Time related routines.
 *
 *      The init time of the OS is the current time. That's the starting point.
 *      Then each 24h rollover adds 1 day to it. So generally speaking a
 *      function get_cur_milis() will return you a milliseconds since 00:00
 *      of the day a machine was booted.
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <x86/i8253.h>
#include "time.h"

/* Conversion macros */
#define SEC_TO_MILIS(sec)   \
    ((sec) * 1000)
#define MIN_TO_MILIS(min)   \
    (SEC_TO_MILIS((min) * 60))
#define HOUR_TO_MILIS(hour) \
    (MIN_TO_MILIS((hour) * 60))
#define DAY_TO_MILIS(day)   \
    (HOUR_TO_MILIS((day) * 24))

struct time_t hw_time;

/*
 * Retrieves correct time from CMOS.
 */
inline static struct time_t *update_clock_hw(struct time_t *t)
{
    int time = rtc_get_time();

    t->day = 0;
    t->hour = RTC_TO_HOUR(time);
    t->min = RTC_TO_MIN(time);
    t->sec = RTC_TO_SEC(time);
    t->mm = 0;

    return t;
}

/*
 * Updates the clock.
 * Should be called by every PIT tick.
 */
inline void update_clock_pit(unsigned int pit_hz)
{
    /*
     * The clock update step.
     * 1000 Hz PIT will add 1mm on every update,
     * 100Hz PIT will add 10mm and so on.
     */
    struct time_t step;
    step.mm = 1000 / pit_hz;
    step.sec = step.min = step.hour = step.day = 0;

    time_add_time(&hw_time, &step);
}

static void check_time_overflow(struct time_t *t)
{
    size_t step;

    if (t->mm >= 1000)
    {
        step = t->mm / 1000;
        t->mm %= 1000;
        t->sec += step;

        if (t->sec >= 60)
        {
            step = t->sec / 60;
            t->sec %= 60;
            t->min += step;

            if (t->min >= 60)
            {
                step = t->min / 60;
                t->min %= 60;
                t->hour += step;

                if (t->hour >= 24)
                {
                    step = t->hour / 24;
                    t->hour %= 24;
                    t->day += step;
                }
            }
        }
    }
}

/*
 * Adds `t2` to `t1`.
 */
struct time_t *time_add_time(struct time_t *t1, struct time_t *t2)
{
    /* Do the addition */
    t1->mm += t2->mm;
    t1->sec += t2->sec;
    t1->min += t2->min;
    t1->hour += t2->hour;
    t1->day += t2->day;

    /* handle overflows */
    check_time_overflow(t1);

    return t1;
}

/*
 * Sets `t` to the current time.
 */
struct time_t *get_cur_time(struct time_t *t)
{
    t->hour = hw_time.hour;
    t->min = hw_time.min;
    t->sec = hw_time.sec;
    t->mm = hw_time.mm;

    return t;
}

/*
 * Converts struct time_t to milliseconds
 */
milis_t time_to_milis(struct time_t *t)
{
    milis_t milis = 0;

    milis += DAY_TO_MILIS(t->day);
    milis += HOUR_TO_MILIS(t->hour);
    milis += MIN_TO_MILIS(t->min);
    milis += SEC_TO_MILIS(t->sec);
    milis += t->mm;

    return milis;
}

milis_t get_cur_milis()
{
    return time_to_milis(&hw_time);
}

void clock_init()
{
    update_clock_hw(&hw_time);
}

/*
 * Delay routine which busy loops for a given number of milliseconds.
 */
void msdelay(unsigned int delay)
{
    unsigned long long start_pit, end_pit, gap;

    start_pit = pit_jiffy;
    gap = delay / (1000 / PIT_HZ);
    end_pit = start_pit + gap;

    while (pit_jiffy < end_pit)
        ;
}
