#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <bl702.h>

#include "bl_efuse.h"
#include "bl_wireless.h"
#include "bl_irq.h"

typedef struct _bl_wireless_env {
    uint8_t mac_addr[8];
} bl_wireless_env_t;

bl_wireless_env_t wireless_env;


int bl_wireless_mac_addr_set(uint8_t mac[8])
{
    memcpy(wireless_env.mac_addr, mac, 8);
    return 0;
}

int bl_wireless_mac_addr_get(uint8_t mac[8])
{
    memcpy(mac, wireless_env.mac_addr, 8);
    return 0;
}
