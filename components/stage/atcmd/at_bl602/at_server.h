#ifndef _AT_SERVER_H_
#define _AT_SERVER_H_

#include <FreeRTOS.h>
#include <event_groups.h>
#include <atcmd/at_command.h>
#include "atcmd/at_command.h"
#include "at_private.h"

#define AT_UART_ID   1

#define AT_ASYNC_WIFI_CONNECTED (0x1 << 0)
#define AT_ASYNC_DATA_IN        (0x1 << 1)
#define AT_ASYNC_PASK_ERROR     (0x1 << 2)
#define AT_ASYNC_NO_AP_FOUND    (0x1 << 3)
#define ATCMDSEND_MAX_BUFF_SIZE (1024)

typedef struct at_sever {
    int wifi_mode;
    uint32_t uart_baud;
    int at_serial_fd;
    SemaphoreHandle_t at_serial_mtx;
    uint8_t queue_buf[ATCMDSEND_MAX_BUFF_SIZE];
    EventGroupHandle_t at_notify_eg;
} at_sever_t;

int at_server_init(void);
int at_server_notify(int event);

int at_serial_open(void);
int at_serial_close(void);
int at_key_value_set(char *key, void *p_value);
int at_key_value_get(char *key, void *p_value);
int at_serial_lock(void);
int at_serial_unlock(void);

int at_serial_cfg_set(uint32_t baud, uint8_t data_bit, uint8_t stop_bit, uint8_t parity, uint8_t hwfc);

#endif

