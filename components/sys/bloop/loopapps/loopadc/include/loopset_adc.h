#ifndef __LOOPSET_ADC_H__
#define __LOOPSET_ADC_H__
#include <stdint.h>

#define EVT_MAP_ADC_TRIGGER     (1U << 0)
#define EVT_MAP_ADC_CREATER     (1U << 1)
#define EVT_MAP_ADC_TIMER       (1U << 2)

void loopapp_adc_trigger(uint32_t volt, int event_type);
void loopapp_adc_process(int flag);
void loopapp_adc_create(int oneshot , int sampling_ms);
int loopapp_adc_hook_on_looprt(void);


#endif
