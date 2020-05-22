#include "bl_sys.h"

void hal_reboot()
{
    bl_sys_reset_por();
}

void hal_poweroff()
{
    bl_sys_hbn(BL_SYS_HBN_SRC_GPIO_ID_10);
}
