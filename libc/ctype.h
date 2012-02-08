/******************************************************************************
 *	Macros and typedefs
 *
 *		Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef __CTYPE_H
#define	__CTYPE_H


/* define NULL */
#ifdef NULL
#undef NULL
#endif
#define NULL    ((void *)0)


/* define boolean */
typedef enum {
	false,
	true
} bool;


/* standard size_t type */
typedef unsigned size_t;


/* Basic macros */
#define ISSPACE(c)      ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define ISASCII(c)      (((c) & ~0x7f) == 0)
#define ISUPPER(c)      ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)      ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)      (isupper(c) || islower(c))
#define ISDIGIT(c)      ((c) >= '0' && (c) <= '9')
#define TOUPPER(c)      ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define TOLOWER(c)      ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))
#define TOASCII(c)		((unsigned)(c) & 0x7F)
/* Math */
#define MAX(a, b)		((a) > (b) ? (a) : (b))
#define MIN(a, b)		((a) > (b) ? (b) : (a))
#define ABS(a)			(((a) < 0) ? -(a) : (a))
#define SWAP(a, b)		do { (a) ^= (b); (b) ^= (a); (a) ^= (b); } while (0)


/* integral sizes */
#define	CHAR_BIT	8
/* signed char */
#define	SCHAR_MIN	(-128)
#define	SCHAR_MAX	127	
/* unsigned char */
#define	UCHAR_MAX	255
/* char */
#define	CHAR_MIN	SCHAR_MIN
#define	CHAR_MAX	SCHAR_MAX
/* short */
#define	SHRT_MIN	(-32768)
#define	SHRT_MAX	32767
/* unsigned short */
#define	USHRT_MAX	65535
/* int */
#define	INT_MIN		(-2147483647-1)
#define	INT_MAX		2147483647
/* unsigned int */
#define	UINT_MAX	4294967295U
/* long */
#define	LONG_MIN	(-2147483647L-1L)
#define	LONG_MAX	2147483647L
/* unsigned long */
#define	ULONG_MAX	4294967295UL
/* long long */
#define	LLONG_MIN	(-9223372036854775807LL-1LL)
#define	LLONG_MAX	9223372036854775807LL
/* unsigned long long */
#define	ULLONG_MAX	18446744073709551615ULL


/* va list parameter list */
typedef unsigned char *va_list;

/* width of stack == width of int */
#define	STACKITEM	int

/* round up width of objects pushed on stack. The expression before the
& ensures that we get 0 for objects of size 0. */
#define	VA_SIZE(TYPE)					\
	((sizeof(TYPE) + sizeof(STACKITEM) - 1)	\
		& ~(sizeof(STACKITEM) - 1))

/* &(LASTARG) points to the LEFTMOST argument of the function call
(before the ...) */
#define	va_start(AP, LASTARG)	\
	(AP=((va_list)&(LASTARG) + VA_SIZE(LASTARG)))

/* nothing for va_end */
#define va_end(AP)

#define va_arg(AP, TYPE)	\
	(AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))


#endif
