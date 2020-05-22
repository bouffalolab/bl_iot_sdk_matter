#ifndef __BL_HBN_H__
#define __BL_HBN_H__
#include <stdint.h>

void hal_hbn_enter_with_gpio_wakeup(uint64_t wakeup_pin_map,uint64_t edge);
void hal_hbn_enter_with_rtc_wakeup(uint32_t sleep_time_ms);

#endif
