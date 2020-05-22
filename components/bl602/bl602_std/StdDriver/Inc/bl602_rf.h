/**
  ******************************************************************************
  * @file    bl602_rf.h
  * @version V1.0
  * @date
  * @brief   This file is the standard driver header file
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
#ifndef __BL602_RF_H__
#define __BL602_RF_H__

#include "rf_reg.h"
#include "pds_reg.h"
#include "bl602_aon.h"
#include "bl602_common.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  RF
 *  @{
 */

/** @defgroup  RF_Public_Types
 *  @{
 */

/**
 *  @brief PLL XTAL type definition
 */
typedef enum {
    PLL_XTAL_24M,                           /*!< XTAL is 24M */
    PLL_XTAL_32M,                           /*!< XTAL is 32M */
    PLL_XTAL_38P4M,                         /*!< XTAL is 38.4M */
    PLL_XTAL_40M,                           /*!< XTAL is 40M */
    PLL_XTAL_RC32M,                         /*!< XTAL is RC32M */
}RF_PLL_XTAL_Type;

/**
 *  @brief PLL output clock type definition
 */
typedef enum {
    PLL_CLK_480M,                           /*!< PLL output clock:480M */
    PLL_CLK_240M,                           /*!< PLL output clock:240M */
    PLL_CLK_192M,                           /*!< PLL output clock:192M */
    PLL_CLK_160M,                           /*!< PLL output clock:160M */
    PLL_CLK_120M,                           /*!< PLL output clock:120M */
    PLL_CLK_96M,                            /*!< PLL output clock:96M */
    PLL_CLK_80M,                            /*!< PLL output clock:80M */
    PLL_CLK_48M,                            /*!< PLL output clock:48M */
    PLL_CLK_32M,                            /*!< PLL output clock:32M */
}RF_PLL_CLK_Type;

/**
 *  @brief PLL configuration structure type definition
 */
typedef struct {
    uint8_t clkpllIcp1u;                    /*!< int mode:0, frac mode:1 */
    uint8_t clkpllIcp5u;                    /*!< int mode:2, frac mode:0 */
    uint8_t clkpllIntFracSw;                /*!< 0:int mode, 1:frac mode */
    uint8_t clkpllC3;                       /*!< int:3, frac:2 */
    uint8_t clkpllCz;                       /*!< int:1, frac:2 */
    uint8_t clkpllRz;                       /*!< int:1, frac:5 */
    uint8_t clkpllR4;                       /*!< int:2, frac:2 */
    uint8_t clkpllR4Short;                  /*!< int:1, frac:0 */
    uint8_t clkpllRefdivRatio;              /*!< ref divider ratio */
    uint8_t clkpllPostdiv;                  /*!< >=8 and should be even number */
    uint32_t clkpllSdmin;                   /*!< sdmin */
    uint8_t clkpllSelFbClk;                 /*!< 0:mod1, 1:mod2, 2:mod3 */
    uint8_t clkpllSelSampleClk;             /*!< 0:[16,63)mod3, 1:[32:127)mod4, 2:[64,255)mod5 */
}RF_PLL_Cfg_Type;

/*@} end of group RF_Public_Types */

/** @defgroup  RF_Public_Constants
 *  @{
 */

/** @defgroup  RF_PLL_XTAL_TYPE
 *  @{
 */
#define IS_RF_PLL_XTAL_TYPE(type)                        (((type) == PLL_XTAL_24M) || \
                                                          ((type) == PLL_XTAL_32M) || \
                                                          ((type) == PLL_XTAL_38P4M) || \
                                                          ((type) == PLL_XTAL_40M) || \
                                                          ((type) == PLL_XTAL_RC32M))

/** @defgroup  RF_PLL_CLK_TYPE
 *  @{
 */
#define IS_RF_PLL_CLK_TYPE(type)                         (((type) == PLL_CLK_480M) || \
                                                          ((type) == PLL_CLK_240M) || \
                                                          ((type) == PLL_CLK_192M) || \
                                                          ((type) == PLL_CLK_160M) || \
                                                          ((type) == PLL_CLK_120M) || \
                                                          ((type) == PLL_CLK_96M) || \
                                                          ((type) == PLL_CLK_80M) || \
                                                          ((type) == PLL_CLK_48M) || \
                                                          ((type) == PLL_CLK_32M))

/*@} end of group RF_Public_Constants */

/** @defgroup  RF_Public_Macros
 *  @{
 */

/*@} end of group RF_Public_Macros */

/** @defgroup  RF_Public_Functions
 *  @{
 */
BL_Err_Type ATTR_CLOCK_SECTION RF_Select_RC32M_As_PLL_Ref(void);
BL_Err_Type ATTR_CLOCK_SECTION RF_Select_XTAL_As_PLL_Ref(void);
BL_Err_Type RF_Power_On_SFReg(void);
BL_Err_Type RF_Power_Off_SFReg(void);
BL_Err_Type RF_Power_On_PLL(RF_PLL_XTAL_Type xtalType);
BL_Err_Type RF_Enable_PLL_All_Clks(void);
BL_Err_Type RF_Enable_PLL_Clk(RF_PLL_CLK_Type pllClk);
BL_Err_Type RF_Disable_PLL_All_Clks(void);
BL_Err_Type RF_Disable_PLL_Clk(RF_PLL_CLK_Type pllClk);
BL_Err_Type RF_Power_Off_PLL(void);

/*@} end of group RF_Public_Functions */

/*@} end of group RF */

/*@} end of group BL602_Peripheral_Driver */

#endif /* __BL602_RF_H__ */
