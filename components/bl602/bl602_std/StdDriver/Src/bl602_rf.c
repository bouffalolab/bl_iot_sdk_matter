/**
  ******************************************************************************
  * @file    bl602_rf.c
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

#include "bl602_rf.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  RF
 *  @{
 */

/** @defgroup  RF_Private_Macros
 *  @{
 */

/*@} end of group RF_Private_Macros */

/** @defgroup  RF_Private_Types
 *  @{
 */

/*@} end of group RF_Private_Types */

/** @defgroup  RF_Private_Variables
 *  @{
 */
/* Unused XTAL config can be removed to reduce code size */
static const ATTR_CLOCK_CONST_SECTION RF_PLL_Cfg_Type pllCfgList[]={
	{
		/* PLL_XTAL_24M */
		.clkpllIcp1u = 0,
		.clkpllIcp5u = 2,
		.clkpllIntFracSw = 0,
		.clkpllC3 = 3,
		.clkpllCz = 1,
		.clkpllRz = 1,
		.clkpllR4 = 2,
		.clkpllR4Short = 1,
		.clkpllRefdivRatio = 2,
		.clkpllPostdiv = 0x14,
		.clkpllSdmin = 0x500000,
		.clkpllSelFbClk = 1,
		.clkpllSelSampleClk = 1
	},
	{
		/* PLL_XTAL_32M */
		.clkpllIcp1u = 0,
		.clkpllIcp5u = 2,
		.clkpllIntFracSw = 0,
		.clkpllC3 = 3,
		.clkpllCz = 1,
		.clkpllRz = 1,
		.clkpllR4 = 2,
		.clkpllR4Short = 1,
		.clkpllRefdivRatio = 2,
		.clkpllPostdiv = 0x14,
		.clkpllSdmin = 0x3C0000,
		.clkpllSelFbClk = 1,
		.clkpllSelSampleClk = 0
	},
	{
		/* PLL_XTAL_38P4M */
		.clkpllIcp1u = 0,
		.clkpllIcp5u = 2,
		.clkpllIntFracSw = 0,
		.clkpllC3 = 3,
		.clkpllCz = 1,
		.clkpllRz = 1,
		.clkpllR4 = 2,
		.clkpllR4Short = 1,
		.clkpllRefdivRatio = 2,
		.clkpllPostdiv = 0x14,
		.clkpllSdmin = 0x240000,
		.clkpllSelFbClk = 1,
		.clkpllSelSampleClk = 0
	},
	{
		/* PLL_XTAL_40M */
		.clkpllIcp1u = 0,
		.clkpllIcp5u = 2,
		.clkpllIntFracSw = 0,
		.clkpllC3 = 3,
		.clkpllCz = 1,
		.clkpllRz = 1,
		.clkpllR4 = 2,
		.clkpllR4Short = 1,
		.clkpllRefdivRatio = 2,
		.clkpllPostdiv = 0x14,
		.clkpllSdmin = 0x280000,
		.clkpllSelFbClk = 1,
		.clkpllSelSampleClk = 0
	},
	{
		/* PLL_XTAL_RC32M */
		.clkpllIcp1u = 0,
		.clkpllIcp5u = 2,
		.clkpllIntFracSw = 0,
		.clkpllC3 = 3,
		.clkpllCz = 1,
		.clkpllRz = 1,
		.clkpllR4 = 2,
		.clkpllR4Short = 1,
		.clkpllRefdivRatio = 2,
		.clkpllPostdiv = 0x14,
		.clkpllSdmin = 0x3C0000,
		.clkpllSelFbClk = 1,
		.clkpllSelSampleClk = 0
	}
};


/*@} end of group RF_Private_Variables */

/** @defgroup  RF_Global_Variables
 *  @{
 */

/*@} end of group RF_Global_Variables */

/** @defgroup  RF_Private_Fun_Declaration
 *  @{
 */

/*@} end of group RF_Private_Fun_Declaration */

/** @defgroup  RF_Private_Functions
 *  @{
 */

/*@} end of group RF_Private_Functions */

/** @defgroup  RF_Public_Functions
 *  @{
 */

/****************************************************************************//**
 * @brief  Select RC32M as PLL ref source
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type ATTR_CLOCK_SECTION RF_Select_RC32M_As_PLL_Ref(void)
{
    uint32_t tmpVal = 0;
	
	tmpVal = BL_RD_REG(RF_BASE,RF_CLKPLL_TOP_CTRL);
	tmpVal = BL_SET_REG_BIT(tmpVal,RF_CLKPLL_XTAL_RC32M_SEL);
	BL_WR_REG(GLB_BASE,RF_CLKPLL_TOP_CTRL,tmpVal);
	
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
BL_Err_Type ATTR_CLOCK_SECTION RF_Select_XTAL_As_PLL_Ref(void)
{
    uint32_t tmpVal = 0;
	
	tmpVal = BL_RD_REG(RF_BASE,RF_CLKPLL_TOP_CTRL);
	tmpVal = BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_XTAL_RC32M_SEL);
	BL_WR_REG(GLB_BASE,RF_CLKPLL_TOP_CTRL,tmpVal);
	
	return SUCCESS;
}

/****************************************************************************//**
 * @brief  Power on SF register
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type ATTR_CLOCK_SECTION RF_Power_On_SFReg(void)
{
    uint32_t tmpVal = 0;
    
    /* Power up RF SF Reg */
    tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_SFREG);
    BL_WR_REG(RF_BASE,RF_PUCR1,tmpVal);
    
    tmpVal=BL_RD_REG(PDS_BASE,PDS_CTL);
    tmpVal=BL_CLR_REG_BIT(tmpVal,PDS_CR_PDS_PU_SFREG_DIS);
    BL_WR_REG(PDS_BASE,PDS_CTL,tmpVal);
    
    /* Delay >10us */
    BL602_Delay_US(10);
	
	return SUCCESS;
}

/****************************************************************************//**
 * @brief  Power off SF register
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type ATTR_CLOCK_SECTION RF_Power_Off_SFReg(void)
{
    uint32_t tmpVal = 0;

    /* Power off RF SF Reg */
    tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
    tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_SFREG);
    BL_WR_REG(RF_BASE,RF_PUCR1,tmpVal);
	
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
BL_Err_Type ATTR_CLOCK_SECTION RF_Power_On_PLL(RF_PLL_XTAL_Type xtalType)
{
    uint32_t tmpVal = 0;
    const RF_PLL_Cfg_Type *pCfg=&pllCfgList[xtalType];
	
    /* Check parameter*/
    CHECK_PARAM(IS_RF_PLL_XTAL_TYPE(xtalType));
    
	/**************************/
	/* select PLL XTAL source */
	/**************************/
	
    if(xtalType>=PLL_XTAL_RC32M){
        GLB_Trim_RC32M();
        RF_Select_RC32M_As_PLL_Ref();
    }else{
        RF_Select_XTAL_As_PLL_Ref();
    }
	
	/*******************************************/
	/* PLL power down first, not indispensable */
	/*******************************************/
	/* power off PLL first, this step is not indispensable */
	RF_Power_Off_PLL();
	
	/********************/
	/* PLL param config */
	/********************/
	
	/* clkpll_icp_1u */
	/* clkpll_icp_5u */
	/* clkpll_int_frac_sw */
	tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_CP);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_ICP_1U,pCfg->clkpllIcp1u);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_ICP_5U,pCfg->clkpllIcp5u);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_INT_FRAC_SW,pCfg->clkpllIntFracSw);
	BL_WR_REG(PDS_BASE,RF_CLKPLL_CP,tmpVal);
	
	/* clkpll_c3 */
	/* clkpll_cz */
	/* clkpll_rz */
	/* clkpll_r4 */
	/* clkpll_r4_short */
	tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_RZ);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_C3,pCfg->clkpllC3);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_CZ,pCfg->clkpllCz);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_RZ,pCfg->clkpllRz);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_R4,pCfg->clkpllR4);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_R4_SHORT,pCfg->clkpllR4Short);
	BL_WR_REG(PDS_BASE,RF_CLKPLL_RZ,tmpVal);
	
	/* clkpll_refdiv_ratio */
	/* clkpll_postdiv */
	tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_TOP_CTRL);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_REFDIV_RATIO,pCfg->clkpllRefdivRatio);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_POSTDIV,pCfg->clkpllPostdiv);
	BL_WR_REG(PDS_BASE,RF_CLKPLL_TOP_CTRL,tmpVal);
	
	/* clkpll_sdmin */
	tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_SDM);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_SDMIN,pCfg->clkpllSdmin);
	BL_WR_REG(PDS_BASE,RF_CLKPLL_SDM,tmpVal);
	
	/* clkpll_sel_fb_clk */
	/* clkpll_sel_sample_clk */
	tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_FBDV);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_SEL_FB_CLK,pCfg->clkpllSelFbClk);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_CLKPLL_SEL_SAMPLE_CLK,pCfg->clkpllSelSampleClk);
	BL_WR_REG(PDS_BASE,RF_CLKPLL_FBDV,tmpVal);
	
	/*************************/
	/* PLL power up sequence */
	/*************************/
	
	/* pu_clkpll_sfreg=1 */
	/* pu_clkpll=1 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_CLKPLL_SFREG);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_CLKPLL);
	BL_WR_REG(PDS_BASE,RF_PUCR1,tmpVal);
	
	/* clkpll_pu_cp=1 */
	/* clkpll_pu_pfd=1 */
	/* clkpll_pu_fbdv=1 */
	/* clkpll_pu_postdiv=1 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PU_RST_CLKPLL);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_CLKPLL_PU_CP);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_CLKPLL_PU_PFD);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_CLKPLL_PU_FBDV);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_CLKPLL_PU_POSTDIV);
	BL_WR_REG(PDS_BASE,RF_PU_RST_CLKPLL,tmpVal);
	
	/* clkpll_sdm_reset=1 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PU_RST_CLKPLL);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_CLKPLL_SDM_RESET);
	BL_WR_REG(PDS_BASE,RF_PU_RST_CLKPLL,tmpVal);
	BL602_Delay_US(1);
	/* clkpll_reset_fbdv=1 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PU_RST_CLKPLL);
	tmpVal=BL_SET_REG_BIT(tmpVal,RF_CLKPLL_RESET_FBDV);
	BL_WR_REG(PDS_BASE,RF_PU_RST_CLKPLL,tmpVal);
	BL602_Delay_US(2);
	/* clkpll_reset_fbdv=0 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PU_RST_CLKPLL);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_RESET_FBDV);
	BL_WR_REG(PDS_BASE,RF_PU_RST_CLKPLL,tmpVal);
	BL602_Delay_US(1);
	/* clkpll_sdm_reset=0 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PU_RST_CLKPLL);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_SDM_RESET);
	BL_WR_REG(PDS_BASE,RF_PU_RST_CLKPLL,tmpVal);
	
	return SUCCESS;
}
/** PLL output config **/
/*
[8]	1'h0	r/w	clkpll_en_32m
[7]	1'h0	r/w	clkpll_en_48m
[6]	1'h0	r/w	clkpll_en_80m
[5]	1'h0	r/w	clkpll_en_96m
[4]	1'h0	r/w	clkpll_en_120m
[3]	1'h0	r/w	clkpll_en_160m
[2]	1'h0	r/w	clkpll_en_192m
[1]	1'h0	r/w	clkpll_en_240m
[0]	1'h0	r/w	clkpll_en_480m
*/

/****************************************************************************//**
 * @brief  Enable all PLL clock
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type ATTR_CLOCK_SECTION RF_Enable_PLL_All_Clks(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN);
    tmpVal |= 0x1FF;
    BL_WR_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN,tmpVal);
	
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
BL_Err_Type ATTR_CLOCK_SECTION RF_Disable_PLL_All_Clks(void)
{
    uint32_t tmpVal = 0;
    
    tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN);
    tmpVal &=(~0x1FF);
    BL_WR_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN,tmpVal);
	
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
BL_Err_Type ATTR_CLOCK_SECTION RF_Enable_PLL_Clk(RF_PLL_CLK_Type pllClk)
{
    uint32_t tmpVal = 0;
    
    /* Check parameter*/
    CHECK_PARAM(IS_RF_PLL_CLK_TYPE(pllClk));
    
    tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN);
    tmpVal |= (1<<pllClk);
    BL_WR_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN,tmpVal);
	
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
BL_Err_Type ATTR_CLOCK_SECTION RF_Disable_PLL_Clk(RF_PLL_CLK_Type pllClk)
{
    uint32_t tmpVal = 0;
    
    /* Check parameter*/
    CHECK_PARAM(IS_RF_PLL_CLK_TYPE(pllClk));
    
    tmpVal=BL_RD_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN);
    tmpVal &= (~(1<<pllClk));
    BL_WR_REG(RF_BASE,RF_CLKPLL_OUTPUT_EN,tmpVal);
	
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
BL_Err_Type ATTR_CLOCK_SECTION RF_Power_Off_PLL(void)
{
    uint32_t tmpVal = 0;
    
	/* pu_clkpll_sfreg=0 */
	/* pu_clkpll=0 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_CLKPLL_SFREG);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_CLKPLL);
	BL_WR_REG(PDS_BASE,RF_PUCR1,tmpVal);
	
	/* clkpll_pu_cp=0 */
	/* clkpll_pu_pfd=0 */
	/* clkpll_pu_fbdv=0 */
	/* clkpll_pu_postdiv=0 */
	tmpVal=BL_RD_REG(RF_BASE,RF_PU_RST_CLKPLL);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_PU_CP);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_PU_PFD);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_PU_FBDV);
	tmpVal=BL_CLR_REG_BIT(tmpVal,RF_CLKPLL_PU_POSTDIV);
	BL_WR_REG(PDS_BASE,RF_PU_RST_CLKPLL,tmpVal);
	
	return SUCCESS;
}

/*@} end of group RF_Public_Functions */

/*@} end of group RF */

/*@} end of group BL602_Peripheral_Driver */
