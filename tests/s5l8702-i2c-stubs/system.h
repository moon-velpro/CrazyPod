#ifndef TEST_S5L8702_SYSTEM_H
#define TEST_S5L8702_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

/* Prevent the real MMIO header from replacing the fake register lvalues. */
#ifndef __S5L87XX_H__
#define __S5L87XX_H__
#endif

extern volatile uint32_t fake_iiccon[2];
extern volatile uint32_t fake_iicstat[2];
extern volatile uint32_t fake_iicadd[2];
extern volatile uint32_t fake_iicds[2];
extern volatile uint32_t fake_iicunk10[2];
extern volatile uint32_t fake_iicunk14[2];
extern volatile uint32_t fake_iicunk18[2];
extern volatile uint32_t fake_iicsta2[2];
extern volatile uint32_t fake_pcon3;
extern volatile uint64_t fake_usec;

#define IICCON(bus)   fake_iiccon[(bus)]
#define IICSTAT(bus)  fake_iicstat[(bus)]
#define IICADD(bus)   fake_iicadd[(bus)]
#define IICDS(bus)    fake_iicds[(bus)]
#define IICUNK10(bus) fake_iicunk10[(bus)]
#define IICUNK14(bus) fake_iicunk14[(bus)]
#define IICUNK18(bus) fake_iicunk18[(bus)]
#define IICSTA2(bus)  fake_iicsta2[(bus)]
#define PCON3         fake_pcon3
#define I2CCLKGATE(bus) (bus)
#define I2CCLKGATE_2(bus) (bus)
#define USEC_TIMER fake_usec

static inline void udelay(unsigned usecs)
{
    fake_usec += usecs;
}

#endif
