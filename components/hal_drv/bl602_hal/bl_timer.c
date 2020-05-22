#include "bl_timer.h"

#define MTIMER_TICKS_PER_US     (1)
uint32_t bl_timer_now_us(void)
{
    uint32_t tick_now;

    tick_now = *(volatile uint32_t*)0x4000A52C;
    return MTIMER_TICKS_PER_US * tick_now;
}
