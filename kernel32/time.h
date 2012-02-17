/******************************************************************************
 *		Time
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef TIME_Q0HBOH0O
#define TIME_Q0HBOH0O

#include <x86/cmos.h>

struct time_t {
	char hour;
	char min;
	char sec;
};

#define RTC_TO_SEC(t)	\
	((t) & 0xFF)
#define RTC_TO_MIN(t)	\
	((t) >> 8)
#define RTC_TO_HOUR(t)	\
	((t) >> 16)

inline struct time_t *get_time(struct time_t *t)
{
	int time = rtc_get_time();

	t->hour = RTC_TO_HOUR(time);
	t->min = RTC_TO_MIN(time);
	t->sec = RTC_TO_SEC(time);

	return t;
}

#endif /* end of include guard: TIME_Q0HBOH0O */
