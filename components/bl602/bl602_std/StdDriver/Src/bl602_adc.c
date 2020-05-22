/**
  ******************************************************************************
  * @file    bl602_adc.c
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

#include "bl602_adc.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  ADC
 *  @{
 */

/** @defgroup  ADC_Private_Macros
 *  @{
 */
#define GPIP_CLK_SET_DUMMY_WAIT          {__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();}
#define AON_CLK_SET_DUMMY_WAIT          {__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();}

/*@} end of group ADC_Private_Macros */

/** @defgroup  ADC_Private_Types
 *  @{
 */

/*@} end of group ADC_Private_Types */

/** @defgroup  ADC_Private_Variables
 *  @{
 */
static intCallback_Type * adcIntCbfArra[1][GPIP_ADC_INT_ALL]={{NULL,NULL,NULL}};


/*@} end of group ADC_Private_Variables */

/** @defgroup  ADC_Global_Variables
 *  @{
 */

/*@} end of group ADC_Global_Variables */

/** @defgroup  ADC_Private_Fun_Declaration
 *  @{
 */

/*@} end of group ADC_Private_Fun_Declaration */

/** @defgroup  ADC_Private_Functions
 *  @{
 */

/*@} end of group ADC_Private_Functions */

/** @defgroup  ADC_Public_Functions
 *  @{
 */

/****************************************************************************//**
 * @brief  AON ADC sensor init
 *
 * @param  senCfg: AON ADC sensor config type
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Sensor_Init(AON_ADC_SENSOR_Type* senCfg)
{
	uint32_t tmpVal;
	
	CHECK_PARAM(IS_AON_ADC_OUTPUT_CHANNEL_TYPE(senCfg->chSel));
	CHECK_PARAM(IS_AON_ADC_TSEN_MOD_TYPE(senCfg->tsenDioMod));
	
	/* cmd */
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_SEN_TEST_EN,senCfg->senDCTestMuxEn);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_SEN_SEL,senCfg->chSel);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_CHIP_SEN_PU,senCfg->senEn);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,tmpVal);
	
	/* config 2 */
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG2);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_TSVBE_LOW,senCfg->tsvbeLow);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_TS_EN,senCfg->tsenEn);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_TSEXT_SEL,senCfg->tsenDioMod);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG2,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC micbias init
 *
 * @param  micCfg: AON ADC micbias config type
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Micbias_Init(AON_ADC_MIC_Type* micCfg)
{
	uint32_t tmpVal;
	
	CHECK_PARAM(IS_AON_ADC_MICBOOST_DB_TYPE(micCfg->micboostDb));
	CHECK_PARAM(IS_AON_ADC_PGA2_GAIN_TYPE(micCfg->micPga2Gain));
	CHECK_PARAM(IS_AON_ADC_MIC_MODE_TYPE(micCfg->mic1Mode));
	CHECK_PARAM(IS_AON_ADC_MIC_MODE_TYPE(micCfg->mic2Mode));
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_MICBOOST_32DB_EN,micCfg->micboostDb);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_MIC_PGA2_GAIN,micCfg->micPga2Gain);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_MIC1_DIFF,micCfg->mic1Mode);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_MIC2_DIFF,micCfg->mic2Mode);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_DWA_EN,micCfg->dwaEn);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_BYP_MICBOOST,micCfg->micboostBypassEn);
	tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_MICPGA_EN,micCfg->micPgaEn);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC VBAT enable
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Vbat_Enable(void)
{
	uint32_t tmpVal;
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG2);
	tmpVal=BL_SET_REG_BIT(tmpVal,AON_GPADC_VBAT_EN);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG2,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC VBAT disable
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Vbat_Disable(void)
{
	uint32_t tmpVal;
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG2);
	tmpVal=BL_CLR_REG_BIT(tmpVal,AON_GPADC_VBAT_EN);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG2,tmpVal);
}

/****************************************************************************//**
 * @brief  AON software reset the whole ADC
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Reset(void)
{
	uint32_t regCmd;
	
	/* reset ADC */
	regCmd=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,BL_SET_REG_BIT(regCmd,AON_GPADC_SOFT_RST));
	AON_CLK_SET_DUMMY_WAIT;
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,BL_CLR_REG_BIT(regCmd,AON_GPADC_SOFT_RST));
}

/****************************************************************************//**
 * @brief  AON ADC glable enable
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Global_Enable(void)
{
	uint32_t tmpVal;
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	tmpVal=BL_SET_REG_BIT(tmpVal,AON_GPADC_GLOBAL_EN);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC glable disable
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Global_Disable(void)
{
	uint32_t tmpVal;
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	tmpVal=BL_CLR_REG_BIT(tmpVal,AON_GPADC_GLOBAL_EN);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC normal mode init
 *
 * @param  nCfg: AON ADC normal mode configuration
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Normal_Config(AON_ADC_Normal_CFG_Type* nCfg)
{
	uint32_t regCfg1;
	uint32_t regCfg2;
	uint32_t regCalib;
	
	CHECK_PARAM(IS_AON_ADC_CLK_TYPE(nCfg->clkDiv));
	CHECK_PARAM(IS_AON_ADC_PGA_GAIN_TYPE(nCfg->gain1));
	CHECK_PARAM(IS_AON_ADC_PGA_GAIN_TYPE(nCfg->gain2));
	CHECK_PARAM(IS_AON_ADC_BIAS_SEL_TYPE(nCfg->biasSel));
	CHECK_PARAM(IS_AON_ADC_VREF_TYPE(nCfg->vref));
	CHECK_PARAM(IS_AON_ADC_SIG_INPUT_TYPE(nCfg->mode));
	CHECK_PARAM(IS_AON_ADC_DATA_WIDTH_TYPE(nCfg->resWidth));
	
	/* config 1 */
	regCfg1=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG1);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_V18_SEL,2);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_V11_SEL,1);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_DITHER_EN,DISABLE);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_SCAN_EN,DISABLE);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_SCAN_LENGTH,0);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CLK_DIV_RATIO,nCfg->clkDiv);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CLK_ANA_INV,DISABLE);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CAL_OS_EN,nCfg->offsetCalibEn);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG1,regCfg1);
	AON_CLK_SET_DUMMY_WAIT;
	
	/* config 2 */
	regCfg2=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG2);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_DLY_SEL,0);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA1_GAIN,nCfg->gain1);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA2_GAIN,nCfg->gain2);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_BIAS_SEL,nCfg->biasSel);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_CHOP_MODE,3);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_VCMI_EN,DISABLE);
	if((nCfg->gain1!=AON_ADC_PGA_GAIN_NONE)||(nCfg->gain2!=AON_ADC_PGA_GAIN_NONE)){
		regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_EN,ENABLE);
	}else{
		regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_EN,DISABLE);
	}
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_OS_CAL,8);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_VCM,2);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_VREF_SEL,nCfg->vref);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_DIFF_MODE,nCfg->mode);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_RES_SEL,nCfg->resWidth);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG2,regCfg2);
	
	/* calibration offset */
	regCalib=BL_RD_REG(AON_BASE,AON_GPADC_REG_DEFINE);
	regCalib=BL_SET_REG_BITS_VAL(regCalib,AON_GPADC_OS_CAL_DATA,nCfg->offsetCalibVal);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_DEFINE,regCalib);
}

/****************************************************************************//**
 * @brief  AON ADC normal mode channel config
 *
 * @param  posCh: ADC pos channel type
 * @param  negCh: ADC neg channel type
 * @param  contEn: ENABLE or DISABLE continuous mode
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Normal_Channel_Config(AON_ADC_Chan_Type posCh,AON_ADC_Chan_Type negCh,BL_Fun_Type contEn)
{
	uint32_t regCmd;
	uint32_t regCfg1;
	
	CHECK_PARAM(IS_AON_ADC_CHAN_TYPE(posCh));
	CHECK_PARAM(IS_AON_ADC_CHAN_TYPE(negCh));
	
	/* set channel */
	regCmd=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	regCmd=BL_SET_REG_BITS_VAL(regCmd,AON_GPADC_POS_SEL,posCh);
	regCmd=BL_SET_REG_BITS_VAL(regCmd,AON_GPADC_NEG_SEL,negCh);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,regCmd);
	
	/* set continuous mode */
	regCfg1=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG1);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CONT_CONV_EN,contEn);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG1,regCfg1);
}

/****************************************************************************//**
 * @brief  AON ADC normal mode convert start
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Normal_Start(void)
{
	uint32_t regCmd;
	
	/* enable convert start */
	regCmd=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	regCmd=BL_SET_REG_BIT(regCmd,AON_GPADC_CONV_START);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,regCmd);
}

/****************************************************************************//**
 * @brief  AON ADC normal mode convert stop
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Normal_Stop(void)
{
	uint32_t regCmd;
	
	/* disable convert start */
	regCmd=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
	regCmd=BL_CLR_REG_BIT(regCmd,AON_GPADC_CONV_START);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,regCmd);
}

/****************************************************************************//**
 * @brief  AON ADC scan mode init
 *
 * @param  sCfg: AON ADC scan mode configuration
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Scan_Config(AON_ADC_SCAN_CFG_Type* sCfg)
{
	uint32_t regCfg1;
	uint32_t regCfg2;
	uint32_t regCalib;
	
	CHECK_PARAM(IS_AON_ADC_CLK_TYPE(sCfg->clkDiv));
	CHECK_PARAM(IS_AON_ADC_PGA_GAIN_TYPE(sCfg->gain1));
	CHECK_PARAM(IS_AON_ADC_PGA_GAIN_TYPE(sCfg->gain2));
	CHECK_PARAM(IS_AON_ADC_BIAS_SEL_TYPE(sCfg->biasSel));
	CHECK_PARAM(IS_AON_ADC_VREF_TYPE(sCfg->vref));
	CHECK_PARAM(IS_AON_ADC_SIG_INPUT_TYPE(sCfg->mode));
	CHECK_PARAM(IS_AON_ADC_DATA_WIDTH_TYPE(sCfg->resWidth));
	
	/* config 1 */
	regCfg1=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG1);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_V18_SEL,2);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_V11_SEL,1);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_DITHER_EN,DISABLE);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_SCAN_EN,ENABLE);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_SCAN_LENGTH,0);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CLK_DIV_RATIO,sCfg->clkDiv);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CLK_ANA_INV,DISABLE);
	regCfg1=BL_SET_REG_BITS_VAL(regCfg1,AON_GPADC_CAL_OS_EN,sCfg->offsetCalibEn);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG1,regCfg1);
	AON_CLK_SET_DUMMY_WAIT;
	
	/* config 2 */
	regCfg2=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG2);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_DLY_SEL,0);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA1_GAIN,sCfg->gain1);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA2_GAIN,sCfg->gain2);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_BIAS_SEL,sCfg->biasSel);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_CHOP_MODE,3);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_VCMI_EN,DISABLE);
	if((sCfg->gain1!=AON_ADC_PGA_GAIN_NONE)||(sCfg->gain2!=AON_ADC_PGA_GAIN_NONE)){
		regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_EN,ENABLE);
	}else{
		regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_EN,DISABLE);
	}
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_OS_CAL,8);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_PGA_VCM,2);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_VREF_SEL,sCfg->vref);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_DIFF_MODE,sCfg->mode);
	regCfg2=BL_SET_REG_BITS_VAL(regCfg2,AON_GPADC_RES_SEL,sCfg->resWidth);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG2,regCfg2);
	
	/* calibration offset */
	regCalib=BL_RD_REG(AON_BASE,AON_GPADC_REG_DEFINE);
	regCalib=BL_SET_REG_BITS_VAL(regCalib,AON_GPADC_OS_CAL_DATA,sCfg->offsetCalibVal);
	BL_WR_REG(AON_BASE,AON_GPADC_REG_DEFINE,regCalib);
}

/****************************************************************************//**
 * @brief  AON ADC scan mode channel config
 *
 * @param  posChList[]: ADC pos channel list type
 * @param  negChList[]: ADC neg channel list type
 * @param  scanLength: ADC scan length
 * @param  contEn: ENABLE or DISABLE continuous mode
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Scan_Channel_Config(AON_ADC_Chan_Type posChList[],AON_ADC_Chan_Type negChList[],uint8_t scanLength,BL_Fun_Type contEn)
{
    uint32_t tmpVal,i;
    uint32_t dealLen;
	
	CHECK_PARAM((scanLength<13));
    
    /* Deal with the first 6 */
    dealLen=6;
    if(scanLength<dealLen){
        dealLen=scanLength;
    }
    /* Set first 6 scan channels */
    tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_SCN_POS1);
    for(i=0;i<dealLen;i++){
        tmpVal=tmpVal&(~(0x1F<<(i*5)));
        tmpVal|=(posChList[i]<<(i*5));
    }
    BL_WR_REG(AON_BASE,AON_GPADC_REG_SCN_POS1,tmpVal);
    
    tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_SCN_NEG1);
    for(i=0;i<dealLen;i++){
        tmpVal=tmpVal&(~(0x1F<<(i*5)));
        tmpVal|=(negChList[i]<<(i*5));
    }
    BL_WR_REG(AON_BASE,AON_GPADC_REG_SCN_NEG1,tmpVal);
    
    /* Set the left channels */
    if(scanLength>dealLen){
        tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_SCN_POS2);
        for(i=0;i<scanLength-dealLen;i++){
            tmpVal=tmpVal&(~(0x1F<<(i*5)));
            tmpVal|=(posChList[i+dealLen]<<(i*5));
        }
        BL_WR_REG(AON_BASE,AON_GPADC_REG_SCN_POS2,tmpVal);
        
        tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_SCN_NEG2);
        for(i=0;i<scanLength-dealLen;i++){
            tmpVal=tmpVal&(~(0x1F<<(i*5)));
            tmpVal|=(negChList[i+dealLen]<<(i*5));
        }
        BL_WR_REG(AON_BASE,AON_GPADC_REG_SCN_NEG2,tmpVal);
    }
    
    /* Scan mode */
    tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG1);     
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_SCAN_LENGTH,scanLength-1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_CONT_CONV_EN,contEn);
    tmpVal=BL_SET_REG_BIT(tmpVal,AON_GPADC_SCAN_EN);
    BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG1,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC scan mode convert start
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Scan_Start(void)
{
    uint32_t tmpVal;
	
    /* enable convert start */
    tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
    tmpVal=BL_SET_REG_BIT(tmpVal,AON_GPADC_CONV_START);
    BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC scan mode convert stop
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Scan_Stop(void)
{
    uint32_t tmpVal;
        
    /* stop convert */
    tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CMD);
    tmpVal=BL_CLR_REG_BIT(tmpVal,AON_GPADC_CONV_START);    
    BL_WR_REG(AON_BASE,AON_GPADC_REG_CMD,tmpVal);
    
    /* disable scan mode */
    tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_CONFIG1);
    tmpVal=BL_CLR_REG_BIT(tmpVal,AON_GPADC_SCAN_EN);
    BL_WR_REG(AON_BASE,AON_GPADC_REG_CONFIG1,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC status mask
 *
 * @param  chType: ADC channel type
 * @param  statusMask: MASK or UNMASK
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Status_Mask(AON_CHANNEL_Type chType, BL_Mask_Type statusMask)
{
	uint32_t tmpVal;
	
	CHECK_PARAM(IS_AON_CHANNEL_TYPE(chType));
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_ISR);
	if(chType==AON_CHANNEL_POS){
		tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_POS_SATUR_MASK,statusMask);
	}else{
		tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_GPADC_NEG_SATUR_MASK,statusMask);
	}
	BL_WR_REG(AON_BASE,AON_GPADC_REG_ISR,tmpVal);
}

/****************************************************************************//**
 * @brief  AON ADC status clear
 *
 * @param  chType: ADC channel type
 *
 * @return None
 *
*******************************************************************************/
void AON_ADC_Status_Clr(AON_CHANNEL_Type chType)
{
	uint32_t tmpVal;
	
	CHECK_PARAM(IS_AON_CHANNEL_TYPE(chType));
	
	tmpVal=BL_RD_REG(AON_BASE,AON_GPADC_REG_ISR);
	if(chType==AON_CHANNEL_POS){
		tmpVal=BL_SET_REG_BIT(tmpVal,AON_GPADC_POS_SATUR_CLR);
	}else{
		tmpVal=BL_SET_REG_BIT(tmpVal,AON_GPADC_NEG_SATUR_CLR);
	}
	BL_WR_REG(AON_BASE,AON_GPADC_REG_ISR,tmpVal);
}

/****************************************************************************//**
 * @brief  Get ADC status
 *
 * @param  chType: ADC channel type
 *
 * @return SET or RESET
 *
*******************************************************************************/
BL_Sts_Type AON_ADC_Get_Status(AON_CHANNEL_Type chType)
{
	if(chType==AON_CHANNEL_POS){
		return BL_GET_REG_BITS_VAL(BL_RD_REG(AON_BASE,AON_GPADC_REG_ISR),AON_GPADC_POS_SATUR)?SET:RESET;
	}else{
		return BL_GET_REG_BITS_VAL(BL_RD_REG(AON_BASE,AON_GPADC_REG_ISR),AON_GPADC_NEG_SATUR)?SET:RESET;
	}
}

/****************************************************************************//**
 * @brief  ADC FIFO configuration
 *
 * @param  fifoCfg: ADC FIFO confifuration pointer
 *
 * @return None
 *
*******************************************************************************/
void GPIP_ADC_FIFO_Cfg(GPIP_ADC_FIFO_Cfg_Type *fifoCfg)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_GPIP_ADC_FIFO_THRESHOLD_TYPE(fifoCfg->fifoThreshold));

    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,GPIP_GPADC_FIFO_THL,fifoCfg->fifoThreshold);
    
    /* Enable DMA */
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,GPIP_GPADC_DMA_EN,fifoCfg->dmaEn);
    
    BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);  
}

/****************************************************************************//**
 * @brief  ADC get DMA FIFO data count
 *
 * @param  None
 *
 * @return data count in FIFO
 *
*******************************************************************************/
uint8_t GPIP_ADC_Get_FIFO_Count(void)
{
    uint32_t tmpVal;
     
    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    
    return BL_GET_REG_BITS_VAL(tmpVal,GPIP_GPADC_FIFO_DATA_COUNT);
}

/****************************************************************************//**
 * @brief  ADC mask or unmask certain or all interrupt
 *
 * @param  intType: interrupt type
 * @param  intMask: mask or unmask
 *
 * @return None
 *
*******************************************************************************/
void GPIP_ADC_IntMask(GPIP_ADC_INT_Type intType, BL_Mask_Type intMask)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_GPIP_ADC_INT_TYPE(intType));
    CHECK_PARAM(IS_BL_MASK_TYPE(intMask));

    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    
    switch(intType)
    {
        case GPIP_ADC_INT_FIFO_UNDERRUN:
            if(intMask == UNMASK){
                /* Enable this interrupt */
                tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_MASK);
            }else{
                /* Disable this interrupt */
                tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_MASK);
            }
            break;
        case GPIP_ADC_INT_FIFO_OVERRUN:
            if(intMask == UNMASK){
                /* Enable this interrupt */
                tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_MASK);
            }else{
                /* Disable this interrupt */
                tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_MASK);
            }
            break;
        case GPIP_ADC_INT_ADC_READY:
            if(intMask == UNMASK){
                /* Enable this interrupt */
                tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_RDY_MASK);
            }else{
                /* Disable this interrupt */
                tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_RDY_MASK);
            }
            break;
        case GPIP_ADC_INT_ALL:
            if(intMask == UNMASK){
                /* Enable this interrupt */
                tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_MASK);
                tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_MASK);
                tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_RDY_MASK);
            }else{
                /* Disable this interrupt */
                tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_MASK);
                tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_MASK);
                tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_RDY_MASK);
            }
            break;
        default:
            break;
    }
    BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);
}

/****************************************************************************//**
 * @brief  ADC clear certain or all interrupt
 *
 * @param  intType: interrupt type
 *
 * @return None
 *
*******************************************************************************/
void GPIP_ADC_IntClr(GPIP_ADC_INT_Type intType)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_GPIP_ADC_INT_TYPE(intType));

    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    
    switch(intType)
    {
        case GPIP_ADC_INT_FIFO_UNDERRUN:
            tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_CLR);
            BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);
            tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_CLR);
            break;
        case GPIP_ADC_INT_FIFO_OVERRUN:
            tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_CLR);
            BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);
            tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_CLR);
            break;
        case GPIP_ADC_INT_ADC_READY:
            tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_RDY_CLR);
            BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);
            tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_RDY_CLR);
            break;
        case GPIP_ADC_INT_ALL:
            tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_CLR);
            tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_CLR);
            tmpVal=BL_SET_REG_BIT(tmpVal,GPIP_GPADC_RDY_CLR);
            BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);
            tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_UNDERRUN_CLR);
            tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_FIFO_OVERRUN_CLR);
            tmpVal=BL_CLR_REG_BIT(tmpVal,GPIP_GPADC_RDY_CLR);
            break;
        default:
            break;
    }
    BL_WR_REG(GPIP_BASE,GPIP_GPADC_CONFIG,tmpVal);
}

/****************************************************************************//**
 * @brief  ADC get interrupt status
 *
 * @param  intType: interrupt type
 *
 * @return SET or RESET
 *
*******************************************************************************/
BL_Sts_Type GPIP_ADC_GetIntStatus(GPIP_ADC_INT_Type intType)
{
    uint32_t tmpVal;
    BL_Sts_Type bitStatus = RESET;

    /* Check the parameters */
    CHECK_PARAM(IS_GPIP_ADC_INT_TYPE(intType));

    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    
    switch(intType)
    {
        case GPIP_ADC_INT_FIFO_UNDERRUN:
            bitStatus = (BL_IS_REG_BIT_SET(tmpVal,GPIP_GPADC_FIFO_UNDERRUN)) ? SET : RESET;
            break;
        case GPIP_ADC_INT_FIFO_OVERRUN:
            bitStatus = (BL_IS_REG_BIT_SET(tmpVal,GPIP_GPADC_FIFO_OVERRUN)) ? SET : RESET;
            break;
        case GPIP_ADC_INT_ADC_READY:
            bitStatus = (BL_IS_REG_BIT_SET(tmpVal,GPIP_GPADC_RDY)) ? SET : RESET;
            break;
        case GPIP_ADC_INT_ALL:
            break;
        default:
            break;
    }
    return bitStatus;
}

/****************************************************************************//**
 * @brief  ADC get DMA FIFO full status
 *
 * @param  None
 *
 * @return SET or RESET
 *
*******************************************************************************/
BL_Sts_Type GPIP_ADC_FIFO_Full(void)
{
    uint32_t tmpVal;
     
    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    
    if(BL_IS_REG_BIT_SET(tmpVal,GPIP_GPADC_FIFO_FULL)){
        return SET;
    }else{
        return RESET;
    }
}

/****************************************************************************//**
 * @brief  ADC get DMA FIFO empty status
 *
 * @param  None
 *
 * @return SET or RESET
 *
*******************************************************************************/
BL_Sts_Type GPIP_ADC_FIFO_Empty(void)
{
    uint32_t tmpVal;
     
    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_CONFIG);
    
    if(BL_IS_REG_BIT_SET(tmpVal,GPIP_GPADC_FIFO_NE)){
        return RESET;
    }else{
        return SET;
    }
}

/****************************************************************************//**
 * @brief  ADC read DMA FIFO data
 *
 * @param  None
 *
 * @return ADC result
 *
*******************************************************************************/
uint32_t GPIP_ADC_Read_FIFO(void)
{
    uint32_t tmpVal;
     
    tmpVal=BL_RD_REG(GPIP_BASE,GPIP_GPADC_DMA_RDATA);
    
    return (tmpVal);
}

/****************************************************************************//**
 * @brief  GPIP ADC install interrupt callback
 *
 * @param  intType: GPIP ADC interrupt type
 * @param  cbFun: GPIP ADC interrupt callback
 *
 * @return None
 *
*******************************************************************************/
void GPIP_ADC_Int_Callback_Install(GPIP_ADC_INT_Type intType,intCallback_Type* cbFun)
{
    /* Check the parameters */
    CHECK_PARAM(IS_GPIP_ADC_INT_TYPE(intType));
	
    adcIntCbfArra[0][intType] = cbFun;
}

/****************************************************************************//**
 * @brief  ADC DMA interrupt handler
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
#ifndef BL602_USE_HAL_DRIVER
void GPADC_DMA_IRQHandler(void)
{
    if( GPIP_ADC_GetIntStatus(GPIP_ADC_INT_FIFO_UNDERRUN)==SET ){
        GPIP_ADC_IntClr(GPIP_ADC_INT_FIFO_UNDERRUN);
        if(adcIntCbfArra[0][GPIP_ADC_INT_FIFO_UNDERRUN] != NULL){
            adcIntCbfArra[0][GPIP_ADC_INT_FIFO_UNDERRUN]();
        }
    }
    
    if( GPIP_ADC_GetIntStatus(GPIP_ADC_INT_FIFO_OVERRUN)==SET ){
        GPIP_ADC_IntClr(GPIP_ADC_INT_FIFO_OVERRUN);
        if(adcIntCbfArra[0][GPIP_ADC_INT_FIFO_OVERRUN] != NULL){
            adcIntCbfArra[0][GPIP_ADC_INT_FIFO_OVERRUN]();
        }
    }
    
    if( GPIP_ADC_GetIntStatus(GPIP_ADC_INT_ADC_READY)==SET ){
        GPIP_ADC_IntClr(GPIP_ADC_INT_ADC_READY);
        if(adcIntCbfArra[0][GPIP_ADC_INT_ADC_READY] != NULL){
            adcIntCbfArra[0][GPIP_ADC_INT_ADC_READY]();
        }
    }
}
#endif

/*@} end of group ADC_Public_Functions */

/*@} end of group ADC */

/*@} end of group BL602_Peripheral_Driver */
