/******************************************************************************
 *      CMOS driver
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef CMOS_XIBFJE9T
#define CMOS_XIBFJE9T

/* Write data */
#define CMOS_DISKETTE 0x10
/* Read data */
#define CMOS_DISKETTE_NO_DRIVE 0x0
#define CMOS_DISKETTE_360k 0x1
#define CMOS_DISKETTE_1M2 0x2
#define CMOS_DISKETTE_720k 0x3
#define CMOS_DISKETTE_1M44 0x4
/* Checking */
#define CMOS_DISKETTE_TYPE_DRIVE0(data) \
    (((data) >> 4) & 0xF)
#define CMOS_DISKETTE_TYPE_DRIVE1(data) \
    ((data) & 0xF)


int cmos_init();
int rtc_get_time();
int cmos_get_flp_status();

#endif /* end of include guard: CMOS_XIBFJE9T */
