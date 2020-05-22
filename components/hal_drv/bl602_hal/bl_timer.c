#include "bl_timer.h"
//TODO use unified mtimer clock HZ

#define MTIMER_TICKS_PER_US     (1 * 1000 * 1000 / 32 * 1000)
uint32_t bl_timer_now_us(void)
{
    uint32_t tick_now;

    tick_now = *(volatile uint32_t*)0x0200BFF8;
    return MTIMER_TICKS_PER_US * tick_now;
}
