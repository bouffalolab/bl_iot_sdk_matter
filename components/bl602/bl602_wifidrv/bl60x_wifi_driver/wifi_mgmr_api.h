#ifndef __WIFI_MGMR_API_H__
#define __WIFI_MGMR_API_H__
#include <stdint.h>

/**
 ****************************************************************************************
 *
 * @file wifi_mgmr_api.h
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

int wifi_mgmr_api_connect(char *ssid, char *psk, char *pmk, uint8_t *mac, uint8_t band, uint16_t freq);
int wifi_mgmr_api_ip_got(uint32_t ip, uint32_t mask, uint32_t gw, uint32_t dns1, uint32_t dns2);
int wifi_mgmr_api_reconnect(void);
int wifi_mgmr_api_disconnect(void);
int wifi_mgmr_api_rate_config(uint16_t config);
int wifi_mgmr_api_ifaceup(void);
int wifi_mgmr_api_sniffer_enable(void);
int wifi_mgmr_api_ap_start(char *ssid, char *passwd, int channel);
int wifi_mgmr_api_ap_stop(void);
int wifi_mgmr_api_idle(void);
int wifi_mgmr_api_channel_set(int channel, int use_40Mhz);
int wifi_mgmr_api_raw_send(uint8_t *pkt, int len);
int wifi_mgmr_api_set_country_code(char *country_code);

/*section for fw api*/
int wifi_mgmr_api_fw_disconnect(void);
int wifi_mgmr_api_fw_scan(void);
#define WIFI_MGMR_API_FW_POWERSAVING_MODE_OFF           (1)
#define WIFI_MGMR_API_FW_POWERSAVING_MODE_ON            (2)
#define WIFI_MGMR_API_FW_POWERSAVING_MODE_DYNAMIC_ON    (3)
int wifi_mgmr_api_fw_powersaving(int mode);
int wifi_mgmr_api_disable_autoreconnect(void);
int wifi_mgmr_api_enable_autoreconnect(void);

/*section for global event*/
int wifi_mgmr_api_scan_item_beacon(uint8_t channel, int8_t rssi, uint8_t auth, uint8_t mac[], uint8_t ssid[], int len, int8_t ppm_abs, int8_t ppm_rel);
#endif
