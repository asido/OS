#ifndef __LIBC_H_
#define __LIBC_H_

#include "ctype.h"

/* Note: some systems use 0xB0000 Instead of 0xB8000 */
#define VIDEO_MEMORY 0xB8000
#define MAX_CRS_X 80
#define MAX_CRS_Y 25
/* colors */
#define VID_CLR_BLACK 0
#define VID_CLR_BLUE 1
#define VID_CLR_GREEN 2
#define VID_CLR_CYAN 3
#define VID_CLR_RED 4
#define VID_CLR_MAGENTA 5
#define VID_CLR_BROWN 6
#define VID_CLR_LIGHT_GRAY 7
#define VID_CLR_DARK_GRAY 8
#define VID_CLR_LIGHT_BLUE 9
#define VID_CLR_LIGHT_GREEN 10
#define VID_CLR_LIGHT_CYAN 11
#define VID_CLR_LIGHT_RED 12
#define VID_CLR_LIGHT_MAGENTA 13
#define VID_CLR_YELLOW 14
#define VID_CLR_WHITE 15

/* End Of File character */
#define EOF (-1)


/* std functions */
int putchar(int c);
int puts(const char *text);
int printf(const char *format, ...);
int strcmp(const char* str1, const char* str2);
size_t strlen(const char* str);
void *memcpy(void *dest, const void *src, size_t num);
void *memset(void *dest, int val, size_t count);
int pow(int base, int exp);
int atoi(const char *str);
char *itoa(int value, char *str, int base);
/* long strtol(const char *nptr, char** endptr, int base); */
/* unsigned long strtoul(const char* nptr, char** endptr, int base); */

/* custom functions */
void goto_xy(unsigned x, unsigned y);
void set_color(unsigned char backgrnd, unsigned char forgrnd);
void clear_screen();

unsigned char inportb (unsigned short _port);
void outportb (unsigned short _port, unsigned char _data);

#endif /* end of include guard: __LIBC_H_ */
