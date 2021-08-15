#ifndef __WIFI_NETIF_H__
#define __WIFI_NETIF_H__
typedef struct wifi_netif_ip_info {
    uint32_t ip;
    uint32_t mask;
    uint32_t gw;
    uint32_t dns1;
    uint32_t dns2;
    uint8_t  dhcp_use;
    uint8_t gw_mac[6];
} wifi_netif_ip_info_t;

int wifi_netif_dhcp_start(struct netif *netif);
int wifi_netif_network_ip_set(struct netif *netif, struct wifi_netif_ip_info *ip_info, uint32_t timeout);
int wifi_netif_arp_add(struct netif *netif, uint32_t ip, uint8_t *mac);
int wifi_netif_arp_request(struct netif *netif, uint32_t ip, uint32_t gw);
#endif
