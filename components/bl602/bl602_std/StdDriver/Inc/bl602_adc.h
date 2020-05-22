/**
  ******************************************************************************
  * @file    bl602_adc.h
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
#ifndef __BL602_ADC_H__
#define __BL602_ADC_H__

#include "aon_reg.h"
#include "gpip_reg.h"
#include "bl602_common.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  ADC
 *  @{
 */

/** @defgroup  ADC_Public_Types
 *  @{
 */

/**
 *  @brief ADC trigger source type definition
 */
typedef enum {
    AON_ADC_OUTPUT_CHANNEL_SENVN0,          /*!< pos channel senvn0, neg channel senvn0 */
    AON_ADC_OUTPUT_CHANNEL_SENVN1,          /*!< pos channel senvn1, neg channel senvn1 */
    AON_ADC_OUTPUT_CHANNEL_SENVN2,          /*!< pos channel senvn2, neg channel senvn2 */
    AON_ADC_OUTPUT_CHANNEL_SENVN3,          /*!< pos channel senvn3, neg channel senvn3 */
}AON_ADC_OUTPUT_CHANNEL_Type;

/**
 *  @brief ADC channel type definition
 */
typedef enum {
    AON_ADC_Chan0,                          /*!< GPIO 0, ADC channel 0 */
    AON_ADC_Chan1,                          /*!< GPIO 1, ADC channel 1 */
    AON_ADC_Chan2,                          /*!< GPIO 2, ADC channel 2 */
    AON_ADC_Chan3,                          /*!< GPIO 3, ADC channel 3 */
    AON_ADC_Chan4,                          /*!< GPIO 4, ADC channel 4 */
    AON_ADC_Chan5,                          /*!< GPIO 5, ADC channel 5 */
    AON_ADC_Chan6,                          /*!< GPIO 6, ADC channel 6 */
    AON_ADC_Chan7,                          /*!< GPIO 7, ADC channel 7 */
    AON_ADC_Chan8,                          /*!< GPIO 8, ADC channel 8 */
    AON_ADC_Chan9,                          /*!< GPIO 9, ADC channel 9 */
    AON_ADC_Chan10,                         /*!< GPIO 10, ADC channel 10 */
    AON_ADC_Chan11,                         /*!< GPIO 11, ADC channel 11 */
    AON_ADC_Chan12,                         /*!< daca, ADC channel 12 */
    AON_ADC_Chan13,                         /*!< dacb, ADC channel 13 */
    AON_ADC_Chan14,                         /*!< tsenp, ADC channel 14 */
    AON_ADC_Chan15,                         /*!< tsenn, ADC channel 15 */
    AON_ADC_Chan16,                         /*!< vref, ADC channel 16 */
    AON_ADC_Chan17,                         /*!< dctest, ADC channel 17 */
    AON_ADC_Chan18,                         /*!< vbat, ADC channel 18 */
    AON_ADC_Chan19,                         /*!< senvp3, ADC channel 19 */
    AON_ADC_Chan20,                         /*!< senvp2, ADC channel 20 */
    AON_ADC_Chan21,                         /*!< senvp1, ADC channel 21 */
    AON_ADC_Chan22,                         /*!< senvp0, ADC channel 22 */
    AON_ADC_Chan23,                         /*!< GND, ADC channel 23 */
    AON_ADC_Chan24,                         /*!< GND, ADC channel 24 */
    AON_ADC_Chan25,                         /*!< GND, ADC channel 25 */
    AON_ADC_Chan26,                         /*!< GND, ADC channel 26 */
    AON_ADC_Chan27,                         /*!< GND, ADC channel 27 */
    AON_ADC_Chan28,                         /*!< GND, ADC channel 28 */
    AON_ADC_Chan29,                         /*!< GND, ADC channel 29 */
    AON_ADC_Chan30,                         /*!< GND, ADC channel 30 */
    AON_ADC_Chan31,                         /*!< GND, ADC channel 31 */
}AON_ADC_Chan_Type;

/**
 *  @brief ADC V18 selection type definition
 */
typedef enum {
    AON_ADC_V18_SEL_1P62V,                  /*!< V18 select 1.62V */
    AON_ADC_V18_SEL_1P72V,                  /*!< V18 select 1.72V */
    AON_ADC_V18_SEL_1P82V,                  /*!< V18 select 1.82V */
    AON_ADC_V18_SEL_1P92V,                  /*!< V18 select 1.92V */
}AON_ADC_V18_SEL_Type;

/**
 *  @brief ADC V11 selection type definition
 */
typedef enum {
    AON_ADC_V11_SEL_1P0V,                   /*!< V11 select 1.0V */
    AON_ADC_V11_SEL_1P1V,                   /*!< V11 select 1.1V */
    AON_ADC_V11_SEL_1P18V,                  /*!< V11 select 1.18V */
    AON_ADC_V11_SEL_1P26V,                  /*!< V11 select 1.26V */
}AON_ADC_V11_SEL_Type;

/**
 *  @brief ADC clock type definition
 */
typedef enum {
    AON_ADC_CLK_DIV_1,                      /*!< ADC clock:on 32M clock is 32M */
    AON_ADC_CLK_DIV_4,                      /*!< ADC clock:on 32M clock is 8M */
    AON_ADC_CLK_DIV_8,                      /*!< ADC clock:on 32M clock is 4M */
    AON_ADC_CLK_DIV_12,                     /*!< ADC clock:on 32M clock is 2.666M */
    AON_ADC_CLK_DIV_16,                     /*!< ADC clock:on 32M clock is 2M */
    AON_ADC_CLK_DIV_20,                     /*!< ADC clock:on 32M clock is 1.6M */
    AON_ADC_CLK_DIV_24,                     /*!< ADC clock:on 32M clock is 1.333M */
    AON_ADC_CLK_DIV_32,                     /*!< ADC clock:on 32M clock is 1M */
}AON_ADC_CLK_Type;

/**
 *  @brief ADC conversion speed type definition
 */
typedef enum {
    AON_ADC_DELAY_SEL_0,                    /*!< Select delay 0 */
    AON_ADC_DELAY_SEL_1,                    /*!< Select delay 1 */
    AON_ADC_DELAY_SEL_2,                    /*!< Select delay 2 */
    AON_ADC_DELAY_SEL_3,                    /*!< Select delay 3 */
    AON_ADC_DELAY_SEL_4,                    /*!< Select delay 4, not recommend */
    AON_ADC_DELAY_SEL_5,                    /*!< Select delay 5, not recommend */
    AON_ADC_DELAY_SEL_6,                    /*!< Select delay 6, not recommend */
    AON_ADC_DELAY_SEL_7,                    /*!< Select delay 7, not recommend */
}AON_ADC_DELAY_SEL_Type;

/**
 *  @brief ADC PGA gain type definition
 */
typedef enum {
    AON_ADC_PGA_GAIN_NONE,                  /*!< No PGA gain */
    AON_ADC_PGA_GAIN_1,                     /*!< PGA gain 1 */
    AON_ADC_PGA_GAIN_2,                     /*!< PGA gain 2 */
    AON_ADC_PGA_GAIN_4,                     /*!< PGA gain 4 */
    AON_ADC_PGA_GAIN_8,                     /*!< PGA gain 8 */
    AON_ADC_PGA_GAIN_16,                    /*!< PGA gain 16 */
    AON_ADC_PGA_GAIN_32,                    /*!< PGA gain 32 */
}AON_ADC_PGA_GAIN_Type;

/**
 *  @brief ADC analog portion low power mode selection type definition
 */
typedef enum {
    AON_ADC_BIAS_SEL_MAIN_BANDGAP,          /*!< ADC current from main bandgap */
    AON_ADC_BIAS_SEL_AON_BANDGAP,           /*!< ADC current from aon bandgap for HBN mode */
}AON_ADC_BIAS_SEL_Type;

/**
 *  @brief ADC chop mode type definition
 */
typedef enum {
    AON_ADC_CHOP_MOD_ALL_OFF,               /*!< all off */
    AON_ADC_CHOP_MOD_AZ_ON,                 /*!< Vref AZ on */
    AON_ADC_CHOP_MOD_AZ_PGA_ON,             /*!< Vref AZ and PGA chop on */
    AON_ADC_CHOP_MOD_AZ_PGA_RPC_ON,         /*!< Vref AZ and PGA chop+RPC on */
}AON_ADC_CHOP_MOD_Type;

/**
 *  @brief ADC audio PGA output common mode control type definition
 */
typedef enum {
    AON_ADC_PGA_VCM_1V,                     /*!< cm=1V */
    AON_ADC_PGA_VCM_1P2V,                   /*!< cm=1.2V */
    AON_ADC_PGA_VCM_1P4V,                   /*!< cm=1.4V */
    AON_ADC_PGA_VCM_1P6V,                   /*!< cm=1.6V */
}AON_ADC_PGA_VCM_Type;

/**
 *  @brief ADC tsen diode mode type definition
 */
typedef enum {
    AON_ADC_TSEN_MOD_INTERNAL_DIODE,        /*!< internal diode mode */
    AON_ADC_TSEN_MOD_EXTERNAL_DIODE,        /*!< external diode mode */
}AON_ADC_TSEN_MOD_Type;

/**
 *  @brief ADC voltage reference type definition
 */
typedef enum {
    AON_ADC_VREF_3P3V,                      /*!< ADC select 3.3V as reference voltage */
    AON_ADC_VREF_2V,                        /*!< ADC select 2V as reference voltage */
}AON_ADC_VREF_Type;

/**
 *  @brief ADC signal input type definition
 */
typedef enum {
    AON_ADC_INPUT_SINGLE_END,               /*!< ADC signal is single end */
    AON_ADC_INPUT_DIFF,                     /*!< ADC signal is differential */
}AON_ADC_SIG_INPUT_Type;

/**
 *  @brief ADC data width type definition
 */
typedef enum {
    AON_ADC_DATA_WIDTH_12,                  /*!< ADC 12 bits */
    AON_ADC_DATA_WIDTH_14_WITH_16_AVERAGE,  /*!< ADC 14 bits,and the value is average of 16 converts */
    AON_ADC_DATA_WIDTH_16_WITH_64_AVERAGE,  /*!< ADC 16 bits,and the value is average of 64 converts */
    AON_ADC_DATA_WIDTH_16_WITH_256_AVERAGE, /*!< ADC 16 bits,and the value is average of 256 converts */
}AON_ADC_Data_Width_Type;

/**
 *  @brief ADC channel type definition
 */
typedef enum {
    AON_CHANNEL_POS,                        /*!< ADC pos channel */
    AON_CHANNEL_NEG,                        /*!< ADC neg channel */
}AON_CHANNEL_Type;

/**
 *  @brief AON ADC sensor type definition
 */
typedef struct {
    BL_Fun_Type senDCTestMuxEn;             /*!< enable sensor dc test mux */
    AON_ADC_OUTPUT_CHANNEL_Type chSel;      /*!< selected output current channel and measurement channel */
    BL_Fun_Type senEn;                      /*!< enable chip sensor test */
    BL_Fun_Type tsvbeLow;                   /*!< tsen diode current */
    BL_Fun_Type tsenEn;                     /*!< Enable temperature sensor */
    AON_ADC_TSEN_MOD_Type tsenDioMod;       /*!< Temperature sensor diode mode */
}AON_ADC_SENSOR_Type;

/**
 *  @brief ADC micboost 32db type definition
 */
typedef enum {
    AON_ADC_MICBOOST_DB_16DB,               /*!< micboost 16db */
    AON_ADC_MICBOOST_DB_32DB,               /*!< micboost 32db */
}AON_ADC_MICBOOST_DB_Type;

/**
 *  @brief ADC pga2 gain type definition
 */
typedef enum {
    AON_ADC_PGA2_GAIN_0DB,                  /*!< mic pga2 gain 0db */
    AON_ADC_PGA2_GAIN_6DB,                  /*!< mic pga2 gain 6db */
    AON_ADC_PGA2_GAIN_N6DB,                 /*!< mic pga2 gain -6db */
    AON_ADC_PGA2_GAIN_12DB,                 /*!< mic pga2 gain 12db */
}AON_ADC_PGA2_GAIN_Type;

/**
 *  @brief ADC mic mode type definition
 */
typedef enum {
    AON_ADC_MIC_MODE_SINGLE,                /*!< mic single mode */
    AON_ADC_MIC_MODE_DIFF,                  /*!< mic diff mode */
}AON_ADC_MIC_MODE_Type;

/**
 *  @brief AON ADC mic type definition
 */
typedef struct {
    AON_ADC_MICBOOST_DB_Type micboostDb;    /*!< micboost db */
    AON_ADC_PGA2_GAIN_Type micPga2Gain;     /*!< mic pga2 gain */
    AON_ADC_MIC_MODE_Type mic1Mode;         /*!< mic1 single or diff */
    AON_ADC_MIC_MODE_Type mic2Mode;         /*!< mic2 single or diff */
    BL_Fun_Type dwaEn;                      /*!< dwa enable or disable */
    BL_Fun_Type micboostBypassEn;           /*!< micboost amp bypass enable or disable */
    BL_Fun_Type micPgaEn;                   /*!< mic pga enable or disable */
}AON_ADC_MIC_Type;

/**
 *  @brief AON ADC normal mode configuration type definition
 */
typedef struct {
    AON_ADC_CLK_Type clkDiv;                /*!< clock divider */
    BL_Fun_Type offsetCalibEn;              /*!< offset calibration enable */
    int16_t offsetCalibVal;                 /*!< offset calibration value */
    AON_ADC_PGA_GAIN_Type gain1;            /*!< PGA gain 1 */
    AON_ADC_PGA_GAIN_Type gain2;            /*!< PGA gain 2 */
    AON_ADC_BIAS_SEL_Type biasSel;          /*!< ADC current form main bandgap or aon bandgap */
    AON_ADC_VREF_Type vref;                 /*!< ADC voltage reference */
    AON_ADC_SIG_INPUT_Type mode;            /*!< ADC input signal type */
    AON_ADC_Data_Width_Type resWidth;       /*!< ADC resolution and oversample rate */
}AON_ADC_Normal_CFG_Type;

/**
 *  @brief AON ADC scan mode configuration type definition
 */
typedef struct {
    AON_ADC_CLK_Type clkDiv;                /*!< clock divider */
    BL_Fun_Type offsetCalibEn;              /*!< offset calibration enable */
    int16_t offsetCalibVal;                 /*!< offset calibration value */
    AON_ADC_PGA_GAIN_Type gain1;            /*!< PGA gain 1 */
    AON_ADC_PGA_GAIN_Type gain2;            /*!< PGA gain 2 */
    AON_ADC_BIAS_SEL_Type biasSel;          /*!< ADC current form main bandgap or aon bandgap */
    AON_ADC_VREF_Type vref;                 /*!< ADC voltage reference */
    AON_ADC_SIG_INPUT_Type mode;            /*!< ADC input signal type */
    AON_ADC_Data_Width_Type resWidth;       /*!< ADC resolution and oversample rate */
}AON_ADC_SCAN_CFG_Type;

/**
 *  @brief ADC FIFO threshold type definition
 */
typedef enum {
    GPIP_ADC_FIFO_THRESHOLD_1,              /*!< ADC FIFO threshold is 1 */
    GPIP_ADC_FIFO_THRESHOLD_4,              /*!< ADC FIFO threshold is 4 */
    GPIP_ADC_FIFO_THRESHOLD_8,              /*!< ADC FIFO threshold is 8 */
    GPIP_ADC_FIFO_THRESHOLD_16,             /*!< ADC FIFO threshold is 16 */
}GPIP_ADC_FIFO_Threshold_Type;

/**
 *  @brief ADC interrupt type definition
 */
typedef enum {
    GPIP_ADC_INT_FIFO_UNDERRUN,             /*!< ADC FIFO underrun interrupt */
    GPIP_ADC_INT_FIFO_OVERRUN,              /*!< ADC FIFO overrun interrupt */
    GPIP_ADC_INT_ADC_READY,                 /*!< ADC data ready interrupt */
    GPIP_ADC_INT_ALL,                        /*!< ADC all the interrupt */
}GPIP_ADC_INT_Type;

/**
 *  @brief ADC FIFO configuration structure type definition
 */
typedef struct {
    GPIP_ADC_FIFO_Threshold_Type fifoThreshold;    /*!< ADC FIFO threshold */
    BL_Fun_Type dmaEn;                      /*!< ADC DMA enable */
}GPIP_ADC_FIFO_Cfg_Type;

/*@} end of group ADC_Public_Types */

/** @defgroup  ADC_Public_Constants
 *  @{
 */

/** @defgroup  AON_ADC_OUTPUT_CHANNEL_TYPE
 *  @{
 */
#define IS_AON_ADC_OUTPUT_CHANNEL_TYPE(type)             (((type) == AON_ADC_OUTPUT_CHANNEL_SENVN0) || \
                                                          ((type) == AON_ADC_OUTPUT_CHANNEL_SENVN1) || \
                                                          ((type) == AON_ADC_OUTPUT_CHANNEL_SENVN2) || \
                                                          ((type) == AON_ADC_OUTPUT_CHANNEL_SENVN3))

/** @defgroup  AON_ADC_CHAN_TYPE
 *  @{
 */
#define IS_AON_ADC_CHAN_TYPE(type)                       (((type) == AON_ADC_Chan0) || \
                                                          ((type) == AON_ADC_Chan1) || \
                                                          ((type) == AON_ADC_Chan2) || \
                                                          ((type) == AON_ADC_Chan3) || \
                                                          ((type) == AON_ADC_Chan4) || \
                                                          ((type) == AON_ADC_Chan5) || \
                                                          ((type) == AON_ADC_Chan6) || \
                                                          ((type) == AON_ADC_Chan7) || \
                                                          ((type) == AON_ADC_Chan8) || \
                                                          ((type) == AON_ADC_Chan9) || \
                                                          ((type) == AON_ADC_Chan10) || \
                                                          ((type) == AON_ADC_Chan11) || \
                                                          ((type) == AON_ADC_Chan12) || \
                                                          ((type) == AON_ADC_Chan13) || \
                                                          ((type) == AON_ADC_Chan14) || \
                                                          ((type) == AON_ADC_Chan15) || \
                                                          ((type) == AON_ADC_Chan16) || \
                                                          ((type) == AON_ADC_Chan17) || \
                                                          ((type) == AON_ADC_Chan18) || \
                                                          ((type) == AON_ADC_Chan19) || \
                                                          ((type) == AON_ADC_Chan20) || \
                                                          ((type) == AON_ADC_Chan21) || \
                                                          ((type) == AON_ADC_Chan22) || \
                                                          ((type) == AON_ADC_Chan23) || \
                                                          ((type) == AON_ADC_Chan24) || \
                                                          ((type) == AON_ADC_Chan25) || \
                                                          ((type) == AON_ADC_Chan26) || \
                                                          ((type) == AON_ADC_Chan27) || \
                                                          ((type) == AON_ADC_Chan28) || \
                                                          ((type) == AON_ADC_Chan29) || \
                                                          ((type) == AON_ADC_Chan30) || \
                                                          ((type) == AON_ADC_Chan31))

/** @defgroup  AON_ADC_V18_SEL_TYPE
 *  @{
 */
#define IS_AON_ADC_V18_SEL_TYPE(type)                    (((type) == AON_ADC_V18_SEL_1P62V) || \
                                                          ((type) == AON_ADC_V18_SEL_1P72V) || \
                                                          ((type) == AON_ADC_V18_SEL_1P82V) || \
                                                          ((type) == AON_ADC_V18_SEL_1P92V))

/** @defgroup  AON_ADC_V11_SEL_TYPE
 *  @{
 */
#define IS_AON_ADC_V11_SEL_TYPE(type)                    (((type) == AON_ADC_V11_SEL_1P0V) || \
                                                          ((type) == AON_ADC_V11_SEL_1P1V) || \
                                                          ((type) == AON_ADC_V11_SEL_1P18V) || \
                                                          ((type) == AON_ADC_V11_SEL_1P26V))

/** @defgroup  AON_ADC_CLK_TYPE
 *  @{
 */
#define IS_AON_ADC_CLK_TYPE(type)                        (((type) == AON_ADC_CLK_DIV_1) || \
                                                          ((type) == AON_ADC_CLK_DIV_4) || \
                                                          ((type) == AON_ADC_CLK_DIV_8) || \
                                                          ((type) == AON_ADC_CLK_DIV_12) || \
                                                          ((type) == AON_ADC_CLK_DIV_16) || \
                                                          ((type) == AON_ADC_CLK_DIV_20) || \
                                                          ((type) == AON_ADC_CLK_DIV_24) || \
                                                          ((type) == AON_ADC_CLK_DIV_32))

/** @defgroup  AON_ADC_DELAY_SEL_TYPE
 *  @{
 */
#define IS_AON_ADC_DELAY_SEL_TYPE(type)                  (((type) == AON_ADC_DELAY_SEL_0) || \
                                                          ((type) == AON_ADC_DELAY_SEL_1) || \
                                                          ((type) == AON_ADC_DELAY_SEL_2) || \
                                                          ((type) == AON_ADC_DELAY_SEL_3) || \
                                                          ((type) == AON_ADC_DELAY_SEL_4) || \
                                                          ((type) == AON_ADC_DELAY_SEL_5) || \
                                                          ((type) == AON_ADC_DELAY_SEL_6) || \
                                                          ((type) == AON_ADC_DELAY_SEL_7))

/** @defgroup  AON_ADC_PGA_GAIN_TYPE
 *  @{
 */
#define IS_AON_ADC_PGA_GAIN_TYPE(type)                   (((type) == AON_ADC_PGA_GAIN_NONE) || \
                                                          ((type) == AON_ADC_PGA_GAIN_1) || \
                                                          ((type) == AON_ADC_PGA_GAIN_2) || \
                                                          ((type) == AON_ADC_PGA_GAIN_4) || \
                                                          ((type) == AON_ADC_PGA_GAIN_8) || \
                                                          ((type) == AON_ADC_PGA_GAIN_16) || \
                                                          ((type) == AON_ADC_PGA_GAIN_32))

/** @defgroup  AON_ADC_BIAS_SEL_TYPE
 *  @{
 */
#define IS_AON_ADC_BIAS_SEL_TYPE(type)                   (((type) == AON_ADC_BIAS_SEL_MAIN_BANDGAP) || \
                                                          ((type) == AON_ADC_BIAS_SEL_AON_BANDGAP))

/** @defgroup  AON_ADC_CHOP_MOD_TYPE
 *  @{
 */
#define IS_AON_ADC_CHOP_MOD_TYPE(type)                   (((type) == AON_ADC_CHOP_MOD_ALL_OFF) || \
                                                          ((type) == AON_ADC_CHOP_MOD_AZ_ON) || \
                                                          ((type) == AON_ADC_CHOP_MOD_AZ_PGA_ON) || \
                                                          ((type) == AON_ADC_CHOP_MOD_AZ_PGA_RPC_ON))

/** @defgroup  AON_ADC_PGA_VCM_TYPE
 *  @{
 */
#define IS_AON_ADC_PGA_VCM_TYPE(type)                    (((type) == AON_ADC_PGA_VCM_1V) || \
                                                          ((type) == AON_ADC_PGA_VCM_1P2V) || \
                                                          ((type) == AON_ADC_PGA_VCM_1P4V) || \
                                                          ((type) == AON_ADC_PGA_VCM_1P6V))

/** @defgroup  AON_ADC_TSEN_MOD_TYPE
 *  @{
 */
#define IS_AON_ADC_TSEN_MOD_TYPE(type)                   (((type) == AON_ADC_TSEN_MOD_INTERNAL_DIODE) || \
                                                          ((type) == AON_ADC_TSEN_MOD_EXTERNAL_DIODE))

/** @defgroup  AON_ADC_VREF_TYPE
 *  @{
 */
#define IS_AON_ADC_VREF_TYPE(type)                       (((type) == AON_ADC_VREF_3P3V) || \
                                                          ((type) == AON_ADC_VREF_2V))

/** @defgroup  AON_ADC_SIG_INPUT_TYPE
 *  @{
 */
#define IS_AON_ADC_SIG_INPUT_TYPE(type)                  (((type) == AON_ADC_INPUT_SINGLE_END) || \
                                                          ((type) == AON_ADC_INPUT_DIFF))

/** @defgroup  AON_ADC_DATA_WIDTH_TYPE
 *  @{
 */
#define IS_AON_ADC_DATA_WIDTH_TYPE(type)                 (((type) == AON_ADC_DATA_WIDTH_12) || \
                                                          ((type) == AON_ADC_DATA_WIDTH_14_WITH_16_AVERAGE) || \
                                                          ((type) == AON_ADC_DATA_WIDTH_16_WITH_64_AVERAGE) || \
                                                          ((type) == AON_ADC_DATA_WIDTH_16_WITH_256_AVERAGE))

/** @defgroup  AON_CHANNEL_TYPE
 *  @{
 */
#define IS_AON_CHANNEL_TYPE(type)                        (((type) == AON_CHANNEL_POS) || \
                                                          ((type) == AON_CHANNEL_NEG))

/** @defgroup  AON_ADC_MICBOOST_DB_TYPE
 *  @{
 */
#define IS_AON_ADC_MICBOOST_DB_TYPE(type)                (((type) == AON_ADC_MICBOOST_DB_16DB) || \
                                                          ((type) == AON_ADC_MICBOOST_DB_32DB))

/** @defgroup  AON_ADC_PGA2_GAIN_TYPE
 *  @{
 */
#define IS_AON_ADC_PGA2_GAIN_TYPE(type)                  (((type) == AON_ADC_PGA2_GAIN_0DB) || \
                                                          ((type) == AON_ADC_PGA2_GAIN_6DB) || \
                                                          ((type) == AON_ADC_PGA2_GAIN_N6DB) || \
                                                          ((type) == AON_ADC_PGA2_GAIN_12DB))

/** @defgroup  AON_ADC_MIC_MODE_TYPE
 *  @{
 */
#define IS_AON_ADC_MIC_MODE_TYPE(type)                   (((type) == AON_ADC_MIC_MODE_SINGLE) || \
                                                          ((type) == AON_ADC_MIC_MODE_DIFF))

/** @defgroup  GPIP_ADC_FIFO_THRESHOLD_TYPE
 *  @{
 */
#define IS_GPIP_ADC_FIFO_THRESHOLD_TYPE(type)            (((type) == GPIP_ADC_FIFO_THRESHOLD_1) || \
                                                          ((type) == GPIP_ADC_FIFO_THRESHOLD_4) || \
                                                          ((type) == GPIP_ADC_FIFO_THRESHOLD_8) || \
                                                          ((type) == GPIP_ADC_FIFO_THRESHOLD_16))

/** @defgroup  GPIP_ADC_INT_TYPE
 *  @{
 */
#define IS_GPIP_ADC_INT_TYPE(type)                       (((type) == GPIP_ADC_INT_FIFO_UNDERRUN) || \
                                                          ((type) == GPIP_ADC_INT_FIFO_OVERRUN) || \
                                                          ((type) == GPIP_ADC_INT_ADC_READY) || \
                                                          ((type) == GPIP_ADC_INT_ALL))

/*@} end of group ADC_Public_Constants */

/** @defgroup  ADC_Public_Macros
 *  @{
 */

/*@} end of group ADC_Public_Macros */

/** @defgroup  ADC_Public_Functions
 *  @{
 */
void AON_ADC_Sensor_Init(AON_ADC_SENSOR_Type* senCfg);
void AON_ADC_Micbias_Init(AON_ADC_MIC_Type* micCfg);
void AON_ADC_Vbat_Enable(void);
void AON_ADC_Vbat_Disable(void);
void AON_ADC_Reset(void);
void AON_ADC_Global_Enable(void);
void AON_ADC_Global_Disable(void);
void AON_ADC_Normal_Config(AON_ADC_Normal_CFG_Type* nCfg);
void AON_ADC_Normal_Channel_Config(AON_ADC_Chan_Type posCh,AON_ADC_Chan_Type negCh,BL_Fun_Type contEn);
void AON_ADC_Normal_Start(void);
void AON_ADC_Normal_Stop(void);
void AON_ADC_Scan_Config(AON_ADC_SCAN_CFG_Type* sCfg);
void AON_ADC_Scan_Channel_Config(AON_ADC_Chan_Type posChList[],AON_ADC_Chan_Type negChList[],uint8_t scanLength,
                                 BL_Fun_Type contEn);
void AON_ADC_Scan_Start(void);
void AON_ADC_Scan_Stop(void);
void AON_ADC_Status_Mask(AON_CHANNEL_Type chType, BL_Mask_Type statusMask);
void AON_ADC_Status_Clr(AON_CHANNEL_Type chType);
BL_Sts_Type AON_ADC_Get_Status(AON_CHANNEL_Type chType);
void GPIP_ADC_FIFO_Cfg(GPIP_ADC_FIFO_Cfg_Type *fifoCfg);
uint8_t GPIP_ADC_Get_FIFO_Count(void);
void GPIP_ADC_IntMask(GPIP_ADC_INT_Type intType, BL_Mask_Type intMask);
void GPIP_ADC_IntClr(GPIP_ADC_INT_Type intType);
BL_Sts_Type GPIP_ADC_GetIntStatus(GPIP_ADC_INT_Type intType);
BL_Sts_Type GPIP_ADC_FIFO_Full(void);
BL_Sts_Type GPIP_ADC_FIFO_Empty(void);
uint32_t GPIP_ADC_Read_FIFO(void);
void GPIP_ADC_Int_Callback_Install(GPIP_ADC_INT_Type intType,intCallback_Type* cbFun);

/*@} end of group ADC_Public_Functions */

/*@} end of group ADC */

/*@} end of group BL602_Peripheral_Driver */

#endif /* __BL602_ADC_H__ */
