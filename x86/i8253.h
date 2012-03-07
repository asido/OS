/******************************************************************************
 *      Intel 8253 PIT controller - Programmable Interval Timer
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#ifndef I8253_IA5WC1E3
#define I8253_IA5WC1E3

extern unsigned long long pit_jiffy;

/* PIT interrupt Hz */
#define PIT_HZ 1000
#define PIT_CLOCK_TICK 1193181
#define PIT_FREQ    (PIT_CLOCK_TICK / PIT_HZ)

/* Ports */
#define PIT_PORT_BASE 0x40
#define PIT_PORT_PIT_0 (PIT_PORT_BASE)
#define PIT_PORT_PIT_1 (PIT_PORT_BASE + 0x1)
#define PIT_PORT_PIT_2 (PIT_PORT_BASE + 0x2)
#define PIT_PORT_MODE (PIT_PORT_BASE + 0x3)

/* 1st bit - BCD */
#define PIT_CTRL_BCD_BIN 0x0
#define PIT_CTRL_BCD_DEC 0x1
/* 2-4th bit - Mode */
#define PIT_CTRL_MODE_ON_TERM_CNT 0
#define PIT_CTRL_MODE_ONE_SHOT 0x2  /* 00000010 */
#define PIT_CTRL_MODE_RATE_GEN 0x4  /* 00000100 */
#define PIT_CTRL_MODE_SQR_WAVE 0x6  /* 00000110 */
#define PIT_CTRL_MODE_SOFT_TRIG 0x8 /* 00001000 */
#define PIT_CTRL_MODE_HARD_TRIG 0xA /* 00001010 */
/* 5-6th bit - RL (Read/Load) */
#define PIT_CTRL_RL_MOST_SIG       0x20 /* 00100000 */
#define PIT_CTRL_RL_LEAST_SIG      0x10 /* 00010000 */
#define PIT_CTRL_RL_LEAST_MOST_SIG 0x30 /* 00110000 */
/* 7-8th bit - SC */
#define PIT_CTRL_SELECT_0 0
#define PIT_CTRL_SELECT_1 0x40 /* 01000000 */
#define PIT_CTRL_SELECT_2 0x80 /* 10000000 */

int i8253_init();

#endif /* end of include guard: I8253_IA5WC1E3 */
