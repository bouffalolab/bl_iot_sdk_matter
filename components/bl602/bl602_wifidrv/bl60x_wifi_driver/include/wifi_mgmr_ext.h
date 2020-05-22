#ifndef __WIFI_MGMR_EXT_H__
#define __WIFI_MGMR_EXT_H__
#include <lwip/netif.h>

typedef struct wifi_mgmr_ap_item {
    char ssid[32];
    char ssid_tail[1];//always put ssid_tail after ssid
    uint32_t ssid_len;
    uint8_t bssid[6];
    uint8_t channel;
    int8_t rssi;
} wifi_mgmr_ap_item_t;

typedef struct wifi_mgmr_sta_connect_ind_stat_info {
    uint16_t status_code;
    /*mgmr recv ind event from fw when connect or disconnect  */
    uint8_t type_ind;
    char ssid[32];
    char psk[32];
    uint8_t bssid[6];
    uint16_t chan_freq;
    uint8_t chan_band;
} wifi_mgmr_sta_connect_ind_stat_info_t;

typedef struct wifi_sta_basic_info {
    uint8_t sta_idx;
    uint8_t is_used;
    uint8_t sta_mac[6];
    uint32_t tsfhi;
    uint32_t tsflo;
    int rssi;
    uint8_t data_rate;
} wifi_sta_basic_info_t;

typedef void *wifi_interface_t;
typedef void (*sniffer_cb_t)(void *env, uint8_t *pkt, int len);
typedef void (*scan_item_cb_t)(wifi_mgmr_ap_item_t *env, uint32_t *param1, wifi_mgmr_ap_item_t *item);
typedef void (*scan_complete_cb_t)(void *data, void *param);

/// Power Save mode setting
enum
{
    /// Power-save off
    PS_MODE_OFF,
    /// Power-save on - Normal mode
    PS_MODE_ON,
    /// Power-save on - Dynamic mode
    PS_MODE_ON_DYN,
};

enum WIFI_STATE_ENUM_LIST {
    WIFI_STATE_UNKNOWN                              = 0x00,
    WIFI_STATE_IDLE                                 = 0x01,
    WIFI_STATE_CONNECTING                           = 0x02,
    WIFI_STATE_CONNECTED_IP_GETTING                 = 0x03,
    WIFI_STATE_CONNECTED_IP_GOT                     = 0x04,
    WIFI_STATE_DISCONNECT                           = 0x05,
    WIFI_STATE_WITH_AP_IDLE                         = 0x11,
    WIFI_STATE_WITH_AP_CONNECTING                   = 0x12,
    WIFI_STATE_WITH_AP_CONNECTED_IP_GETTING         = 0x13,
    WIFI_STATE_WITH_AP_CONNECTED_IP_GOT             = 0x14,
    WIFI_STATE_WITH_AP_DISCONNECT                   = 0x15,
    WIFI_STATE_IFDOWN                               = 0x06,
    WIFI_STATE_SNIFFER                              = 0x07,
};
#define WIFI_STATE_AP_IS_ENABLED(status)       ((status) & 0x10)

typedef struct wifi_conf {
    const char *code;
} wifi_conf_t;

//FIXME implemented in wifi_mgmr.c
int wifi_mgmr_psk_cal(char *password, char *ssid, int ssid_len, char *output);
int wifi_mgmr_drv_init(wifi_conf_t *conf);
int wifi_mgmr_init(void);
void wifi_mgmr_start(void);
void wifi_mgmr_start_background(wifi_conf_t *conf);

wifi_interface_t wifi_mgmr_sta_enable(void);
int wifi_mgmr_sta_disable(wifi_interface_t *interface);
int wifi_mgmr_sta_mac_set(uint8_t mac[6]);
int wifi_mgmr_sta_mac_get(uint8_t mac[6]);
int wifi_mgmr_sta_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask);
int wifi_mgmr_sta_connect(wifi_interface_t *wifi_interface, char *ssid, char *psk, char *pmk, uint8_t *mac, uint8_t band, uint16_t freq);
int wifi_mgmr_sta_disconnect(void);
int wifi_mgmr_sta_powersaving(int ps);
void wifi_mgmr_sta_ssid_set(char *ssid);
void wifi_mgmr_sta_psk_set(char *psk);
void wifi_mgmr_sta_connect_ind_stat_get(wifi_mgmr_sta_connect_ind_stat_info_t *wifi_mgmr_ind_stat);

wifi_interface_t wifi_mgmr_ap_enable(void);
int wifi_mgmr_ap_mac_set(uint8_t mac[6]);
int wifi_mgmr_ap_mac_get(uint8_t mac[6]);
int wifi_mgmr_ap_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask);
int wifi_mgmr_ap_stop(wifi_interface_t *interface);
int wifi_mgmr_ap_start(wifi_interface_t *interface, char *ssid, int md, char *passwd, int channel);
int wifi_mgmr_ap_sta_cnt_get(uint8_t *sta_cnt);
int wifi_mgmr_ap_sta_info_get(struct wifi_sta_basic_info *sta_info, uint8_t idx);
int wifi_mgmr_ap_sta_delete(uint8_t sta_idx);
int wifi_mgmr_ap_set_gateway(char *gateway);
int wifi_mgmr_sniffer_enable(void);
int wifi_mgmr_sniffer_disable(void);
int wifi_mgmr_rate_config(uint16_t config);
int wifi_mgmr_sniffer_register(void *env, sniffer_cb_t cb);
int wifi_mgmr_sniffer_unregister(void *env);
int wifi_mgmr_state_get(int *state);
int wifi_mgmr_rssi_get(int *rssi);
int wifi_mgmr_channel_get(int *channel);
int wifi_mgmr_channel_set(int channel, int use_40Mhz);
int wifi_mgmr_all_ap_scan(wifi_mgmr_ap_item_t **ap_ary, uint32_t *num);
int wifi_mgmr_scan(void *data, scan_complete_cb_t cb);
int wifi_mgmr_scan_complete_callback();
int wifi_mgmr_cli_scanlist(void);
int wifi_mgmr_cli_init(void);
int wifi_mgmr_scan_ap(char *ssid, wifi_mgmr_ap_item_t *item);
int wifi_mgmr_scan_ap_all(wifi_mgmr_ap_item_t *env, uint32_t *param1, scan_item_cb_t cb);
int wifi_mgmr_raw_80211_send(uint8_t *pkt, int len);
int wifi_mgmr_ext_dump_needed();
#endif
