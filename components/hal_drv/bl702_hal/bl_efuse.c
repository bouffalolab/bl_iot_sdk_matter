#include "bl_efuse.h"

int bl_efuse_read_mac(uint8_t mac[6])
{
    mac[0] = 0x00;
    mac[1] = 0x10;
    mac[2] = 0x20;
    mac[3] = 0x30;
    mac[4] = 0x40;
    mac[5] = 0x50;

    return 0;
}
