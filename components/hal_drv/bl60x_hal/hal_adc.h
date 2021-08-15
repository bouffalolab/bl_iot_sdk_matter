
#ifndef __HAL_ADC_H__
#define __HAL_ADC_H__

typedef struct _key_adc_arg {
    int fd;
    uint16_t exit_check_times;  /* 0 */
    uint16_t exit_check_max;    /* 5 */
    uint16_t cycle_times;
    uint16_t key_sum;
    uint16_t key_num;
    uint16_t val_threshold;
    uint16_t *val_buf;          /* dts */
    uint16_t *key_raw;          /* dts */
    uint16_t shake_time;        /* dts if no set 10ms */
    uint16_t cycle_times_max;   /* dts if no set 5 */
    uint16_t cycle_offset;      /* dts if no set 10 */
    uint16_t adc_offset;        /* dts if no set 27 avb */
    uint16_t adc_offset_flag;   /* dts if no set 0 avb *//* +:0 -:1 */
} key_adc_arg_t;

int vfs_adc_init(int pin, const char *name);
int vfs_adc_fdt_init(uint32_t fdt, uint32_t dtb_uart_offset);

key_adc_arg_t *adc_key_getaddr(void);

#endif
