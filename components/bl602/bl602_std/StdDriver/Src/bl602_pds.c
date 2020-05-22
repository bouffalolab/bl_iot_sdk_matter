/**
  ******************************************************************************
  * @file    bl602_pds.c
  * @version V1.0
  * @date
  * @brief   This file is the standard driver c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2019 Bouffalo Lab</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Bouffalo Lab nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "bl602.h"
#include "bl602_pds.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  PDS
 *  @{
 */

/** @defgroup  PDS_Private_Macros
 *  @{
 */

/*@} end of group PDS_Private_Macros */

/** @defgroup  PDS_Private_Types
 *  @{
 */

/*@} end of group PDS_Private_Types */

/** @defgroup  PDS_Private_Variables
 *  @{
 */
static intCallback_Type * pdsIntCbfArra[1][1]={{NULL}};


/*@} end of group PDS_Private_Variables */

/** @defgroup  PDS_Global_Variables
 *  @{
 */

/*@} end of group PDS_Global_Variables */

/** @defgroup  PDS_Private_Fun_Declaration
 *  @{
 */

/*@} end of group PDS_Private_Fun_Declaration */

/** @defgroup  PDS_Private_Functions
 *  @{
 */

/*@} end of group PDS_Private_Functions */

/** @defgroup  PDS_Public_Functions
 *  @{
 */

/****************************************************************************//**
 * @brief  PDS software reset
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION PDS_Reset(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal = *(uint32_t *)0x40000010;
    tmpVal = tmpVal | (1<<14);
    *(uint32_t *)0x40000010 = tmpVal;
    
    tmpVal = *(uint32_t *)0x40000010;
    tmpVal = tmpVal & ~(1<<14);
    *(uint32_t *)0x40000010 = tmpVal;
}

/****************************************************************************//**
 * @brief  Enable power down sleep
 *
 * @param  cfg: power down configuration
 * @param  pdsSleepCnt: power down sleep count cycle
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION PDS_Enable(PDS_CFG_Type *cfg,uint32_t pdsSleepCnt)
{
    uint32_t *p=(uint32_t *)cfg;

    if(pdsSleepCnt-PDS_WARMUP_CNT<=0){
        return;
    }
    
    BL_WR_REG(PDS_BASE, PDS_TIME1,pdsSleepCnt-PDS_WARMUP_CNT);

    /* Set PDS control register  */
    BL_WR_REG(PDS_BASE, PDS_CTL,*p);
}

/****************************************************************************//**
 * @brief  PDS Auto mode wake up counter config
 *
 * @param  sleepDuration: sleep time, total pds = sleep_duration + max_warmup_cnt (32K clock cycles),
 *                        recommend maxWarmCnt*N+2
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION PDS_Auto_Time_Config(uint32_t sleepDuration)
{
    /* PDS_TIME1 */
    BL_WR_REG(PDS_BASE,PDS_TIME1,sleepDuration);
}

/****************************************************************************//**
 * @brief  PDS Auto mode config and enable
 *
 * @param  powerCfg: PDS Auto mode power domain config
 * @param  normalCfg: PDS Auto mode power normal config
 * @param  enable: PDS Auto mode Enable or Disable
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION PDS_Auto_Enable(PDS_AUTO_POWER_DOWN_CFG_Type *powerCfg, PDS_AUTO_NORMAL_CFG_Type *normalCfg, BL_Fun_Type enable)
{
    uint32_t pdsCtl = 0;
    
    CHECK_PARAM(IS_PDS_LDO_VOLTAGE_TYPE(normalCfg->vddcoreVol));
    
    /* power config */
    pdsCtl |= (powerCfg->mbgPower      << 31)|
              (powerCfg->ldo18rfPower  << 30)|
              (powerCfg->sfregPower    << 29)|
              (powerCfg->pllPower      << 28)|
              (powerCfg->cpu0Power     << 19)|
              (powerCfg->rc32mPower    << 17)|
              (powerCfg->xtalPower     << 14)|
              (powerCfg->allPower      << 13)|
              (powerCfg->isoPower      << 11)|
              (powerCfg->bzPower       << 10)|
              (powerCfg->sramDisStanby << 9 )|
              (powerCfg->cgPower       << 8 )|
              (powerCfg->cpu1Power     << 7 )|
              (powerCfg->usbPower      << 3 );
    pdsCtl = BL_SET_REG_BITS_VAL(pdsCtl,PDS_CR_PDS_LDO_VOL,normalCfg->vddcoreVol);
    pdsCtl |= (normalCfg->vddcoreVolEn        << 18)|
              (normalCfg->cpu0NotNeedWFI      << 21)|
              (normalCfg->cpu1NotNeedWFI      << 20)|
              (normalCfg->busReset            << 16)|
              (normalCfg->disIrqWakeUp        << 15)|
              (normalCfg->powerOffXtalForever << 2 )|
              (normalCfg->sleepForever        << 1 );
    BL_WR_REG(PDS_BASE,PDS_CTL,pdsCtl);
    
    
    pdsCtl = BL_RD_REG(PDS_BASE, PDS_CTL);
    if(enable){
        pdsCtl |= (1<<0);
    }else{
        pdsCtl &= ~(1<<0);
    }
    BL_WR_REG(PDS_BASE,PDS_CTL,pdsCtl);
}

/****************************************************************************//**
 * @brief  Enable module power down sleep
 *
 * @param  cfg: module power down configuration
 *
 * @return None
 *
*******************************************************************************/
void PDS_Manual_Enable(PDS_MODULE_CFG_Type *cfg)
{
    uint32_t *p=(uint32_t *)cfg;
    
    /* Set PDS control register  */
    BL_WR_REG(PDS_BASE, PDS_CTL2,*p);
}

/****************************************************************************//**
 * @brief  PDS force turn off XXX domain
 *
 * @param  domain: PDS domain
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION PDS_Manual_Force_Turn_Off(PDS_FORCE_Type domain)
{
    uint32_t tmpVal = 0;
    
    /* memory sleep */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal |= 1<<(domain + PDS_FORCE_MEM_STBY_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* gate clock */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal |= 1<<(domain + PDS_FORCE_GATE_CLK_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* pds reset */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal |= 1<<(domain + PDS_FORCE_PDS_RST_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* isolation on */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal |= 1<<(domain + PDS_FORCE_ISO_EN_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* power off */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal |= 1<<(domain + PDS_FORCE_PWR_OFF_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
}

/****************************************************************************//**
 * @brief  PDS force turn on XXX domain
 *
 * @param  domain: PDS domain
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION PDS_Manual_Force_Turn_On(PDS_FORCE_Type domain)
{
    uint32_t tmpVal = 0;
    
    /* power on */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal &= ~1<<(domain + PDS_FORCE_PWR_OFF_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* isolation off */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal &= ~1<<(domain + PDS_FORCE_ISO_EN_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* pds de_reset */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal &= ~1<<(domain + PDS_FORCE_PDS_RST_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* memory active */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal &= ~1<<(domain + PDS_FORCE_MEM_STBY_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
    
    /* clock on */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CTL2);
    tmpVal &= ~1<<(domain + PDS_FORCE_GATE_CLK_OFFSET);
    BL_WR_REG(PDS_BASE, PDS_CTL2, tmpVal);
}

/****************************************************************************//**
 * @brief  Install PDS interrupt callback function
 *
 * @param  cbFun: cbFun: Pointer to interrupt callback function. The type should be void (*fn)(void)
 *
 * @return None
 *
*******************************************************************************/
void /*ATTR_TCM_SECTION*/ PDS_Int_Callback_Install(intCallback_Type* cbFun)
{
    pdsIntCbfArra[0][0] = cbFun;
}

/****************************************************************************//**
 * @brief  Power down sleep wake up interrupt handler
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
#ifndef BL602_USE_HAL_DRIVER
void PDS_WAKEUP_IRQHandler(void)
{
    uint32_t tmpVal;

    /* Read PDS control register  */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_INT);
    
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CR_PDS_INT_CLR);
    BL_WR_REG(PDS_BASE, PDS_INT,tmpVal);
    
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CR_PDS_INT_CLR);
    BL_WR_REG(PDS_BASE, PDS_INT,tmpVal);
    
    if(pdsIntCbfArra[0][0] != NULL) {
        /* call the callback function */
        pdsIntCbfArra[0][0]();
    }
}
#endif

/****************************************************************************//**
 * @brief  Trim RC32M
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Trim_RC32M(void)
{
    Efuse_Ana_RC32M_Trim_Type trim;
    int32_t tmpVal = 0;
    
    EF_Ctrl_Read_RC32M_Trim(&trim);
    if(trim.trimRc32mExtCodeEn){
        if(trim.trimRc32mCodeFrExtParity==EF_Ctrl_Get_Trim_Parity(trim.trimRc32mCodeFrExt,8)){
            tmpVal=BL_RD_REG(PDS_BASE,PDS_RC32M_CTRL0);
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_RC32M_CODE_FR_EXT,trim.trimRc32mCodeFrExt);
            tmpVal=BL_SET_REG_BIT(tmpVal,PDS_RC32M_EXT_CODE_EN);
            BL_WR_REG(PDS_BASE,PDS_RC32M_CTRL0,tmpVal);
            BL602_Delay_US(2);
            return SUCCESS;
        }
    }
    
    return ERROR;
}

/****************************************************************************//**
 * @brief  Select RC32M as PLL ref source
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Select_RC32M_As_PLL_Ref(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal = BL_RD_REG(PDS_BASE,PDS_CLKPLL_TOP_CTRL);
    tmpVal = BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_XTAL_RC32M_SEL);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_TOP_CTRL,tmpVal);
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Select XTAL as PLL ref source
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Select_XTAL_As_PLL_Ref(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal = BL_RD_REG(PDS_BASE,PDS_CLKPLL_TOP_CTRL);
    tmpVal = BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_REFCLK_SEL);
    tmpVal = BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_XTAL_RC32M_SEL);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_TOP_CTRL,tmpVal);
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Power on PLL
 *
 * @param  xtalType: xtal type
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Power_On_PLL(PDS_PLL_XTAL_Type xtalType)
{
    uint32_t tmpVal = 0;
    
    /* Check parameter*/
    CHECK_PARAM(IS_PDS_PLL_XTAL_TYPE(xtalType));
    
    /**************************/
    /* select PLL XTAL source */
    /**************************/
    
    if((xtalType==PDS_PLL_XTAL_RC32M)||(xtalType==PDS_PLL_XTAL_NONE)){
        PDS_Trim_RC32M();
        PDS_Select_RC32M_As_PLL_Ref();
    }else{
        PDS_Select_XTAL_As_PLL_Ref();
    }
    
    /*******************************************/
    /* PLL power down first, not indispensable */
    /*******************************************/
    /* power off PLL first, this step is not indispensable */
    PDS_Power_Off_PLL();
    
    /********************/
    /* PLL param config */
    /********************/
    
    /* clkpll_icp_1u */
    /* clkpll_icp_5u */
    /* clkpll_int_frac_sw */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_CP);
    if(xtalType==PDS_PLL_XTAL_26M){
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_ICP_1U,1);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_ICP_5U,0);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_INT_FRAC_SW,1);
    }else{
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_ICP_1U,0);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_ICP_5U,2);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_INT_FRAC_SW,0);
    }
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_CP,tmpVal);
    
    /* clkpll_c3 */
    /* clkpll_cz */
    /* clkpll_rz */
    /* clkpll_r4 */
    /* clkpll_r4_short */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_RZ);
    if(xtalType==PDS_PLL_XTAL_26M){
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_C3,2);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_CZ,2);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_RZ,5);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_R4_SHORT,0);
    }else{
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_C3,3);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_CZ,1);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_RZ,1);
        tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_R4_SHORT,1);
    }
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_R4,2);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_RZ,tmpVal);
    
    /* clkpll_refdiv_ratio */
    /* clkpll_postdiv */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_TOP_CTRL);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_POSTDIV,0x14);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_REFDIV_RATIO,2);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_TOP_CTRL,tmpVal);
    
    /* clkpll_sdmin */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_SDM);
    switch(xtalType){
        case PDS_PLL_XTAL_NONE:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x3C0000);
            break;
        case PDS_PLL_XTAL_24M:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x500000);
            break;
        case PDS_PLL_XTAL_32M:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x3C0000);
            break;
        case PDS_PLL_XTAL_38P4M:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x320000);
            break;
        case PDS_PLL_XTAL_40M:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x300000);
            break;
        case PDS_PLL_XTAL_26M:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x49D39D);
            break;
        case PDS_PLL_XTAL_RC32M:
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x3C0000);
            break;
        default :
            tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SDMIN,0x3C0000);
            break;
    }
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_SDM,tmpVal);
    
    /* clkpll_sel_fb_clk */
    /* clkpll_sel_sample_clk can be 0/1, default is 1 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_FBDV);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SEL_FB_CLK,1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,PDS_CLKPLL_SEL_SAMPLE_CLK,1);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_FBDV,tmpVal);
    
    
    /*************************/
    /* PLL power up sequence */
    /*************************/
    
    /* pu_clkpll_sfreg=1 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_PU_CLKPLL_SFREG);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    
    BL602_Delay_US(5);
    
    /* pu_clkpll=1 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_PU_CLKPLL);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    
    /* clkpll_pu_cp=1 */
    /* clkpll_pu_pfd=1 */
    /* clkpll_pu_fbdv=1 */
    /* clkpll_pu_postdiv=1 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_PU_CP);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_PU_PFD);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_PU_FBDV);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_PU_POSTDIV);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    
    BL602_Delay_US(5);
    
    /* clkpll_sdm_reset=1 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_SDM_RESET);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    BL602_Delay_US(1);
    /* clkpll_reset_fbdv=1 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_RESET_FBDV);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    BL602_Delay_US(2);
    /* clkpll_reset_fbdv=0 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_RESET_FBDV);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    BL602_Delay_US(1);
    /* clkpll_sdm_reset=0 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_SDM_RESET);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    
    return SUCCESS;
}
/** PLL output config **/
/*
[8]    1'h0    r/w    clkpll_en_32m
[7]    1'h0    r/w    clkpll_en_48m
[6]    1'h0    r/w    clkpll_en_80m
[5]    1'h0    r/w    clkpll_en_96m
[4]    1'h0    r/w    clkpll_en_120m
[3]    1'h0    r/w    clkpll_en_160m
[2]    1'h0    r/w    clkpll_en_192m
[1]    1'h0    r/w    clkpll_en_240m
[0]    1'h0    r/w    clkpll_en_480m
*/

/****************************************************************************//**
 * @brief  Enable all PLL clock
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Enable_PLL_All_Clks(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN);
    tmpVal |= 0x1FF;
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN,tmpVal);
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Disable all PLL clock
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Disable_PLL_All_Clks(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN);
    tmpVal &= (~0x1FF);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN,tmpVal);
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Enable PLL clock
 *
 * @param  pllClk: PLL clock type
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Enable_PLL_Clk(PDS_PLL_CLK_Type pllClk)
{
    uint32_t tmpVal = 0;
    
    /* Check parameter*/
    CHECK_PARAM(IS_PDS_PLL_CLK_TYPE(pllClk));
    
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN);
    tmpVal |= (1<<pllClk);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN,tmpVal);
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Disable PLL clock
 *
 * @param  pllClk: PLL clock type
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Disable_PLL_Clk(PDS_PLL_CLK_Type pllClk)
{
    uint32_t tmpVal = 0;
    
    /* Check parameter*/
    CHECK_PARAM(IS_PDS_PLL_CLK_TYPE(pllClk));
    
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN);
    tmpVal &= (~(1<<pllClk));
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN,tmpVal);
    
    return SUCCESS;
}

/****************************************************************************//**
 * @brief  Power off PLL
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
__WEAK
BL_Err_Type ATTR_CLOCK_SECTION PDS_Power_Off_PLL(void)
{
    uint32_t tmpVal = 0;
    
    /* pu_clkpll_sfreg=0 */
    /* pu_clkpll=0 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_PU_CLKPLL_SFREG);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_PU_CLKPLL);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    
    /* clkpll_pu_cp=0 */
    /* clkpll_pu_pfd=0 */
    /* clkpll_pu_fbdv=0 */
    /* clkpll_pu_postdiv=0 */
    tmpVal=BL_RD_REG(PDS_BASE,PDS_PU_RST_CLKPLL);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_PU_CP);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_PU_PFD);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_PU_FBDV);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CLKPLL_PU_POSTDIV);
    BL_WR_REG(PDS_BASE,PDS_PU_RST_CLKPLL,tmpVal);
    
    return SUCCESS;
}

/*@} end of group PDS_Public_Functions */

/*@} end of group PDS */

/*@} end of group BL602_Peripheral_Driver */
