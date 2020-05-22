/**
  ******************************************************************************
  * @file    bl602_pds.h
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
#ifndef __BL602_PDS_H__
#define __BL602_PDS_H__

#include "pds_reg.h"
#include "bl602_ef_ctrl.h"
#include "bl602_aon.h"
#include "bl602_common.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  PDS
 *  @{
 */

/** @defgroup  PDS_Public_Types
 *  @{
 */

/**
 *  @brief PDS LDO voltage level type definition
 */
typedef enum {
    PDS_LDO_LEVEL_0P8V=0,                   /*!< PDS LDO voltage 0.8V */
    PDS_LDO_LEVEL_0P9V,                     /*!< PDS LDO voltage 0.9V */
    PDS_LDO_LEVEL_1P00V,                    /*!< PDS LDO voltage 1.00V */
    PDS_LDO_LEVEL_1P05V,                    /*!< PDS LDO voltage 1.05V */
    PDS_LDO_LEVEL_1P10V,                    /*!< PDS LDO voltage 1.10V */
    PDS_LDO_LEVEL_1P15V,                    /*!< PDS LDO voltage 1.15V */
    PDS_LDO_LEVEL_1P20V,                    /*!< PDS LDO voltage 1.20V */
    PDS_LDO_LEVEL_1P30V,                    /*!< PDS LDO voltage 1.30V */
}PDS_LDO_LEVEL_Type;

/**
 *  @brief PDS configuration type definition
 */
typedef struct {
    uint32_t pdsStart                      :  1;    /*!< PDS Start */
    uint32_t sleepForever                  :  1;    /*!< PDS sleep forever */
    uint32_t xtalForceOff                  :  1;    /*!< Power off xtal force */
    uint32_t pdsUsbDis                     :  1;    /*!< Disable Power Down USB */
    uint32_t pdsCs                         :  3;    /*!< Read Only */
    uint32_t pdsApDis                      :  1;    /*!< Disable Power Down AP */
    uint32_t pdsGateClkDis                 :  1;    /*!< Disable clcok geting */
    uint32_t pdsMemStbyDis                 :  1;    /*!< Disable memory stanby */
    uint32_t pdsBzDis                      :  1;    /*!< Disable Power Down BZ */
    uint32_t pdsIsoEnDis                   :  1;    /*!< Disable ISO */
    uint32_t pdsIsoCgDis                   :  1;    /*!< Disable ISO CG */
    uint32_t pdsPwrOffDis                  :  1;    /*!< Disable power off */
    uint32_t pdsRfXtalPuDis                :  1;    /*!< Disable RF XTAL power up */
    uint32_t pdsIrqInDis                   :  1;    /*!< Disable */
    uint32_t pdsRstSocEn                   :  1;    /*!< Enable SOC reset */
    uint32_t pdsRC32mOffDis                :  1;    /*!< Disable RC32M power off */
    uint32_t pdsLdoVselEn                  :  1;    /*!< Enable LDO voltage select */
    uint32_t pdsNpDis                      :  1;    /*!< Disable Power Down NP */
    uint32_t cpu1WfiMask                   :  1;    /*!< Mask CPU1 WFI when enter PDS */
    uint32_t cpu0WfiMask                   :  1;    /*!< Mask CPU0 WFI when enter PDS */
    uint32_t reserved22_23                 :  2;    /*!< Reserved */
    uint32_t pdsLdoVol                     :  4;    /*!< PDS LDO voltage setting */
    uint32_t pdsPuClkPllDis                :  1;    /*!< Disable power up clock pll */
    uint32_t pdsPuSfregDis                 :  1;    /*!< Disable SF REG power up */
    uint32_t pdsPuTopLdo18RfDis            :  1;    /*!< Disable LDO 1.8V power up */
    uint32_t pdsPuMbgDis                   :  1;    /*!< Disable MBG power up */
}PDS_CFG_Type;

/**
 *  @brief PDS configuration type definition
 */
typedef struct {
    uint32_t pdsForceUsbGateClk            :  1;    /*!< manual force USB clock gated */
    uint32_t pdsForceBzGateClk             :  1;    /*!< manual force BZ clock gated */
    uint32_t pdsForceApGateClk             :  1;    /*!< manual force AP clock gated */
    uint32_t pdsForceNpGateClk             :  1;    /*!< manual force NP clock gated */
    uint32_t pdsForceUsbMemStby            :  1;    /*!< manual force USB memory sleep */
    uint32_t pdsForceBzMemStby             :  1;    /*!< manual force BZ memory sleep */
    uint32_t pdsForceApMemStby             :  1;    /*!< manual force AP memory sleep */
    uint32_t pdsForceNpMemStby             :  1;    /*!< manual force NP memory sleep */
    uint32_t pdsForceUsbPdsRst             :  1;    /*!< manual force USB pds reset */
    uint32_t pdsForceBzPdsRst              :  1;    /*!< manual force BZ pds reset */
    uint32_t pdsForceApPdsRst              :  1;    /*!< manual force AP pds reset */
    uint32_t pdsForceNpPdsRst              :  1;    /*!< manual force NP pds reset */
    uint32_t pdsForceUsbIsoEn              :  1;    /*!< manual force USB isolation */
    uint32_t pdsForceBzIsoEn               :  1;    /*!< manual force BZ isolation */
    uint32_t pdsForceApIsoEn               :  1;    /*!< manual force AP isolation */
    uint32_t pdsForceNpIsoEn               :  1;    /*!< manual force NP isolation */
    uint32_t pdsForceUsbPwrOff             :  1;    /*!< manual force USB power off */
    uint32_t pdsForceBzPwrOff              :  1;    /*!< manual force BZ power off */
    uint32_t pdsForceApPwrOff              :  1;    /*!< manual force AP power off */
    uint32_t pdsForceNpPwrOff              :  1;    /*!< manual force NP power off */
    uint32_t reserved20_31                 : 12;    /*!< Reserved */
}PDS_MODULE_CFG_Type;

/**
 *  @brief PDS force type definition
 */
typedef enum {
    PDS_FORCE_NP,                           /*!< PDS force NP */
    PDS_FORCE_WB,                           /*!< PDS force WB */
}PDS_FORCE_Type;

/**
 *  @brief PDS LDO voltage type definition
 */
typedef enum {
    PDS_LDO_VOLTAGE_0P60V,                  /*!< PDS LDO voltage 0.60V */
    PDS_LDO_VOLTAGE_0P65V,                  /*!< PDS LDO voltage 0.65V */
    PDS_LDO_VOLTAGE_0P70V,                  /*!< PDS LDO voltage 0.70V */
    PDS_LDO_VOLTAGE_0P75V,                  /*!< PDS LDO voltage 0.75V */
    PDS_LDO_VOLTAGE_0P80V,                  /*!< PDS LDO voltage 0.80V */
    PDS_LDO_VOLTAGE_0P85V,                  /*!< PDS LDO voltage 0.85V */
    PDS_LDO_VOLTAGE_0P90V,                  /*!< PDS LDO voltage 0.9V */
    PDS_LDO_VOLTAGE_0P95V,                  /*!< PDS LDO voltage 0.95V */
    PDS_LDO_VOLTAGE_1P00V,                  /*!< PDS LDO voltage 1.0V */
    PDS_LDO_VOLTAGE_1P05V,                  /*!< PDS LDO voltage 1.05V */
    PDS_LDO_VOLTAGE_1P10V,                  /*!< PDS LDO voltage 1.1V */
    PDS_LDO_VOLTAGE_1P15V,                  /*!< PDS LDO voltage 1.15V */
    PDS_LDO_VOLTAGE_1P20V,                  /*!< PDS LDO voltage 1.2V */
    PDS_LDO_VOLTAGE_1P25V,                  /*!< PDS LDO voltage 1.25V */
    PDS_LDO_VOLTAGE_1P30V,                  /*!< PDS LDO voltage 1.3V */
    PDS_LDO_VOLTAGE_1P35V,                  /*!< PDS LDO voltage 1.35V */
}PDS_LDO_VOLTAGE_Type;

/**
 *  @brief PDS auto power down configuration type definition
 */
typedef struct {
    BL_Fun_Type mbgPower;                   /*!< PDS auto [31] MBG power */
    BL_Fun_Type ldo18rfPower;               /*!< PDS auto [30] LDO18RF power */
    BL_Fun_Type sfregPower;                 /*!< PDS auto [29] SF_REG power */
    BL_Fun_Type pllPower;                   /*!< PDS auto [28] PLL power */
    BL_Fun_Type cpu0Power;                  /*!< PDS auto [19] NP power */
    BL_Fun_Type rc32mPower;                 /*!< PDS auto [17] RC32M power */
    BL_Fun_Type xtalPower;                  /*!< PDS auto [14] XTAL power */
    BL_Fun_Type allPower;                   /*!< PDS auto [13] all power */
    BL_Fun_Type isoPower;                   /*!< PDS auto [11] ISO power */
    BL_Fun_Type bzPower;                    /*!< PDS auto [10] BZ power */
    BL_Fun_Type sramDisStanby;              /*!< PDS auto [9] SRAM memory stanby disable */
    BL_Fun_Type cgPower;                    /*!< PDS auto [8] CG power */
    BL_Fun_Type cpu1Power;                  /*!< PDS auto [7] AP power */
    BL_Fun_Type usbPower;                   /*!< PDS auto [3] USB power */
}PDS_AUTO_POWER_DOWN_CFG_Type;

/**
 *  @brief PDS auto configuration type definition
 */
typedef struct {
    PDS_LDO_VOLTAGE_Type vddcoreVol;        /*!< PDS auto [27:24] VDDCORE voltage, reference 0x4001F80C[27:24], recommended 0xA */
    BL_Fun_Type vddcoreVolEn;               /*!< PDS auto [18] VDDCORE voltage enable bit */
    BL_Fun_Type cpu0NotNeedWFI;             /*!< PDS auto [21] NP not need WFI to get in PDS mode */
    BL_Fun_Type cpu1NotNeedWFI;             /*!< PDS auto [20] AP not need WFI to get in PDS mode */
    BL_Fun_Type busReset;                   /*!< PDS auto [16] bus reset bit, reset after wake up from PDS mode */
    BL_Fun_Type disIrqWakeUp;               /*!< PDS auto [15] disable IRQ request to wake up from PDS mode, except PDS counter IRQ */
    BL_Fun_Type powerOffXtalForever;        /*!< PDS auto [2] power off xtal after get in PDS mode, and never power on xtal after wake up */
    BL_Fun_Type sleepForever;               /*!< PDS auto [1] sleep forever after get in PDS mode, need reset system to restart */
}PDS_AUTO_NORMAL_CFG_Type;

/**
 *  @brief PLL XTAL type definition
 */
typedef enum {
    PDS_PLL_XTAL_NONE,                      /*!< XTAL is none */
    PDS_PLL_XTAL_24M,                       /*!< XTAL is 24M */
    PDS_PLL_XTAL_32M,                       /*!< XTAL is 32M */
    PDS_PLL_XTAL_38P4M,                     /*!< XTAL is 38.4M */
    PDS_PLL_XTAL_40M,                       /*!< XTAL is 40M */
    PDS_PLL_XTAL_26M,                       /*!< XTAL is 26M */
    PDS_PLL_XTAL_RC32M,                     /*!< XTAL is RC32M */
}PDS_PLL_XTAL_Type;

/**
 *  @brief PLL output clock type definition
 */
typedef enum {
    PDS_PLL_CLK_480M,                       /*!< PLL output clock:480M */
    PDS_PLL_CLK_240M,                       /*!< PLL output clock:240M */
    PDS_PLL_CLK_192M,                       /*!< PLL output clock:192M */
    PDS_PLL_CLK_160M,                       /*!< PLL output clock:160M */
    PDS_PLL_CLK_120M,                       /*!< PLL output clock:120M */
    PDS_PLL_CLK_96M,                        /*!< PLL output clock:96M */
    PDS_PLL_CLK_80M,                        /*!< PLL output clock:80M */
    PDS_PLL_CLK_48M,                        /*!< PLL output clock:48M */
    PDS_PLL_CLK_32M,                        /*!< PLL output clock:32M */
}PDS_PLL_CLK_Type;

/*@} end of group PDS_Public_Types */

/** @defgroup  PDS_Public_Constants
 *  @{
 */

/** @defgroup  PDS_LDO_LEVEL_TYPE
 *  @{
 */
#define IS_PDS_LDO_LEVEL_TYPE(type)                      (((type) == PDS_LDO_LEVEL_0P8V) || \
                                                          ((type) == PDS_LDO_LEVEL_0P9V) || \
                                                          ((type) == PDS_LDO_LEVEL_1P00V) || \
                                                          ((type) == PDS_LDO_LEVEL_1P05V) || \
                                                          ((type) == PDS_LDO_LEVEL_1P10V) || \
                                                          ((type) == PDS_LDO_LEVEL_1P15V) || \
                                                          ((type) == PDS_LDO_LEVEL_1P20V) || \
                                                          ((type) == PDS_LDO_LEVEL_1P30V))

/** @defgroup  PDS_FORCE_TYPE
 *  @{
 */
#define IS_PDS_FORCE_TYPE(type)                          (((type) == PDS_FORCE_NP) || \
                                                          ((type) == PDS_FORCE_WB))

/** @defgroup  PDS_LDO_VOLTAGE_TYPE
 *  @{
 */
#define IS_PDS_LDO_VOLTAGE_TYPE(type)                    (((type) == PDS_LDO_VOLTAGE_0P60V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P65V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P70V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P75V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P80V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P85V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P90V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_0P95V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P00V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P05V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P10V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P15V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P20V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P25V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P30V) || \
                                                          ((type) == PDS_LDO_VOLTAGE_1P35V))

/** @defgroup  PDS_PLL_XTAL_TYPE
 *  @{
 */
#define IS_PDS_PLL_XTAL_TYPE(type)                       (((type) == PDS_PLL_XTAL_NONE) || \
                                                          ((type) == PDS_PLL_XTAL_24M) || \
                                                          ((type) == PDS_PLL_XTAL_32M) || \
                                                          ((type) == PDS_PLL_XTAL_38P4M) || \
                                                          ((type) == PDS_PLL_XTAL_40M) || \
                                                          ((type) == PDS_PLL_XTAL_26M) || \
                                                          ((type) == PDS_PLL_XTAL_RC32M))

/** @defgroup  PDS_PLL_CLK_TYPE
 *  @{
 */
#define IS_PDS_PLL_CLK_TYPE(type)                        (((type) == PDS_PLL_CLK_480M) || \
                                                          ((type) == PDS_PLL_CLK_240M) || \
                                                          ((type) == PDS_PLL_CLK_192M) || \
                                                          ((type) == PDS_PLL_CLK_160M) || \
                                                          ((type) == PDS_PLL_CLK_120M) || \
                                                          ((type) == PDS_PLL_CLK_96M) || \
                                                          ((type) == PDS_PLL_CLK_80M) || \
                                                          ((type) == PDS_PLL_CLK_48M) || \
                                                          ((type) == PDS_PLL_CLK_32M))

/*@} end of group PDS_Public_Constants */

/** @defgroup  PDS_Public_Macros
 *  @{
 */
#define PDS_LDO_MIN_PU_CNT                              25  /* LDO need 25 cycles to power up */
#define PDS_WARMUP_CNT                                  38  /* LDO max wram up cycles */
#define PDS_FORCE_PWR_OFF_OFFSET                        (0)
#define PDS_FORCE_ISO_EN_OFFSET                         (4)
#define PDS_FORCE_PDS_RST_OFFSET                        (8)
#define PDS_FORCE_MEM_STBY_OFFSET                      (12)
#define PDS_FORCE_GATE_CLK_OFFSET                      (16)

/*@} end of group PDS_Public_Macros */

/** @defgroup  PDS_Public_Functions
 *  @{
 */
void PDS_Reset(void);
void PDS_Enable(PDS_CFG_Type *cfg,uint32_t pdsSleepCnt);
void PDS_Auto_Time_Config(uint32_t sleepDuration);
void PDS_Auto_Enable(PDS_AUTO_POWER_DOWN_CFG_Type *powerCfg,PDS_AUTO_NORMAL_CFG_Type *normalCfg,BL_Fun_Type enable);
void PDS_Manual_Enable(PDS_MODULE_CFG_Type *cfg);
void PDS_Manual_Force_Turn_Off(PDS_FORCE_Type domain);
void PDS_Manual_Force_Turn_On(PDS_FORCE_Type domain);
void PDS_Int_Callback_Install(intCallback_Type* cbFun);
/*----------*/
BL_Err_Type PDS_Trim_RC32M(void);
BL_Err_Type PDS_Select_RC32M_As_PLL_Ref(void);
BL_Err_Type PDS_Select_XTAL_As_PLL_Ref(void);
BL_Err_Type PDS_Power_On_PLL(PDS_PLL_XTAL_Type xtalType);
BL_Err_Type PDS_Enable_PLL_All_Clks(void);
BL_Err_Type PDS_Enable_PLL_Clk(PDS_PLL_CLK_Type pllClk);
BL_Err_Type PDS_Disable_PLL_All_Clks(void);
BL_Err_Type PDS_Disable_PLL_Clk(PDS_PLL_CLK_Type pllClk);
BL_Err_Type PDS_Power_Off_PLL(void);

/*@} end of group PDS_Public_Functions */

/*@} end of group PDS */

/*@} end of group BL602_Peripheral_Driver */

#endif /* __BL602_PDS_H__ */
