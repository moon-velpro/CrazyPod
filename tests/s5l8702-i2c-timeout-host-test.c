#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "system.h"

volatile uint32_t fake_iiccon[2];
volatile uint32_t fake_iicstat[2];
volatile uint32_t fake_iicadd[2];
volatile uint32_t fake_iicds[2];
volatile uint32_t fake_iicunk10[2];
volatile uint32_t fake_iicunk14[2];
volatile uint32_t fake_iicunk18[2];
volatile uint32_t fake_iicsta2[2];
volatile uint32_t fake_pcon3;
volatile uint64_t fake_usec;

#include "../firmware/target/arm/s5l8702/i2c-s5l8702.c"

void clockgate_enable(int gate, bool enable)
{
    (void)gate;
    (void)enable;
}

static void reset_fake_bus(void)
{
    memset((void *)fake_iiccon, 0, sizeof(fake_iiccon));
    memset((void *)fake_iicstat, 0, sizeof(fake_iicstat));
    memset((void *)fake_iicadd, 0, sizeof(fake_iicadd));
    memset((void *)fake_iicds, 0, sizeof(fake_iicds));
    memset((void *)fake_iicunk10, 0, sizeof(fake_iicunk10));
    memset((void *)fake_iicunk14, 0, sizeof(fake_iicunk14));
    memset((void *)fake_iicunk18, 0, sizeof(fake_iicunk18));
    memset((void *)fake_iicsta2, 0, sizeof(fake_iicsta2));
    fake_pcon3 = 0;
    fake_usec = 0;

    /* Model the controller's transfer-complete flags being raised. */
    fake_iicsta2[0] = (1u << 8) | (1u << 13);
}

int main(void)
{
    unsigned char value = 0x5a;

    i2c_init();

    reset_fake_bus();
    assert(i2c_write(0, 0xe6, 0x01, 1, &value) == 0);
    assert(fake_iicds[0] == value);

    reset_fake_bus();
    fake_iicunk10[0] = 1; /* controller never becomes ready */
    assert(i2c_write(0, 0xe6, 0x01, 1, &value) != 0);
    assert(fake_usec < 200000);

    reset_fake_bus();
    fake_iicsta2[0] = 0; /* transfer-complete event never arrives */
    assert(i2c_write(0, 0xe6, 0x01, 1, &value) != 0);
    assert(fake_usec < 200000);

    reset_fake_bus();
    assert(i2c_write(2, 0xe6, 0x01, 1, &value) == I2C_ERR_INVALID);
    assert(i2c_read(0, 0xe6, 0x01, 1, NULL) == I2C_ERR_INVALID);

    puts("S5L8702 I2C timeout tests passed");
    return 0;
}
