#ifndef __BL_ADC_H__
#define __BL_ADC_H__
#include <stdint.h>
typedef void (*cb_adc_notify_t)(void *arg);
int bl_adc_gpio_init(uint8_t pin);
int bl_adc_Value_get(uint8_t pin, uint32_t *value);
int bl_adc_finalize(uint8_t port);
int bl_adc_int_notify_register(uint8_t id, cb_adc_notify_t cb, void *arg);
int bl_adc_int_notify_unregister(uint8_t id, cb_adc_notify_t cb, void *arg);
int bl_adc_int_config_disable(uint8_t id);
int bl_adc_int_config_trigger_high(uint8_t id);
int bl_adc_int_config_trigger_low(uint8_t id);
int bl_adc_int_config_trigger_higher(uint8_t id, int level);
int bl_adc_int_config_trigger_lower(uint8_t id, int level);
#endif
