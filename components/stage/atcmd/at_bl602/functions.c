/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <task.h>
#include <time.h>
#include <wifi_mgmr_ext.h>
#include <lwip/tcpip.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip/altcp.h>
#include <lwip/altcp_tcp.h>
#include <lwip/err.h>
#include "atcmd/at_command.h"
#include "at_private.h"
#include <easyflash.h>
#include <bl602_uart.h>
#include "at_server.h"
#include "hal_hbn.h"
#include "bl_hbn.h"
#if 0
#include "kernel/os/os.h"
#include "sys/interrupt.h"

#include "common/cmd/cmd_ping.h"
#include "common/cmd/cmd_util.h"
#include "common/cmd/cmd_wlan.h"
#include "net/wlan/wlan.h"

#include "common/framework/net_ctrl.h"
#include "lwip/inet.h"

#include "driver/chip/hal_rtc.h"

#include "image/fdcm.h"

#include "errno.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "serial.h"

#include "ap_socket.h"
#include "common/framework/sys_ctrl/sys_ctrl.h"
#include "common/framework/sysinfo.h"
#include "driver/chip/hal_gpio.h"
#include "driver/chip/hal_wdg.h"
#include "lwip/dns.h"
#include "net/wlan/wlan_ext_req.h"

#include "ota/ota.h"

#include "atcmd.h"
#include "driver/chip/hal_wakeup.h"
#endif
#include <hal_sys.h>
#include "bl_gpio.h"
#include "hal_gpio.h"
#include <http_client.h>

#define FUN_DEBUG_ON 0

#if FUN_DEBUG_ON == 1
#define FUN_DEBUG(fmt...)                           \
  {                                                 \
    printf("file:%s line:%d ", __FILE__, __LINE__); \
      printf(fmt);                                    \
  }
#else
#define FUN_DEBUG(fmt...)
#endif

#define MANUFACTURER "BOUFFALOLAB"
#define MODEL "serial-to-wifi"
#define SERIAL "01234567"
//#define MAC               {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}

#define CONFIG_FDCM_FLASH (0)

//#define CONFIG_FDCM_ADDR  0x120000UL
//#define CONFIG_FDCM_SIZE  0x10000UL

#define CONFIG_FDCM_ADDR (0x200000UL - 0x8000UL)
#define CONFIG_FDCM_SIZE (0x4000UL)

#define CONFIG_CONTAINNER_SIZE sizeof(config_containner_t)

#define MAX_SCAN_RESULTS 50
#define MAX_SOCKET_NUM 4
#define IP_ADDR_SIZE 15
#define SOCKET_CACHE_BUFFER_SIZE 1024

#define SERVER_THREAD_STACK_SIZE (1 * 1024)

typedef struct {
  s32 cmd;
  AT_ERROR_CODE (*handler)(at_callback_para_t *para, at_callback_rsp_t *rsp);
} callback_handler_t;

typedef struct {
  u32 cnt;
  at_config_t cfg;
} config_containner_t;

enum socket_state {
  SOCK_IDLE_CLOSE = 0,
  SOCK_SERVER_LISTENING,
  SOCK_CLIENT_CONNECTING,
  SOCK_CLIENT_CONNECTED
};

enum socket_type {
  SOCK_TYPE_TCP = 1,
  SOCK_TYPE_UDP,
  SOCK_TYPE_TLS
};

typedef struct {
  char ip[IP_ADDR_SIZE + 1];
  char hostname[AT_PARA_MAX_SIZE];
  u32 port;
  enum socket_type protocol;  // 1:TCP , 2:UDP , 3:TLS
  union {
    struct udp_pcb *udp;
    struct altcp_pcb *tcp;
    struct altcp_pcb *tls;
  } pcb;
  enum socket_state status;
} connect_t;

typedef struct {
  s32 count;
  connect_t connect[MAX_SOCKET_NUM];
} network_t;

typedef struct {
  s32 mode;  // 0:no connections 1: sta connections 2:ap connections
} system_status_t;

#if 0
typedef struct {
  u32 flag;
  s32 offset;
  s32 cnt;
  u8 buffer[SOCKET_CACHE_BUFFER_SIZE];
} socket_cache_t;
#endif

typedef struct {
  s16 port;
  s32 protocol;
} server_arg_t;

typedef struct {
  u32 flag;
  s16 port;
  s32 protocol;
  union {
    struct altcp_pcb *tcp;
    struct altcp_pcb *tls;
    struct udp_pcb *udp;
  } pcb;
} server_ctrl_t;

static server_arg_t g_server_arg;
static server_ctrl_t g_server_ctrl;
// static system_status_t g_status;
static u32 g_errorcode = 0;
static network_t networks;
static wifi_interface_t g_wifi_interface;

static u32 g_server_enable = 0;
static SemaphoreHandle_t g_server_sem;
// static uint16_t net_evevt_state = NET_CTRL_MSG_NETWORK_DOWN;
int is_disp_ipd = 1;

// cipsend data totlen
static int32_t cipsend_totlen = 0;

enum atc_cwjap_cur_type {
  ATC_CWJAP_CUR_OK = 0,
  ATC_CWJAP_CUR_TIMEOUT = '1',
  ATC_CWJAP_CUR_PSK_INVALID = '2',
  ATC_CWJAP_CUR_SSID_NOMATCH = '3',

};

extern int at_serial_lock(void);
extern int at_serial_unlock(void);

static AT_ERROR_CODE http_url_req(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipsend(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipstart(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipsendbuf(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipclose(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE tcpserver(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE set_apcfg(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwqap(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE deep_sleep(at_callback_para_t * para, at_callback_rsp_t * rsp);
static AT_ERROR_CODE cwmode_cur(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwmode_cur_get(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwjap_cur(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwjap_info(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE version(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE restory(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE uart_set(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE uart_get(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE scan(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE ap_sta_get(at_callback_para_t * para, at_callback_rsp_t * rsp);
static AT_ERROR_CODE sockon(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE reset(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipstatus(at_callback_para_t *para, at_callback_rsp_t *rsp);

#if 0
static AT_ERROR_CODE cipdns_cur(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE act(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE mode(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE save(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE load(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE status(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE factory(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE peer(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE ping(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sockw(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sockq(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sockr(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sockc(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sockd(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE wifi(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE reassociate(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE gpioc(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE gpior(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE gpiow(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sleep(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE wakeupgpio(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwlapopt(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwlap(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwdhcp_cur(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipstamac_cur(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipsta_cur(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cwhostname(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipdomain(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE tcpservermaxconn(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipmux(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE cipmode(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE ciprecvdata(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE ciprecvmode(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sysiosetcfg(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sysiogetcfg(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE syssetiodir(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE syssetgpio(at_callback_para_t *para, at_callback_rsp_t *rsp);
static AT_ERROR_CODE sysreadgpio(at_callback_para_t *para, at_callback_rsp_t *rsp);
#endif

static const callback_handler_t callback_tbl[] = {
#if 0
    {ACC_ACT,               act},
#endif
    {ACC_RST,               reset},
#if 0
    {ACC_MODE,              mode},
    {ACC_SAVE,              save},
    {ACC_LOAD,              load},
    {ACC_STATUS,            status},
    {ACC_FACTORY,           factory},
    {ACC_PEER,              peer},
    {ACC_PING,              ping},
    {ACC_SOCKON,            sockon},
    {ACC_SOCKW,             sockw},
    {ACC_SOCKQ,             sockq},
    {ACC_SOCKR,             sockr},
    {ACC_SOCKC,             sockc},
    {ACC_SOCKD,             sockd},
    {ACC_WIFI,              wifi},
    {ACC_REASSOCIATE,       reassociate},
    {ACC_GPIOC,             gpioc},
    {ACC_GPIOR,             gpior},
    {ACC_GPIOW,             gpiow},
#endif
    {ACC_SCAN,              scan},
    {ACC_CIPSTATUS,         cipstatus},
    {ACC_GMR,               version},
    {ACC_RESTORE,           restory},
    {ACC_UART_SET,          uart_set},
    {ACC_UART_GET,          uart_get},
    {ACC_DEEP_SLEEP,        deep_sleep},
#if 0
    {ACC_SLEEP,             sleep},
    {ACC_WAKEUPGPIO,        wakeupgpio},
#endif
    {ACC_CWMODE_CUR,        cwmode_cur},
    {ACC_CWMODE_GET,        cwmode_cur_get},
    {ACC_CWJAP_CUR, cwjap_cur},
    {ACC_CWJAP_INFO, cwjap_info},
#if 0
    {ACC_CWLAPOPT,          cwlapopt},
    {ACC_CWLAP,             cwlap},
#endif
    {ACC_CWQAP, cwqap},
#if 0
    {ACC_CWDHCP_CUR,        cwdhcp_cur},
    {ACC_CIPSTAMAC_CUR,     cipstamac_cur},
    {ACC_CIPSTA_CUR,        cipsta_cur},
    {ACC_CWHOSTNAME,        cwhostname},
    {ACC_CIPDOMAIN,         cipdomain},
#endif
    {ACC_APCFG,             set_apcfg},
    {ACC_AP_STA_GET, ap_sta_get},
    {ACC_CIPSTART, cipstart},
    {ACC_CIPSENDBUF, cipsendbuf},
    {ACC_CIPCLOSE, cipclose},
    {ACC_TCPSERVER, tcpserver},
    {ACC_CIPSEND, cipsend},
    {ACC_HTTP_REQ, http_url_req},
#if 0
    {ACC_TCPSERVERMAXCONN,  tcpservermaxconn},
    {ACC_CIPMUX,            cipmux},
    {ACC_CIPMODE,           cipmode},
    {ACC_CIPRECVDATA,       ciprecvdata},
    {ACC_CIPRECVMODE,       ciprecvmode},
    {ACC_CIPDNS_CUR,        cipdns_cur},
    {ACC_SYSIOSETCFG,       sysiosetcfg},
    {ACC_SYSIOGETCFG,       sysiogetcfg},
    {ACC_SYSGPIODIR,        syssetiodir},
    {ACC_SYSGPIOWRITE,      syssetgpio},
    {ACC_SYSGPIOREAD,       sysreadgpio},
#endif
};

#if 0
static const u32 channel_freq_tbl[] = {
    2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472
};

static const char *event[] = {
    "wlan connected",
    "wlan disconnected",
    "wlan scan success",
    "wlan scan failed",
    "wlan 4way handshake failed",
    "wlan connect failed",
    "wlan connect loss",
    "network up",
    "network down",
};

static const fdcm_handle_t fdcm_hdl_tbl[] = {
    {CONFIG_FDCM_FLASH, CONFIG_FDCM_ADDR, CONFIG_FDCM_SIZE},
    {CONFIG_FDCM_FLASH, CONFIG_FDCM_ADDR - CONFIG_FDCM_SIZE, CONFIG_FDCM_SIZE}
};

/* factory default */
static const at_config_t default_cfg = {
    .blink_led = 0,
    .wind_off_low = 0x0,
    .wind_off_medium = 0x0,
    .wind_off_high = 0x0,
    .user_desc = "XRADIO-AP",
    .escape_seq = "at+s.",
    .localecho1 = 0,
    .console1_speed = 115200,
    .console1_hwfc = 0,
    .console1_enabled = 0,
    .sleep_enabled = 0,
    .standby_enabled = 0,
    .standby_time = 10,
    .wifi_tx_msdu_lifetime = 0,
    .wifi_rx_msdu_lifetime = 0,
    .wifi_operational_mode = 0x00000011,
    .wifi_beacon_wakeup = 1,
    .wifi_beacon_interval = 100,
    .wifi_listen_interval = 0,
    .wifi_rts_threshold = 3000,
    .wifi_ssid = "iot-ap",
    .wifi_ssid_len = 6,
    .wifi_channelnum = 6,
    .wifi_opr_rate_mask = 0xFFFFFFFF,
    .wifi_bas_rate_mask = 0x0000000F,
    .wifi_mode = 1,
    .wifi_auth_type = 0,
    .wifi_powersave = 1,
    .wifi_tx_power = 18,
    .wifi_rssi_thresh = -50,
    .wifi_rssi_hyst = 10,
    .wifi_ap_idle_timeout = 120,
    .wifi_beacon_loss_thresh = 10,
    .wifi_priv_mode = 2,
    /*.wifi_wep_keys[4][16],*/
    /*.wifi_wep_key_lens[4],*/
    .wifi_wep_default_key = 0,
    /*.wifi_wpa_psk_raw[32],*/
    /*.wifi_wpa_psk_text[64],*/
    .ip_use_dhcp = 1,
    .ip_use_httpd = 1,
    .ip_mtu = 1500,
    .ip_hostname = "xr-iot-dev",
    .ip_apdomainname = "xradio.com",
    .ip_ipaddr = {192, 168, 0, 123},
    .ip_netmask = {255, 255, 255, 0},
    .ip_gw = {192, 168, 0, 1},
    .ip_dns = {192, 168, 0, 1},
    .ip_http_get_recv_timeout = 1000,
    .ip_dhcp_timeout = 20,
    .ip_sockd_timeout = 250,
};

static __always_inline int server_is_isr_context(void)
{
    return __get_IPSR();
}


static void server_mutex_lock(void)
{
    if (g_server_mutex == NULL) {
        g_server_mutex = xSemaphoreCreateRecursiveMutex();
    }
    xSemaphoreTakeRecursive(g_server_mutex, portMAX_DELAY);
}

static void server_mutex_unlock(void)
{
    if (g_server_mutex == NULL) {
        return;
    }

    xSemaphoreGiveRecursive(g_server_mutex);
}
#endif

u32 at_get_errorcode(void) { return g_errorcode; }

#if 0
s32 at_cmdline(char *buf, u32 size)
{
    u32 i;

    for (i = 0; i < size; i++) {
        if (buf[i] == AT_LF) {
            return i+1;
        } else if (buf[i] == AT_CR) {
            if (((i+1) < size) && (buf[i+1] == AT_LF)) {
                return i+2;
            } else {
                return i+1;
            }
        }
    }

    return -1;
}




static void at_wakeup_pin_irq_cb(void *arg)
{
    printf("SYSTE wake up!\r\n");
}

static int wkgpio_pins[] = {GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6,GPIO_PIN_7,GPIO_PIN_7,\
                        GPIO_PIN_19,GPIO_PIN_20,GPIO_PIN_21,GPIO_PIN_22,GPIO_PIN_23};

void at_wakeup_gpio_init(int gpioId,int edge)
{
    GPIO_InitParam param;
    GPIO_IrqParam Irq_param;

    if(edge == 1)
    {
        param.driving = GPIO_DRIVING_LEVEL_1;
        param.pull = GPIO_PULL_DOWN;
        param.mode = GPIOx_Pn_F6_EINT;
        HAL_GPIO_Init(GPIO_PORT_A, wkgpio_pins[gpioId], &param);

        Irq_param.event = GPIO_IRQ_EVT_RISING_EDGE;
        Irq_param.callback = at_wakeup_pin_irq_cb;
        Irq_param.arg = (void *)NULL;
        HAL_GPIO_EnableIRQ(GPIO_PORT_A, wkgpio_pins[gpioId], &Irq_param);
        HAL_Wakeup_SetIO(gpioId, WKUPIO_WK_MODE_RISING_EDGE, GPIO_PULL_DOWN);
    }
    else if(edge == 0)
    {
        param.driving = GPIO_DRIVING_LEVEL_1;
        param.pull = GPIO_PULL_UP;
        param.mode = GPIOx_Pn_F6_EINT;
        HAL_GPIO_Init(GPIO_PORT_A, wkgpio_pins[gpioId], &param);

        Irq_param.event = GPIO_IRQ_EVT_FALLING_EDGE;
        Irq_param.callback = at_wakeup_pin_irq_cb;
        Irq_param.arg = (void *)NULL;
        HAL_GPIO_EnableIRQ(GPIO_PORT_A, wkgpio_pins[gpioId], &Irq_param);
        HAL_Wakeup_SetIO(gpioId, WKUPIO_WK_MODE_FALLING_EDGE, GPIO_PULL_UP);
    }

}

#endif
AT_ERROR_CODE callback(AT_CALLBACK_CMD cmd, at_callback_para_t *para, at_callback_rsp_t *rsp) {
  s32 i;

  FUN_DEBUG("callback cmd = %d\r\n", cmd);

  for (i = 0; i < TABLE_SIZE(callback_tbl); i++) {
    if (cmd == callback_tbl[i].cmd) {
      if (callback_tbl[i].handler != NULL) {
        return callback_tbl[i].handler(para, rsp);
      } else {
        /* FUN_DEBUG("callback cmd = %d is unimplimented!\r\n", cmd); */

        return AEC_UNDEFINED;
      }
    }
  }

  FUN_DEBUG("callback cmd = %d is unsupported!\r\n", cmd);

  return AEC_UNSUPPORTED;
}
#if 0

static AT_ERROR_CODE act(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    AT_ERROR_CODE aec = AEC_OK;
    uint8_t ap_ssid[32+1];
    uint8_t ap_psk[] = "12345678";

    switch (para->cfg->wifi_mode) {
    case 0: /* IDLE */

        net_switch_mode(WLAN_MODE_STA);
        wlan_sta_disable();

        break;

    case 1: /* STA */

        net_switch_mode(WLAN_MODE_STA);
        wlan_sta_set(para->cfg->wifi_ssid, para->cfg->wifi_ssid_len, (uint8_t *)para->cfg->wifi_wpa_psk_text);
        wlan_sta_enable();

        break;

    case 2: /* AP */

        net_switch_mode(WLAN_MODE_HOSTAP);
        wlan_ap_disable();
        snprintf((char *)ap_ssid, 32, "xr-ap-%02x%02x%02x", para->cfg->nv_wifi_macaddr[3], para->cfg->nv_wifi_macaddr[4], para->cfg->nv_wifi_macaddr[5]);
        wlan_ap_set(ap_ssid, strlen((char *)ap_ssid), ap_psk);
        wlan_ap_enable();

        break;
#if 0
    case 3: /* IBSS */

        break;
#endif
    default:

        aec = AEC_PARA_ERROR;

        break;
    }

    return aec;
}
#endif
static AT_ERROR_CODE reset(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    AT_ERROR_CODE aec = AEC_OK;

    at_dump("OK");

    vTaskDelay(pdMS_TO_TICKS(2));
    hal_reboot();

    return aec;
}
#if 0
static AT_ERROR_CODE mode(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    AT_ERROR_CODE res = AEC_OK;
    s32 id;
    u8 *buffer;
    s32 len;
    s32 timeout_ms=10;
    fd_set fdset_r,fdset_w;
    int fd;
    struct sockaddr_in address;
    socklen_t addr_len;
    char ip[IP_ADDR_SIZE+1];
    s16 port;
    s32 protocol;

    if (!g_server_enable) { /* as client */
        id = 0;
        fd = networks.connect[id].fd;
        protocol = networks.connect[id].protocol;
        port = networks.connect[id].port;
        strncpy(ip, networks.connect[id].ip, sizeof(ip));

        if (networks.count > 0) {
            if (networks.connect[id].flag) {
                int rc = -1;
                struct timeval tv;

                FD_ZERO(&fdset_w);
                FD_ZERO(&fdset_r);
                FD_SET(fd, &fdset_w);
                FD_SET(fd, &fdset_r);

                tv.tv_sec = timeout_ms / 1000;
                tv.tv_usec = (timeout_ms % 1000) * 1000;

                rc = select(fd + 1, &fdset_r, NULL, NULL, &tv);
                if (rc > 0) {
                    if (FD_ISSET(fd, &fdset_r)) {
                        buffer = socket_cache[id].buffer;
                        len = SOCKET_CACHE_BUFFER_SIZE;

                        if (protocol == 0) { /* TCP */
                            rc = recv(fd, buffer, len, 0);
                        } else if (protocol == 1) { /* UDP */
                            address.sin_port = htons(port);
                            address.sin_family = AF_INET;
                            address.sin_addr.s_addr= inet_addr(ip);

                            addr_len = sizeof(address);

                            rc = recvfrom(fd, buffer, len, 0, (struct sockaddr *)&address, &addr_len);
                        }

                        if (rc > 0) {
                            /* received normally */
                            serial_write(buffer, rc);
                        } else if (rc == 0) {
                            /* has disconnected with server */
                            res = AEC_DISCONNECT;
                        } else {
                            /* network error */
                            res = AEC_NETWORK_ERROR;
                        }
                    }
                } else if (rc == 0) {
                    /* timeouted and sent 0 bytes */
                } else {
                    /* network error */
                    res = AEC_NETWORK_ERROR;
                }

                rc = select(fd + 1, NULL, &fdset_w, NULL, &tv);
                if (rc > 0) {
                    if (FD_ISSET(fd, &fdset_w)) {
                        buffer = para->u.mode.buf;
                        len = para->u.mode.len;

                        if (buffer != NULL && len >0) {
                            if (protocol == 0) { /* TCP */
                                rc = send(fd, buffer, len, 0);
                            } else if (protocol == 1) { /* UDP */
                                address.sin_port = htons(port);
                                address.sin_family = AF_INET;
                                address.sin_addr.s_addr= inet_addr(ip);

                                addr_len = sizeof(address);
                                rc = sendto(fd, buffer, len, 0, (struct sockaddr *)&address, sizeof(address));
                            }

                            if (rc > 0) {
                                /* do nothing */
                            } else if (rc == 0) {
                                /* disconnected with server */
                                res = AEC_DISCONNECT;
                            } else {
                                /* network error */
                                res = AEC_NETWORK_ERROR;
                            }
                        }
                    }
                } else if (rc == 0) {
                    /* timeouted and sent 0 bytes */
                } else {
                    /* network error */
                    res = AEC_NETWORK_ERROR;
                }
            } else {
                res = AEC_SWITCH_MODE;
            }
        } else {
            res = AEC_SWITCH_MODE;
        }
    } else { /* as server */
        if (!g_server_ctrl.flag) {
            g_server_ctrl.protocol = g_server_arg.protocol;

            server_mutex_lock();
            g_server_ctrl.conn_fd = g_server_net.conn_fd;
            g_server_ctrl.flag = g_server_net.flag;
            server_mutex_unlock();
        }

        if (g_server_ctrl.flag) {
            int rc = -1;
            struct timeval tv;

            port = g_server_ctrl.port;
            protocol = g_server_ctrl.protocol;
            fd = g_server_ctrl.conn_fd;

            FD_ZERO(&fdset_w);
            FD_ZERO(&fdset_r);
            FD_SET(fd, &fdset_w);
            FD_SET(fd, &fdset_r);

            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            rc = select(fd + 1, &fdset_r, NULL, NULL, &tv);
            if (rc > 0) {
                if (FD_ISSET(fd, &fdset_r)) {
                    buffer = socket_cache[MAX_SOCKET_NUM].buffer;
                    len = SOCKET_CACHE_BUFFER_SIZE;

                    if (protocol == 0) { /* TCP */
                        rc = recv(fd, buffer, len, 0);
                    } else if (protocol == 1) { /* UDP */
                        address.sin_port = htons(port);
                        address.sin_family = AF_INET;
                        address.sin_addr.s_addr= inet_addr(ip);

                        addr_len = sizeof(address);

                        rc = recvfrom(fd, buffer, len, 0, (struct sockaddr *)&address, &addr_len);
                    }

                    if (rc > 0) {
                        /* received normally */
                        serial_write(buffer, rc);
                    } else if (rc == 0) {
                        /* has disconnected with server */
                        if (protocol == 0) { /* TCP */
                            server_mutex_lock();
                            g_server_net.conn_fd = -1;
                            g_server_net.flag = 0;
                            server_mutex_unlock();

                            OS_SemaphoreRelease(&g_server_sem);

                            g_server_ctrl.flag = 0;
                        }

                        res = AEC_DISCONNECT;
                    } else {
                        /* network error */
                        res = AEC_NETWORK_ERROR;
                    }
                }
            } else if (rc == 0) {
                /* timeouted and sent 0 bytes */
            } else {
                /* network error */
                res = AEC_NETWORK_ERROR;
            }

            rc = select(fd + 1, NULL, &fdset_w, NULL, &tv);
            if (rc > 0) {
                if (FD_ISSET(fd, &fdset_w)) {
                    buffer = para->u.mode.buf;
                    len = para->u.mode.len;

                    if (buffer != NULL && len >0) {
                        if (protocol == 0) { /* TCP */
                            rc = send(fd, buffer, len, 0);
                        } else if (protocol == 1) { /* UDP */
                            FUN_DEBUG("Unsupported!\r\n");
                        }

                        if (rc > 0) {
                            /* do nothing */
                        } else if (rc == 0) {
                            /* disconnected with server */
                            res = AEC_DISCONNECT;
                        } else {
                            /* network error */
                            res = AEC_NETWORK_ERROR;
                        }
                    }
                }
            } else if (rc == 0) {
                /* timeouted and sent 0 bytes */
            } else {
                /* network error */
                res = AEC_NETWORK_ERROR;
            }
        } else {
            res = AEC_DISCONNECT;
        }
    }

    return res;

}

static AT_ERROR_CODE save(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    config_containner_t *containner;
    s32 i;
    fdcm_handle_t *fdcm_hdl;
    u32 flag[2];
    s32 idx,cnt;

    containner = (config_containner_t *)malloc(2*sizeof(config_containner_t));

    if (containner == NULL) {
        FUN_DEBUG("malloc faild.\r\n");
        return AEC_NOT_ENOUGH_MEMORY;
    }

    for (i=0; i<TABLE_SIZE(fdcm_hdl_tbl); i++) {
        flag[i] = 1;

        fdcm_hdl = fdcm_open(fdcm_hdl_tbl[i].flash, fdcm_hdl_tbl[i].addr, fdcm_hdl_tbl[i].size);

        if (fdcm_hdl == NULL) {
            FUN_DEBUG("fdcm_open faild.\r\n");
            free(containner);
            return AEC_UNDEFINED;
        }

        if (fdcm_read(fdcm_hdl, &containner[i], CONFIG_CONTAINNER_SIZE) != CONFIG_CONTAINNER_SIZE) {
            flag[i] = 0;
        }

        fdcm_close(fdcm_hdl);
    }

    if(flag[0] && flag[1]) {
        if (containner[0].cnt > containner[1].cnt) {
            idx = 1;
            cnt = containner[0].cnt;
        } else {
            idx = 0;
            cnt = containner[1].cnt;
        }
    } else if(flag[0]) {
        idx = 1;
        cnt = containner[0].cnt;
    } else if(flag[1]) {
        idx = 0;
        cnt = containner[1].cnt;
    } else {
        idx = 0;
        cnt = 0;
    }

    fdcm_hdl = fdcm_open(fdcm_hdl_tbl[idx].flash, fdcm_hdl_tbl[idx].addr, fdcm_hdl_tbl[idx].size);

    if (fdcm_hdl == NULL) {
        FUN_DEBUG("fdcm_open faild.\r\n");
        free(containner);
        return AEC_UNDEFINED;
    }

    containner[idx].cnt = cnt + 1;
    memcpy(&containner[idx].cfg, para->cfg, sizeof(at_config_t));

    if (fdcm_write(fdcm_hdl, &containner[idx], CONFIG_CONTAINNER_SIZE) != CONFIG_CONTAINNER_SIZE) {
        FUN_DEBUG("fdcm_write faild.\r\n");
        fdcm_close(fdcm_hdl);
        free(containner);
        return AEC_UNDEFINED;
    }

    fdcm_close(fdcm_hdl);
    free(containner);

    return AEC_OK;
}

static AT_ERROR_CODE load(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    config_containner_t *containner;
    s32 i;
    fdcm_handle_t *fdcm_hdl;
    u32 flag[2];
    s32 idx;

    containner = (config_containner_t *)malloc(2*sizeof(config_containner_t));

    if (containner == NULL) {
        FUN_DEBUG("malloc faild.\r\n");
        free(containner);
        return AEC_UNDEFINED;
    }

    for (i=0; i<TABLE_SIZE(fdcm_hdl_tbl); i++) {
        flag[i] = 1;

        fdcm_hdl = fdcm_open(fdcm_hdl_tbl[i].flash, fdcm_hdl_tbl[i].addr, fdcm_hdl_tbl[i].size);

        if (fdcm_hdl == NULL) {
            FUN_DEBUG("fdcm_open faild.\r\n");
            free(containner);
            return AEC_UNDEFINED;
        }

        if (fdcm_read(fdcm_hdl, &containner[i], CONFIG_CONTAINNER_SIZE) != CONFIG_CONTAINNER_SIZE) {
            flag[i] = 0;
        }

        fdcm_close(fdcm_hdl);
    }

    if(flag[0] && flag[1]) {
        if (containner[0].cnt > containner[1].cnt) {
            idx = 0;
        } else {
            idx = 1;
        }
    } else if(flag[0]) {
        idx = 0;
    } else if(flag[1]) {
        idx = 1;
    } else {
        FUN_DEBUG("load fiald.\r\n");
        free(containner);
        return AEC_UNDEFINED;
    }

    memcpy(para->cfg, &containner[idx].cfg, sizeof(at_config_t));

    free(containner);

    return AEC_OK;
}

static AT_ERROR_CODE status(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    struct netif *nif = g_wlan_netif;
    s32 i;

    memset(para->sts, 0, sizeof(*para->sts));

    if (nif == NULL) {
        return AEC_UNDEFINED;
    }

    if (NET_IS_IP4_VALID(nif) && netif_is_link_up(nif)) {
        memcpy(para->sts->ip_ipaddr, &nif->ip_addr, 4);
        memcpy(para->sts->ip_gw, &nif->gw, 4);
        memcpy(para->sts->ip_netmask, &nif->netmask, 4);
    }

    para->sts->current_time = time(NULL);

    para->sts->ip_sock_open = 0;

    for (i=0; i<MAX_SOCKET_NUM; i++) {
        if (networks.connect[i].flag) {
            para->sts->ip_sock_open |= (1<<i);
        }
    }

    if (g_server_enable) {
        para->sts->ip_sockd_port = g_server_arg.port;
    }

    wlan_ap_sta_num(&para->sts->wifi_num_assoc);

    return AEC_OK;
}

static AT_ERROR_CODE factory(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    struct sysinfo *psysinfo;

    memcpy(para->cfg, &default_cfg, sizeof(at_config_t));

    /* non-volatile config */
    strcpy(para->cfg->nv_manuf, MANUFACTURER);
    strcpy(para->cfg->nv_model, MODEL);
    strcpy(para->cfg->nv_serial, SERIAL);
#if PRJCONF_SYSINFO_SAVE_TO_FLASH
    sysinfo_load();
#endif
    psysinfo = sysinfo_get();
    memcpy(para->cfg->nv_wifi_macaddr, psysinfo->mac_addr, sizeof(para->cfg->nv_wifi_macaddr));

    save(para, NULL);

    return AEC_OK;
}

uint16_t net_get_status(void)
{
    return net_evevt_state;
}

extern s32 test_ping(char *hostname, int count);
static AT_ERROR_CODE ping(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    test_ping(para->u.ping.hostname,3);

    return AEC_OK;
}

static AT_ERROR_CODE peer(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    return AEC_OK;
}
#endif

static AT_ERROR_CODE disconnect(s32 id) {
  if (id >= MAX_SOCKET_NUM) {
    return AEC_OUT_OF_RANGE;
  }

  at_dump("+IPS:%d,-1\r\n", id);

  connect_t *connect = &networks.connect[id];
  
  FUN_DEBUG("disconnect id = %d!\r\n", id);
  if (connect->status != SOCK_IDLE_CLOSE) {
    connect->status = SOCK_IDLE_CLOSE; 

    // choose the right way to close
    if (connect->protocol == SOCK_TYPE_UDP){ // UDP
      udp_remove(connect->pcb.udp);
      connect->pcb.udp = NULL;
    } else if (connect->protocol == SOCK_TYPE_TCP || connect->protocol == SOCK_TYPE_TLS) { // TCP or TLS
      altcp_close(connect->pcb.tcp);
      connect->pcb.tcp = NULL;
    }
    connect->protocol = 0;
    networks.count--;
    return AEC_OK;
  }
  return AEC_DISCONNECT;
}

#if 0

typedef struct Timer Timer;

struct Timer {
    unsigned int end_time;
};


/** countdown_ms - set timeout value in mil seconds
 * @param timer - timeout timer where the timeout value save
 * @param timeout_ms - timeout in timeout_ms mil seconds
 */
static void countdown_ms(Timer* timer, unsigned int timeout_ms)
{
    timer->end_time = OS_TicksToMSecs(OS_GetTicks()) + timeout_ms;
}

/** countdown - set timeout value in seconds
 * @param timer - timeout timer where the timeout value save
 * @param timeout - timeout in timeout seconds
 */
/*
static void countdown(Timer* timer, unsigned int timeout)
{
countdown_ms(timer, timeout * 1000);
}
*/

/** left_ms - calculate how much time left before timeout
 * @param timer - timeout timer
 * @return the time left befor timeout, or 0 if it has expired
 */
static int left_ms(Timer* timer)
{
    int diff = (int)(timer->end_time) - (int)(OS_TicksToMSecs(OS_GetTicks()));
    return (diff < 0) ? 0 : diff;
}

/** expired - has it already timeouted
 * @param timer - timeout timer
 * @return 0 if it has already timeout, or otherwise.
 */
static char expired(Timer* timer)
{
    return 0 <= (int)OS_TicksToMSecs(OS_GetTicks()) - (int)(timer->end_time); /* is time_now over than end time */
}

static AT_ERROR_CODE wifi(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    AT_ERROR_CODE aec = AEC_OK;

    switch (para->cfg->wifi_mode) {
    case 0: /* IDLE */
        aec = AEC_IMPROPER_OPERATION;
        break;

    case 1: /* STA */

        if (para->u.wifi.value == 0) {
            wlan_sta_disable();
        } else if (para->u.wifi.value == 1) {
            wlan_sta_enable();
        } else {
            aec = AEC_PARA_ERROR;
        }

        break;

    case 2: /* AP */

        if (para->u.wifi.value == 0) {
            wlan_ap_disable();
        } else if (para->u.wifi.value == 1) {
            wlan_ap_enable();
        } else {
            aec = AEC_PARA_ERROR;
        }

        break;

    case 3: /* IBSS */
        aec = AEC_IMPROPER_OPERATION;
        break;

    default:

        aec = AEC_PARA_ERROR;

        break;
    }

    return aec;
}

static AT_ERROR_CODE reassociate(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    wlan_sta_connect();

    return AEC_OK;
}

static AT_ERROR_CODE gpioc(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    return AEC_OK;
}

static AT_ERROR_CODE gpior(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    return AEC_OK;
}

static AT_ERROR_CODE gpiow(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    return AEC_OK;
}

static s32 freq_to_chan(s32 freq)
{
    s32 i;

    for (i = 0; i < TABLE_SIZE(channel_freq_tbl); ++i) {
        if(freq == channel_freq_tbl[i]) {
            break;
        }
    }

    if(i >= TABLE_SIZE(channel_freq_tbl)) {
        return -1;
    }

    return i+1;
}
#endif

extern char *wifi_mgmr_auth_to_str(uint8_t auth);

static void insert_sort(wifi_mgmr_ap_item_t *ap_ary, int *a, int n)
{
    int i, j, x;

    for(i = 1; i < n; i++){
        if(ap_ary[a[i]].rssi > ap_ary[a[i-1]].rssi){
            j= i-1;
            x = a[i];
            while(j>-1 && ap_ary[x].rssi > ap_ary[a[j]].rssi){
                a[j+1] = a[j];
                j--;
            }
            a[j+1] = x;
        }
    }
}

static void scan_result_sort(wifi_mgmr_ap_item_t *ap_ary, int cnt, uint8_t opt)
{
    int i;

    int *a = malloc(cnt * sizeof(int *));
    if (a == NULL) {
        printf("mem not enought\r\n");
        return;
    }

    for (i = 0; i < cnt; i++) {
        a[i] = i;
    }

    if (BIT_GET(opt, 5)) {
        insert_sort(ap_ary, a, cnt);
    }

    at_serial_lock();
    at_dump("+CWLAP:");
    for (i = 0; i < cnt; i++) {
        if (strlen((char *)ap_ary[a[i]].ssid) > 0) {
            at_dump_noend(AT_CMD_ECHOSTART"%02d", i);
            if (BIT_GET(opt, 1)) {
                at_dump_noend(",%s", ap_ary[a[i]].ssid);
            }
            if (BIT_GET(opt, 3)) {
                at_dump_noend(",%02x:%02x:%02x:%02x:%02x:%02x",
                              ap_ary[a[i]].bssid[0],
                              ap_ary[a[i]].bssid[1],
                              ap_ary[a[i]].bssid[2],
                              ap_ary[a[i]].bssid[3],
                              ap_ary[a[i]].bssid[4],
                              ap_ary[a[i]].bssid[5]);
            }
            if (BIT_GET(opt, 4)) {
                at_dump_noend(",%02d", ap_ary[a[i]].channel);
            }
            if (BIT_GET(opt, 2)) {
                at_dump_noend(",%02d", ap_ary[a[i]].rssi);
            }
            if (BIT_GET(opt, 0)) {
                at_dump_noend(",%s", wifi_mgmr_auth_to_str(ap_ary[a[i]].auth));
            }
            at_dump_noend(AT_CMD_TAILED);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    at_serial_unlock();
    free(a);
}

static AT_ERROR_CODE scan(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
#if 0
    AT_ERROR_CODE aec = AEC_OK;
    int ret = -1;
    int size;
    char ssid[32+1];
    wlan_sta_scan_results_t results;

    if (para->cfg->wifi_mode != 1) { /* STA */
        return AEC_IMPROPER_OPERATION;
    }

    wlan_sta_scan_once();

    size = MAX_SCAN_RESULTS;

    results.ap = cmd_malloc(size * sizeof(wlan_sta_ap_t));
    if (results.ap == NULL) {
        aec = AEC_SCAN_FAIL;
    }

    if (aec == AEC_OK) {
        results.size = size;
        ret = wlan_sta_scan_result(&results);

        if (ret == 0) {
            /* cmd_wlan_sta_print_scan_results(&results); */
            int i;

            for (i = 0; i < results.num; i++) {
                memcpy(ssid, results.ap[i].ssid.ssid, results.ap[i].ssid.ssid_len);
                ssid[results.ap[i].ssid.ssid_len] = '\0';
                at_dump("%2d    BSS %02X:%02X:%02X:%02X:%02X:%02X    SSID: %-32.32s    "
                        "CHAN: %2d    RSSI: %d    flags: %08x    wpa_key_mgmt: %08x    "
                        "wpa_cipher: %08x    wpa2_key_mgmt: %08x    wpa2_cipher: %08x\n",
                        i + 1, (results.ap[i].bssid)[0], (results.ap[i].bssid)[1],
                        (results.ap[i].bssid)[2], (results.ap[i].bssid)[3],
                        (results.ap[i].bssid)[4], (results.ap[i].bssid)[5],
                        ssid, freq_to_chan(results.ap[i].freq),results.ap[i].level,
                        results.ap[i].wpa_flags, results.ap[i].wpa_key_mgmt,
                        results.ap[i].wpa_cipher,results.ap[i].wpa2_key_mgmt,
                        results.ap[i].wpa2_cipher);
            }
        }

        cmd_free(results.ap);
    }

    return aec;
#else
    AT_ERROR_CODE aec = AEC_OK;
    uint32_t cnt;
    int ret;
    wifi_mgmr_ap_item_t *ap_ary = NULL;
    uint8_t opt = 0;

    if (at_key_value_get(SAVE_KEY_WIFISCAN_OPT, &opt) != 0) {
        opt = 0xff;
    }
    ret = wifi_mgmr_all_ap_scan(&ap_ary, &cnt);
    if (ret != 0) {
        printf("wifi all ap scan error %d\r\n", ret);
    }
    else {
        printf("Scan done: %lu ap info\r\n", cnt);
        scan_result_sort(ap_ary, cnt, opt);
        if (ap_ary != NULL) {
            vPortFree(ap_ary);
        }
    }
    return aec;
#endif
}

static AT_ERROR_CODE version(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    at_dump("+GMR:fw:\"%s-%s\"", BL_SDK_PHY_VER, BL_SDK_RF_VER);

    at_dump("+GMR:sdk:\""BL_SDK_VER"\"");

    at_dump("+GMR:tm:\"%s %s\"",__DATE__,__TIME__);
    return AEC_OK;
}

static AT_ERROR_CODE restory(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\n");

    ef_env_set_default();

    return AEC_OK;
}

#if 0
// UART Config
static UART_CFG_Type uartCfg =
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
static UART_FifoCfg_Type fifoCfg =
{
    .txFifoDmaThreshold     = 0x10,
    .rxFifoDmaThreshold     = 0x10,
    .txFifoDmaEnable        = DISABLE,
    .rxFifoDmaEnable        = DISABLE,
};
#endif

static AT_ERROR_CODE uart_get(at_callback_para_t * para, at_callback_rsp_t * rsp) {
  
//  para->u.uart.uartBaud = uartCfg.baudRate;
//  para->u.uart.dataBit = uartCfg.dataBits + 5;
//  para->u.uart.stopBit = uartCfg.stopBits + 1;
//  para->u.uart.parity = uartCfg.parity;
//
//  if (uartCfg.ctsFlowControl == ENABLE && uartCfg.rtsSoftwareControl == ENABLE) {
//    para->u.uart.hwfc = 3;
//  } else if (uartCfg.ctsFlowControl == DISABLE && uartCfg.rtsSoftwareControl == ENABLE) {
//    para->u.uart.hwfc = 1;
//  } else if (uartCfg.ctsFlowControl == ENABLE && uartCfg.rtsSoftwareControl == DISABLE) {
//    para->u.uart.hwfc = 2;
//  } else if (uartCfg.ctsFlowControl == DISABLE && uartCfg.rtsSoftwareControl == DISABLE) {
//    para->u.uart.hwfc = 0;
//  } else {
//    return AEC_CMD_ERROR;
//  }
  at_key_value_get(SAVE_KEY_UART_BAUD, &para->u.uart.uartBaud);
  at_key_value_get(SAVE_KEY_UART_DATABIT, &para->u.uart.dataBit);
  at_key_value_get(SAVE_KEY_UART_STOPBIT, &para->u.uart.stopBit);
  at_key_value_get(SAVE_KEY_UART_PARITY, &para->u.uart.parity);
  at_key_value_get(SAVE_KEY_UART_HWFC, &para->u.uart.hwfc);
  
  return AEC_OK;
}

static AT_ERROR_CODE uart_set(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    int ret;

    printf("at_para.baudrate = %d\r\n", para->u.uart.uartBaud);

    ret = at_serial_cfg_set(para->u.uart.uartBaud,
                            para->u.uart.dataBit,
                            para->u.uart.stopBit,
                            para->u.uart.parity,
                            para->u.uart.hwfc);
    if (ret != 0) {
        return AEC_PARA_ERROR;
    }
    at_key_value_set(SAVE_KEY_UART_BAUD, &para->u.uart.uartBaud);
    at_key_value_set(SAVE_KEY_UART_DATABIT, &para->u.uart.dataBit);
    at_key_value_set(SAVE_KEY_UART_STOPBIT, &para->u.uart.stopBit);
    at_key_value_set(SAVE_KEY_UART_PARITY, &para->u.uart.parity);
    at_key_value_set(SAVE_KEY_UART_HWFC, &para->u.uart.hwfc);

    return AEC_OK;
}

static AT_ERROR_CODE deep_sleep(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    uint32_t sleep_time = para->u.sleep.sleep_time;
    uint8_t weakup_pin = para->u.sleep.weakup_pin;
    FUN_DEBUG("----->\r\n");

    if ((weakup_pin != 7) && (weakup_pin != 8)) {
        return AEC_CMD_FAIL;
    }

    hal_hbn_init(&weakup_pin, 1);
    at_dump("OK");
    vTaskDelay(100);
    sleep_time = sleep_time * 1000;
    hal_hbn_enter(sleep_time);
    at_dump("+GSLP:WEAKUP");
    return AEC_NO_RESPONSE;
}

#if 0
/*
enum suspend_state_t {
    PM_MODE_ON              = 0,
    PM_MODE_SLEEP           = 1,
    PM_MODE_STANDBY         = 2,
    PM_MODE_HIBERNATION     = 3,
    PM_MODE_POWEROFF        = 4,
    PM_MODE_MAX             = 5,
};
*/
#include "pm/pm.h"

static AT_ERROR_CODE sleep(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");

    if(para->u.sleep.sleepMode < 0 || para->u.sleep.sleepMode > 5) {
        printf("The sleep mode is not support \r\n");
        return AEC_CMD_ERROR;
    } else {
        at_dump("OK");

        at_wakeup_gpio_init(5,0);
        OS_MSleep(5);
        pm_enter_mode(para->u.sleep.sleepMode);

    }
    return AEC_BLANK_LINE;
}

static AT_ERROR_CODE wakeupgpio(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("-->%s\r\n",__func__);

    at_wakeup_gpio_init(para->u.wakeupgpio.gpioId,para->u.wakeupgpio.edge);

    return AEC_OK;
}
#endif

enum wlan_mode {
    WIFI_DISABLE = 0,
    WIFI_STATION_MODE,
    WIFI_SOFTAP_MODE,
    WIFI_AP_STA_MODE,
};

static char g_soft_ap_ssid[65] = {0};

static AT_ERROR_CODE __wifimode_set(int mode)
{
    memset(g_soft_ap_ssid, 0, sizeof(g_soft_ap_ssid));
    if (mode == WIFI_DISABLE) {
        wifi_mgmr_sta_disconnect();
        vTaskDelay(1000);
        wifi_mgmr_sta_disable(NULL);
        wifi_mgmr_ap_stop(NULL);
    } else if (mode == WIFI_STATION_MODE) {
        wifi_mgmr_sta_disable(NULL);
        wifi_mgmr_ap_stop(NULL);
        g_wifi_interface = wifi_mgmr_sta_enable();
    } else if (mode == WIFI_SOFTAP_MODE) {
        wifi_mgmr_sta_disconnect();
        vTaskDelay(1000);
        wifi_mgmr_sta_disable(NULL);
        wifi_mgmr_ap_stop(NULL);
        g_wifi_interface = wifi_mgmr_ap_enable();
    } else {
        printf("The mode is not support \r\n");
        return AEC_CMD_ERROR;
    }
    return AEC_OK;
}

static AT_ERROR_CODE cwmode_cur(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    int now_mode;
    FUN_DEBUG("----->\r\n");

    if(para->u.wifiMode.mode < 0 || para->u.wifiMode.mode >= WIFI_AP_STA_MODE) {
        printf("The mode is not support \r\n");
        return AEC_CMD_ERROR;
    }
    at_key_value_get(SAVE_KEY_WIFI_MODE, &now_mode);
    if (now_mode == para->u.wifiMode.mode) {
        return AEC_OK;
    }
    at_key_value_set(SAVE_KEY_WIFI_MODE, &para->u.wifiMode.mode);
    return __wifimode_set(para->u.wifiMode.mode);
}

static AT_ERROR_CODE cwmode_cur_get(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    at_key_value_get(SAVE_KEY_WIFI_MODE, &para->u.wifiMode.mode);
    FUN_DEBUG("----->\r\n");
    return AEC_OK;
}
static char cur_ssid[SYSINFO_SSID_LEN_MAX];
static char cur_psk[SYSINFO_PSK_LEN_MAX];
static AT_ERROR_CODE cwjap_cur(at_callback_para_t *para, at_callback_rsp_t *rsp) {
  // uint32_t wep_open_connect_timeout_ms = 15500;
  // uint32_t timeout = xTaskGetTickCount() +
  // pdMS_TO_TICKS(wep_open_connect_timeout_ms);
  int wifiMode;
  int auto_conn;
  at_key_value_get(SAVE_KEY_WIFI_MODE, &wifiMode);

  if (wifiMode != WIFI_STATION_MODE && wifiMode != WIFI_AP_STA_MODE) {
      return AEC_CMD_FAIL;
  }
  __wifimode_set(wifiMode);

  memset(cur_ssid, 0, SYSINFO_SSID_LEN_MAX * sizeof(char));
  memset(cur_psk, 0, SYSINFO_PSK_LEN_MAX * sizeof(char));
  memcpy(cur_ssid, para->u.joinParam.ssid, strlen(para->u.joinParam.ssid));
  memcpy(cur_psk, para->u.joinParam.pwd, strlen(para->u.joinParam.pwd));
  FUN_DEBUG("----->\r\n");

  if (para->u.joinParam.ssid == NULL) {
    printf("The ssid is NULL");
    return AEC_CMD_ERROR;
  } else {
    // wlan_sta_disable();
    // OS_MSleep(50);
    // FIXME joinParam pwd field is less
    // wlan_sta_enable();
    wifi_mgmr_sta_connect(g_wifi_interface, (char *)para->u.joinParam.ssid,
                          (char *)para->u.joinParam.pwd, NULL, NULL, 0, 0);
    at_key_value_get(SAVE_KEY_WIFI_AUTO, &auto_conn);
    if (auto_conn) {
        wifi_mgmr_sta_autoconnect_enable();
    } else {
        wifi_mgmr_sta_autoconnect_disable();
    }
    at_key_value_set(SAVE_KEY_WIFI_SSID, &para->u.joinParam.ssid);
    at_key_value_set(SAVE_KEY_WIFI_PASK, &para->u.joinParam.pwd);
    // enable wifi autoreconnect
    return AEC_OK;
  }

  return AEC_CMD_ERROR;
}

static AT_ERROR_CODE cwjap_info(at_callback_para_t *para, at_callback_rsp_t *rsp) {
    wifi_mgmr_sta_connect_ind_stat_info_t info = {0};
    int state;
    ip4_addr_t ip = {0}, gw = {0}, mask = {0};

    wifi_mgmr_state_get(&state);

    if (state == WIFI_STATE_CONNECTED_IP_GETTING || state == WIFI_STATE_CONNECTED_IP_GOT) {
        wifi_mgmr_sta_connect_ind_stat_get(&info);
        wifi_mgmr_sta_ip_get(&ip.addr, &gw.addr, &mask.addr);
    }
    printf("---------------state %d-------------\r\n", state);

    at_dump("+CWJAP:%s,"
            "%02x:%02x:%02x:%02x:%02x:%02x,"
            "%s",
            info.ssid,
            info.bssid[0],
            info.bssid[1],
            info.bssid[2],
            info.bssid[3],
            info.bssid[4],
            info.bssid[5],
            ip4addr_ntoa(&ip));
    return AEC_OK;
}

#if 0
static AT_ERROR_CODE cwlapopt(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");
    return AEC_OK;
}


static AT_ERROR_CODE cwlap(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");

    scan(para, rsp);

    return AEC_OK;
}
#endif

static AT_ERROR_CODE cwqap(at_callback_para_t *para, at_callback_rsp_t *rsp) {
  FUN_DEBUG("----->\r\n");

  wifi_mgmr_sta_disconnect();
  vTaskDelay(1000);
  wifi_mgmr_sta_disable(NULL);

  return AEC_OK;
}

#if 0
static AT_ERROR_CODE cwdhcp_cur(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");

    save(para, rsp);

    return AEC_OK;
}

static AT_ERROR_CODE cipstamac_cur(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");

    save(para, rsp);

    return AEC_OK;
}


static AT_ERROR_CODE cipsta_cur(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");

    save(para, rsp);//ip_ipaddr

    return AEC_OK;
}


static AT_ERROR_CODE cwhostname(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    FUN_DEBUG("----->\r\n");

    save(para, rsp);

    return AEC_OK;
}

static AT_ERROR_CODE cipdomain(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");

    struct hostent *server;
    //struct sockaddr_in serv_addr;

    server = gethostbyname(para->u.dns_parse.hostname);
    if (server == NULL) {
        printf("not find the  host \r\n");
        return AEC_CMD_FAIL;
    }

    for(int i=0; server->h_addr_list[i]; i++) {
        printf("IP addr %d: %s\r\n", i+1, inet_ntoa( *(struct in_addr*)server->h_addr_list[i]));
    }

    at_dump("+IPDNS:\"%s\"\n",inet_ntoa( *(struct in_addr*)server->h_addr_list[0]));
    return AEC_OK;
}

#endif

static const char *get_socket_type(enum socket_type type){
  switch (type) {
    case SOCK_TYPE_UDP:
      return "UDP";
    case SOCK_TYPE_TCP:
      return "TCP";
    case SOCK_TYPE_TLS:
      return "SSL";
    default:
      return "UNKNOW";
  }
}

static AT_ERROR_CODE cipstatus(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    uint16_t connect_cnt;

    FUN_DEBUG("----->\r\n");

    for (connect_cnt = 0; connect_cnt < MAX_SOCKET_NUM; connect_cnt++) {
        if (networks.connect[connect_cnt].status != SOCK_IDLE_CLOSE) {
            at_dump("+CIPSTART:%d,%s,%s,%d",
                    connect_cnt,
                    get_socket_type(networks.connect[connect_cnt].protocol),
                    networks.connect[connect_cnt].ip,
                    networks.connect[connect_cnt].port);
        }
    }

    return AEC_OK;
}

static AT_ERROR_CODE ap_sta_get(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    uint8_t cnt;
    uint8_t connected = 0;
    struct wifi_sta_basic_info sta_info;
    ip4_addr_t ip = {0}, gw = {0}, mask = {0};
    uint8_t mac[6];

    wifi_mgmr_ap_sta_cnt_get(&connected);
    wifi_mgmr_ap_ip_get(&ip.addr, &gw.addr, &mask.addr);
    wifi_mgmr_ap_mac_get(mac);
    at_dump("+SOFTAP:"
            "%s,"
            "%s,"
            "%02x:%02x:%02x:%02x:%02x:%02x\r\n",
            g_soft_ap_ssid,
            ip4addr_ntoa(&ip),
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);

    for (cnt = 0; cnt < connected; cnt++) {
        wifi_mgmr_ap_sta_info_get(&sta_info, cnt);
        if (!sta_info.is_used) {
            continue;
        }
        at_dump("+STA:%d,"
                "%02x:%02x:%02x:%02x:%02x:%02x,"
                "%d",
                sta_info.sta_idx,
                sta_info.sta_mac[0],
                sta_info.sta_mac[1],
                sta_info.sta_mac[2],
                sta_info.sta_mac[3],
                sta_info.sta_mac[4],
                sta_info.sta_mac[5],
                sta_info.rssi);
    }
    return AEC_OK;
}
static AT_ERROR_CODE set_apcfg(at_callback_para_t * para, at_callback_rsp_t * rsp)
{
    int psk_len, wifiMode;

    FUN_DEBUG("----->\r\n");

    at_key_value_get(SAVE_KEY_WIFI_MODE, &wifiMode);

    if (wifiMode != WIFI_SOFTAP_MODE && wifiMode != WIFI_AP_STA_MODE) {
        return AEC_CMD_FAIL;
    }
    __wifimode_set(wifiMode);

    psk_len = strlen(para->u.apcfgParam.psk);

    printf("para->u.apcfgParam.ssid=%s,para->u.apcfgParam.psk=%s\r\n",para->u.apcfgParam.ssid,para->u.apcfgParam.psk);

    if (psk_len > 0) {
        wifi_mgmr_ap_start(g_wifi_interface,
                           para->u.apcfgParam.ssid, 0,
                           para->u.apcfgParam.psk, 1);
    } else {
        wifi_mgmr_ap_start(g_wifi_interface,
                           para->u.apcfgParam.ssid, 0,
                           NULL, 1);
    }
    strcpy(g_soft_ap_ssid, para->u.apcfgParam.ssid);
    return AEC_OK;
}


static err_t tcp_receive_callback(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err) {
  int linkid = (int)arg;
  struct pbuf *pt;

  if (linkid >= MAX_SOCKET_NUM) {
    printf("receive callback arg error!!!!\r\n");
    return ERR_ARG;
  }
  if (networks.connect[linkid].status != SOCK_CLIENT_CONNECTED) {
    printf("linkid %d connection status error!\r\n", linkid);
    return ERR_CONN;
  }
  if (p == NULL){
    networks.connect[linkid].status = SOCK_IDLE_CLOSE;
    at_dump("+IPS:%d,CLOSED", linkid);
    altcp_close(networks.connect[linkid].pcb.tcp);
    networks.connect[linkid].pcb.tcp = NULL;

    return ERR_OK;
  }
  at_serial_lock();
  at_dump("+IPD:");
  if (is_disp_ipd == 1) at_dump("%d,%d\r\n", linkid, p->tot_len);
  for (pt = p; pt; pt = pt->next) {
    at_data_output((char *)pt->payload, pt->len);
  }
  at_serial_unlock();
  // release recv window size
  altcp_recved(networks.connect[linkid].pcb.tcp, p->tot_len);
  //XXX(Zhuoran) pbuf need free?
  pbuf_free(p);
  return ERR_OK;
}

static err_t tcp_sent_callback(void *arg, struct altcp_pcb *conn, u16_t len) {
  int linkid = (int)arg;
  if (linkid >= MAX_SOCKET_NUM) {
    printf("receive callback arg error!!!!\r\n");
    return ERR_ARG;
  }
  if (networks.connect[linkid].status != SOCK_CLIENT_CONNECTED) {
    printf("linkid %d connection status error!\r\n", linkid);
    return ERR_CONN;
  }
  printf("linkid %d peer acknowlege %d\r\n", linkid, len);
  cipsend_totlen -= len;
  if (cipsend_totlen <= 0){
    at_dump("+IPS:%d,SEND DONE\n", linkid);
  }
  
  return ERR_OK;
}

// in this case, the pcb already freed
static void tcp_err_callback(void *arg, err_t err) {
  int linkid = (int)arg;
  if (linkid >= MAX_SOCKET_NUM) {
    printf("connected callback arg error!!!!\r\n");
    return ERR_ARG;
  }

  networks.connect[linkid].status = SOCK_IDLE_CLOSE;
  at_dump("+IPS:%d,FAILED", linkid);
  networks.connect[linkid].pcb.tcp = NULL;
  return ERR_OK;
}

static err_t tcp_connected_callback(void *arg, struct altcp_pcb *conn, err_t err) {
  int linkid = (int)arg;
  if (linkid >= MAX_SOCKET_NUM) {
    printf("connected callback arg error!!!!\r\n");
    return ERR_ARG;
  }
  if (err == ERR_OK) {
    networks.connect[linkid].status = SOCK_CLIENT_CONNECTED;
    at_dump("+IPS:%d,CONNECTED", linkid);
    // register recv callbacks
    altcp_recv(conn, tcp_receive_callback);
    altcp_sent(conn, tcp_sent_callback);
  } else {
    networks.connect[linkid].status = SOCK_IDLE_CLOSE;
    at_dump("+IPS:%d,FAILED", linkid);
    altcp_close(networks.connect[linkid].pcb.tcp);
    networks.connect[linkid].pcb.tcp = NULL;
  }
  return ERR_OK;
}

static void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, 
    const ip_addr_t *addr, u16_t port) {
  struct pbuf *pt;
  int linkid = (int)arg;
  if (linkid >= MAX_SOCKET_NUM) {
    printf("receive callback arg error!!!!\r\n");
    pbuf_free(p);
    return ;
  }
  if (networks.connect[linkid].status == SOCK_IDLE_CLOSE) {
    printf("linkid %d connection status error!\r\n", linkid);
    pbuf_free(p);
    return ;
  }
  at_serial_lock();
  at_dump("+IPD:");
  if (is_disp_ipd == 1) at_dump("%d,%d\r\n", linkid, p->tot_len);
  for (pt = p; pt; pt = pt->next) {
    at_data_output((char *)pt->payload, pt->len);
  }
  at_serial_unlock();
  pbuf_free(p);
  return ;
}

static AT_ERROR_CODE cipstart(at_callback_para_t *para, at_callback_rsp_t *rsp) {
  FUN_DEBUG("----->\r\n");
  int linkid                      = para->u.net_param.linkId;
  struct addrinfo *result         = NULL;
  struct addrinfo hints_tcp       = {0, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};
  struct addrinfo hints_udp       = {0, AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, NULL, NULL, NULL};

  if (linkid >= MAX_SOCKET_NUM) {
    return AEC_OUT_OF_RANGE;
  }

  rsp->status = 0;

  if (networks.connect[linkid].status != SOCK_IDLE_CLOSE) {
    sprintf(rsp->vptr, "+IPS:%d,ALREADY CONNECT", linkid);
    rsp->type = 1;
    rsp->status = 1;
    return AEC_SOCKET_EXISTING;
  }

  // FIXME(Zhuoran) PLZ check ip length
  strncpy(networks.connect[linkid].ip, para->u.net_param.hostname, IP_ADDR_SIZE);
  networks.connect[linkid].port = para->u.net_param.port;

  if (strcmp(para->u.net_param.networkType, "TCP") == 0) {
    networks.connect[linkid].protocol = SOCK_TYPE_TCP;
  } else if (strcmp(para->u.net_param.networkType, "UDP") == 0) {
    networks.connect[linkid].protocol = SOCK_TYPE_UDP;
  } else if (strcmp(para->u.net_param.networkType, "SSL") == 0) {
    networks.connect[linkid].protocol = SOCK_TYPE_TLS;
  } else {
    return AEC_PARA_ERROR;
  }

  if (networks.connect[linkid].protocol == SOCK_TYPE_TCP) {
    ip_addr_t ip_addr;
    int rc;

    //FIXME(Zhuoran) only support ipv4 now
    if ((rc = getaddrinfo(networks.connect[linkid].ip, NULL, &hints_tcp, &result)) == 0) {
      struct addrinfo *res = result;

      while (res) {
        if (res->ai_family == AF_INET) {
          result = res;
          break;
        }
        res = res->ai_next;
      }

      if (result->ai_family == AF_INET) {
        ip_addr_t tmp = IPADDR4_INIT(((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr); 
        ip_addr = tmp;
      } else
        rc = -1;

      freeaddrinfo(result);
    }

    if (rc == 0) {
      //FIXME(Zhuoran) only support ipv4 now
      networks.connect[linkid].pcb.tcp = altcp_tcp_new_ip_type(IPADDR_TYPE_V4);
      if (networks.connect[linkid].pcb.tcp == NULL)
        return AEC_SOCKET_FAIL;

      altcp_arg(networks.connect[linkid].pcb.tcp, (void *)(intptr_t)linkid);
      rc = altcp_connect(networks.connect[linkid].pcb.tcp, &ip_addr, 
          networks.connect[linkid].port, tcp_connected_callback);
      if (rc != ERR_OK) {
        altcp_close(networks.connect[linkid].pcb.tcp);
        networks.connect[linkid].pcb.tcp = NULL;
        return AEC_CONNECT_FAIL;
      }
      altcp_err(networks.connect[linkid].pcb.tcp, tcp_err_callback);

      networks.connect[linkid].status = SOCK_CLIENT_CONNECTING;
    } else {
      return AEC_CONNECT_FAIL;
    }

    networks.count++;

    rsp->status = 0;
    return AEC_OK;
  } else if (networks.connect[linkid].protocol == SOCK_TYPE_UDP) {
    networks.connect[linkid].pcb.udp = udp_new_ip_type(IPADDR_TYPE_V4);
    if (networks.connect[linkid].pcb.udp == NULL){
        return AEC_SOCKET_FAIL;
    }
    if (udp_bind(networks.connect[linkid].pcb.udp, IP4_ADDR_ANY, 0) != ERR_OK) {
      udp_remove(networks.connect[linkid].pcb.udp);
      networks.connect[linkid].pcb.udp = NULL;
      return AEC_SOCKET_FAIL;
    }

    networks.connect[linkid].status = SOCK_CLIENT_CONNECTED;

    networks.count++;
    rsp->status = 0;
    return AEC_OK;
  } else {
    printf("not support tls now\r\n");
    return AEC_UNSUPPORTED;
  }
}

static AT_ERROR_CODE cipsendbuf(at_callback_para_t *para, at_callback_rsp_t *rsp) {
  FUN_DEBUG("----->\r\n");

  return AEC_OK;
}

static AT_ERROR_CODE cipclose(at_callback_para_t *para, at_callback_rsp_t *rsp) {
  FUN_DEBUG("----->\r\n");

  if (para->u.close_id.id < MAX_SOCKET_NUM) {
      if (networks.connect[para->u.close_id.id].status == SOCK_IDLE_CLOSE) {
          return AEC_SOCKET_EXISTING;
      }
      disconnect(para->u.close_id.id);
      //at_dump("+CIPCLOSE:%d", para->u.close_id.id);
  } else {
      return AEC_PARA_ERROR;
  }
  return AEC_OK;
}

#if 0
static AT_ERROR_CODE tcpservermaxconn(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");

    return AEC_OK;
}
#endif

static err_t tcp_accept_callback(void *arg, struct altcp_pcb *new_conn, err_t err) {
  int linkid;
  ip_addr_t *remote_ip;

  if (err != ERR_OK){
    printf("accept error!\r\n");
    // XXX(Zhuoran) Is there a better way to deal with it?
    altcp_close(new_conn);
    return ERR_CONN;
  }
  // find free slot to put connection
  for (linkid = 0; linkid < MAX_SOCKET_NUM; linkid++) {
    if (networks.connect[linkid].status == SOCK_IDLE_CLOSE){
      memset(&networks.connect[linkid], 0, sizeof(networks.connect[linkid]));
      break;
    }
  }
  if (networks.connect[linkid].status != SOCK_IDLE_CLOSE) {
    printf("no avalible connect slot\r\n");
    // XXX(Zhuoran) Is there a better way to deal with it?
    altcp_close(new_conn);
    return ERR_CONN;
  }
  networks.count++;
  // fill up necessary field
  networks.connect[linkid].pcb.tcp = new_conn;
  networks.connect[linkid].status = SOCK_CLIENT_CONNECTED;
  networks.connect[linkid].protocol = SOCK_TYPE_TCP;
  remote_ip = altcp_get_ip(new_conn, 0);
  ipaddr_ntoa_r(remote_ip, networks.connect[linkid].ip, sizeof(networks.connect[linkid].ip));
  networks.connect[linkid].port = altcp_get_port(new_conn, 0);
  
  at_dump("+IPS:%d,CONNECTED", linkid);
  altcp_arg(new_conn, (void *)(intptr_t)linkid);
  altcp_recv(new_conn, tcp_receive_callback);
  altcp_sent(new_conn, tcp_sent_callback);
  altcp_err(new_conn, tcp_err_callback);
  return ERR_OK;
}

static AT_ERROR_CODE tcpserver(at_callback_para_t *para, at_callback_rsp_t *rsp) {
  FUN_DEBUG("----->\r\n");
  s32 protocol;
  int enable;

  enable = para->u.tcp_server.enable;
  protocol = SOCK_TYPE_TCP; //tcp

  if (enable) {
    if (!g_server_enable) {
      g_server_arg.port = para->u.tcp_server.port;
      g_server_arg.protocol = SOCK_TYPE_TCP;
      // too early
      memset(&g_server_ctrl, 0, sizeof(g_server_ctrl));

      if (protocol == SOCK_TYPE_TCP) { /* TCP */
        err_t err;
        g_server_ctrl.pcb.tcp = altcp_tcp_new_ip_type(IPADDR_TYPE_V4);
        if (g_server_ctrl.pcb.tcp == NULL) {
          return AEC_NOT_ENOUGH_MEMORY;
        }

        err = altcp_bind(g_server_ctrl.pcb.tcp, IP4_ADDR_ANY, g_server_arg.port);
        if (err != ERR_OK) {
          altcp_close(g_server_ctrl.pcb.tcp);
          g_server_ctrl.pcb.tcp = NULL;
          return AEC_BIND_FAIL;
        }

        // listen
        g_server_ctrl.pcb.tcp = altcp_listen_with_backlog(g_server_ctrl.pcb.tcp, 5);
        if (g_server_ctrl.pcb.tcp == NULL) {
          // listen failed , origin pcb has been freed
          return AEC_SOCKET_FAIL;
        }
        g_server_enable = 1;
        
        altcp_accept(g_server_ctrl.pcb.tcp, tcp_accept_callback);
        
        return AEC_OK;
      } else if (protocol == SOCK_TYPE_UDP) { /* UDP */

      }
      return AEC_OK;
    }
  } else {
    if (g_server_enable) {
      g_server_enable = 0;

      if (g_server_arg.protocol == SOCK_TYPE_TCP) {
        altcp_close(g_server_ctrl.pcb.tcp);
        g_server_ctrl.pcb.tcp = NULL;
      } else if (g_server_arg.protocol == SOCK_TYPE_UDP) { /* UDP */
        /* Do nothing */
      }
      return AEC_OK;
    }
  }

  return AEC_SOCKET_FAIL;
}
#if 0
static AT_ERROR_CODE cipdns_cur(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");
    ip_addr_t dnsip;

    memcpy((char *)(&dnsip), &para->u.set_dns.setdnsip, sizeof(at_ip_t));

    dns_setserver(0,&dnsip);
    return AEC_OK;

}

static AT_ERROR_CODE cipmux(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");

    return AEC_OK;
}


static AT_ERROR_CODE cipmode(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");

    return AEC_OK;
}

static AT_ERROR_CODE ciprecvdata(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");

    return AEC_OK;
}
static AT_ERROR_CODE ciprecvmode(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    FUN_DEBUG("----->\r\n");

    return AEC_OK;
}
#endif

static SemaphoreHandle_t g_http_xSemaphore = NULL;
static void cb_httpc_result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    at_callback_rsp_t *rsp = (at_callback_rsp_t *)arg;
    printf("[HTTPC] Transfer finished err %d. rx_content_len is %lu\r\n", err, rx_content_len);
    if (rsp && (!rsp->vptr)) {
        at_dump_noend(AT_CMD_TAILED);
    }
    xSemaphoreGive(g_http_xSemaphore);
    if (err == 0) {
        at_dump("+RSP:OK");
    } else {
        at_dump("+RSP:%d", err);
    }
}

static err_t cb_httpc_headers_done_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    at_callback_rsp_t *rsp = (at_callback_rsp_t *)arg;

    printf("[HTTPC] hdr_len is %u, content_len is %lu\r\n", hdr_len, content_len);
    printf((char *)hdr->payload);
    if (rsp && (!rsp->vptr)) {
        at_dump_noend(AT_CMD_ECHOSTART);
        at_dump_noend("+HTTPC:%d,", content_len);
    }
    return ERR_OK;
}

static err_t cb_altcp_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    static int count = 0;
    at_callback_rsp_t *rsp = (at_callback_rsp_t *)arg;

    if (0 == ((count++) & 0x3F)) {
        puts("\r\n");
    }
    altcp_recved(conn, p->tot_len);
    if (rsp) {
        if (!rsp->vptr && p->tot_len) {
            at_data_output(p->payload, p->tot_len);
        } else if (strlen((char *)p->payload) <= rsp->vsize) {
            if (rsp->vptr) {
                strcpy(rsp->vptr, (char *)p->payload);
            }
        } else {
            printf("rsp buffer size not enought\r\n");
        }
    }
    //printf("%s\r\n", (char *)p->payload);
    pbuf_free(p);

    return ERR_OK;
}

static AT_ERROR_CODE http_url_req(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    char *uri = "/";
    char *param, *server_name;
    static httpc_connection_t settings;
    static httpc_state_t *req = NULL;
    in_addr_t ip_addr;
    char *p_port;
    int port = 80;

    if (g_http_xSemaphore == NULL) {
        g_http_xSemaphore = xSemaphoreCreateBinary();
    }
    server_name = pvPortMalloc(128);
    if (server_name== NULL) {
        return AEC_CMD_FAIL;
    }
    memset(&settings, 0, sizeof(settings));
    settings.use_proxy = 0;
    settings.req_type  = para->u.http_req.type;
    settings.content_type = para->u.http_req.content_type;
    settings.data = para->u.http_req.data;
    settings.result_fn = cb_httpc_result;
    settings.headers_done_fn = cb_httpc_headers_done_fn;

    if ((param = strstr(para->u.http_req.hostname, uri)) == NULL) {
        param = uri;
    }
    strcpy(server_name, para->u.http_req.hostname);
    server_name[param - para->u.http_req.hostname] = '\0';
    if ((p_port = strstr(server_name, ":")) != NULL) {
        p_port[0] = '\0';
        p_port++;
        port = atoi(p_port);
    }
    if ((ip_addr = inet_addr(server_name)) != INADDR_NONE) {
        httpc_get_file((const ip_addr_t* )&ip_addr,
                        port,
                        param,
                        &settings,
                        cb_altcp_recv_fn,
                        rsp,
                        &req);
    } else {
        httpc_get_file_dns(
                server_name,
                port,
                param,
                &settings,
                cb_altcp_recv_fn,
                rsp,
                &req);
    }

    if (xSemaphoreTake(g_http_xSemaphore, (TickType_t)-1) != pdTRUE) {
        vPortFree(server_name);
        return AEC_NETWORK_ERROR;
    }
    vPortFree(server_name);
    return AEC_OK;
}

// callback context
struct send_data_ctx {
  int linkid;
  uint8_t *buf;
};
// Send TCP/TLS data
static void tcp_send_data(void *arg){
  assert(arg != NULL);
  connect_t *conn;
  struct send_data_ctx *ctx = (struct send_data_ctx *)arg;
  assert(ctx->buf != NULL);

  conn = &networks.connect[ctx->linkid];
  
  altcp_write(conn->pcb.tcp, ctx->buf, cipsend_totlen, 0);
  free(arg);
}

// Send UDP data
static void udp_send_data(void *arg){
  assert(arg != NULL);
  connect_t *conn;
  struct send_data_ctx *ctx = (struct send_data_ctx *)arg;
  assert(ctx->buf != NULL);

  // create pbuf
  // copy data to pbuf
  // send data
  //

  free(arg);
}

static AT_ERROR_CODE cipsend(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    u8 *buffer;
    connect_t *conn;
    struct send_data_ctx *ctx;
    FUN_DEBUG("------>%s\r\n",__func__);

    if (para->u.sockw.id >= MAX_SOCKET_NUM) {
        return AEC_DISCONNECT;
    }
    conn = &networks.connect[para->u.sockw.id];
    if (conn->status != SOCK_CLIENT_CONNECTED) {
      printf("linkid %d is not connected!\r\n", para->u.sockw.id);
      return AEC_CMD_FAIL;
    }
    
    // Record the total length of the data in order to judge 
    // whether the transmission is completed in the asynchronous callback
    cipsend_totlen = para->u.sockw.len;
    
    if (conn->protocol == SOCK_TYPE_TCP || conn->protocol == SOCK_TYPE_TLS) {
      ctx = malloc(sizeof(struct send_data_ctx));
      if (ctx == NULL) {
        return AEC_NOT_ENOUGH_MEMORY;
      }
      
      ctx->linkid = para->u.sockw.id;
      ctx->buf = para->u.sockw.buf;
      tcpip_callback(tcp_send_data, (void *)ctx);
    } else if (conn->protocol == SOCK_TYPE_UDP) {
      ctx = malloc(sizeof(struct send_data_ctx));
      if (ctx == NULL) {
        return AEC_NOT_ENOUGH_MEMORY;
      }
      
      ctx->linkid = para->u.sockw.id;
      ctx->buf = para->u.sockw.buf;
      tcpip_callback(udp_send_data, (void *)ctx);
    } else {
      return AEC_UNSUPPORTED;
    }
    
    return AEC_OK;
}

#if 0
int is_netconnet_ap(void)
{
    if((net_evevt_state == NET_CTRL_MSG_NETWORK_UP) || (net_evevt_state == NET_CTRL_MSG_WLAN_CONNECTED))
        return 1;

    return 0;
}

typedef struct {
    GPIO_Pin    GPIONum;
    GPIO_Port   GPIOPort;
    GPIO_InitParam  GPIOPara;
} GPIO_CFG;
GPIO_CFG BoardGPIO[3]=
{
    {GPIO_PIN_19,GPIO_PORT_A,{GPIOx_Pn_F0_INPUT,GPIO_DRIVING_LEVEL_1,GPIO_PULL_NONE}},
    {GPIO_PIN_19,GPIO_PORT_A,{GPIOx_Pn_F0_INPUT,GPIO_DRIVING_LEVEL_1,GPIO_PULL_NONE}},
    {GPIO_PIN_19,GPIO_PORT_A,{GPIOx_Pn_F0_INPUT,GPIO_DRIVING_LEVEL_1,GPIO_PULL_NONE}}
};

static AT_ERROR_CODE sysiosetcfg(at_callback_para_t *para, at_callback_rsp_t *rsp)
{

    if(para->u.setgpio_para.ID > 2)
        return AEC_PARA_ERROR;

    para->u.setgpio_para.ID -= para->u.setgpio_para.ID;

    BoardGPIO[para->u.setgpio_para.ID].GPIOPara.driving = GPIO_DRIVING_LEVEL_1;

    if(para->u.setgpio_para.mode <= 1)
        BoardGPIO[para->u.setgpio_para.ID].GPIOPara.mode = para->u.setgpio_para.mode;
    else
        return AEC_PARA_ERROR;

    if(para->u.setgpio_para.pull <= 2)
        BoardGPIO[para->u.setgpio_para.ID].GPIOPara.pull = para->u.setgpio_para.pull;
    else
        return AEC_PARA_ERROR;

    HAL_GPIO_Init(BoardGPIO[para->u.setgpio_para.ID].GPIOPort,  \
                    BoardGPIO[para->u.setgpio_para.ID].GPIONum, \
                    &BoardGPIO[para->u.setgpio_para.ID].GPIOPara);

    return AEC_OK;
}


static AT_ERROR_CODE sysiogetcfg(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    char *modestr[]={"Input","Output"};
    char *pullstr[]={"pull none","pull up","pull down"};
    at_dump("OI1 driver level is %d\r\n",BoardGPIO[0].GPIOPara.driving);
    at_dump("OI1 mode is %s\r\n",modestr[BoardGPIO[0].GPIOPara.mode]);
    at_dump("OI1 pull is %s\r\n",pullstr[BoardGPIO[0].GPIOPara.pull]);

    at_dump("OI2 driver level is %d\r\n",BoardGPIO[1].GPIOPara.driving);
    at_dump("OI2 mode is %s\r\n",modestr[BoardGPIO[1].GPIOPara.mode]);
    at_dump("OI2 pull is %s\r\n",pullstr[BoardGPIO[1].GPIOPara.pull]);

    at_dump("OI3 driver level is %d\r\n",BoardGPIO[2].GPIOPara.driving);
    at_dump("OI3 mode is %s\r\n",modestr[BoardGPIO[2].GPIOPara.mode]);
    at_dump("OI3 pull is %s\r\n",pullstr[BoardGPIO[2].GPIOPara.pull]);

    return AEC_OK;
}


static AT_ERROR_CODE syssetiodir(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    if(para->u.setiodir_para.ID > 2)
        return AEC_PARA_ERROR;

    if(para->u.setiodir_para.mode <= 1)
        BoardGPIO[para->u.setiodir_para.ID].GPIOPara.mode = para->u.setiodir_para.mode;
    else
        return AEC_PARA_ERROR;

    HAL_GPIO_Init(BoardGPIO[para->u.setiodir_para.ID].GPIOPort,     \
                    BoardGPIO[para->u.setiodir_para.ID].GPIONum,    \
                    &BoardGPIO[para->u.setiodir_para.ID].GPIOPara);

    return AEC_OK;
}

static AT_ERROR_CODE syssetgpio(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    if(para->u.writeiodata_para.ID > 2)
        return AEC_PARA_ERROR;

    if(para->u.writeiodata_para.data == 0)
    {
        HAL_GPIO_WritePin(BoardGPIO[para->u.writeiodata_para.ID].GPIOPort,  \
                        BoardGPIO[para->u.writeiodata_para.ID].GPIONum, \
                        GPIO_PIN_LOW);
    }
    else
    {
        HAL_GPIO_WritePin(BoardGPIO[para->u.writeiodata_para.ID].GPIOPort,  \
                        BoardGPIO[para->u.writeiodata_para.ID].GPIONum, \
                        GPIO_PIN_HIGH);
    }

    return AEC_OK;
}

static AT_ERROR_CODE sysreadgpio(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    if(para->u.readiodata_para.ID > 2)
        return AEC_PARA_ERROR;

    at_dump("Level is %d\r\n",HAL_GPIO_ReadPin(BoardGPIO[para->u.readiodata_para.ID].GPIOPort,  \
                        BoardGPIO[para->u.readiodata_para.ID].GPIONum));
    return AEC_OK;
}

#if 0
static AT_ERROR_CODE deleteap(at_callback_para_t *para, at_callback_rsp_t *rsp)
{
    if(para->u.deleteap_para.apnum > SYSINFO_HISTORY_AP_MAX)
        return AEC_PARA_ERROR;

    if(delete_history_ap(para->u.deleteap_para.apnum) == -1)
    {
        return AEC_CMD_FAIL;
    }
    else
        return AEC_OK;
}
#endif

int OTA_start = 0;
void occur(uint32_t evt, uint32_t data, void *arg)
{
    int idx = EVENT_SUBTYPE(evt);
    switch (idx) {
    case NET_CTRL_MSG_WLAN_CONNECTED:
        net_evevt_state = NET_CTRL_MSG_WLAN_CONNECTED;
        if(OTA_start == 0)
            at_dump("+EVT:2\n");
        break;
    case NET_CTRL_MSG_WLAN_DISCONNECTED:
        net_evevt_state = NET_CTRL_MSG_WLAN_DISCONNECTED;
        if(OTA_start == 0)
            at_dump("+EVT:3\n");
        break;
    case NET_CTRL_MSG_WLAN_SCAN_SUCCESS:
        break;
    case NET_CTRL_MSG_WLAN_CONNECT_FAILED:
        break;
    case NET_CTRL_MSG_NETWORK_UP:
        if(net_evevt_state == NET_CTRL_MSG_NETWORK_UP)
            return;
        net_evevt_state = NET_CTRL_MSG_NETWORK_UP;
        if(OTA_start == 0)
            at_dump("+EVT:4\n");
        break;
    case NET_CTRL_MSG_NETWORK_DOWN:

        break;
    case NET_CTRL_MSG_WLAN_SCAN_FAILED:
    case NET_CTRL_MSG_WLAN_4WAY_HANDSHAKE_FAILED:
        break;
    default:
        break;
    }

    if(idx >= 0 && idx < TABLE_SIZE(event)) {
        if (at_event(idx)) {
            if(OTA_start == 0)
            at_dump("msg:%d\r\n%s\r\n", idx, event[idx]);
        }
    } else {
        FUN_DEBUG("Unsupported.\r\n");
    }

}

TaskHandle_t ap_task_ctrl_thread;
static int create_port;
void tcp_server_task(void *pvParameters)
{
    while (1) {
        ap_socket_task(create_port);
    }
}

void ap_server_task(int port)
{
    create_port = port;

    if (xTaskCreate(tcp_server_task,
                    "tcp_server_task",
                    1024,
                    NULL,
                    10,
                    &ap_task_ctrl_thread) != pdPASS) {
        printf("thread create error\n");
    }
}

void ap_task_delete(void)
{
    vTaskDelete(ap_task_ctrl_thread);
}
#endif
