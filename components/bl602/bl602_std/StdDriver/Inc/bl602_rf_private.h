/**
  ******************************************************************************
  * @file    bl602_rf_private.h
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
#ifndef __BL602_RF_PRIVATE_H__
#define __BL602_RF_PRIVATE_H__

#include "rf_reg.h"
#include "pds_reg.h"
#include "aon_reg.h"
#include "bl602_common.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  RF_PRIVATE
 *  @{
 */

/** @defgroup  RF_PRIVATE_Public_Types
 *  @{
 */

/*@} end of group L1C_Public_Types */

/** @defgroup  L1C_Public_Constants
 *  @{
 */
enum {
    E_RF_BW_NA = 0,
    E_RF_BW_10M = 1,  
    E_RF_BW_20M = 2,
    E_RF_BW_40M = 3
};

enum {
    E_RF_GC_RRF_12DB = 0,
    E_RF_GC_RRF_18DB = 1,
    E_RF_GC_RRF_24DB = 2,
    E_RF_GC_RRF_30DB = 3,
    E_RF_GC_RRF_36DB = 4,
    E_RF_GC_RRF_42DB = 5,
    E_RF_GC_RRF_48DB = 6,
    E_RF_GC_RRF_54DB = 7,
    E_RF_GC_RRF_MIN = E_RF_GC_RRF_12DB,
    E_RF_GC_RRF_MAX = E_RF_GC_RRF_54DB
};

enum {
    E_RF_GC_RBB1_N6DB = 0,
    E_RF_GC_RBB1_0DB = 1,
    E_RF_GC_RBB1_6DB = 2,
    E_RF_GC_RBB1_12DB = 3,
    E_RF_GC_RBB1_MIN = E_RF_GC_RBB1_N6DB,
    E_RF_GC_RBB1_MAX = E_RF_GC_RBB1_12DB
};

enum {
    E_RF_GC_RBB2_N6DB = 0,
    E_RF_GC_RBB2_N4DB,
    E_RF_GC_RBB2_N2DB,
    E_RF_GC_RBB2_0DB,
    E_RF_GC_RBB2_2DB,
    E_RF_GC_RBB2_4DB,
    E_RF_GC_RBB2_6DB,
    E_RF_GC_RBB2_8DB,
    E_RF_GC_RBB2_MIN = E_RF_GC_RBB2_N6DB,
    E_RF_GC_RBB2_MAX = E_RF_GC_RBB2_8DB
};

enum {
    E_RF_DAC_IN_PAD = 0,
    E_RF_DAC_IN_SRAM = 1,
    E_RF_DAC_IN_TXSIN = 2,
    E_RF_DAC_IN_ADCLOOP = 3
};

enum {
    E_RF_TXSIN_PHASE_SAME = 0,
    E_RF_TXSIN_PHASE_ORTH = 1
};

enum {
    E_RF_GC_TBB_0DB = 0,
    E_RF_GC_TBB_6DB = 1,
    E_RF_GC_TBB_12DB = 2,
    E_RF_GC_TBB_18DB = 3,
    E_RF_GC_TBB_24DB = 4
};

enum {
    E_RF_GC_TMX_OFF = 0,
    E_RF_GC_TMX_N12DB = 1,
    E_RF_GC_TMX_N6DB = 2,
    E_RF_GC_TMX_MAXGAIN = 3
};

enum {
    E_RF_PM_ACCLEN_16 = 0,
    E_RF_PM_ACCLEN_32,
    E_RF_PM_ACCLEN_64,
    E_RF_PM_ACCLEN_128,
    E_RF_PM_ACCLEN_256,
    E_RF_PM_ACCLEN_512,
    E_RF_PM_ACCLEN_1024,
    E_RF_PM_ACCLEN_2048,
    E_RF_PM_ACCLEN_4096,
    E_RF_PM_ACCLEN_CUSTOM = 15,
    E_RF_PM_ACCLEN_DEFAULT = E_RF_PM_ACCLEN_1024
};

enum {
    E_RF_MODE_IDLE = 0,
    E_RF_MODE_TX,
    E_RF_MODE_RX,
    E_RF_MODE_ROSCAL,
    E_RF_MODE_RCCAL,
    E_RF_MODE_TXCAL,
    E_RF_MODE_LO_ACAL,
    E_RF_MODE_LO_FCAL
};

enum
{
	E_RF_CHANNEL_2412M = 1,
	E_RF_CHANNEL_2417M,
	E_RF_CHANNEL_2422M,
	E_RF_CHANNEL_2427M,
	E_RF_CHANNEL_2432M,
	E_RF_CHANNEL_2437M,
	E_RF_CHANNEL_2442M,
	E_RF_CHANNEL_2447M,
	E_RF_CHANNEL_2452M,
	E_RF_CHANNEL_2457M,
	E_RF_CHANNEL_2462M,
	E_RF_CHANNEL_2467M,
	E_RF_CHANNEL_2472M,
	E_RF_CHANNEL_2484M,
    E_RF_CHANNEL_2505P6M,
    E_RF_CHANNEL_NUM = E_RF_CHANNEL_2505P6M
};

enum {
    E_RF_BRANCH_I = 0,
    E_RF_BRANCH_Q = 1,
    E_RF_GAIN = 2,
    E_RF_PHASE = 3
};

enum {
    E_RF_DISABLE = 0,
    E_RF_ENABLE = 1,
    E_RF_OFF = E_RF_DISABLE,
    E_RF_ON  = E_RF_ENABLE
};

enum {
    E_RF_TXCAL_SEQ_IQGAIN  = 1,
    E_RF_TXCAL_SEQ_IQPHASE = 2,
    E_RF_TXCAL_SEQ_LOL     = 0
};

enum {
    E_RF_RXCAL_GAIN_CNT = 4,
    E_RF_TXCAL_GAIN_CNT = 8
};

/*@} end of group L1C_Public_Constants */

/** @defgroup  L1C_Public_Macros
 *  @{
 */

/*@} end of group L1C_Public_Macros */

/** @defgroup  L1C_Public_Functions
 *  @{
 */
uint32_t rf_pri_channel_freq_to_index(uint32_t freq);
void rf_pri_wait_us(uint32_t us);
void rf_pri_wait_ms(uint32_t ms);
void rf_pri_init(void);
void rf_pri_config_mode(uint32_t mode);
void rf_pri_config_bandwidth(uint32_t bw);
void rf_pri_config_channel(uint32_t channel_index);
void rf_pri_manu_pu(uint32_t mode);
void rf_pri_auto_gain(void);
void rf_pri_save_state_for_cal();
void rf_pri_restore_state_for_cal();
uint16_t rf_pri_fcal_meas(uint32_t cw);
void rf_pri_fcal(void);
void rf_pri_full_cal(void);
void rf_pri_set_cal_reg(void);

/*@} end of group L1C_Public_Functions */

/*@} end of group L1C */

/*@} end of group BL602_Peripheral_Driver */

#endif /* __BL602_L1C_H__ */
