/******************************************************************************
 *      CMOS driver
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <libc.h>
#include "cmos.h"
#include "cpu.h"

#define CMOS_INDEX_PORT 0x70
#define CMOS_DATA_PORT 0x71

#define CMOS_NMI_BIT 0x40

#define NMI_ENABLE(data)    \
    ((data) | (CMOS_NMI_BIT))
#define NMI_DISABLE(data)   \
    ((data) & (~(CMOS_NMI_BIT)))

/* these are in BCD */
#define RTC_CUR_SEC 0x0
#define RTC_ALARM_SEC 0x1
#define RTC_CUR_MIN 0x2
#define RTC_ALARM_MIN 0x3
#define RTC_CUR_HOUR 0x4
#define RTC_ALARM_HOUR 0x5
#define RTC_DAY_OF_WEEK 0x6
#define RTC_DAY_OF_MONTH 0x7
#define RTC_MONTH 0x8
#define RTC_YEAR 0x9

#define STATUS_REG_A 0xA
#define STATUS_REG_A_UPDATE_IN_PROGRESS 0x80
#define STATUS_REG_B 0xB
#define STATUS_REG_C 0xC
#define STATUS_REG_D 0xD

#define CMOS_DIAGNOSTIC_STATUS 0xE
#define CMOS_DIAGNOSTIC_RTC_LOST_POWER 0x80

#define CMOS_SHUTDOWN_STATUS 0xF

#define BCD_TO_INT(bcd) \
    (((((bcd) & 0xF0) >> 4) * 10) + ((bcd) & 0xF))

/*
 * CMOS OUT port
 */
static int cmos_select_ram(int idx)
{
    outportb(CMOS_INDEX_PORT, idx);
    return 0;
}

/*
 * CMOS IN port
 */
inline static char cmos_read_ram()
{
    return inportb(CMOS_DATA_PORT);
}

inline static int cmos_write_ram(int data)
{
    outportb(CMOS_DATA_PORT, data);
    return 0;
}

/*
 * Returns 1 if RTC update is in progress.
 * In such case returned time might be incorrect.
 */
static int rtc_in_update()
{
    int ram = NMI_DISABLE(STATUS_REG_A);

    cmos_select_ram(ram);
    char res = cmos_read_ram();

    return (res & STATUS_REG_A_UPDATE_IN_PROGRESS);
}

/* 
 * Checks if CMOS has any problems.
 * Returns 0 on pass.
 */
static int cmos_diagnostic()
{
    char ram;

    ram = CMOS_DIAGNOSTIC_STATUS;
    ram = NMI_DISABLE(ram);
    cmos_select_ram(ram);

    return cmos_read_ram();
}

/*
 * Returns the number of floppy drives attached to the PC.
 */
int cmos_get_flp_status()
{
    char ram;

    ram = NMI_DISABLE(CMOS_DISKETTE);
    cmos_write_ram(ram);
    cmos_select_ram(ram);
    return cmos_read_ram();
}

#define return_time(time)   \
    do {    \
        int ram = NMI_DISABLE(time);    \
        cmos_select_ram(ram);   \
        return BCD_TO_INT(cmos_read_ram()); \
    } while(0);

static int rtc_get_sec()
{
    return_time(RTC_CUR_SEC);
}
static int rtc_get_min()
{
    return_time(RTC_CUR_MIN);
}
static int rtc_get_hour()
{
    return_time(RTC_CUR_HOUR);
}
#undef return_time

/*
 * Returns time in an integer. Do shifting to extract specific values.
 * Bytes:
 *      0 - 7:  seconds
 *      8 -15:  minutes
 *      16-23:  hours
 *      24-31:  unused
 */
int rtc_get_time()
{
    int time = 0;

    /* busy loop while the RTC is updating itself */
    while (rtc_in_update())
        ;

    time |= rtc_get_sec();
    time |= (rtc_get_min() << 8);
    time |= (rtc_get_hour() << 16);

    return time;
}

/*
 * CMOS init.
 * Returns 0 on success.
 */
int cmos_init()
{
    int res;

    res = cmos_diagnostic();
    if (res)
    {
        /* if CMOS battery is not present, have to set the error bit off */
        if (res & CMOS_DIAGNOSTIC_RTC_LOST_POWER)
        {
            res &= ~CMOS_DIAGNOSTIC_RTC_LOST_POWER;
            cmos_select_ram(CMOS_DIAGNOSTIC_STATUS);
            cmos_write_ram(res);
        }
        else
            return -1; /* other diagnostic errors mean something not good */
    }

    return 0;
}
