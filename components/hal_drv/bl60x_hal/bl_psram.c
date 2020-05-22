#include <bl60x_glb.h>
#include <bl60x_psram_ctrl.h>
#include "bl_psram.h"

/**
 ****************************************************************************************
 *
 * @file bl_psram.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */
int bl_psram_init()
{
    PSRAM_WinbX8_Cfg_Type psramCfg={
        .latency=0x0,
        .drvSt=2,
        .burstLen=1,
        .fixLatency=0,
        .dpdDisable=1,
        .parRefresh=0,
        .hybirdSleep=0,
        .rsvd=0,
    };
    PSRAM_Ctrl_Cfg_Type psramCtrlCfg={
        .vendor=PSRAM_CTRL_VENDOR_WINBOND,
        .ioMode=PSRAM_CTRL_X8_MODE,
        .delayDqs=0x0c,
    };
  
    *(uint32_t*)0x40000060 = 0x00206F00;

    GLB_Set_PSRAM_CLK(GLB_PSRAM_CLK_160M, 0);
    PSram_Ctrl_Init(&psramCtrlCfg);
    //*(uint32_t*)0x40015010 = 0xFCFC;
#if 0
    PSram_Ctrl_Init(PSRAM_CTRL_X8_MODE,&delayCfg);
#endif
    PSram_Ctrl_WinbX8_Cfg_Set(&psramCfg);

#if 1
#if 0
    PSram_Cache_Enable(0x00);
#else
    *(uint32_t*)0x40000060 = 0x0400000;
    *(uint32_t*)0x40000060 = 0x0408003 | (1 << 13);
#endif
#endif
    return 0;
}
