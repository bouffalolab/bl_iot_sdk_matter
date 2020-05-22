#include <device/vfs_uart.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/soc/uart.h>
#include <aos/kernel.h>

#include "bl_uart.h"
#include "hal_uart.h"

#include <utils_log.h>


typedef struct uart_priv_data {
    aos_mutex_t    mutex;
} uart_priv_data_t;

static int8_t inited;
static uart_dev_t *dev_uart0 = NULL;
static uart_dev_t *dev_uart1 = NULL;

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

int32_t hal_uart_send_trigger(uart_dev_t *uart)
{
    bl_uart_int_tx_enable(uart->port);
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

int vfs_uart_init(uint32_t fdt, uint32_t dtb_uart_offset)
{
    if (inited == 1) {
        return VFS_SUCCESS;
    }

    dev_uart_init(0, "/dev/ttyS0");

    inited = 1;

    return VFS_SUCCESS;
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

void hal_uart_setconfig(uart_dev_t *uart, uint32_t baud, hal_uart_parity_t parity)
{
}
