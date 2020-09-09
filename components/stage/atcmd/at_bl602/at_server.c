#include <FreeRTOS.h>
#include <semphr.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <task.h>
#include <timers.h>

#include <aos/kernel.h>
#include <aos/yloop.h>
#include <blog.h>
#include <vfs.h>
#include <device/vfs_uart.h>
#include <hal/soc/soc.h>
#include <vfs_file.h>
#include "at_server.h"
#include <easyflash.h>
#include <bl602_uart.h>

#if (AT_UART_ID==0)
#define UART_NAME "/dev/ttyS0"
#elif (AT_UART_ID==1)
#define UART_NAME "/dev/ttyS1"
#else
#error "AT uart id error"
#endif
#define AT_UART AT_UART_ID

static at_sever_t g_at_server;
extern at_config_t at_cfg;

AT_ERROR_CODE callback(AT_CALLBACK_CMD cmd, at_callback_para_t *para, at_callback_rsp_t *rsp);

static uint32_t __at_serial_baud(void)
{
    char *val = NULL;

    if ((val = ef_get_env(SAVE_KEY_UART_BAUD)) == NULL) {
        return 115200;
    }
    return atoi(val);
}

static uint32_t __at_wifi_mode(void)
{
    char *val = NULL;

    if ((val = ef_get_env(SAVE_KEY_WIFI_MODE)) == NULL) {
        return 0;
    }
    return atoi(val);
}

static int at_serial_read(unsigned char *buf, int size) {
  if (g_at_server.at_serial_fd < 0) {
    printf("[ERROR][AT] AT serial not init\r\n");
    return 0;
  }
  return aos_read(g_at_server.at_serial_fd, buf, size);
}

static int at_serial_write(unsigned char *buf, int len) {
  int ret;

  if (g_at_server.at_serial_fd < 0) {
    printf("[ERROR][AT] AT serial not init\r\n");
    return 0;
  }

  ret = aos_write(g_at_server.at_serial_fd, buf, len);
  return ret;
}

int at_serial_cfg_set(uint32_t baud, uint8_t data_bit, uint8_t stop_bit, uint8_t parity, uint8_t hwfc)
{
    const uint8_t uart_div = 3;
    UART_CFG_Type cfg_tmp =
    {
        160*1000*1000,                                        /* UART clock */
        115200,                                              /* UART Baudrate */
        UART_DATABITS_8,                                     /* UART data bits length */
        UART_STOPBITS_1,                                     /* UART data stop bits length */
        UART_PARITY_NONE,                                    /* UART no parity */
        DISABLE,                                             /* Disable auto flow control */
        DISABLE,                                             /* Disable rx input de-glitch function */
        DISABLE,                                             /* Disable RTS output SW control mode */
        UART_LSB_FIRST                                       /* UART each data byte is send out LSB-first */
    };

    cfg_tmp.baudRate = baud;
    cfg_tmp.uartClk = (160 * 1000 * 1000) / (uart_div + 1);

    if (data_bit >= 5 && data_bit <= 8) {
      cfg_tmp.dataBits = data_bit - 5;
    } else {
      return AEC_PARA_ERROR;
    }

    if (stop_bit >= 1 && stop_bit <= 3) {
      cfg_tmp.stopBits = stop_bit - 1;
    } else {
      return AEC_PARA_ERROR;
    }

    if (parity <= 2) {
      cfg_tmp.parity = parity;
    } else {
      return AEC_PARA_ERROR;
    }

    switch (hwfc) {
      case 0:
        cfg_tmp.ctsFlowControl = DISABLE;
        cfg_tmp.rtsSoftwareControl = DISABLE;
        break;

      case 1:
        cfg_tmp.ctsFlowControl = ENABLE;
        cfg_tmp.rtsSoftwareControl = DISABLE;
        break;

      case 2:
        cfg_tmp.ctsFlowControl = DISABLE;
        cfg_tmp.rtsSoftwareControl = ENABLE;
        break;

      case 3:
        cfg_tmp.ctsFlowControl = ENABLE;
        cfg_tmp.rtsSoftwareControl = ENABLE;
        break;

      default:
        return AEC_PARA_ERROR;
    }

    /* Disable all interrupt */
//    UART_IntMask(AT_UART, UART_INT_ALL, MASK);

    /* Disable uart before config */
    UART_Disable(AT_UART, UART_TXRX);

    /* UART init */
    UART_Init(AT_UART, &cfg_tmp);

    /* Enable tx free run mode */
    UART_TxFreeRun(AT_UART, ENABLE);

    /* FIFO Config*/
    //UART_FifoConfig(AT_UART, &fifoCfg);

    /* Enable uart */
    UART_Enable(AT_UART, UART_TXRX);
    return 0;
}

int at_serial_open(void)
{
    g_at_server.at_serial_fd = aos_open(UART_NAME, 0);
    return 0;
}
int at_serial_close(void)
{
    aos_close(g_at_server.at_serial_fd);
    g_at_server.at_serial_fd = -1;
    return 0;
}

int at_serial_lock(void) {
  if (xSemaphoreTakeRecursive(g_at_server.at_serial_mtx, portMAX_DELAY) != pdPASS) {
    printf("get send buf mutex failed!\r\n");
    return -1;
  }
  return 0;
}

int at_serial_unlock(void) {
  if (xSemaphoreGiveRecursive(g_at_server.at_serial_mtx) != pdPASS) {
    printf("give send buf mutex failed!\r\n");
    return -1;
  }
  return 0;
}

int at_data_output(char *buf, int size) {
  int len;

  if (xSemaphoreTakeRecursive(g_at_server.at_serial_mtx, portMAX_DELAY) != pdPASS) {
    printf("get send buf mutex failed!\r\n");
    return -1;
  }

  len = at_callback.dump_cb((uint8_t *)buf, size);
  xSemaphoreGiveRecursive(g_at_server.at_serial_mtx);

  return len;
}

extern int wifi_mgmr_sta_autoconnect_enable(void);
extern int wifi_mgmr_sta_autoconnect_disable(void);

int at_key_value_set(char *key, void *p_value)
{
    char buf[65] = {0};
    uint8_t *value = (uint8_t *)p_value;

    if (strcmp(key, SAVE_KEY_WIFI_MODE) == 0) {
        g_at_server.wifi_mode = *value;
        sprintf(buf, "%d", *value);
    } else if (strcmp(key, SAVE_KEY_WIFI_SSID) == 0) {
        sprintf(buf, "%s", value);
    } else if (strcmp(key, SAVE_KEY_WIFI_PASK) == 0) {
        sprintf(buf, "%s", value);
    } else if (strcmp(key, SAVE_KEY_WIFI_AUTO) == 0) {
        sprintf(buf, "%d", *value);
        if (*value) {
            wifi_mgmr_sta_autoconnect_enable();
        } else {
            wifi_mgmr_sta_autoconnect_disable();
        }
    } else if (strcmp(key, SAVE_KEY_UART_BAUD) == 0) {
        sprintf(buf, "%d", *(int *)value);
        g_at_server.uart_baud = *(int *)value;
        aos_ioctl(g_at_server.at_serial_fd, IOCTL_UART_IOC_BAUD_MODE, *(int *)value);
    } else {
        sprintf(buf, "%d", *value);
    }

    ef_set_env(key, buf);
    return  0;
}

int at_key_value_get(char *key, void *p_value)
{
    char *val = NULL;
    uint8_t *value = (uint8_t *)p_value;

    if (value == NULL) {
        return -1;
    }
    if ((val = ef_get_env(key)) == NULL) {
        if (strcmp(key, SAVE_KEY_WIFI_MODE) == 0) {
            *value = g_at_server.wifi_mode;
        } else if (strcmp(key, SAVE_KEY_WIFI_PASK) == 0) {
            value[0] = '\0';
        } else if (strcmp(key, SAVE_KEY_UART_BAUD) == 0) {
            *(int *)value = g_at_server.uart_baud;
        } else if (strcmp(key, SAVE_KEY_UART_DATABIT) == 0) {
            *(int *)value = 8;
        } else if (strcmp(key, SAVE_KEY_UART_STOPBIT) == 0) {
            *(int *)value = 1;
        } else if (strcmp(key, SAVE_KEY_UART_PARITY) == 0) {
            *(int *)value = 0;
        } else if (strcmp(key, SAVE_KEY_UART_HWFC) == 0) {
            *(int *)value = 0;
        } else {
            return -1;
        }
    } else {
        if (strcmp(key, SAVE_KEY_WIFI_SSID) == 0) {
            strcpy((char *)value, val);
        } else if (strcmp(key, SAVE_KEY_WIFI_PASK) == 0) {
            strcpy((char *)value, val);
        } else {
            *(int *)value = atoi(val);
        }
    }
    return 0;
}

s32 at_dump_noend(char *format, ...)
{
    int len;
    va_list vp;
    uint8_t *p_buf = NULL;

    if (xSemaphoreTakeRecursive(g_at_server.at_serial_mtx, portMAX_DELAY) != pdPASS) {
      printf("get send buf mutex failed!\r\n");
      return -1;
    }

    va_start(vp, format);
    len = vsnprintf(NULL, 0, format, vp);
    len += 10;
    p_buf = pvPortMalloc(len);
    if (p_buf == NULL) {
        printf("mem not enought\r\n");
        return -1;
    }
    len = vsnprintf((char *)p_buf, len, format, vp);
    va_end(vp);

    len = at_callback.dump_cb(p_buf, len);

    //printf("%s,%d\r\n", p_buf, strlen((char *)p_buf));
    xSemaphoreGiveRecursive(g_at_server.at_serial_mtx);
    vPortFree(p_buf);

    return len;
}

void at_async_event(void *param) {

  EventBits_t event;
  extern int wifi_mgmr_sta_autoconnect_disable(void);

  while (1) {
    event = xEventGroupWaitBits(g_at_server.at_notify_eg, (EventBits_t)0xFF, pdTRUE, pdFALSE, portMAX_DELAY);
    if (event & AT_ASYNC_WIFI_CONNECTED) {
      at_dump("+EVT:0:wifi connected");
    } else if (event & AT_ASYNC_DATA_IN) {
      at_dump("+EVT:1:data in");
    } else if (event & AT_ASYNC_PASK_ERROR) {
      at_dump("+EVT:2:wifi pask error");
    } else if (event & AT_ASYNC_NO_AP_FOUND) {
      at_dump("+EVT:3:wifi no ap found");
    } else {
      printf("[AT]:Unknow event!\r\n");
    }
  }
}

void at_cmd_init(void) {
  at_callback_t at_cb;

  // at_wakeup_gpio_init(5,0);
  at_queue_init(g_at_server.queue_buf, sizeof(g_at_server.queue_buf), at_serial_read);

  at_cb.handle_cb = callback;
  at_cb.dump_cb = at_serial_write;

  at_init(&at_cb);
}

// AT execute thread
static void at_cmd_exec(void *param) {
  at_parse();

  printf("return form at_parse!\r\n");
  while (1);
}

int at_server_init(void) {
  BaseType_t ret;
  TaskHandle_t async_handle;
  int echo = 0;
  uint8_t data_bit = 8, stop_bit = 1, parity = 0, hwfc = 0;
  memset(&g_at_server, 0, sizeof(g_at_server));

  at_serial_open();
  if (g_at_server.at_serial_fd < 0) {
    printf("open ttyS1 failed!\r\n");
    return -1;
  }
  at_key_value_get(SAVE_KEY_UART_ECHO, &echo);
  at_cfg.localecho1 = echo;
  g_at_server.wifi_mode = __at_wifi_mode();
  g_at_server.uart_baud = __at_serial_baud();

  at_key_value_get(SAVE_KEY_UART_DATABIT, &data_bit);
  at_key_value_get(SAVE_KEY_UART_STOPBIT, &stop_bit);
  at_key_value_get(SAVE_KEY_UART_PARITY, &parity);
  at_key_value_get(SAVE_KEY_UART_HWFC, &hwfc);
  ret = at_serial_cfg_set(g_at_server.uart_baud,
                          data_bit,
                          stop_bit,
                          parity,
                          hwfc);
  if (ret != 0) {
    return AEC_PARA_ERROR;
  }
  // create send mutex
  g_at_server.at_serial_mtx = xSemaphoreCreateRecursiveMutex();
  if (g_at_server.at_serial_mtx == NULL) {
    printf("create send buf mutex failed!\r\n");
    aos_close(g_at_server.at_serial_fd);
    return -1;
  }

  g_at_server.at_notify_eg = xEventGroupCreate();
  if (g_at_server.at_notify_eg == NULL) {
    printf("create async notify failed\r\n");

    vSemaphoreDelete(g_at_server.at_serial_mtx);
    aos_close(g_at_server.at_serial_fd);

    return -1;
  }

  // init AT server
  at_cmd_init();

  ret = xTaskCreate(at_async_event, "at async", 256, NULL, 3, &async_handle);
  if (ret != pdPASS) {
    printf("create AT notify thread failed!\r\n");
    vEventGroupDelete(g_at_server.at_notify_eg);
    vSemaphoreDelete(g_at_server.at_serial_mtx);
    aos_close(g_at_server.at_serial_fd);

    return -1;
  }

  ret = xTaskCreate(at_cmd_exec, "at_server", 1024, NULL, 0, NULL);
  if (ret != pdPASS) {
    printf("create at server task failed!\r\n");

    vTaskDelete(async_handle);
    vEventGroupDelete(g_at_server.at_notify_eg);
    vSemaphoreDelete(g_at_server.at_serial_mtx);
    aos_close(g_at_server.at_serial_fd);

    return -1;
  }

  printf("at_server start success!\r\n");
  return 0;
}

int at_server_notify(int event) {
    // notify AT server
    xEventGroupSetBits(g_at_server.at_notify_eg, event);
    return 0;
}

void at_uart_reinit(at_serial_para_t *at_para)
{
//    at_para.baudrate = para->u.uart.uartBaud;
//    at_para.dataBit = para->u.uart.dataBit;
//    at_para.parity = para->u.uart.parity;
//    at_para.stopBit = para->u.uart.stopBit;
//    at_para.hwfc = para->u.uart.hwfc;
  aos_ioctl(g_at_server.at_serial_fd, IOCTL_UART_IOC_BAUD_MODE, at_para->baudrate);
}
