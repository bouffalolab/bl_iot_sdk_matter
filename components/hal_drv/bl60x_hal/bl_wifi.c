#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <bl60x_glb.h>
#include <bl_efuse.h>
#include <bl_wifi.h>
#include <bl60x_fw_api.h>


#define MACIRQ_IRQn 56
#define HOSTIRQ_IRQn 63

typedef struct _bl_wifi_env {
    uint8_t sta_mac_addr_board[6];
    uint8_t sta_mac_addr_usr[6];
    uint8_t ap_mac_addr_board[6];
    uint8_t ap_mac_addr_usr[6];
    uint8_t country_code;
    bl_wifi_ap_info_t ap_info;
    uint8_t ap_info_en;
} bl_wifi_env_t;

bl_wifi_env_t wifi_env;


static int wifi_mac_addr_is_vaild(uint8_t mac[6])
{
    uint8_t mac_idx = 0;
    for (mac_idx = 0; mac_idx < 6; ++mac_idx) {
        if (mac[mac_idx] != 0) {
            return 1;
        }
    }
    return 0;
}

int bl_wifi_clock_enable(void)
{
    static int called = 0;
    if (0 == called) {
        called = 1;
        //GLB_Set_System_CLK(PLL_XTAL_38P4M, GLB_PLL_CLK_160M);
        //GLB_Set_WiFi_Core_CLK(1);//0: 80MHZ, 1: 40MHZ
    }
    return 0;
}

int bl_wifi_enable_irq(void)
{
    NVIC_SetPriority((IRQn_Type)HOSTIRQ_IRQn, 7);
    NVIC_EnableIRQ((IRQn_Type)HOSTIRQ_IRQn);

    NVIC_SetPriority((IRQn_Type)MACIRQ_IRQn, 7);
    NVIC_EnableIRQ((IRQn_Type)MACIRQ_IRQn);

    //NVIC_SetPriority((IRQn_Type)5, 0);
    //NVIC_EnableIRQ((IRQn_Type)5);
    puts("Enable BMX IRQ\r\n");
    //NVIC_EnableIRQ((IRQn_Type)0);
    //NVIC_EnableIRQ((IRQn_Type)1);
    //NVIC_EnableIRQ((IRQn_Type)2);
    //NVIC_EnableIRQ((IRQn_Type)3);
    //NVIC_EnableIRQ((IRQn_Type)4);
    //*(uint32_t*)0x40000050 = ((0xF << 28) | (1 << 24));

    return 0;
}


int bl_wifi_sta_mac_addr_set(uint8_t mac[6])
{
    memcpy(wifi_env.sta_mac_addr_board, mac, 6);
    return 0;
}

int bl_wifi_ap_mac_addr_set(uint8_t mac[6])
{
    memcpy(wifi_env.ap_mac_addr_board, mac, 6);
    return 0;
}

int bl_wifi_mac_addr_set(uint8_t mac[6])
{
    memcpy(wifi_env.sta_mac_addr_usr, mac, 6);
    return 0;
}

int bl_wifi_mac_addr_get(uint8_t mac[6])
{
    static int init = 0;

    /*Use mac provided by user API as first priority*/
    if (wifi_mac_addr_is_vaild(wifi_env.sta_mac_addr_usr)) {
        if (0 == init) {
            init = 1;
            puts("[MAC from user]");
        }
        memcpy(mac, wifi_env.sta_mac_addr_usr, 6);
        return 0;
    }
    /*efuse mac address as second priority*/
    bl_efuse_read_mac(mac);
    if (wifi_mac_addr_is_vaild(mac)) {
        if (0 == init) {
            init = 1;
            puts("[MAC from EFUSE]");
        }
        return 0;
    }
    /*board mac address as third priority*/
    if (wifi_mac_addr_is_vaild(wifi_env.sta_mac_addr_board)) {
        memcpy(mac, wifi_env.sta_mac_addr_board, 6);
        if (0 == init) {
            init = 1;
            puts("[MAC from Flash]");
        }
        return 0;
    }
    return -1;
}


int bl_wifi_country_code_set(uint8_t country_code)
{
    wifi_env.country_code = country_code;
    return 0;
}


int bl_wifi_power_table_set(bl_tx_pwr_tbl_t* tx_pwr_tbl)
{
    bl60x_fw_rf_tx_power_table_set(tx_pwr_tbl);
    return 0;
}


int bl_wifi_ap_info_set(uint8_t* ssid, uint8_t ssid_len, 
                      uint8_t* psk, uint8_t psk_len,
                      uint8_t chan)
{
    memset(&wifi_env.ap_info, 0, sizeof(bl_wifi_ap_info_t));
    memcpy(wifi_env.ap_info.ssid, ssid, ssid_len);
    memcpy(wifi_env.ap_info.psk, psk, psk_len);
    wifi_env.ap_info.chan = chan;
    wifi_env.ap_info_en = 1;
    return 0;
}

int bl_wifi_ap_info_get(bl_wifi_ap_info_t* ap_info)
{
    if (wifi_env.ap_info_en != 1) {
        return -1;
    }
    memcpy(ap_info, &wifi_env.ap_info, sizeof(bl_wifi_ap_info_t));
    return 0;
}

