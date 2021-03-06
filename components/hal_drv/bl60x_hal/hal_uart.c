#include <device/vfs_uart.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/soc/uart.h>
#include <aos/kernel.h>

#include "hal_uart.h"
#include "bl_uart.h"

#include <libfdt.h>

#include <utils_log.h>

static int inited;

typedef struct uart_priv_data {
    aos_mutex_t    mutex;
} uart_priv_data_t;

static uart_dev_t *dev_uart0 = NULL;
static uart_dev_t *dev_uart1 = NULL;
static uart_dev_t *dev_uart2 = NULL;

static void uart_dev_setdef(uart_dev_t **pdev, uint8_t id)
{
    if (*pdev == NULL) {
        log_error("mem err.\r\n");
        return;
    }

    (*pdev)->port = id;

    (*pdev)->config.baud_rate = 115200;
    (*pdev)->config.data_width = DATA_WIDTH_8BIT;
    (*pdev)->config.parity = NO_PARITY;
    (*pdev)->config.stop_bits = STOP_BITS_1;
    (*pdev)->config.flow_control = FLOW_CONTROL_DISABLED;
    (*pdev)->config.mode = MODE_TX_RX;
}

static int uart_dev_malloc(uart_dev_t **pdev)
{
    if (*pdev) {
        log_error("arg err.\r\n");
        return -1;
    }

    *pdev = pvPortMalloc(sizeof(uart_dev_t));
    if (*pdev == 0) {
        log_error("mem err.\r\n");
        return -1;
    }

    (*pdev)->priv = NULL;
    (*pdev)->priv = pvPortMalloc(sizeof(uart_priv_data_t));
    if ((*pdev)->priv == NULL) {
        log_error("mem err.\r\n");
        return -1;
    }

    return 0;
}

void uart_dev_free(uart_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }

    if (dev->priv != NULL) {
        vPortFree(dev->priv);
    }
    vPortFree(dev);
}

static int dev_uart_init(uint8_t id, const char *path)
{
    uart_dev_t **pdev = NULL;
    int ret;

    if ((id >= 3) || (path == 0)) {
        log_error("arg err.\r\n");
        return -1;
    }

    switch (id) {
        case 0:
        {
            pdev = &dev_uart0;
        } break;
        case 1:
        {
            pdev = &dev_uart1;
        } break;
        case 2:
        {
            pdev = &dev_uart2;
        } break;
        default:
        {
            log_error("err.\r\n");
            return -1;
        } break;
    }

    if (uart_dev_malloc(pdev) != 0) {
        return -1;
    }

    uart_dev_setdef(pdev, id);
    ret = aos_register_driver(path, &uart_ops, *pdev);
    if (ret != VFS_SUCCESS) {
        return ret;
    }

    return 0;
}

int32_t hal_uart_init(uart_dev_t *uart)
{
    uart_priv_data_t *data;

    data = uart->priv;
    if (aos_mutex_new(&(data->mutex))) {
        /*we should assert here?*/
        return -1;
    }
    bl_uart_int_enable(
            uart->port,
            uart->ring_rx_buffer, &(uart->ring_rx_idx_write), &(uart->ring_rx_idx_read),
            uart->ring_tx_buffer, &(uart->ring_tx_idx_write), &(uart->ring_tx_idx_read)
    );

    return 0;
}

int32_t hal_uart_send_flush(uart_dev_t *uart, uint32_t timeout)
{
    bl_uart_flush(uart->port);                                                                                                                                                                 
    return 0;
}

void hal_uart_setbaud(uart_dev_t *uart, uint32_t baud)
{
    bl_uart_setbaud(uart->port, baud);
}

int32_t hal_uart_send(uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    uint32_t i = 0;

    while (i < size) {
        bl_uart_data_send(uart->port, ((uint8_t*)data)[i]);
        i++;
    }
    return 0;
}

int32_t hal_uart_send_trigger(uart_dev_t *uart)
{
    bl_uart_int_tx_enable(uart->port);
    return 0;
}

int32_t hal_uart_recv_II(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout)
{
    int ch;
    uint32_t counter = 0;

    while (counter < expect_size && (ch = bl_uart_data_recv(uart->port)) >= 0) {
        ((uint8_t*)data)[counter] = ch;
        counter++;
    }

    *recv_size = counter;
    return 0;
}

int32_t hal_uart_finalize(uart_dev_t *uart)
{
    uart_priv_data_t *data;

    data = uart->priv;
    aos_mutex_free(&(data->mutex));

    return 0;
}

/*TODO better glue for ring buffer?*/
int32_t hal_uart_notify_register(uart_dev_t *uart, void (*cb)(void *arg))
{
    bl_uart_int_cb_notify_register(uart->port, cb, uart);
    return 0;
}

int32_t hal_uart_notify_unregister(uart_dev_t *uart, void (*cb)(void *arg))
{
    bl_uart_int_cb_notify_unregister(uart->port, cb, uart);
    return 0;
}

int hal_uart_data_notify(int number, int dir)
{
    if (0 == dir) {
        /*0 is output*/

    } else if (1 == dir) {
        /* 1 is input*/

    }

    return 0;
}

#define BL_FDT32_TO_U8(addr, byte_offset)   ((uint8_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U16(addr, byte_offset)  ((uint16_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))
#define BL_FDT32_TO_U32(addr, byte_offset)  ((uint32_t)fdt32_to_cpu(*(uint32_t *)((uint8_t *)addr + byte_offset)))

static void fdt_uart_module_init(const void *fdt, int uart_offset)
{
    #define UART_MODULE_MAX 3
    #define PIN_INVALID_VALUE 0XFF

    int offset1 = 0;
    int offset2 = 0;

    const uint32_t *addr_prop = 0;
    int lentmp = 0;
    const char *result = 0;
    int countindex = 0;
    int i, j;

    uint8_t id;
    char *path = NULL;
    uint32_t baudrate;

    const char *uart_node[UART_MODULE_MAX] = {
        "uart@40010000",
        "uart@40010100",
        "uart@40010200"
    };

    struct _feature_pin {
        char *featue_name;
        char *pin_name;
        uint8_t value;
    } feature_pin[4] = {
        {
            .featue_name = "tx",
            .pin_name = "tx",
            .value = PIN_INVALID_VALUE
        },
        {
            .featue_name = "rx",
            .pin_name = "rx",
            .value = PIN_INVALID_VALUE
        },
        {
            .featue_name = "cts",
            .pin_name = "cts",
            .value = PIN_INVALID_VALUE
        },
        {
            .featue_name = "rts",
            .pin_name = "rts",
            .value = PIN_INVALID_VALUE
        }
    };

    for (i = 0; i < UART_MODULE_MAX; i++) {
        offset1 = fdt_subnode_offset(fdt, uart_offset, uart_node[i]);
        if (0 >= offset1) {
            log_info("uart[%d] %s NULL.\r\n", i, uart_node[i]);
            continue;
        }

        countindex = fdt_stringlist_count(fdt, offset1, "status");
        if (countindex != 1) {
            log_info("uart[%d] status_countindex = %d NULL.\r\n", i, countindex);
            continue;
        }
        result = fdt_stringlist_get(fdt, offset1, "status", 0, &lentmp);
        if ((lentmp != 4) || (memcmp("okay", result, 4) != 0)) {
            log_info("uart[%d] status = %s\r\n", i, result);
            continue;
        }

        /* set path */
        countindex = fdt_stringlist_count(fdt, offset1, "path");
        if (countindex != 1) {
            log_info("uart[%d] path_countindex = %d NULL.\r\n", i, countindex);
            continue;
        }
        result = fdt_stringlist_get(fdt, offset1, "path", 0, &lentmp);
        if ((lentmp < 0) || (lentmp > 32))
        {
            log_info("uart[%d] path lentmp = %d\r\n", i, lentmp);
        }
        path = (char *)result;

        /* set id */
        addr_prop = fdt_getprop(fdt, offset1, "baudrate", &lentmp);
        if (addr_prop == NULL) {
            log_info("uart[%d] baudrate NULL.\r\n", i);
            continue;
        }
        baudrate = BL_FDT32_TO_U32(addr_prop, 0);

        /* set id */
        addr_prop = fdt_getprop(fdt, offset1, "id", &lentmp);
        if (addr_prop == NULL) {
            log_info("uart[%d] id NULL.\r\n", i);
            continue;
        }
        id = BL_FDT32_TO_U8(addr_prop, 0);

        for (j = 0; j < 4; j++) {
            offset2 = fdt_subnode_offset(fdt, offset1, "feature");
            if (0 >= offset2) {
                log_info("uart[%d] feature NULL.\r\n", i);
                continue;
            }
            countindex = fdt_stringlist_count(fdt, offset2, feature_pin[j].featue_name);
            if (countindex != 1) {
                log_info("uart[%d] %s countindex = %d.\r\n", i, feature_pin[j].featue_name, countindex);
                continue;
            }
            result = fdt_stringlist_get(fdt, offset2, feature_pin[j].featue_name, 0, &lentmp);
            if ((lentmp != 4) || (memcmp("okay", result, 4) != 0)) {
                log_info("uart[%d] %s status = %s lentmp = %d\r\n", i, feature_pin[j].featue_name, result, lentmp);
                continue;
            }

            /* get pin_name */
            offset2 = fdt_subnode_offset(fdt, offset1, "pin");
            if (0 >= offset2) {
                log_info("uart[%d] pin NULL.\r\n", i);
                break;
            }
            addr_prop = fdt_getprop(fdt, offset2, feature_pin[j].pin_name, &lentmp);
            if (addr_prop == NULL) {
                log_info("uart[%d] %s NULL.\r\n", i, feature_pin[j].pin_name);
                continue;
            }
            feature_pin[j].value = BL_FDT32_TO_U8(addr_prop, 0);
        }
        log_info("id = %d, %s = %d, %s = %d, %s = %d, %s = %d baudrate = %ld.\r\n",
            id,
            feature_pin[0].pin_name, feature_pin[0].value,
            feature_pin[1].pin_name, feature_pin[1].value,
            feature_pin[2].pin_name, feature_pin[2].value,
            feature_pin[3].pin_name, feature_pin[3].value,
            baudrate);

        /*Flush UART FIFO*/
        bl_uart_flush(id);

        bl_uart_init(id, feature_pin[0].value, feature_pin[1].value,
            feature_pin[2].value, feature_pin[3].value, baudrate);

        log_info("bl_uart_init %d ok.\r\n", id);
        log_info("bl_uart_init %d baudrate = %ld ok.\r\n", id, baudrate);

        if (dev_uart_init(id, (const char *)path) != 0) {
            log_error("dev_uart_init err.\r\n");
        }
    }
    #undef UART_MODULE_MAX
    #undef PIN_INVALID_VALUE
}

int vfs_uart_init(uint32_t fdt, uint32_t dtb_uart_offset)
{
    if (inited == 1) {
        return VFS_SUCCESS;
    }

    fdt_uart_module_init((const void *)fdt, (int)dtb_uart_offset);

    inited = 1;

    return VFS_SUCCESS;
}
