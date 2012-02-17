/******************************************************************************
 *		Time
 *
 *			Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef TIME_Q0HBOH0O
#define TIME_Q0HBOH0O

#include <libc.h>
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

inline struct time_t *get_time(struct time_t *t);
inline void update_clock();

#endif /* end of include guard: TIME_Q0HBOH0O */
