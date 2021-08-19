#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <vfs.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>
#include <cli.h>

#include <lwip/tcpip.h>
#include <netutils/netutils.h>

#include <bl602_glb.h>
#include <bl602_hbn.h>

#include <bl_uart.h>
#include <bl_chip.h>
#include <bl_wifi.h>
#include <bl_sec.h>
#include <bl_irq.h>
#include <bl_dma.h>
#include <bl_timer.h>
#include <bl_gpio_cli.h>
#include <hal_uart.h>
#include <hal_sys.h>
#include <hal_gpio.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <looprt.h>
#include <loopset.h>
#include <sntp.h>
#include <bl_sys_time.h>
#include <bl_sys_ota.h>
#include <bl_romfs.h>
#include "hosal_dma.h"
#include <bl60x_fw_api.h>
#include <wifi_mgmr_ext.h>
#include <utils_log.h>
#include <libfdt.h>
#include <blog.h>
#include <fdt.h>
#include <bl_mtd.h>

#include <netbus_mgmr.h>
//#include <sdiowifi_test.h>
#include "ble_lib_api.h"

#define TASK_PRIORITY_FW            ( 30 )
#define ENABLE_SDIOWIFI 1

#define GPIO_LED_PIN_NUM           (5)

#define USER_FOR_RASPBERRY         (1)

#ifdef USER_FOR_RASPBERRY
#define UART0_TX_PIN (16)
#define UART0_RX_PIN (7)
#define UART1_TX_PIN (17)
#define UART1_RX_PIN (20)
#else
#define UART0_TX_PIN (17)
#define UART0_RX_PIN (20)
#define UART1_TX_PIN (16)
#define UART1_RX_PIN (7)
#endif

typedef struct _fastconnect_config {
    int fc_en;
    uint16_t crc;
    char ssid[33];
    char pawd[65];
    wifi_mgmr_network_config_adv_t adv;
    fw_rf_calib_data_tag fw_rf_data;
} fastconnect_config_t;
static fastconnect_config_t fconfig;
extern int bl_sys_em_config(void);

static bl_mtd_handle_t psmhdl;
void psm_mtd_init(void)
{
    int ret;

    ret = bl_mtd_open(BL_MTD_PARTITION_NAME_PSM, &psmhdl, BL_MTD_OPEN_FLAG_BUSADDR);
    if (ret < 0) {
        log_error("[EF] [PART] [XIP] error when get PSM partition %d\r\n", ret);
        log_error("[EF] [PART] [XIP] Dead Loop. Reason: no Valid PSM partition found\r\n");
        while (1) {
        }
    }
}
void psm_mtd_write(void *inputbuf, size_t size)
{
    uint8_t *buf = (uint8_t *)inputbuf;

    bl_mtd_erase(psmhdl, 16*1024, size);
    bl_mtd_write(psmhdl, 16*1024, size, (const uint8_t*)buf);
}
void psm_mtd_read(void *outbuf, size_t size)
{
    uint8_t *buf = (uint8_t *)outbuf;

    bl_mtd_read(psmhdl, 16*1024, size, (uint8_t*)buf);
}
static void cmd_fc_mtest(char *buf, int len, int argc, char **argv)
{
    uint8_t *buf1 = NULL, *buf2 = NULL;
    buf1 = aos_malloc(4096);
    buf2 = aos_malloc(4096);
    if (!buf1 || !buf2) {
        goto psmtest_exit;
    }
    memset(buf1, 0x31, 4096);
    memset(buf2, 0x32, 4096);

    psm_mtd_init();
    psm_mtd_write(buf1, 4096);
    psm_mtd_read(buf2, 4096);
    if (0 != memcmp(buf1, buf2, 4096)) {
        log_error("pmtest error.\r\n");
        goto psmtest_exit;
    }

    memset(buf1, 0xFF, 4096);
    psm_mtd_write(buf1, 4096);

    log_info("fast connect mem test success.\r\n");

psmtest_exit:
    if (buf1) {
        aos_free(buf1);
    }
    if (buf2) {
        aos_free(buf2);
    }
}

static void cmd_fc_merase(char *buf, int len, int argc, char **argv)
{
    bl_mtd_erase(psmhdl, 16*1024, 4096);
    log_info("fast connect mem erase success.\r\n");
}

static void cmd_fc_enable(char *buf, int len, int argc, char **argv)
{
    if (argc != 2) {
        log_info("Usage: fcenable 0 or 1\r\n");
        return;
    }

    fconfig.fc_en = atoi(argv[1]);
    if (fconfig.fc_en != 1 && fconfig.fc_en !=0) {
        log_info("param is invalid\r\n");
        fconfig.fc_en = 0;
        return;
    }

    psm_mtd_write(&fconfig, sizeof(fconfig));
}

static void print_fc_str(char *name, void *addr, int size)
{
    uint8_t *buf = (uint8_t *)addr;

    if (0 != buf[size - 1]) {
        log_warn("%s is invalid.\r\n", name);
        buf[size - 1] = 0;
    }

    log_info("%s = %s\r\n", name, buf);
}

static void cmd_fc_mdump(char *buf, int len, int argc, char **argv)
{
    fastconnect_config_t mconfig;
    psm_mtd_read(&mconfig, sizeof(fastconnect_config_t));

    log_info("mconfig.fc_en        = %d\r\n", mconfig.fc_en);
    print_fc_str("mconfig.ssid        ", mconfig.ssid, sizeof(mconfig.ssid));
    print_fc_str("mconfig.pawd        ", mconfig.pawd, sizeof(mconfig.pawd));
    print_fc_str("mconfig.adv.pmk     ", mconfig.adv.pmk, sizeof(mconfig.adv.pmk));
    log_info("mconfig.adv.mac      = %02X:%02X:%02X:%02X:%02X:%02X\r\n",
            mconfig.adv.mac[0], mconfig.adv.mac[1], mconfig.adv.mac[2],
            mconfig.adv.mac[3], mconfig.adv.mac[4], mconfig.adv.mac[5]);
    log_info("mconfig.adv.freq     = %d\r\n", mconfig.adv.freq);
    log_info("mconfig.adv.band     = %d\r\n", mconfig.adv.band);
    log_info("mconfig.adv.ip       = 0x%08lX\r\n", mconfig.adv.ip);
    log_info("mconfig.adv.mask     = 0x%08lX\r\n", mconfig.adv.mask);
    log_info("mconfig.adv.gw       = 0x%08lX\r\n", mconfig.adv.gw);
    log_info("mconfig.adv.dns1     = 0x%08lX\r\n", mconfig.adv.dns1);
    log_info("mconfig.adv.dns2     = 0x%08lX\r\n", mconfig.adv.dns2);
    log_info("mconfig.adv.dhcp_use = %d\r\n", mconfig.adv.dns2);
    log_info("...adv.ip_lease_time = %lld\r\n", mconfig.adv.ip_lease_time);
    log_info("...adv.timestamp     = %lld\r\n", mconfig.adv.timestamp);
    log_info("mconfig.adv.gw_mac   = %02X:%02X:%02X:%02X:%02X:%02X\r\n",
            mconfig.adv.gw_mac[0], mconfig.adv.gw_mac[1], mconfig.adv.gw_mac[2],
            mconfig.adv.gw_mac[3], mconfig.adv.gw_mac[4], mconfig.adv.gw_mac[5]);
#if 1
    log_info("[APP] rf.cal.gpadc_oscode = %u\r\n", fconfig.fw_rf_data.cal.gpadc_oscode);
    log_info("[APP] rf.cal.rx_offset_q = %u\r\n", fconfig.fw_rf_data.cal.rx_offset_q);
    log_info("[APP] rf.cal.rbb_cap1_fc_i = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap1_fc_i);
    log_info("[APP] rf.cal.rbb_cap1_fc_q = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap1_fc_q);
    log_info("[APP] rf.cal.rbb_cap2_fc_i = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap2_fc_i);
    log_info("[APP] rf.cal.rbb_cap2_fc_q = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap2_fc_q);
    log_info("[APP] rf.cal.tx_dc_comp_i = %u\r\n", fconfig.fw_rf_data.cal.tx_dc_comp_i);
    log_info("[APP] rf.cal.tx_dc_comp_q = %u\r\n", fconfig.fw_rf_data.cal.tx_dc_comp_q);
    log_info("[APP] rf.cal.tmx_cs = %u\r\n", fconfig.fw_rf_data.cal.tmx_cs);
    log_info("[APP] rf.cal.txpwr_att_rec = %u\r\n", fconfig.fw_rf_data.cal.txpwr_att_rec);
    log_info("[APP] rf.cal.pa_pwrmx_osdac = %u\r\n", fconfig.fw_rf_data.cal.pa_pwrmx_osdac);
    log_info("[APP] rf.cal.tmx_csh = %u\r\n", fconfig.fw_rf_data.cal.tmx_csh);
    log_info("[APP] rf.cal.tmx_csl = %u\r\n", fconfig.fw_rf_data.cal.tmx_csl);
    log_info("[APP] rf.cal.tsen_refcode_rfcal = %u\r\n", fconfig.fw_rf_data.cal.tsen_refcode_rfcal);
    log_info("[APP] rf.cal.tsen_refcode_corner = %u\r\n", fconfig.fw_rf_data.cal.tsen_refcode_corner);
    log_info("[APP] rf.cal.rc32k_code_fr_ext = %u\r\n", fconfig.fw_rf_data.cal.rc32k_code_fr_ext);
    log_info("[APP] rf.cal.rc32m_code_fr_ext = %u\r\n", fconfig.fw_rf_data.cal.rc32m_code_fr_ext);
    log_info("[APP] rf.cal.saradc_oscode = %u\r\n", fconfig.fw_rf_data.cal.saradc_oscode);
    log_info("[APP] rf.cal.fcal_4osmx = %u\r\n", fconfig.fw_rf_data.cal.fcal_4osmx);

	for(int i=0;i<21;i++) {
        log_info("[APP] rf.lo[%d] fcal=%u, acal=%u\r\n", i, fconfig.fw_rf_data.lo[i].fcal, fconfig.fw_rf_data.lo[i].acal);
	}
	for(int j=0;j<4;j++) {
        log_info("[APP] rf.rxcal[%d] rosdac_i=%u, rosdac_q=%u, rx_iq_gain_comp=%u, rx_iq_phase_comp=%u\r\n", j, fconfig.fw_rf_data.rxcal[j].rosdac_i, fconfig.fw_rf_data.rxcal[j].rosdac_q, fconfig.fw_rf_data.rxcal[j].rx_iq_gain_comp, fconfig.fw_rf_data.rxcal[j].rx_iq_phase_comp);
	}
	for(int k=0;k<8;k++) {
        log_info("[APP] rf.txcal[%d] tosdac_i=%u, tosdac_q=%u, tx_iq_gain_comp=%u, tx_iq_phase_comp=%u\r\n", k, fconfig.fw_rf_data.txcal[k].tosdac_i, fconfig.fw_rf_data.txcal[k].tosdac_q, fconfig.fw_rf_data.txcal[k].tx_iq_gain_comp, fconfig.fw_rf_data.txcal[k].tx_iq_phase_comp);
	}
#endif
}

/* TODO: const */
volatile uint32_t uxTopUsedPriority __attribute__((used)) =  configMAX_PRIORITIES - 1;
static uint32_t time_main;
static uint32_t time_main2;

static wifi_conf_t conf =
{
    .country_code = "CN",
};
extern uint8_t _heap_start;
extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size; // @suppress("Type cannot be resolved")
static HeapRegion_t xHeapRegions[] =
{
        { &_heap_start,  (unsigned int) &_heap_size}, //set on runtime
        { &_heap_wifi_start, (unsigned int) &_heap_wifi_size },
        { NULL, 0 }, /* Terminates the array. */
        { NULL, 0 } /* Terminates the array. */
};

static wifi_interface_t wifi_interface;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName )
{
    puts("Stack Overflow checked\r\n");
    while (1) {
        /*empty here*/
    }
}

void vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n",
        xPortGetFreeHeapSize()
    );
    while (1) {
        /*empty here*/
		vTaskDelay(3000);
        printf("Memory Allocate Failed. Current left size is %d bytes\r\n",
            xPortGetFreeHeapSize()
        );
    }
}

void vApplicationIdleHook(void)
{
#if 0
    __asm volatile(
            "   wfi     "
    );
    /*empty*/
#else
    (void)uxTopUsedPriority;
#endif
}

static void wifi_mgmr_fastconnect_connected_config(void)
{
    wifi_mgmr_sta_connect_ind_stat_info_t wifi_mgmr_ind_stat;

    memset(&wifi_mgmr_ind_stat, 0, sizeof(wifi_mgmr_ind_stat));
    wifi_mgmr_sta_connect_ind_stat_get(&wifi_mgmr_ind_stat);

    //save ssid into psm
    memset(fconfig.ssid, 0, sizeof(fconfig.ssid));
    memcpy(fconfig.ssid, wifi_mgmr_ind_stat.ssid, strlen(wifi_mgmr_ind_stat.ssid));
    //save psk into psm
    memset(fconfig.pawd, 0, sizeof(fconfig.pawd));
    memcpy(fconfig.pawd, wifi_mgmr_ind_stat.psk, strlen(wifi_mgmr_ind_stat.psk));
    //save pmk into psm
    memset(fconfig.adv.pmk, 0, sizeof(fconfig.adv.pmk));
    memcpy(fconfig.adv.pmk, wifi_mgmr_ind_stat.pmk, 64);

    //log_buf(fconfig.adv.pmk, 65);

    //save bssid into psm
    memcpy(fconfig.adv.mac, wifi_mgmr_ind_stat.bssid, 6);

    //save channel into psm
    fconfig.adv.freq = wifi_mgmr_ind_stat.chan_freq;
    fconfig.adv.band= wifi_mgmr_ind_stat.chan_band;

    //save rf cal data
    fw_rf_get_cal_data(&fconfig.fw_rf_data);
}

static void wifi_mgmr_dump_fastconnect_info(void *arg)
{
    cmd_fc_mdump(NULL, 0, 0, NULL);
}

static void wifi_mgmr_fastconnect_gotip_config(void *arg)
{
    wifi_mgmr_network_config_adv_t *adv_conf = &fconfig.adv;

    //memset(&fconfig.adv, 0, sizeof(wifi_mgmr_network_config_adv_t));
    wifi_mgmr_sta_connect_get_network_config(&(fconfig.adv));

    if (fconfig.adv.pmk[0] == 0) {
        wifi_mgmr_psk_cal(
                    fconfig.pawd,
                    fconfig.ssid,
                    strlen(fconfig.ssid),
                    fconfig.adv.pmk
                    );
        log_info("[APP] pmk invalid, produce new one");
    }
    log_info("[APP] static IP:%u.%u.%u.%u, "
        "MASK: %u.%u.%u.%u, "
        "Gateway: %u.%u.%u.%u, "
        "dns1: %u.%u.%u.%u, "
        "dns2: %u.%u.%u.%u\r\n",
        (unsigned int)((adv_conf->ip & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->ip & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->ip & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->ip & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->mask & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->mask & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->mask & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->mask & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->gw & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->gw & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->gw & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->gw & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->dns1 & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->dns1 & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->dns1 & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->dns1 & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->dns2 & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->dns2 & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->dns2 & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->dns2 & 0xFF000000) >> 24)
    );
    //save ip into psm
    //save mask into psm
    //save gw into psm
    //save dns1 into psm
    //save dns2 into psm
    //save ip_lease_time into psm
}

static void _connect_wifi()
{
    wifi_mgmr_network_config_adv_t *adv_conf = &fconfig.adv;

    if(!fconfig.fc_en) {
        log_info("fast connect mode disable\r\n");
        return;
    }

    wifi_interface = wifi_mgmr_sta_enable();
    blog_info("[APP] [WIFI] [T] %lld\r\n"
           "[APP]   Get STA %p from Wi-Fi Mgmr, ssid ptr %p, password %p\r\n",
           aos_now_ms(),
           wifi_interface,
           fconfig.ssid,
           fconfig.pawd
    );

    netbus_wifi_mgmr_set_mode(&g_netbus_wifi_mgmr_env, NETBUS_WIFI_MODE_STA);
    netbus_wifi_mgmr_set_ip_got(&g_netbus_wifi_mgmr_env, true);

    log_info("ssid[%d] %s\r\n", strlen(fconfig.ssid), fconfig.ssid);
    log_info("pawd[%d] %s\r\n", strlen(fconfig.pawd), fconfig.pawd);
    log_info("pmk %s\r\n", adv_conf->pmk);
    log_info("channel band %d\r\n", adv_conf->band);
    log_info("channel freq %d\r\n", adv_conf->freq);

    wifi_mgmr_sta_ssid_set(fconfig.ssid);
    wifi_mgmr_sta_psk_set(fconfig.pawd);
    wifi_mgmr_sta_pmk_set(adv_conf->pmk);
    //memcpy(adv_conf->gw_mac, fconfig.adv.gw_mac, 6);
    adv_conf->dhcp_use = 1;   //default use dhcp

    log_info("[APP] static IP:%u.%u.%u.%u, "
        "MASK: %u.%u.%u.%u, "
        "Gateway: %u.%u.%u.%u, "
        "dns1: %u.%u.%u.%u, "
        "dns2: %u.%u.%u.%u, "
        "mac:  %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        (unsigned int)((adv_conf->ip & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->ip & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->ip & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->ip & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->mask & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->mask & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->mask & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->mask & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->gw & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->gw & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->gw & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->gw & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->dns1 & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->dns1 & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->dns1 & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->dns1 & 0xFF000000) >> 24),
        (unsigned int)((adv_conf->dns2 & 0x000000FF) >> 0),
        (unsigned int)((adv_conf->dns2 & 0x0000FF00) >> 8),
        (unsigned int)((adv_conf->dns2 & 0x00FF0000) >> 16),
        (unsigned int)((adv_conf->dns2 & 0xFF000000) >> 24),
        adv_conf->gw_mac[0],
        adv_conf->gw_mac[1],
        adv_conf->gw_mac[2],
        adv_conf->gw_mac[3],
        adv_conf->gw_mac[4],
        adv_conf->gw_mac[5]
    );

    log_info("IP LEASE TIME = 0x%llx\r\n", adv_conf->ip_lease_time);
    log_info("bouffalolabcal wifi_mgmr_sta_connect = %lums\r\n", bl_timer_now_us()/1000);
    wifi_mgmr_sta_connect(wifi_interface, fconfig.ssid, fconfig.pawd, adv_conf);
}

#if 0
static void notify_led_repeat(void *arg)
{
    struct udp_pcb *gw_pcb = (struct udp_pcb*)arg;
    struct pbuf *p;
    static int count = 9;

    //printf("Led notify %d...\r\n", count);
    p = pbuf_alloc(PBUF_TRANSPORT, 16, PBUF_RAM);
    if (p) {
        udp_sendto(gw_pcb, p, IP_ADDR_BROADCAST, 31602);
        pbuf_free(p);
        puts("x");
    }
    if (--count > 0) {
        aos_post_delayed_action(1, notify_led_repeat, gw_pcb);
    }
}

static void __notify_led(void)
{
     struct udp_pcb *gw_pcb;
     struct pbuf *p;
    ip_addr_t gw_addr;

    gw_pcb = udp_new();
    udp_bind(gw_pcb, IP_ADDR_ANY, 0);
    p = pbuf_alloc(PBUF_TRANSPORT, 16, PBUF_RAM);
    ipaddr_aton("192.168.11.1", &gw_addr);
#if 1
    udp_sendto(gw_pcb, p, IP_ADDR_BROADCAST, 31602);
#else
    udp_sendto(gw_pcb, p, &gw_addr, 31602);
#endif
    aos_post_delayed_action(1, notify_led_repeat, gw_pcb);
    pbuf_free(p);
}
#endif


static void wifi_gw_mac_obtaining_timeout(TimerHandle_t xTimer)
{
    wifi_mgmr_network_config_adv_t *adv_conf = &fconfig.adv;
    uint8_t gw_mac[6];

    printf("get wifi gw mac waiting timeout\r\n");
    memset(gw_mac, 0, sizeof(gw_mac));
    wifi_mgmr_sta_gw_mac_get(gw_mac);
    if (gw_mac[0] | gw_mac[1] | gw_mac[2] | gw_mac[3] | gw_mac[4] | gw_mac[5]) {
        printf("get valid gw mac, save it\r\n");
        printf("[APP] gw mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            gw_mac[0],
            gw_mac[1],
            gw_mac[2],
            gw_mac[3],
            gw_mac[4],
            gw_mac[5]
        );
        if (memcmp(adv_conf->gw_mac, gw_mac, 6)) {
            printf("New MAC config, save it. Old %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                adv_conf->gw_mac[0],
                adv_conf->gw_mac[1],
                adv_conf->gw_mac[2],
                adv_conf->gw_mac[3],
                adv_conf->gw_mac[4],
                adv_conf->gw_mac[5]
            );
            memcpy(adv_conf->gw_mac, gw_mac, 6);
        } else {
            puts("Old MAC config, skip save\r\n");
        }
    } else {
        puts("get gw mac failure, use bssid instead of gw mac\r\n");
        memcpy(adv_conf->gw_mac, adv_conf->mac, 6);
    }
    /* fconfig.fc_en = 1; */
    /* psm_mtd_write(&fconfig, sizeof(fconfig)); */
}

static void wifi_mgmr_get_and_save_gw_mac()
{
    TimerHandle_t xTimerUser;

    printf("[APP] Create gw mac timer\r\n");
    xTimerUser = xTimerCreate
        ("wifi gw mac obtaining",
        8,
        pdFALSE,
        ( void * ) 0,
        wifi_gw_mac_obtaining_timeout);

    if( xTimerUser != NULL) {
        printf("start get gw mac timer\r\n");
        xTimerStart( xTimerUser, 0 );
    }
}

static void event_cb_wifi_event(input_event_t *event, void *private_data)
{
    netbus_wifi_mgmr_msg_t swm_msg;
    switch (event->code) {
        case CODE_WIFI_ON_INIT_DONE:
        {
			log_info("[APP] [EVT] INIT DONE = %lums\r\n", bl_timer_now_us()/1000);
            wifi_mgmr_start_background(&conf);
        }
        break;
        case CODE_WIFI_ON_MGMR_DONE:
        {
			log_info("[APP] [EVT] MGMR DONE, now %lums\r\n", bl_timer_now_us()/1000);
			_connect_wifi();
#if ENABLE_SDIOWIFI
            netbus_wifi_mgmr_start(&g_netbus_wifi_mgmr_env);
#endif
        }
        break;
        case CODE_WIFI_ON_MGMR_DENOISE:
        {
            printf("[APP] [EVT] Microwave Denoise is ON %lld\r\n", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_SCAN_DONE:
        {
			log_info("[APP] [EVT] SCAN Done %lums\r\n", bl_timer_now_us()/1000);
            wifi_mgmr_cli_scanlist();
        }
        break;
        case CODE_WIFI_ON_SCAN_DONE_ONJOIN:
        {
            printf("[APP] [EVT] SCAN On Join %lums\r\n", bl_timer_now_us()/1000);
        }
        break;
        case CODE_WIFI_ON_DISCONNECT:
        {
            printf("[APP] [EVT] disconnect %lld, Reason: %s\r\n",
                aos_now_ms(),
                wifi_mgmr_status_code_str(event->value)
            );
#if ENABLE_SDIOWIFI
            memset(&swm_msg, 0, sizeof(swm_msg));
            swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_CMD;
            swm_msg.u.cmd.cmd = BFLB_CMD_AP_DISCONNECTED_IND;

            netbus_wifi_mgmr_msg_send(&g_netbus_wifi_mgmr_env, &swm_msg, true, false);
#endif
        }
        break;
        case CODE_WIFI_ON_CONNECTING:
        {
            printf("[APP] [EVT] Connecting %lums\r\n", bl_timer_now_us()/1000);
        }
        break;
        case CODE_WIFI_CMD_RECONNECT:
        {
            printf("[APP] [EVT] Reconnect %lld\r\n", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_CONNECTED:
        {
            printf("[APP] [EVT] connected %lums\r\n", bl_timer_now_us()/1000);
            //__notify_led();
			//bl_gpio_output_set(GPIO_LED_PIN_NUM, 1);
#if ENABLE_SDIOWIFI
            memset(&swm_msg, 0, sizeof(swm_msg));
            swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_CMD;
            swm_msg.u.cmd.cmd = BFLB_CMD_AP_CONNECTED_IND;

            netbus_wifi_mgmr_msg_send(&g_netbus_wifi_mgmr_env, &swm_msg, true, false);
#endif
            wifi_mgmr_fastconnect_connected_config();
        }
        break;
        case CODE_WIFI_ON_PRE_GOT_IP:
        {
            printf("[APP] [EVT] pre got ip %lums\r\n", bl_timer_now_us()/1000);
        }
        break;
        case CODE_WIFI_ON_GOT_IP:
        {
            printf("[APP] [EVT] GOT IP %lums\r\n", bl_timer_now_us()/1000);
            printf("[SYS] Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());
            netbus_wifi_mgmr_set_ip_got(&g_netbus_wifi_mgmr_env, true);
            netbus_wifi_mgmr_send_ip_update(&g_netbus_wifi_mgmr_env);
			//aos_post_delayed_action(2000, wifi_mgmr_fastconnect_gotip_config, NULL);
            wifi_mgmr_fastconnect_gotip_config(NULL);
			wifi_mgmr_get_and_save_gw_mac();
			//aos_post_delayed_action(2000, wifi_mgmr_dump_fastconnect_info, NULL);
        }
        break;
        default:
        {
            printf("[APP] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
            /*nothing*/
        }
    }
}

static void cmd_stack_wifi(char *buf, int len, int argc, char **argv)
{
    /*wifi fw stack and thread stuff*/
    static StackType_t wifi_fw_stack[1024];
    static StaticTask_t wifi_fw_task;
    static uint8_t stack_wifi_init  = 0;


    if (1 == stack_wifi_init) {
        puts("Wi-Fi Stack Started already!!!\r\n");
        return;
    }
    stack_wifi_init = 1;

    printf("Start Wi-Fi fw @%lums\r\n", bl_timer_now_us()/1000);
    xTaskCreateStatic(wifi_main, (char*)"fw", 1024, NULL, TASK_PRIORITY_FW, wifi_fw_stack, &wifi_fw_task);
    /*Trigger to start Wi-Fi*/
    printf("Start Wi-Fi fw is Done @%lums\r\n", bl_timer_now_us()/1000);
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);

}

const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
	    {"fcmtest", "pmtest", cmd_fc_mtest},
        {"fcmerase", "pmtest", cmd_fc_merase},
        {"fcenable", "fast connect function enable/disable", cmd_fc_enable},
        {"fcmdump", "pmtest", cmd_fc_mdump},
};

static void _cli_init()
{
    /*Put CLI which needs to be init here*/
    wifi_mgmr_cli_init();
    bl_gpio_cli_init();
    // sdiowifi_test_cli_init();
}

static int get_dts_addr(const char *name, uint32_t *start, uint32_t *off)
{
    uint32_t addr = hal_board_get_factory_addr();
    const void *fdt = (const void *)addr;
    uint32_t offset;

    if (!name || !start || !off) {
        return -1;
    }

    offset = fdt_subnode_offset(fdt, 0, name);
    if (offset <= 0) {
       log_error("%s NULL.\r\n", name);
       return -1;
    }

    *start = (uint32_t)fdt;
    *off = offset;

    return 0;
}

extern void uart_init(uint8_t uartid);
static TaskHandle_t ble_init_task_hdl;

static void ble_init_task_entry(void *pvParameters)
{
    bl_sys_em_config();
    // Initialize UART component
    uart_init(0);
    ble_controller_init(configMAX_PRIORITIES - 1);
    vTaskDelete(NULL);
}

static void aos_loop_proc(void *pvParameters)
{
    int fd_console;
    uint32_t fdt = 0, offset = 0;

    vfs_init();
    vfs_device_init();

    /* uart */
#if 0
    if (0 == get_dts_addr("uart", &fdt, &offset)) {
        vfs_uart_init(fdt, offset);
    }
#else
    vfs_uart_init_simple_mode(0, UART0_TX_PIN, UART0_RX_PIN, 2 * 1000 * 1000, "/dev/ttyS0");
//    vfs_uart_init_simple_mode(1, UART1_TX_PIN, UART1_RX_PIN, 115200, "/dev/ttyS1");
#endif

    aos_loop_init();

    fd_console = aos_open("/dev/ttyS0", 0);
    if (fd_console >= 0) {
        printf("Init CLI with event Driven\r\n");
        aos_cli_init(0);
        aos_poll_read_fd(fd_console, aos_cli_event_cb_read_get(), (void*)0x12345678);
        _cli_init();
    }

    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);
    cmd_stack_wifi(NULL, 0, 0, NULL);

    // start controller
//	xTaskCreate(ble_init_task_entry, (char*)"bleinit", 256, NULL, 15, &ble_init_task_hdl);

    aos_loop_run();

    puts("------------------------------------------\r\n");
    puts("+++++++++Critical Exit From Loop++++++++++\r\n");
    puts("******************************************\r\n");
    vTaskDelete(NULL);
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vAssertCalled(void)
{
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    taskDISABLE_INTERRUPTS();
    while( ulSetTo1ToExitFunction != 1 ) {
        __asm volatile( "NOP" );
    }
}

static void _dump_boot_info(void)
{
    char chip_feature[40];
    const char *banner;

    puts("Booting BL602 Chip...\r\n");

    /*Display Banner*/
    if (0 == bl_chip_banner(&banner)) {
        puts(banner);
    }
    puts("\r\n");
    /*Chip Feature list*/
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");
    puts("RISC-V Core Feature:");
    bl_chip_info(chip_feature);
    puts(chip_feature);
    puts("\r\n");

    puts("Build Version: ");
    puts(BL_SDK_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    puts("PHY   Version: ");
    puts(BL_SDK_PHY_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    puts("RF    Version: ");
    puts(BL_SDK_RF_VER); // @suppress("Symbol is not resolved")
    puts("\r\n");

    puts("Build Date: ");
    puts(__DATE__);
    puts("\r\n");

    puts("Build Time: ");
    puts(__TIME__);
    puts("\r\n");
    puts("------------------------------------------------------------\r\n");

}

void static get_wifi_config_info()
{
    memset(&fconfig, 0, sizeof(fconfig));
    psm_mtd_init();

    log_info("bouffalolabcal ef start = %lums\r\n", bl_timer_now_us()/1000);
    psm_mtd_read(&fconfig, sizeof(fconfig));
    log_info("bouffalolabcal ef end = %lums\r\n", bl_timer_now_us()/1000);
    if (fconfig.ssid[0] == 0xFF) {
        log_info("invalid ssid.\r\n");
		fconfig.fc_en = 0;
        return;
    }

	fw_rf_set_cal_data(&fconfig.fw_rf_data);
#if 0
    log_info("start set rf data = %lums\r\n", bl_timer_now_us()/1000);
    log_info("[psm read] rf.cal.gpadc_oscode = %u\r\n", fconfig.fw_rf_data.cal.gpadc_oscode);
    log_info("[psm read] rf.cal.rx_offset_q = %u\r\n", fconfig.fw_rf_data.cal.rx_offset_q);
    log_info("[psm read] rf.cal.rbb_cap1_fc_i = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap1_fc_i);
    log_info("[psm read] rf.cal.rbb_cap1_fc_q = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap1_fc_q);
    log_info("[psm read] rf.cal.rbb_cap2_fc_i = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap2_fc_i);
    log_info("[psm read] rf.cal.rbb_cap2_fc_q = %u\r\n", fconfig.fw_rf_data.cal.rbb_cap2_fc_q);
    log_info("[psm read] rf.cal.tx_dc_comp_i = %u\r\n", fconfig.fw_rf_data.cal.tx_dc_comp_i);
    log_info("[psm read] rf.cal.tx_dc_comp_q = %u\r\n", fconfig.fw_rf_data.cal.tx_dc_comp_q);
    log_info("[psm read] rf.cal.tmx_cs = %u\r\n", fconfig.fw_rf_data.cal.tmx_cs);
    log_info("[psm read] rf.cal.txpwr_att_rec = %u\r\n", fconfig.fw_rf_data.cal.txpwr_att_rec);
    log_info("[psm read] rf.cal.pa_pwrmx_osdac = %u\r\n", fconfig.fw_rf_data.cal.pa_pwrmx_osdac);
    log_info("[psm read] rf.cal.tmx_csh = %u\r\n", fconfig.fw_rf_data.cal.tmx_csh);
    log_info("[psm read] rf.cal.tmx_csl = %u\r\n", fconfig.fw_rf_data.cal.tmx_csl);
    log_info("[psm read] rf.cal.tsen_refcode_rfcal = %u\r\n", fconfig.fw_rf_data.cal.tsen_refcode_rfcal);
    log_info("[psm read] rf.cal.tsen_refcode_corner = %u\r\n", fconfig.fw_rf_data.cal.tsen_refcode_corner);
    log_info("[psm read] rf.cal.rc32k_code_fr_ext = %u\r\n", fconfig.fw_rf_data.cal.rc32k_code_fr_ext);
    log_info("[psm read] rf.cal.rc32m_code_fr_ext = %u\r\n", fconfig.fw_rf_data.cal.rc32m_code_fr_ext);
    log_info("[psm read] rf.cal.saradc_oscode = %u\r\n", fconfig.fw_rf_data.cal.saradc_oscode);
    log_info("[psm read] rf.cal.fcal_4osmx = %u\r\n", fconfig.fw_rf_data.cal.fcal_4osmx);

	for(int i=0;i<21;i++) {
        log_info("[psm read] rf.lo[%d] fcal=%u, acal=%u\r\n", i, fconfig.fw_rf_data.lo[i].fcal, fconfig.fw_rf_data.lo[i].acal);
	}
	for(int j=0;j<4;j++) {
        log_info("[psm read] rf.rxcal[%d] rosdac_i=%u, rosdac_q=%u, rx_iq_gain_comp=%u, rx_iq_phase_comp=%u\r\n", j, fconfig.fw_rf_data.rxcal[j].rosdac_i, fconfig.fw_rf_data.rxcal[j].rosdac_q, fconfig.fw_rf_data.rxcal[j].rx_iq_gain_comp, fconfig.fw_rf_data.rxcal[j].rx_iq_phase_comp);
	}
	for(int k=0;k<8;k++) {
        log_info("[psm read] rf.txcal[%d] rosdac_i=%u, rosdac_q=%u, tx_iq_gain_comp=%u, tx_iq_phase_comp=%u\r\n", k, fconfig.fw_rf_data.txcal[k].tosdac_i, fconfig.fw_rf_data.txcal[k].tosdac_q, fconfig.fw_rf_data.txcal[k].tx_iq_gain_comp, fconfig.fw_rf_data.txcal[k].tx_iq_phase_comp);
	}
#endif
}

static void system_init(void)
{
	time_main2 = bl_timer_now_us();
    log_info("bouffalolabcal system_init start = %ld\r\n", time_main2);
    blog_init();
    bl_irq_init();
    bl_sec_init();
    bl_sec_test();
    hosal_dma_init();
    hal_boot2_init();

    /* board config is set after system is init*/
    hal_board_cfg(0);
    time_main2 = bl_timer_now_us();
    log_info("bouffalolabcal system_init end = %ld\r\n", time_main2);
}

static void system_thread_init()
{
    /*nothing here*/
}

void bfl_main()
{
    static StackType_t aos_loop_proc_stack[1024];
    static StaticTask_t aos_loop_proc_task;

    time_main = bl_timer_now_us();
    /*Init UART In the first place*/
    bl_uart_init(0, UART0_TX_PIN, UART0_RX_PIN, 255, 255, 2 * 1000 * 1000);
    puts("Starting bl602 now....\r\n");

    _dump_boot_info();

    vPortDefineHeapRegions(xHeapRegions);
    printf("Heap %u@%p, %u@%p\r\n",
            (unsigned int)&_heap_size, &_heap_start,
            (unsigned int)&_heap_wifi_size, &_heap_wifi_start
    );
    printf("Boot2 consumed %lums\r\n", time_main / 1000);

    get_wifi_config_info();

    system_init();
    system_thread_init();

    puts("[OS] Starting aos_loop_proc task...\r\n");
    xTaskCreateStatic(aos_loop_proc, (char*)"event_loop", 1024, NULL, 15, aos_loop_proc_stack, &aos_loop_proc_task);
    puts("[OS] Starting TCP/IP Stack...\r\n");
    tcpip_init(NULL, NULL);

    puts("[OS] Starting OS Scheduler...\r\n");
    vTaskStartScheduler();
}
