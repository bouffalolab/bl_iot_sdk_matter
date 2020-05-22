#include "bl_efuse.h"

#include <bl60x_ef_ctrl.h>

int bl_efuse_read_mac(uint8_t mac[6])
{
    EF_Ctrl_Read_MAC_Address(mac);
    return 0;
}
