#include <stdio.h>
#include <bl60x_glb.h>
#include <bl60x_hbn.h>
#include <bl60x_aon.h>
#include "bl_sys.h"
//FIXME any idea about implict puts?
extern int puts(const char *s);

int bl_sys_reset_por(void)
{
    __disable_irq();
    GLB_SW_POR_Reset();
    return 0;
}

int bl_sys_mem_info(char *buffer, int total)
{
    extern uint32_t _info_ram_itcm_total;
    extern uint32_t _info_ram_itcm_used;
    extern uint32_t _info_ram_itcm_left;
    extern uint32_t _info_ram_dtcm_total;
    extern uint32_t _info_ram_dtcm_used;
    extern uint32_t _info_ram_dtcm_left;
    extern uint32_t _info_ram_ocram_total;
    extern uint32_t _info_ram_ocram_used;
    extern uint32_t _info_ram_ocram_left;
    extern uint32_t _info_ram_wifi_total;
    extern uint32_t _info_ram_wifi_used;
    extern uint32_t _info_ram_wifi_left;
#if 0
#ifndef CONF_USER_ENABLE_PSRAM
    extern uint32_t _info_ram_cache_total;
    extern uint32_t _info_ram_cache_used;
    extern uint32_t _info_ram_cache_left;
#endif
#endif

    sprintf(buffer, "ITCM   :size %dKBytes, used %dKBytes, left:%dKBytes\r\n",
            ((int)&_info_ram_itcm_total) >> 10,
            ((int)&_info_ram_itcm_used) >> 10,
            ((int)&_info_ram_itcm_left) >> 10
    );
    puts(buffer);
    sprintf(buffer, "DTCM   :size %dKBytes, used %dKBytes, left:%dKBytes\r\n",
            ((int)&_info_ram_dtcm_total) >> 10,
            ((int)&_info_ram_dtcm_used) >> 10,
            ((int)&_info_ram_dtcm_left) >> 10
    );
    puts(buffer);
    sprintf(buffer, "OCRAM  :size %dKBytes, used %dKBytes, left:%dKBytes\r\n",
            ((int)&_info_ram_ocram_total) >> 10,
            ((int)&_info_ram_ocram_used) >> 10,
            ((int)&_info_ram_ocram_left) >> 10
    );
    puts(buffer);
    sprintf(buffer, "WIFI   :size %dKBytes, used %dKBytes, left:%dKBytes\r\n",
            ((int)&_info_ram_wifi_total) >> 10,
            ((int)&_info_ram_wifi_used) >> 10,
            ((int)&_info_ram_wifi_left) >> 10
    );
    puts(buffer);
#if 0
#ifndef CONF_USER_ENABLE_PSRAM
    sprintf(buffer, "CACHE  :size %dKBytes, used %dKBytes, left:%dKBytes\r\n",
            ((int)&_info_ram_cache_total) >> 10,
            ((int)&_info_ram_cache_used) >> 10,
            ((int)&_info_ram_cache_left) >> 10
    );
    puts(buffer);
#endif
#endif
    sprintf(buffer, "TOTAL  :size %dKBytes, used %dKBytes, left:%dKBytes\r\n",
            (((int)&_info_ram_itcm_total) + ((int)&_info_ram_dtcm_total) + ((int)&_info_ram_ocram_total) + + ((int)&_info_ram_wifi_total)) >> 10,
            (((int)&_info_ram_itcm_used) + ((int)&_info_ram_dtcm_used) + ((int)&_info_ram_ocram_used) + + ((int)&_info_ram_wifi_used)) >> 10,
            (((int)&_info_ram_itcm_left) + ((int)&_info_ram_dtcm_left) + ((int)&_info_ram_ocram_left) + + ((int)&_info_ram_wifi_left)) >> 10
    );
    puts(buffer);

    return 0;
}

int bl_sys_hbn(int source)
{
    switch (source) {
        case BL_SYS_HBN_SRC_GPIO_ID_10:
        /*Use GPIO10 as the wakeup source*/
        {
            printf("[HBN] Tring to HBN now\r\n");
            HBN_32K_Sel(HBN_32K_RC);
            AON_Power_Off_Xtal_32K();
            HBN_Clear_IRQ(HBN_INT_GPIO7); 
            HBN_Clear_IRQ(HBN_INT_GPIO8); 
            HBN_Clear_IRQ(HBN_INT_GPIO9); 
            HBN_Clear_IRQ(HBN_INT_GPIO10);
            HBN_Clear_IRQ(HBN_INT_GPIO29);
            HBN_Clear_IRQ(HBN_INT_GPIO30);
            HBN_Clear_IRQ(HBN_INT_GPIO31);
            HBN_Clear_IRQ(HBN_INT_GPIO32);
        }
        break;
        default:
        {
            /*nothing here*/
        }
    }
    return 0;
}

int bl_cpu0_ipc_enable(void)
{
    NVIC_EnableIRQ((IRQn_Type)48);
    NVIC_SetPriority((IRQn_Type)48, 7);
    return 0;
}

void bl_sys_cpu1_cache_enable(void)
{
BL_Err_Type SFlash_Cache16K_Enable_Set(uint8_t wayDisable);
    SFlash_Cache16K_Enable_Set(0);
}
