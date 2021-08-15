#include <device/vfs_gpio.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/soc/gpio.h>
#include <aos/kernel.h>
#include <bl60x_gpio.h>
#include <bl60x_glb.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <event_groups.h>

#include "bl_gpio.h"
#include "hal_gpio.h"
#include "libfdt.h"
#include "utils_log.h"

static int inited;

typedef struct gpio_priv_data {
    aos_mutex_t    mutex;
} gpio_priv_data_t;
static gpio_priv_data_t gpio16_data;

gpio_priv_data_t *gpio_priv_buf = NULL;

gpio_dev_t gpio_16 = {
    .port = 16,
    .config = OUTPUT_PUSH_PULL,
    .priv = NULL,
    .gpioMode = GPIO_MODE_OUTPUT,
    .pullType = GPIO_PULL_UP
};

int32_t hal_gpio_init(gpio_dev_t *gpio)
{
    gpio_priv_data_t *data;

    data = gpio->priv;
    if (aos_mutex_new(&(data->mutex))) {
        /*we should assert here?*/
        return -1;
    }
    GLB_GPIO_Cfg_Type cfg;
    cfg.drive = 0;
    cfg.smtCtrl = 1;
    cfg.gpioPin = gpio->port;
    cfg.gpioFun = GPIO0_FUN_REG_GPIO_0;//all the function number of GPIO is the same, we use def from GPIO0 here
    cfg.gpioMode = gpio->gpioMode;
    cfg.pullType = gpio->pullType;
    GLB_GPIO_Init(&cfg);
    return 0;
}

int32_t hal_gpio_output_high(gpio_dev_t *gpio)
{
    bl_gpio_output_set(gpio->port, 1);
    gpio->level = 1;
    return 0;
}

int32_t hal_gpio_output_low(gpio_dev_t *gpio)
{
    bl_gpio_output_set(gpio->port, 0);
    gpio->level = 0;
    return 0;
}

int32_t hal_gpio_output_toggle(gpio_dev_t *gpio)
{
    if (gpio->level) {
        bl_gpio_output_set(gpio->port, 0);
        gpio->level = 0;
    } else {
        bl_gpio_output_set(gpio->port, 1);
        gpio->level = 1;
    }
    return 0;
}

int32_t hal_gpio_input_get(gpio_dev_t *gpio, uint8_t *value)
{
    bl_gpio_input_get(gpio->port, value);
    return 0;
}

int hal_gpio_pulltype_set(gpio_dev_t *gpio, gpio_config_t conf)
{
    GLB_GPIO_Cfg_Type cfg;
    cfg.drive = 0;
    cfg.smtCtrl = 1;
    cfg.gpioPin = gpio->port;
    cfg.gpioFun = GPIO0_FUN_REG_GPIO_0;//all the function number of GPIO is the same, we use def from GPIO0 here
    switch (conf) {
        case GPIO_CONFIG_MODE_OUTPUT:
        {
            cfg.gpioMode = GPIO_MODE_OUTPUT;
        }
        break;
        case GPIO_CONFIG_MODE_INPUT:
        {
            cfg.gpioMode = GPIO_MODE_INPUT;
        }
        break;
        case GPIO_CONFIG_MODE_AF:
        {
            cfg.gpioMode = GPIO_MODE_AF;
        }
        default:
        {
            cfg.gpioMode = GPIO_MODE_INPUT;
        }
    }
    cfg.pullType = pulltype;
    GLB_GPIO_Init(&cfg);
    return 0;
}

int32_t hal_gpio_finalize(gpio_dev_t *gpio)
{
    GLB_GPIO_Cfg_Type cfg;
    cfg.drive = 0;
    cfg.smtCtrl = 1;
    cfg.gpioPin = gpio->port;
    cfg.gpioFun = GPIO0_FUN_REG_GPIO_0;//all the function number of GPIO is the same, we use def from GPIO0 here
    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_NONE;
    gpio->gpioMode = GPIO_MODE_AF;
    gpio->pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    gpio_priv_data_t *data;

    data = gpio->priv;
    aos_mutex_free(&(data->mutex));
    return 0;
}

int vfs_gpio_init(void)
{
    int ret;

    if (inited == 1) {
        return VFS_SUCCESS;
    }
    gpio_16.priv = &gpio16_data;
    ret = aos_register_driver("/dev/gpio16", &gpio_ops, &gpio_16);
    if (ret != VFS_SUCCESS) {
        return ret;
    }

    inited = 1;

    return VFS_SUCCESS;
}

int vfs_gpio_init_fullname(const char *fullname, uint8_t port, uint32_t gpioMode, uint32_t pullType)
{
    int ret, len;
    gpio_dev_t *gpio;

    len = strlen(fullname);
    if (len + 1 > 32) {
        log_error("arg err.\r\n");
        return -EINVAL;
    }

    //TODO use one bigger mem for these two small struct
    if (NULL == gpio_priv_buf) {
        gpio_priv_buf = (gpio_priv_data_t*)aos_malloc(sizeof(gpio_priv_data_t));
        if (NULL == gpio_priv_buf) {
            log_error("mem err.\r\n");
            return -ENOMEM;
        }
        memset(gpio_priv_buf, 0, sizeof(gpio_priv_data_t));
    }

    gpio = (gpio_dev_t*)aos_malloc(sizeof(gpio_dev_t));
    if (NULL == gpio) {
        log_error("mem err.\r\n");
        aos_free(gpio_priv_buf);
        return -ENOMEM;
    }

    memset(gpio, 0, sizeof(gpio_dev_t));

    gpio->port = port;
    gpio->gpioMode = gpioMode;
    gpio->pullType = pullType;
    gpio->priv = gpio_priv_buf;

    log_info("[HAL] [GPIO] Register Under %s for :\r\nport=%u, gpiomode=%lu, pulltype=%lu\r\n",
        fullname, port, gpioMode, pullType);

    ret = aos_register_driver(fullname, &gpio_ops, gpio);
    if (ret != VFS_SUCCESS) {
        aos_free(gpio);
        aos_free(gpio_priv_buf);
        return ret;
    }

    return VFS_SUCCESS;

}

#define BL_FDT32_TO_U8(addr, byte_offset)   ((uint8_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U16(addr, byte_offset)  ((uint16_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U32(addr, byte_offset)  ((uint32_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))

int gpio_arg_set_fdt2(const void * fdt, uint32_t dtb_gpio_offset)
{
    #define GPIO_MODULE_MAX 25
    uint8_t port;
    uint32_t gpioMode;
    uint32_t pullType;
    char *path = NULL;

    int offset1 = 0;
    const uint32_t *addr_prop = 0;
    int lentmp = 0;
    const char *result = 0;
    int countindex = 0;

    int i;


    for (i = 0; i < GPIO_MODULE_MAX; i++) {
        /* get gpio0 ? gpio1 ? gpio2 offset1 */
        if (i == 0) {
            offset1 = fdt_first_subnode(fdt, dtb_gpio_offset);
            if (0 >= offset1) {
                continue;
            }
        } else {
            offset1 = fdt_next_subnode(fdt, offset1);
            if (0 >= offset1) {
                continue;
            }
        }

        result = fdt_stringlist_get(fdt, offset1, "status", 0, &lentmp);
        if ((lentmp != 4) || (memcmp("okay", result, 4) != 0)) {
            log_info("gpio[%d] status != okay\r\n", i);
            continue;
        }

        /* set path */
        countindex = fdt_stringlist_count(fdt, offset1, "path");
        if (countindex != 1) {
            log_info("gpio[%d] path_countindex = %d NULL.\r\n", i, countindex);
            continue;
        }
        result = fdt_stringlist_get(fdt, offset1, "path", 0, &lentmp);

        if ((lentmp < 0) || (lentmp > 32)) {
            log_info("gpio[%d] path lentmp = %d\r\n", i, lentmp);
        }
        path = (char *)result;

        /* get gpiomode */
        addr_prop = fdt_getprop(fdt, offset1, "gpioMode", &lentmp);
        if (addr_prop == NULL) {
            log_info("gpio[%d] mode NULL.\r\n", i);
            continue;
        }
        gpioMode = BL_FDT32_TO_U32(addr_prop, 0);

        /* get pullType */
        addr_prop = fdt_getprop(fdt, offset1, "pullType", &lentmp);
        if (addr_prop == NULL) {
            log_info("gpio[%d] pullType NULL.\r\n", i);
            continue;
        }
        pullType = BL_FDT32_TO_U32(addr_prop, 0);

        /* get gpio_port */
        addr_prop = fdt_getprop(fdt, offset1, "port", &lentmp);
        if (addr_prop == NULL) {
            log_info("gpio[%d] port NULL.\r\n", i);
            continue;
        }
        port = BL_FDT32_TO_U8(addr_prop, 0);

        vfs_gpio_init_fullname((const char *)path, port, gpioMode, pullType);

    }
    return 0;
}

int vfs_gpio_fdt_init(uint32_t fdt, uint32_t dtb_gpio_offset)
{
    gpio_arg_set_fdt2((const void *)fdt, dtb_gpio_offset);
    log_info("vfs_gpio_fdt_init ok.\r\n");
    return 0;
}
