#include <stdio.h>
#include <string.h>
#include <lwip/netifapi.h>
#include <lwip/dns.h>

#include "wifi_mgmr.h"
#include "wifi_netif.h"

int wifi_netif_dhcp_start(struct netif *netif)
{
    netifapi_dhcp_start(netif);
    return 0;
}


int wifi_netif_arp_add(struct netif *netif, uint32_t ip, uint8_t *mac)
{
    uint8_t IP_ADDRESS[4];

    IP_ADDRESS[0] = (ip & 0x000000FF) >> 0;
    IP_ADDRESS[1] = (ip & 0x0000FF00) >> 8;
    IP_ADDRESS[2] = (ip & 0x00FF0000) >> 16;
    IP_ADDRESS[3] = (ip & 0xFF000000) >> 24;

    IP4_ADDR(&netif->arp_table_ip, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
    memcpy(netif->arp_table_ethaddr, mac, ETH_HWADDR_LEN);
    netifapi_dynamic_arp_add(netif);

    return 0;
}

int wifi_netif_arp_request(struct netif *netif, uint32_t ip, uint32_t gw)
{
    uint8_t IP_ADDRESS[4];
    uint8_t GATEWAY_ADDRESS[4];

    IP_ADDRESS[0] = (ip & 0x000000FF) >> 0;
    IP_ADDRESS[1] = (ip & 0x0000FF00) >> 8;
    IP_ADDRESS[2] = (ip & 0x00FF0000) >> 16;
    IP_ADDRESS[3] = (ip & 0xFF000000) >> 24;
    GATEWAY_ADDRESS[0] = (gw & 0x000000FF) >> 0;
    GATEWAY_ADDRESS[1] = (gw & 0x0000FF00) >> 8;
    GATEWAY_ADDRESS[2] = (gw & 0x00FF0000) >> 16;
    GATEWAY_ADDRESS[3] = (gw & 0xFF000000) >> 24;

    IP4_ADDR(&netif->arp_ip_addr, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
    IP4_ADDR(&netif->arp_gw, GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3]);
    netifapi_dhcp_arp_request(netif);
    return 0;
}


int wifi_netif_network_ip_set(struct netif *netif, struct wifi_netif_ip_info *ip_info, uint32_t timeout)
{
    uint8_t IP_ADDRESS[4];
    uint8_t NETMASK_ADDRESS[4];
    uint8_t GATEWAY_ADDRESS[4];
    uint8_t DNS1_ADDRESS[4];
    uint8_t DNS2_ADDRESS[4];

    if (!ip_info->ip || !ip_info->mask || !ip_info->gw){
        printf("ip is invalid\r\n");
        return -1;
    }

    IP_ADDRESS[0] = (ip_info->ip & 0x000000FF) >> 0;
    IP_ADDRESS[1] = (ip_info->ip & 0x0000FF00) >> 8;
    IP_ADDRESS[2] = (ip_info->ip & 0x00FF0000) >> 16;
    IP_ADDRESS[3] = (ip_info->ip & 0xFF000000) >> 24;
    NETMASK_ADDRESS[0] = (ip_info->mask & 0x000000FF) >> 0;
    NETMASK_ADDRESS[1] = (ip_info->mask & 0x0000FF00) >> 8;
    NETMASK_ADDRESS[2] = (ip_info->mask & 0x00FF0000) >> 16;
    NETMASK_ADDRESS[3] = (ip_info->mask & 0xFF000000) >> 24;
    GATEWAY_ADDRESS[0] = (ip_info->gw & 0x000000FF) >> 0;
    GATEWAY_ADDRESS[1] = (ip_info->gw & 0x0000FF00) >> 8;
    GATEWAY_ADDRESS[2] = (ip_info->gw & 0x00FF0000) >> 16;
    GATEWAY_ADDRESS[3] = (ip_info->gw & 0xFF000000) >> 24;
    DNS1_ADDRESS[0] = (ip_info->dns1 & 0x000000FF) >> 0;
    DNS1_ADDRESS[1] = (ip_info->dns1 & 0x0000FF00) >> 8;
    DNS1_ADDRESS[2] = (ip_info->dns1 & 0x00FF0000) >> 16;
    DNS1_ADDRESS[3] = (ip_info->dns1 & 0xFF000000) >> 24;
    DNS2_ADDRESS[0] = (ip_info->dns2 & 0x000000FF) >> 0;
    DNS2_ADDRESS[1] = (ip_info->dns2 & 0x0000FF00) >> 8;
    DNS2_ADDRESS[2] = (ip_info->dns2 & 0x00FF0000) >> 16;
    DNS2_ADDRESS[3] = (ip_info->dns2 & 0xFF000000) >> 24;
    printf("wifi_netif_network_ip_set IP:%u.%u.%u.%u, "
           "MASK: %u.%u.%u.%u, "
           "Gateway: %u.%u.%u.%u, "
           "DNS1: %u.%u.%u.%u, "
           "DNS2: %u.%u.%u.%u \r\n",
           IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3],
           NETMASK_ADDRESS[0], NETMASK_ADDRESS[1] , NETMASK_ADDRESS[2], NETMASK_ADDRESS[3],
           GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3],
           DNS1_ADDRESS[0], DNS1_ADDRESS[1], DNS1_ADDRESS[2], DNS1_ADDRESS[3],
           DNS2_ADDRESS[0], DNS2_ADDRESS[1], DNS2_ADDRESS[2], DNS2_ADDRESS[3]
    );

    IP4_ADDR(&netif->arp_ip_addr, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
    IP4_ADDR(&netif->arp_netmask, NETMASK_ADDRESS[0], NETMASK_ADDRESS[1] , NETMASK_ADDRESS[2], NETMASK_ADDRESS[3]);
    IP4_ADDR(&netif->arp_gw, GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3]);
    IP4_ADDR(&netif->dns1, DNS1_ADDRESS[0], DNS1_ADDRESS[1], DNS1_ADDRESS[2], DNS1_ADDRESS[3]);
    IP4_ADDR(&netif->dns2, DNS2_ADDRESS[0], DNS2_ADDRESS[1], DNS2_ADDRESS[2], DNS2_ADDRESS[3]);

    if (0 == ip_info->dhcp_use){
        netif_set_addr(netif, &netif->arp_ip_addr, &netif->arp_netmask, &netif->arp_gw);
        dns_setserver(0, &netif->dns1);
        dns_setserver(1, &netif->dns2);
        return 0;
    }

    netif->timeout = timeout;
    if (!netif->timeout){
        netif_set_addr(netif, &netif->arp_ip_addr, &netif->arp_netmask, &netif->arp_gw);
        dns_setserver(0, &netif->dns1);
        dns_setserver(1, &netif->dns2);
        netifapi_dhcp_start(netif);
    } else {
        netifapi_dhcp_arp_check(netif);
    }

    return 0;
}

