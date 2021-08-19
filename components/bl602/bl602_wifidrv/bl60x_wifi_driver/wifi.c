#include <string.h>
#include <ethernetif.h>
#include <netif/etharp.h>
#include <lwip/dns.h>
#include <lwip/dhcp.h>
#include <bl_efuse.h>
#include <bl_wifi.h>
#include <aos/kernel.h>
#include "utils_log.h"
#include "bl_defs.h"
#include "bl_tx.h"
#include "bl_msg_tx.h"
#include "os_hal.h"
#include "wifi_mgmr.h"
#include "wifi_mgmr_api.h"
#include "bl60x_fw_api.h"

#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
#include <netbus_utils.h>
#endif

#define WIFI_MTU_SIZE 1514

#if 0
#define NET_TRACE
#define ETH_RX_DUMP
#define ETH_TX_DUMP
#endif
#define MAX_ADDR_LEN    6

#ifdef NET_TRACE
#define NET_DEBUG         os_printf
#else
#define NET_DEBUG(...)
#endif
/**
 ****************************************************************************************
 *
 * @file wifi.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */


extern int bl_main_rtthread_start(struct bl_hw **bl_hw);

struct net_device
{
    struct bl_hw *bl_hw;
};

static struct net_device bl606a0_sta;

#if 1
/* ethernet device interface */
/* Transmit packet. */
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
err_t wifi_tx(struct netif *netif, struct pbuf* p)
#else
static err_t wifi_tx(struct netif *netif, struct pbuf* p)
#endif
{
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
    bool can_tx = false;
    bool from_local = false;
    if ((uintptr_t)p & 1) {
        can_tx = true;
    } else {
#if 0
        from_local = true;
        if (npf_is_arp(p)) {
            can_tx = true;
        }
        if (npf_is_dhcp(p)) {
            can_tx = true;
        }
#endif
    }
    if (!can_tx) {
        log_warn("[wifi tx] can not tx %p\r\n", p); 
        return ERR_IF;
    }
    p = (struct pbuf *)((uintptr_t)p & ~1);
#endif
    struct wlan_netif *wlan;
#if 0
    struct net_device * bl606a0_sta = (struct net_device *)dev;
#endif
    struct pbuf* q;
    static uint32_t ticks;
#define WARNING_LIMIT_TICKS_TX_SIZE  2000
#ifdef ETH_TX_DUMP
    int dump_count = 0;
    uint8_t * dump_ptr;
    int dump_i;
#endif

    if (p->tot_len > WIFI_MTU_SIZE) {
        if (os_tick_get() - ticks > WARNING_LIMIT_TICKS_TX_SIZE) {
            os_printf("[TX] %s, TX size too big: %u bytes\r\n", __func__, p->tot_len);
            ticks = os_tick_get();
        }
        return ERR_IF;
    }

#ifdef ETH_TX_DUMP
    NET_DEBUG("tx_dump, size:%d\r\n", p->tot_len);
#endif
    for (q = p; q != NULL; q = q->next)
    {
#ifdef ETH_RX_DUMP
        dump_ptr = q->payload;
        for(dump_i=0; dump_i<q->len; dump_i++)
        {
            NET_DEBUG("%02x ", *dump_ptr);
            if( ((dump_count+1)%8) == 0 )
            {
                NET_DEBUG("  ");
            }
            if( ((dump_count+1)%16) == 0 )
            {
                NET_DEBUG("\r\n");
            }
            dump_count++;
            dump_ptr++;
        }
#endif
    }
#ifdef ETH_RX_DUMP
    NET_DEBUG("\r\n");
#endif
    wlan = container_of(netif, struct wlan_netif, netif);
#ifdef GLOBAL_NETBUS_WIFI_APP_ENABLE
    return bl_output(bl606a0_sta.bl_hw, netif, p, 0 == wlan->mode, from_local);
#else
    return bl_output(bl606a0_sta.bl_hw, netif, p, 0 == wlan->mode);
#endif
}
#endif

static void netif_status_callback(struct netif *netif)
{
    struct dhcp *dhcp = netif_dhcp_data(netif);
    long long time_now, ip_lease_time = 0;

    os_printf("[lwip] netif status callback\r\n"
                "  IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    os_printf("  MK: %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
    os_printf("  GW: %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif)));

    if (netif->name[0] == 'a') {
        return;
    }
    if (ip4_addr_isany(netif_ip4_addr(netif))) {
        os_printf(" SKIP Notify for set Empty Address\r\n");
    } else {
        if (dhcp != NULL && dhcp->offered_t0_lease > 0){
            ip_lease_time = ((long long)dhcp->offered_t0_lease) * 1000 * 1000;
            os_printf(" IP LEASE TIME from dhcp: 0x%llx us\r\n", ip_lease_time);
            bl60x_current_time_us(&time_now);
            ip_lease_time += time_now;
            printf("time now = 0x%llu, IP EXPIRED TIME = 0x%llu\r\n", time_now, ip_lease_time);
        }

        wifi_mgmr_api_ip_update();
        wifi_mgmr_api_ip_got(
            netif_ip4_addr(netif)->addr,
            netif_ip4_netmask(netif)->addr,
            netif_ip4_gw(netif)->addr,
            ((const ip4_addr_t*)ip_2_ip4(dns_getserver(0)))->addr,
            ((const ip4_addr_t*)ip_2_ip4(dns_getserver(1)))->addr,
            ip_lease_time
        );
	}
}

err_t bl606a0_wifi_netif_init(struct netif *netif)
{
    netif->hostname = "bl606a0";
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    /* set netif maximum transfer unit */
    netif->mtu = 1500;
    /* Accept broadcast address and ARP traffic */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    netif->output = etharp_output;
    netif->linkoutput = wifi_tx;
    netif_set_status_callback(netif, netif_status_callback);

    return 0;
}

int bl606a0_wifi_init(wifi_conf_t *conf)
{
    uint8_t mac[6];
    int ret;

    os_printf("\r\n\r\n[BL] Initi Wi-Fi");
    memset(mac, 0, sizeof(mac));
    bl_wifi_mac_addr_get(mac);
    os_printf(" with MAC #### %02X:%02X:%02X:%02X:%02X:%02X ####\r\n", mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]
    );
    bl_msg_update_channel_cfg(conf->country_code);
    os_printf("-----------------------------------------------------\r\n");
    bl_wifi_clock_enable();//Enable wifi clock
    memset(&bl606a0_sta, 0, sizeof(bl606a0_sta));
    ret = bl_main_rtthread_start(&(bl606a0_sta.bl_hw));

    return ret;
}
