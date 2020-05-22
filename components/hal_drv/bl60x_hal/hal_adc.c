#include <string.h>
#include <stdio.h>
#include <device/vfs_adc.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/soc/adc.h>
#include <aos/kernel.h>

#include "bl_adc.h"
#include "hal_adc.h"

#include <libfdt.h>
#include <utils_log.h>

#define HAL_ADC_DEV_PREFIX  "/dev/"
typedef struct adc_priv_data {
    uint8_t nothing;
} adc_priv_data_t;

int32_t hal_adc_init(adc_dev_t *adc)
{
    adc_priv_data_t *data;

    data = adc->priv;
    (void)data;
    bl_adc_gpio_init(adc->port);

    return 0;
}

int32_t hal_adc_value_get(adc_dev_t *adc, uint32_t *output, uint32_t timeout)
{
    return bl_adc_Value_get(adc->port, output);
}

int32_t hal_adc_finalize(adc_dev_t *adc)
{
    adc_priv_data_t *data;

    (void)data;
    data = adc->priv;
    bl_adc_finalize(adc->port);
    return 0;
}

int hal_adc_notify_unregister(adc_dev_t *adc, void (*cb)(void *arg))
{
    if (bl_adc_int_notify_unregister(adc->port, cb, adc)) {
        return -1;
    }

    return 0;
}

int hal_adc_notify_register(adc_dev_t *adc, void (*cb)(void *arg))
{
    if (bl_adc_int_notify_register(adc->port, cb, adc)) {
        return -1;
    }

    return 0;
}

int hal_adc_notify_register_config_disable(adc_dev_t *adc)
{
    if (bl_adc_int_config_disable(adc->port)) {
        return -1;
    }

    return 0;
}

int hal_adc_notify_register_config_high(adc_dev_t *adc)
{
    if (bl_adc_int_config_trigger_high(adc->port)) {
        return -1;
    }

    return 0;
}

int hal_adc_notify_register_config_low(adc_dev_t *adc)
{
    if (bl_adc_int_config_trigger_low(adc->port)) {
        return -1;
    }

    return 0;
}

int hal_adc_notify_register_config_higher(adc_dev_t *adc, int level)
{
    if (bl_adc_int_config_trigger_higher(adc->port, level)) {
        return -1;
    }

    return 0;
}

int hal_adc_notify_register_config_lower(adc_dev_t *adc, int level)
{
    if (bl_adc_int_config_trigger_lower(adc->port, level)) {
        return -1;
    }

    return 0;
}

int vfs_adc_init(int pin, const char *name)
{
    int ret, len;
    char fullname[32];
    adc_priv_data_t *adc_data;
    adc_dev_t *adc;

    len = strlen(name);
    len += strlen(HAL_ADC_DEV_PREFIX);
    if (len + 1 > sizeof(fullname)) {
        return -EINVAL;
    }

    //TODO use one bigger mem for these two small struct
    adc_data = (adc_priv_data_t*)aos_malloc(sizeof(adc_priv_data_t));
    if (NULL == adc_data) {
        return -ENOMEM;
    }
    adc = (adc_dev_t*)aos_malloc(sizeof(adc_dev_t));
    if (NULL == adc) {
        aos_free(adc_data);
        return -ENOMEM;
    }

    memset(adc_data, 0, sizeof(adc_priv_data_t));
    memset(adc, 0, sizeof(adc_dev_t));
    adc->port = pin;
    adc->config.sampling_cycle = 100;
    adc->priv = adc_data;

    snprintf(fullname, sizeof(fullname), "%s%s", HAL_ADC_DEV_PREFIX, name);
    printf("[HAL] [ADC] Register Under %s for PIN %d\r\n", fullname, pin);
    ret = aos_register_driver(fullname, &adc_ops, adc);
    if (ret != VFS_SUCCESS) {
        aos_free(adc);
        aos_free(adc_data);
        return ret;
    }

    return VFS_SUCCESS;
}

int vfs_adc_init_fullname(int pin, const char *fullname)
{
    int ret, len;
    adc_priv_data_t *adc_data;
    adc_dev_t *adc;

    len = strlen(fullname);
    if (len + 1 > 32) {
        return -EINVAL;
    }

    //TODO use one bigger mem for these two small struct
    adc_data = (adc_priv_data_t*)aos_malloc(sizeof(adc_priv_data_t));
    if (NULL == adc_data) {
        return -ENOMEM;
    }
    adc = (adc_dev_t*)aos_malloc(sizeof(adc_dev_t));
    if (NULL == adc) {
        aos_free(adc_data);
        return -ENOMEM;
    }

    memset(adc_data, 0, sizeof(adc_priv_data_t));
    memset(adc, 0, sizeof(adc_dev_t));
    adc->port = pin;
    adc->config.sampling_cycle = 100;
    adc->priv = adc_data;

    printf("[HAL] [ADC] Register Under %s for PIN %d\r\n", fullname, pin);
    ret = aos_register_driver(fullname, &adc_ops, adc);
    if (ret != VFS_SUCCESS) {
        aos_free(adc);
        aos_free(adc_data);
        return ret;
    }

    return VFS_SUCCESS;
}

key_adc_arg_t *adc_desc = NULL;

key_adc_arg_t *adc_key_getaddr(void)
{
    return (key_adc_arg_t *)(adc_desc);
}
#define BL_FDT32_TO_U8(addr, byte_offset)   ((uint8_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U16(addr, byte_offset)  ((uint16_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U32(addr, byte_offset)  ((uint32_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))


static void adc_arg_set_default(key_adc_arg_t *arg)
{
    if (!arg) {
        log_error("arg err.\r\n");
        return;
    }

    arg->fd = -1;
    arg->key_sum = 5;
    arg->key_num = arg->key_sum;
    arg->val_buf = NULL;
    arg->key_raw = NULL;
    // arg->val_buf = aos_malloc(arg->key_sum * sizeof(uint16_t));
    // arg->key_raw = aos_malloc(arg->key_sum * sizeof(uint16_t));
    // if ((!arg->val_buf) || (!arg->key_raw)) {
    //     log_error("mem err.\r\n");
    // }

    // memset(arg->val_buf, 0, (arg->key_sum * sizeof(uint16_t)));
    // memset(arg->key_raw, 0, (arg->key_sum * sizeof(uint16_t)));

    // arg->val_buf[0] = 0;
    // arg->val_buf[1] = 100;
    // arg->val_buf[2] = 200;
    // arg->val_buf[3] = 300;
    // arg->val_buf[4] = 380;

    arg->adc_offset = 27;
    arg->adc_offset_flag = 0;   /* +:0 -:1 */

    arg->val_threshold = 10;    /* +-10 mv */

    arg->cycle_times = 0;
    arg->shake_time = 10;       /* 10 ms */
    arg->cycle_times_max = 5;   /* 3 times */
    arg->cycle_offset = 10;     /* 10 ms */

    arg->exit_check_max = 3;

    // vfs_adc_init_fullname(10, "/dev/adc0");
}

static void adc_arg_set_fdt2(const void *fdt, int adc_offset)
{
    #define ADC_MODULE_MAX 1
    key_adc_arg_t *adc;
    int offset1 = 0;

    const uint32_t *addr_prop = 0;
    int lentmp = 0;
    const char *result = 0;
    int countindex = 0;
    int i;

    char *path = NULL;
    int pin;
    uint16_t key_threshold;
    uint16_t key_sum;

    const char *adc_node[ADC_MODULE_MAX] = {
        "adc_key"
    };

    adc_desc = aos_malloc(sizeof(key_adc_arg_t));
    if (!adc_desc) {
        log_error("mem err.\r\n");
    }
    adc_arg_set_default(adc_desc);
    adc = adc_desc;

    for (i = 0; i < ADC_MODULE_MAX; i++) {
        /* adc_key */
        offset1 = fdt_subnode_offset(fdt, adc_offset, adc_node[i]);
        if (0 >= offset1) {
            continue;
        }

        /* get status */
        countindex = fdt_stringlist_count(fdt, offset1, "status");
        if (countindex != 1) {
            log_info("adc[%d] status_countindex = %d NULL.\r\n", i, countindex);
            continue;
        }
        result = fdt_stringlist_get(fdt, offset1, "status", 0, &lentmp);
        if ((lentmp != 4) || (0 != memcmp(result, "okay", 4))) {
            log_info("adc[%d] status lentmp = %d , result = %s\r\n", i, lentmp, result);
            continue;
        }

        /* set path */
        countindex = fdt_stringlist_count(fdt, offset1, "path");
        if (countindex != 1) {
            log_info("adc[%d] path_countindex = %d NULL.\r\n", i, countindex);
            continue;
        }
        result = fdt_stringlist_get(fdt, offset1, "path", 0, &lentmp);
        if ((lentmp < 0) || (lentmp > 32)) {
            log_info("adc[%d] path lentmp = %d\r\n", i, lentmp);
        }
        path = (char *)result;

        /* set pin */
        addr_prop = fdt_getprop(fdt, offset1, "pin", &lentmp);
        if (addr_prop == NULL) {
            log_info("adc[%d] pin NULL.\r\n", i);
            continue;
        }
        pin = (int)BL_FDT32_TO_U16(addr_prop, 0);

        /* set key_threshold */
        addr_prop = fdt_getprop(fdt, offset1, "key_threshold", &lentmp);
        if (addr_prop == NULL) {
            log_info("adc[%d] key_threshold NULL.\r\n", i);
            continue;
        }
        key_threshold = BL_FDT32_TO_U16(addr_prop, 0);

        /* set adc_offset */
        addr_prop = fdt_getprop(fdt, offset1, "adc_offset", &lentmp);
        if (addr_prop == NULL) {
            log_info("adc[%d] adc_offset NULL.\r\n", i);
            continue;
        }
        adc->adc_offset = BL_FDT32_TO_U16(addr_prop, 0);

        /* set adc_offset_flag */
        addr_prop = fdt_getprop(fdt, offset1, "adc_offset_flag", &lentmp);
        if (addr_prop == NULL) {
            log_info("adc[%d] adc_offset_flag NULL.\r\n", i);
            continue;
        }
        adc->adc_offset_flag = BL_FDT32_TO_U16(addr_prop, 0);

        /* set key_sum */
        addr_prop = fdt_getprop(fdt, offset1, "key_sum", &lentmp);
        if (addr_prop == NULL) {
            log_info("adc[%d] key_sum NULL.\r\n", i);
            continue;
        }
        key_sum = BL_FDT32_TO_U16(addr_prop, 0);

        /* get sum and malloc val_buf key_raw */
        adc->key_sum = key_sum;
        adc->val_threshold = key_threshold;
        log_info("set key_sum = %d\r\n", adc->key_sum);
        log_info("set val_threshold = %d mv\r\n", adc->val_threshold);

        adc->val_buf = aos_malloc(adc->key_sum * sizeof(uint16_t));
        adc->key_raw = aos_malloc(adc->key_sum * sizeof(uint16_t));
        if ((!adc->val_buf) || (!adc->key_raw)) {
            log_error("mem err.\r\n");
        }
        memset(adc->val_buf, 0, (adc->key_sum * sizeof(uint16_t)));
        memset(adc->key_raw, 0, (adc->key_sum * sizeof(uint16_t)));

        /* set val_buf */
        addr_prop = fdt_getprop(fdt, offset1, "key_vol", &lentmp);
        if (lentmp == adc->key_sum*4) {
            for (i = 0; i < adc->key_sum; i++) {
                adc->val_buf[i] = BL_FDT32_TO_U16(addr_prop, 4*i);
                log_info("set val_buf[%d] = %d mv\r\n", i, adc->val_buf[i]);
            }
        }

        /* set key_raw */
        addr_prop = fdt_getprop(fdt, offset1, "key_raw", &lentmp);
        if (lentmp == adc->key_sum*4) {
            for (i = 0; i < adc->key_sum; i++) {
                adc->key_raw[i] = BL_FDT32_TO_U16(addr_prop, 4*i);
                log_info("set key_raw[%d] = %d\r\n", i, adc->key_raw[i]);
            }
        }

        /* vfs init */
        vfs_adc_init_fullname(pin, path);
        log_info("vfs_adc_init_fullname pin(%d) path(%s)\r\n", pin, path);
    }
}

int vfs_adc_fdt_init(uint32_t fdt, uint32_t dtb_uart_offset)
{
    log_info("adc fdt init.\r\n");
    adc_arg_set_fdt2((const void *)fdt, dtb_uart_offset);
    return 0;
}
