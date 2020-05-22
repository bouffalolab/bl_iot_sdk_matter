#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <cli.h>

#include <bl_efuse.h>
#include <hal_sys.h>
#include <bl60x_fw_api.h>
#include <wifi_mgmr.h>
#include <wifi_mgmr_api.h>
#include <utils_hexdump.h>
#include <wifi_mgmr_ext.h>


int wifi_mgmr_cli_powersaving_on()
{
    wifi_mgmr_api_fw_powersaving(2);
    return 0;
}

int wifi_mgmr_cli_scanlist(void)
{
    int i;

    printf("cached scan list\r\n");
    printf("****************************************************************************************************\r\n");
    for (i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]); i++) {
        if (wifiMgmr.scan_items[i].is_used) {
            printf("index[%02d]: channel %02u, bssid %02X:%02X:%02X:%02X:%02X:%02X, rssi %3d, ppm abs:rel %3d : %3d, SSID %s\r\n",
                    i,
                    wifiMgmr.scan_items[i].channel,
                    wifiMgmr.scan_items[i].bssid[0],
                    wifiMgmr.scan_items[i].bssid[1],
                    wifiMgmr.scan_items[i].bssid[2],
                    wifiMgmr.scan_items[i].bssid[3],
                    wifiMgmr.scan_items[i].bssid[4],
                    wifiMgmr.scan_items[i].bssid[5],
                    wifiMgmr.scan_items[i].rssi,
                    wifiMgmr.scan_items[i].ppm_abs,
                    wifiMgmr.scan_items[i].ppm_rel,
                    wifiMgmr.scan_items[i].ssid
            );
        } else {
            printf("index[%02d]: empty\r\n", i);
        }
    }
    printf("----------------------------------------------------------------------------------------------------\r\n");
    return 0;
}

static void cmd_rf_dump(char *buf, int len, int argc, char **argv)
{
    //bl60x_fw_dump_data();
}

static void wifi_capcode_cmd(char *buf, int len, int argc, char **argv)
{
    int capcode = 0;

    if (2 != argc) {
        printf("Usage: %s capcode\r\n", argv[0]);
        return;
    }
    capcode = atoi(argv[1]);
    printf("capcode is %d\r\n", capcode);

    if (capcode > 0) {
        hal_sys_capcode_update(capcode, capcode);
    }
}

static void wifi_scan_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_scan(NULL, NULL);
}

static void wifi_mon_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_sniffer_enable();
}

static uint8_t packet_raw[] = {
    0x48, 0x02,
    0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00
};

static void cmd_wifi_raw_send(char *buf, int len, int argc, char **argv)
{
    static uint32_t seq = 0;

    packet_raw[sizeof(packet_raw) - 2] = ((seq << 4) & 0xFF);
    packet_raw[sizeof(packet_raw) - 1] = (((seq << 4) & 0xFF00) >> 8);
    seq++;

    if (wifi_mgmr_raw_80211_send(packet_raw, sizeof(packet_raw))) {
        puts("Raw send failed\r\n");
    } else {
        puts("Raw send succeed\r\n");
    }
}

static void wifi_disconnect_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_sta_disconnect();
    /*XXX Must make sure sta is already disconnect, otherwise sta disable won't work*/
    vTaskDelay(1000);
    wifi_mgmr_sta_disable(NULL);
}

static void wifi_connect_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_interface_t wifi_interface;

    if (3 != argc) {
        printf("[USAGE]: %s [ssid] [password]\r\n", argv[0]);
        return;
    }

    wifi_interface = wifi_mgmr_sta_enable();
    wifi_mgmr_sta_connect(wifi_interface, argv[1], argv[2], NULL, NULL, 0, 0);
}


static void wifi_rc_fixed_enable(char *buf, int len, int argc, char **argv)
{
    uint8_t mcs = 0;
    uint8_t gi = 0;
    uint16_t rc = 0x1000; //format mode is HT_MF only

    if (argc != 3) {
        printf("rc_fix_en [MCS] [GI]");
        return;
    }
    mcs = atoi(argv[1]);
    gi = atoi(argv[2]);

    rc |= gi << 9 | mcs;

    wifi_mgmr_rate_config(rc);
}

static void wifi_rc_fixed_disable(char *buf, int len, int argc, char **argv)
{
    uint16_t rc = 0xFFFF;

    wifi_mgmr_rate_config(rc);
}

#if 0
static void wifi_capcode_update(char *buf, int len, int argc, char **argv)
{
    uint8_t cap_in, cap_out;

    if (argc == 1) {
        bl60x_fw_xtal_capcode_get(&cap_in, &cap_out);
        printf("[RF] [CAP] Dump capcode in:out %u:%u\r\n", cap_in, cap_out);
        return;
    }
    if (argc != 3) {
        printf("%s [capcode_in] [capcode_out]\r\n", argv[0]);
        return;
    }

    bl60x_fw_xtal_capcode_get(&cap_in, &cap_out);
    printf("[RF] [CAP] Dump capcode in:out %u:%u\r\n", cap_in, cap_out);
    cap_in = atoi(argv[1]);
    cap_out = atoi(argv[2]);
    printf("[RF] [CAP] Updating capcode to in:out %u:%u\r\n", cap_in, cap_out);
    bl60x_fw_xtal_capcode_update(cap_in, cap_out);
    bl60x_fw_xtal_capcode_get(&cap_in, &cap_out);
    printf("[RF] [CAP] Dump Again capcode in:out %u:%u\r\n", cap_in, cap_out);
}
#endif

static void wifi_power_saving_on_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_sta_powersaving(1);
}

static void wifi_power_saving_off_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_sta_powersaving(0);
}

static void sniffer_cb(void *env, uint8_t *pkt, int len)
{
    static unsigned int sniffer_counter, sniffer_last;
    static unsigned int last_tick;

    sniffer_counter++;
    if ((int)xTaskGetTickCount() - (int)last_tick > 10 * 1000) {
        printf("[SNIFFER] PKT Number is %d\r\n",
                (int)sniffer_last - (int)sniffer_counter
        );
        last_tick = xTaskGetTickCount();
        sniffer_last = sniffer_counter;
    }
}

static void wifi_sniffer_on_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_sniffer_enable();
    wifi_mgmr_sniffer_register(NULL, sniffer_cb);
}

static void wifi_sniffer_off_cmd(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_sniffer_disable();
    wifi_mgmr_sniffer_unregister(NULL);
}

static void cmd_wifi_ap_start(char *buf, int len, int argc, char **argv)
{
    uint8_t mac[6];
    char ssid_name[32];
    wifi_interface_t wifi_interface;

    memset(mac, 0, sizeof(mac));
    bl_efuse_read_mac(mac);
    memset(ssid_name, 0, sizeof(ssid_name));
    snprintf(ssid_name, sizeof(ssid_name), "BL60X_uAP_%02X%02X%02X", mac[3], mac[4], mac[5]);
    ssid_name[sizeof(ssid_name) - 1] = '\0';

    wifi_interface = wifi_mgmr_ap_enable();
    if (1 == argc) {
        /*no password when only one param*/
        wifi_mgmr_ap_start(wifi_interface, ssid_name, 0, NULL, 1);
    } else {
        /*hardcode password*/
        wifi_mgmr_ap_start(wifi_interface, ssid_name, 0, "bouffalolab", 1);
    }
}

static void cmd_wifi_ap_stop(char *buf, int len, int argc, char **argv)
{
    wifi_mgmr_ap_stop(NULL);
    printf("--->>> cmd_wifi_ap_stop\r\n");
}

static void cmd_wifi_dump(char *buf, int len, int argc, char **argv)
{
    if (argc > 1) {
        puts("[CLI] Dump statistic use forced mode\r\n");
        taskENTER_CRITICAL();
        bl60x_fw_dump_statistic(1);
        taskEXIT_CRITICAL();
    } else {
        puts("[CLI] Dump statistic use normal mode\r\n");
        taskENTER_CRITICAL();
        bl60x_fw_dump_statistic(0);
        taskEXIT_CRITICAL();
    }
}

static void cmd_wifi_mib(char *buf, int len, int argc, char **argv)
{
void hal_mib_dump();
    hal_mib_dump();
    utils_hexdump(argv[0], 30);
}

static int pkt_counter = 0;
int wifi_mgmr_ext_dump_needed()
{
    if (pkt_counter > 0) {
        pkt_counter--;
        return 1;
    }
    return 0;
}

static void cmd_dump_reset(char *buf, int len, int argc, char **argv)
{
    pkt_counter = 10;
}

void coex_wifi_rf_forece_enable(int enable);
static void cmd_wifi_coex_rf_force_on(char *buf, int len, int argc, char **argv)
{
    coex_wifi_rf_forece_enable(1);
}

static void cmd_wifi_coex_rf_force_off(char *buf, int len, int argc, char **argv)
{
    coex_wifi_rf_forece_enable(0);
}

void coex_wifi_pti_forece_enable(int enable);
static void cmd_wifi_coex_pti_force_on(char *buf, int len, int argc, char **argv)
{
    coex_wifi_pti_forece_enable(1);
}

static void cmd_wifi_coex_pti_force_off(char *buf, int len, int argc, char **argv)
{
    coex_wifi_pti_forece_enable(0);
}

void coex_wifi_pta_forece_enable(int enable);
static void cmd_wifi_coex_pta_force_on(char *buf, int len, int argc, char **argv)
{
    coex_wifi_pta_forece_enable(1);
}

static void cmd_wifi_coex_pta_force_off(char *buf, int len, int argc, char **argv)
{
    coex_wifi_pta_forece_enable(0);
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        { "rf_dump", "rf dump", cmd_rf_dump},
        { "wifi_capcode", "wifi capcode", wifi_capcode_cmd},
        { "wifi_scan", "wifi scan", wifi_scan_cmd},
        { "wifi_mon", "wifi monitor", wifi_mon_cmd},
        { "wifi_raw_send", "wifi raw send test", cmd_wifi_raw_send},
        { "wifi_sta_disconnect", "wifi station disconnect", wifi_disconnect_cmd},
        { "wifi_sta_connect", "wifi station connect", wifi_connect_cmd},
        { "rc_fix_en", "wifi rate control fixed rate enable", wifi_rc_fixed_enable},
        { "rc_fix_dis", "wifi rate control fixed rate diable", wifi_rc_fixed_disable},
#if 0
        { "wifi_capcode", "capcode utils\r\n wifi_capcode [cap_in] [cap_out]", wifi_capcode_update},
#endif
        { "wifi_sta_ps_on", "wifi power saving mode ON", wifi_power_saving_on_cmd},
        { "wifi_sta_ps_off", "wifi power saving mode OFF", wifi_power_saving_off_cmd},
        { "wifi_sniffer_on", "wifi sniffer mode on", wifi_sniffer_on_cmd},
        { "wifi_sniffer_off", "wifi sniffer mode off", wifi_sniffer_off_cmd},
        { "wifi_ap_start", "start Ap mode", cmd_wifi_ap_start},
        { "wifi_ap_stop", "start Ap mode", cmd_wifi_ap_stop},
        { "wifi_dump", "dump fw statistic", cmd_wifi_dump},
        { "wifi_mib", "dump mib statistic", cmd_wifi_mib},
        { "wifi_pkt", "wifi dump needed", cmd_dump_reset},
        { "wifi_coex_rf_force_on", "wifi coex RF forece on", cmd_wifi_coex_rf_force_on},
        { "wifi_coex_rf_force_off", "wifi coex RF forece off", cmd_wifi_coex_rf_force_off},
        { "wifi_coex_pti_force_on", "wifi coex PTI forece on", cmd_wifi_coex_pti_force_on},
        { "wifi_coex_pti_force_off", "wifi coex PTI forece off", cmd_wifi_coex_pti_force_off},
        { "wifi_coex_pta_force_on", "wifi coex PTA forece on", cmd_wifi_coex_pta_force_on},
        { "wifi_coex_pta_force_off", "wifi coex PTA forece off", cmd_wifi_coex_pta_force_off},
};                                                                                   

int wifi_mgmr_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));          
    return 0;
}
