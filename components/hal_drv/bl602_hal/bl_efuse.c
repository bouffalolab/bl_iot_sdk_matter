#include <bl602_ef_ctrl.h>
#include "bl_efuse.h"

int bl_efuse_read_mac(uint8_t mac[6])
{
    EF_Ctrl_Read_MAC_Address(mac);
    return 0;
}

int bl_efuse_read_mac_factory(uint8_t mac[6])
{
    return -1;
}

int bl_efuse_read_capcode(uint8_t *capcode)
{
    return -1;
}

int bl_efuse_read_pwroft(int8_t poweroffset[14])
{
    return -1;
}

