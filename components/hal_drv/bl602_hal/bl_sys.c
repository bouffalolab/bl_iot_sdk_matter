#include <bl602_romdriver.h>
#include <bl602_glb.h>

#include <stdio.h>
#include "bl_sys.h"

int bl_sys_reset_por(void)
{
    RomDriver_GLB_SW_POR_Reset();

    return 0;
}

int bl_sys_isxipaddr(uint32_t addr)
{
    if (((addr & 0xFF000000) == 0x23000000) || ((addr & 0xFF000000) == 0x43000000)) {
        return 1;
    }
    return 0;
}

int bl_sys_em_config(void)
{
    extern uint8_t __LD_CONFIG_EM_SEL;
    uint32_t em_size;

    em_size = (uint32_t)&__LD_CONFIG_EM_SEL;

    switch (em_size) {
        case 0 * 1024:
        {
            GLB_Set_EM_Sel(GLB_EM_0KB);
        }
        break;
        case 8 * 1024:
        {
            GLB_Set_EM_Sel(GLB_EM_8KB);
        }
        break;
        case 16 * 1024:
        {
            GLB_Set_EM_Sel(GLB_EM_16KB);
        }
        break;
        default:
        {
            /*nothing here*/
        }
    }

    return 0;
}

int bl_sys_early_init(void)
{
    /*debuger may NOT ready don't print anything*/
    
    return 0;
}

int bl_sys_init(void)
{
    bl_sys_em_config();

    return 0;
}
