#include <bl702_romdriver.h>
#include <bl702_glb.h>

#include <stdio.h>
#include <stdbool.h>
#include "bl_sys.h"

volatile bool sys_log_all_enable = true;

int bl_sys_logall_enable(void)
{
    sys_log_all_enable = true;
    return 0;
}

int bl_sys_logall_disable(void)
{
    sys_log_all_enable = false;
    return 0;
}

int bl_sys_reset_por(void)
{
    GLB_SW_POR_Reset();

    return 0;
}

void bl_sys_reset_system(void)
{
    GLB_SW_System_Reset();
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

int bl_sys_pkg_config(void)
{
    Efuse_Device_Info_Type dev_info;
    uint8_t isInternalFlash;
    uint8_t isInternalPsram;
    uint32_t tmpVal;

    // Get device information from efuse
    EF_Ctrl_Read_Device_Info(&dev_info);

    // flash_cfg:
    // 0: external flash using SF2
    // 1: internal 0.5M flash
    // 2: internal 1M flash
    // 3: external flash using SF1
    if(dev_info.flash_cfg==1||dev_info.flash_cfg==2){
        isInternalFlash=1;
    }else{
        isInternalFlash=0;
    }

    // psram_cfg:
    // 0: no psram
    // 1: internal 2M psram
    // 2: external psram
    // 3: reserved
    if(dev_info.psram_cfg==1){
        isInternalPsram=1;
    }else{
        isInternalPsram=0;
    }

    tmpVal=BL_RD_REG(GLB_BASE,GLB_GPIO_USE_PSRAM__IO);

    if(isInternalFlash==1&&isInternalPsram==1){
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,GLB_CFG_GPIO_USE_PSRAM_IO,0x00);
    }else if(isInternalFlash==1&&isInternalPsram==0){
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,GLB_CFG_GPIO_USE_PSRAM_IO,0x3f);
    }else{
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,GLB_CFG_GPIO_USE_PSRAM_IO,0x00);
    }

    BL_WR_REG(GLB_BASE,GLB_GPIO_USE_PSRAM__IO,tmpVal);

    return 0;
}

int bl_sys_early_init(void)
{
    extern BL_Err_Type HBN_Aon_Pad_IeSmt_Cfg(uint8_t padCfg);
    HBN_Aon_Pad_IeSmt_Cfg(0x1F);

    extern void freertos_risc_v_trap_handler(void); //freertos_riscv_ram/portable/GCC/RISC-V/portASM.S
    write_csr(mtvec, &freertos_risc_v_trap_handler);

    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_UART0);
    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_UART1);
    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_TMR);

    bl_sys_pkg_config();

    /*debuger may NOT ready don't print anything*/
    return 0;
}

int bl_sys_init(void)
{
    bl_sys_em_config();

    return 0;
}
