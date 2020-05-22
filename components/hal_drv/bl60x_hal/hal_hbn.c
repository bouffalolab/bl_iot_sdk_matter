#include "bl_hbn.h"

void hal_hbn_enter_with_gpio_wakeup(uint64_t wakeup_pin_map,uint64_t edge)
{
    __disable_irq();
    bl_hbn_enter_with_gpio_wakeup(wakeup_pin_map,edge);
    __enable_irq();
}

void hal_hbn_enter_with_rtc_wakeup(uint32_t sleep_time_ms)
{
    __disable_irq();
    bl_hbn_enter_with_rtc_wakeup(sleep_time_ms);
    __enable_irq();
}
