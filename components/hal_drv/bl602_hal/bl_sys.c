#include <bl602_romdriver.h>
#include "bl_sys.h"

int bl_sys_reset_por(void)
{
    RomDriver_GLB_SW_POR_Reset();

    return 0;
}
