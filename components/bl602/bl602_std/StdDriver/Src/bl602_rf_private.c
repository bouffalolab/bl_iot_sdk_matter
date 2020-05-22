/**
  ******************************************************************************
  * @file    bl602_rf_private.c
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

#include "bl602_rf_private.h"
#include "bl602_common.h"
#include "bl602_rf_calib_data.h"

/** @addtogroup  BL602_Peripheral_Driver
 *  @{
 */

/** @addtogroup  RF_PRIVATE
 *  @{
 */

/** @defgroup  RF_PRIVATE_Private_Macros
 *  @{
 */
#define BL602_DBG (0)
// #define printf 	dbg_test_print

#define CNT_TIMES           (40)
#define NUM_CHANNELS        (14)

#define XTAL24M (0)
#define XTAL26M (0)
#define XTAL32M (0)
#define XTAL38P4M (0)
#define XTAL40M (1)
#define XTAL52M (0)


/*@} end of group RF_PRIVATE_Private_Macros */

/** @defgroup  RF_PRIVATE_Private_Types
 *  @{
 */

/*@} end of group RF_PRIVATE_Private_Types */

/** @defgroup  RF_PRIVATE_Private_Variables
 *  @{
 */
static uint32_t state_rf_fsm_ctrl_hw;
static uint32_t state_rfctrl_hw_en;
static uint32_t state_rfcal_ctrlen;
static uint32_t state_pucr1;
static uint32_t state_fbdv;
static uint32_t state_sdm1;
static uint32_t state_sdm2;

static uint32_t channel_div_table[E_RF_CHANNEL_NUM] = {
#if XTAL38P4M
    0x14F00000 , /* Channel 2412 MHz */
    0x14FB1C71 , /* Channel 2417 MHz */
    0x150638E3 , /* Channel 2422 MHz */
    0x15115555 , /* Channel 2427 MHz */
    0x151C71C7 , /* Channel 2432 MHz */
    0x15278E38 , /* Channel 2437 MHz */
    0x1532AAAA , /* Channel 2442 MHz */
    0x153DC71C , /* Channel 2447 MHz */
    0x1548E38E , /* Channel 2452 MHz */
    0x15540000 , /* Channel 2457 MHz */
    0x155F1C71 , /* Channel 2462 MHz */
    0x156A38E3 , /* Channel 2467 MHz */
    0x15755555 , /* Channel 2472 MHz */
    0x15900000 , /* Channel 2484 MHz */
    0x15C00000 , /* Channel 2505.6 MHz */
#endif
#if XTAL24M
    0x21800000 , /* Channel 2412 MHz */
    0x2191C71C , /* Channel 2417 MHz */
    0x21A38E38 , /* Channel 2422 MHz */
    0x21B55555 , /* Channel 2427 MHz */
    0x21C71C71 , /* Channel 2432 MHz */
    0x21D8E38E , /* Channel 2437 MHz */
    0x21EAAAAA , /* Channel 2442 MHz */
    0x21FC71C7 , /* Channel 2447 MHz */
    0x220E38E3 , /* Channel 2452 MHz */
    0x22200000 , /* Channel 2457 MHz */
    0x2231C71C , /* Channel 2462 MHz */
    0x22438E38 , /* Channel 2467 MHz */
    0x22555555 , /* Channel 2472 MHz */
    0x22800000 , /* Channel 2484 MHz */
    0x22CCCCCC , /* Channel 2505.6 MHz */
#endif
#if XTAL26M
    0x1EEC4EC4 , /* Channel 2412 MHz */
    0x1EFCB7CB , /* Channel 2417 MHz */
    0x1F0D20D2 , /* Channel 2422 MHz */
    0x1F1D89D8 , /* Channel 2427 MHz */
    0x1F2DF2DF , /* Channel 2432 MHz */
    0x1F3E5BE5 , /* Channel 2437 MHz */
    0x1F4EC4EC , /* Channel 2442 MHz */
    0x1F5F2DF2 , /* Channel 2447 MHz */
    0x1F6F96F9 , /* Channel 2452 MHz */
    0x1F800000 , /* Channel 2457 MHz */
    0x1F906906 , /* Channel 2462 MHz */
    0x1FA0D20D , /* Channel 2467 MHz */
    0x1FB13B13 , /* Channel 2472 MHz */
    0x1FD89D89 , /* Channel 2484 MHz */
    0x201F81F8 , /* Channel 2505.6 MHz */
#endif
#if XTAL32M
    0x19200000 , /* Channel 2412 MHz */
    0x192D5555 , /* Channel 2417 MHz */
    0x193AAAAA , /* Channel 2422 MHz */
    0x19480000 , /* Channel 2427 MHz */
    0x19555555 , /* Channel 2432 MHz */
    0x1962AAAA , /* Channel 2437 MHz */
    0x19700000 , /* Channel 2442 MHz */
    0x197D5555 , /* Channel 2447 MHz */
    0x198AAAAA , /* Channel 2452 MHz */
    0x19980000 , /* Channel 2457 MHz */
    0x19A55555 , /* Channel 2462 MHz */
    0x19B2AAAA , /* Channel 2467 MHz */
    0x19C00000 , /* Channel 2472 MHz */
    0x19E00000 , /* Channel 2484 MHz */
    0x1A199999 , /* Channel 2505.6 MHz */
#endif
#if XTAL40M
    0x14199999 , /* Channel 2412 MHz */
    0x14244444 , /* Channel 2417 MHz */
    0x142EEEEE , /* Channel 2422 MHz */
    0x14399999 , /* Channel 2427 MHz */
    0x14444444 , /* Channel 2432 MHz */
    0x144EEEEE , /* Channel 2437 MHz */
    0x14599999 , /* Channel 2442 MHz */
    0x14644444 , /* Channel 2447 MHz */
    0x146EEEEE , /* Channel 2452 MHz */
    0x14799999 , /* Channel 2457 MHz */
    0x14844444 , /* Channel 2462 MHz */
    0x148EEEEE , /* Channel 2467 MHz */
    0x14999999 , /* Channel 2472 MHz */
    0x14B33333 , /* Channel 2484 MHz */
    0x14E147AE , /* Channel 2505.6 MHz */
#endif
#if XTAL52M
    0xF762762 , /* Channel 2412 MHz */
    0xF7E5BE5 , /* Channel 2417 MHz */
    0xF869069 , /* Channel 2422 MHz */
    0xF8EC4EC , /* Channel 2427 MHz */
    0xF96F96F , /* Channel 2432 MHz */
    0xF9F2DF2 , /* Channel 2437 MHz */
    0xFA76276 , /* Channel 2442 MHz */
    0xFAF96F9 , /* Channel 2447 MHz */
    0xFB7CB7C , /* Channel 2452 MHz */
    0xFC00000 , /* Channel 2457 MHz */
    0xFC83483 , /* Channel 2462 MHz */
    0xFD06906 , /* Channel 2467 MHz */
    0xFD89D89 , /* Channel 2472 MHz */
    0xFEC4EC4 , /* Channel 2484 MHz */
    0x100FC0FC , /* Channel 2505.6 MHz */
#endif
};

static uint16_t channel_cnt_table[NUM_CHANNELS] = {
#if XTAL38P4M
    0xa780, //2412MHz
    0xa7d8, //2417MHz
    0xa831, //2422MHz
    0xa88a, //2427MHz
    0xa8e3, //2432MHz
    0xa93c, //2437MHz
    0xa995, //2442MHz
    0xa9ee, //2447MHz
    0xaa47, //2452MHz
    0xaaa0, //2457MHz
    0xaaf8, //2462MHz
    0xab51, //2467MHz
    0xabaa, //2472MHz
    0xac80  //2484MHz
#endif
#if XTAL24M
    0xa780, //2412MHz
    0xa7d8, //2417MHz
    0xa831, //2422MHz
    0xa88a, //2427MHz
    0xa8e3, //2432MHz
    0xa93c, //2437MHz
    0xa995, //2442MHz
    0xa9ee, //2447MHz
    0xaa47, //2452MHz
    0xaaa0, //2457MHz
    0xaaf8, //2462MHz
    0xab51, //2467MHz
    0xabaa, //2472MHz
    0xac80  //2484MHz
#endif
#if XTAL26M
    0xa78a, //2412MHz
    0xa7e3, //2417MHz
    0xa83c, //2422MHz
    0xa895, //2427MHz
    0xa8ed, //2432MHz
    0xa946, //2437MHz
    0xa99f, //2442MHz
    0xa9f8, //2447MHz
    0xaa51, //2452MHz
    0xaaaa, //2457MHz
    0xab03, //2462MHz
    0xab5c, //2467MHz
    0xabb5, //2472MHz
    0xac8a  //2484MHz
#endif
#if XTAL32M
    0xa788, //2412MHz
    0xa7e1, //2417MHz
    0xa83a, //2422MHz
    0xa893, //2427MHz
    0xa8ec, //2432MHz
    0xa944, //2437MHz
    0xa99d, //2442MHz
    0xa9f6, //2447MHz
    0xaa4f, //2452MHz
    0xaaa8, //2457MHz
    0xab01, //2462MHz
    0xab5a, //2467MHz
    0xabb3, //2472MHz
    0xac88  //2484MHz
#endif
#if XTAL40M
    0xA779, //2412MHz
    0xA7D2, //2417MHz
    0xA82B, //2422MHz
    0xA883, //2427MHz
    0xA8DC, //2432MHz
    0xA935, //2437MHz
    0xA98E, //2442MHz
    0xA9E7, //2447MHz
    0xAA40, //2452MHz
    0xAA99, //2457MHz
    0xAAF2, //2462MHz
    0xAB4A, //2467MHz
    0xABA3, //2472MHz
    0xAC79  //2484MHz
#endif
#if XTAL52M
    0xA77A, //2412MHz
    0xA7D3, //2417MHz
    0xA82C, //2422MHz
    0xA885, //2427MHz
    0xA8DE, //2432MHz
    0xA937, //2437MHz
    0xA990, //2442MHz
    0xA9E8, //2447MHz
    0xAA41, //2452MHz
    0xAA9A, //2457MHz
    0xAAF3, //2462MHz
    0xAB4C, //2467MHz
    0xABA5, //2472MHz
    0xAC7A  //2484MHz
#endif
};

static uint16_t channel_cw_table[NUM_CHANNELS];
static uint16_t channel_cnt_opt_table[CNT_TIMES];
/*@} end of group RF_PRIVATE_Private_Variables */

/** @defgroup  RF_PRIVATE_Global_Variables
 *  @{
 */

/*@} end of group RF_PRIVATE_Global_Variables */

/** @defgroup  RF_PRIVATE_Private_Fun_Declaration
 *  @{
 */

/*@} end of group RF_PRIVATE_Private_Fun_Declaration */

/** @defgroup  RF_PRIVATE_Private_Functions
 *  @{
 */

/*@} end of group RF_PRIVATE_Private_Functions */

/** @defgroup  RF_PRIVATE_Public_Functions
 *  @{
 */
uint32_t rf_pri_channel_freq_to_index(uint32_t freq)
{
    if (freq == 2412) return E_RF_CHANNEL_2412M;
    if (freq == 2417) return E_RF_CHANNEL_2417M;
    if (freq == 2422) return E_RF_CHANNEL_2422M;
    if (freq == 2427) return E_RF_CHANNEL_2427M;
    if (freq == 2432) return E_RF_CHANNEL_2432M;
    if (freq == 2437) return E_RF_CHANNEL_2437M;
    if (freq == 2442) return E_RF_CHANNEL_2442M;
    if (freq == 2447) return E_RF_CHANNEL_2447M;
    if (freq == 2452) return E_RF_CHANNEL_2452M;
    if (freq == 2457) return E_RF_CHANNEL_2457M;
    if (freq == 2462) return E_RF_CHANNEL_2462M;
    if (freq == 2467) return E_RF_CHANNEL_2467M;
    if (freq == 2472) return E_RF_CHANNEL_2472M;
    if (freq == 2484) return E_RF_CHANNEL_2484M;
    if (freq == 2505) return E_RF_CHANNEL_2505P6M;
    return E_RF_CHANNEL_2412M;
}

void rf_pri_wait_us(uint32_t us)
{
	BL602_Delay_US(us);
}

void rf_pri_wait_ms(uint32_t ms)
{
    BL602_Delay_MS(ms);
}

void rf_pri_init(void)
{
    uint32_t tmpVal = 0;

    tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_SFREG);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_ADDA_LDO);
    BL_WR_REG(RF_BASE,RF_PUCR1,tmpVal);

    tmpVal=BL_RD_REG(AON_BASE,AON_XTAL_CFG);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_XTAL_CAPCODE_IN_AON,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,AON_XTAL_CAPCODE_OUT_AON,32);
    BL_WR_REG(AON_BASE,AON_XTAL_CFG,tmpVal);
    rf_pri_wait_ms(10);

    tmpVal=BL_RD_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_48M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_80M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_96M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_120M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_160M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_192M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_240M);
    tmpVal=BL_SET_REG_BIT(tmpVal,PDS_CLKPLL_EN_480M);
    BL_WR_REG(PDS_BASE,PDS_CLKPLL_OUTPUT_EN,tmpVal);

    rf_pri_init_calib_mem();
    rf_pri_full_cal();

    rf_pri_config_bandwidth(E_RF_BW_20M);
}

void rf_pri_config_mode(uint32_t mode)
{
    rf_pri_manu_pu(mode);
}

void rf_pri_config_bandwidth(uint32_t bw)
{
    uint32_t tmpVal = 0;
    // Set RF RBB bandwidth, 0: n/a, 1: 5M mode, 2:10M mode, 3: 20M mode
    tmpVal=BL_RD_REG(RF_BASE,RF_RBB3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RBB_BW,bw);
    BL_WR_REG(RF_BASE,RF_RBB3,tmpVal);
}

void rf_pri_config_channel(uint32_t channel_index)
{
    uint32_t tmpVal = 0;
    // power up LO
    tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_VCO);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_FBDV);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_PFD);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_OSMX);
    BL_WR_REG(RF_BASE,RF_PUCR1,tmpVal);

    // set channel
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_VCO_FREQ_CW,rf_calib_data->lo[channel_index-1].fcal);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_VCO_IDAC_CW,rf_calib_data->lo[channel_index-1].acal);
    BL_WR_REG(RF_BASE,RF_VCO1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_LODIST);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_OSMX_CAP,rf_calib_data->lo[channel_index-1].fcal >> 4);
    BL_WR_REG(RF_BASE,RF_LODIST,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_SDM2);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_SDMIN,channel_div_table[channel_index-1]);
    BL_WR_REG(RF_BASE,RF_SDM2,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_SDM1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_SDM_BYPASS,(channel_index == E_RF_CHANNEL_2505P6M));
    BL_WR_REG(RF_BASE,RF_SDM1,tmpVal);

    // wait channel lock
    while (1) {
        tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
        tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_FBDV_RST);
        BL_WR_REG(RF_BASE,RF_FBDV,tmpVal);
        rf_pri_wait_us(10);
        tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
        tmpVal=BL_CLR_REG_BIT(tmpVal,RF_LO_FBDV_RST);
        BL_WR_REG(RF_BASE,RF_FBDV,tmpVal);
        rf_pri_wait_us(50);
        tmpVal=BL_RD_REG(RF_BASE,RF_PFDCP);
        tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_PFD_RST_CSD);
        BL_WR_REG(RF_BASE,RF_PFDCP,tmpVal);
        rf_pri_wait_us(10);
        tmpVal=BL_RD_REG(RF_BASE,RF_PFDCP);
        tmpVal=BL_CLR_REG_BIT(tmpVal,RF_LO_PFD_RST_CSD);
        BL_WR_REG(RF_BASE,RF_PFDCP,tmpVal);
        rf_pri_wait_us(50);

        tmpVal=BL_RD_REG(RF_BASE,RF_LO);
        if (BL_GET_REG_BITS_VAL(tmpVal,RF_LO_SLIPPED_DN) || BL_GET_REG_BITS_VAL(tmpVal,RF_LO_SLIPPED_UP)) {
            bflb_platform_prints(".");
        }
        else {
            //dbg_test_print("LO locked %d 0x%x\r\n", channel_index, mixed_reg->vco.BF.lo_freq_cw);
            break;
        }
    }
}

void rf_pri_manu_pu(uint32_t mode)
{
    uint32_t tmpVal = 0;
    tmpVal=BL_RD_REG(RF_BASE,RF_FSM_CTRL_HW);
    tmpVal=BL_CLR_REG_BIT(tmpVal,RF_FSM_CTRL_EN);
    BL_WR_REG(RF_BASE,RF_FSM_CTRL_HW,tmpVal);
    BL_WR_REG(RF_BASE,RFCTRL_HW_EN,0);
    
    switch (mode) {
        case E_RF_MODE_LO_FCAL:
            tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);           
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_LNA);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RMXGM);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RMX);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RBB);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_ADC);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_ADC_CLK_EN);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_PKDET);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_ROSDAC);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_PWRMX);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_PA);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TMX);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TBB);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_DAC);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RXBUF);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TXBUF);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_TRSW_EN);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TOSDAC);
            tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_OSMX);
            tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_PFD);
            tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_FBDV);
            tmpVal=BL_SET_REG_BIT(tmpVal,RF_PU_VCO);  
            BL_WR_REG(RF_BASE,RF_PUCR1,tmpVal);                                                 
            break;
        case E_RF_MODE_IDLE:
        default:
            tmpVal=BL_RD_REG(RF_BASE,RF_PUCR1);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_LNA);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RMXGM);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RMX);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RBB);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_ADC);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_ADC_CLK_EN);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_PKDET);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_ROSDAC);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_PWRMX);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_PA);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TMX);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TBB);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_DAC);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_RXBUF);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TXBUF);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_TRSW_EN);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_PU_TOSDAC);
            BL_WR_REG(RF_BASE,RF_PUCR1,tmpVal);
            break;
    }
}

/****************************************************************************//**
 * @brief  RF set auto gain
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void rf_pri_auto_gain(void)
{
    uint32_t tmpVal = 0;

    tmpVal = BL_RD_REG(RF_BASE,RFCTRL_HW_EN);
    /* Because RF_RX_GAIN_CTRL_HW is only one bit, we can use BL_SET_REG_BIT
     * or BL_CLR_REG_BIT */
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_RX_GAIN_CTRL_HW);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_TX_GAIN_CTRL_HW);
    BL_WR_REG(RF_BASE,RFCTRL_HW_EN,tmpVal);
}

void rf_pri_save_state_for_cal()
{
    state_rf_fsm_ctrl_hw = BL_RD_REG(RF_BASE,RF_FSM_CTRL_HW);
    state_rfctrl_hw_en = BL_RD_REG(RF_BASE,RFCTRL_HW_EN);
    state_rfcal_ctrlen = BL_RD_REG(RF_BASE,RFCAL_CTRLEN);
    state_pucr1 = BL_RD_REG(RF_BASE,RF_PUCR1);
    state_fbdv = BL_RD_REG(RF_BASE,RF_FBDV);  
    state_sdm1 = BL_RD_REG(RF_BASE,RF_SDM1);
    state_sdm2 = BL_RD_REG(RF_BASE,RF_SDM2);
}

void rf_pri_restore_state_for_cal()
{
    BL_WR_REG(RF_BASE,RF_FSM_CTRL_HW,state_rf_fsm_ctrl_hw);
    BL_WR_REG(RF_BASE,RFCTRL_HW_EN,state_rfctrl_hw_en);
    BL_WR_REG(RF_BASE,RFCAL_CTRLEN,state_rfcal_ctrlen);
    BL_WR_REG(RF_BASE,RF_PUCR1,state_pucr1);
    BL_WR_REG(RF_BASE,RF_FBDV,state_fbdv);  
    BL_WR_REG(RF_BASE,RF_SDM1,state_sdm1);
    BL_WR_REG(RF_BASE,RF_SDM2,state_sdm2);
}

uint16_t rf_pri_fcal_meas(uint32_t cw)
{
    uint32_t tmpVal = 0;
    uint16_t cnt;

    tmpVal=BL_RD_REG(RF_BASE,RF_VCO1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_VCO_FREQ_CW,cw);
    BL_WR_REG(RF_BASE,RF_VCO1,tmpVal);
    rf_pri_wait_us(100);
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO4);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_FCAL_CNT_START);
    BL_WR_REG(RF_BASE,RF_VCO4,tmpVal);
    while (1)
    {
        tmpVal=BL_RD_REG(RF_BASE,RF_VCO4);
        if (BL_GET_REG_BITS_VAL(tmpVal,RF_FCAL_CNT_RDY))
        {
            break;
        }
    }
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    cnt = BL_GET_REG_BITS_VAL(tmpVal,RF_FCAL_CNT_OP);
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO4);
    tmpVal=BL_CLR_REG_BIT(tmpVal,RF_FCAL_CNT_START);
    BL_WR_REG(RF_BASE,RF_VCO4,tmpVal);

    return cnt;
}

void rf_pri_fcal(void)
{
    uint32_t tmpVal = 0;
    int16_t i,n;
    uint16_t cw,cw_binary;
    uint16_t cnt;
    uint16_t bit_width = 8;
    uint16_t count1 = 0xa700;    //0xa715
    uint16_t count2 = 0xa779;		//Adjust the lower boundary (0xa780 --> 0xa779)  --Ling 2018/10/13
    uint16_t count3 = 0xac8a;		//Adjust the upper boundary (0xac80 --> 0xac8a)  --Ling 2018/10/13

    tmpVal=BL_RD_REG(RF_BASE,RFCAL_STATUS);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_STATUS,1);
    BL_WR_REG(RF_BASE,RFCAL_STATUS,tmpVal);
    
    // Save for calibration
    rf_pri_save_state_for_cal();
    
    // Setup for lo fcal
    rf_pri_config_mode(E_RF_MODE_LO_FCAL);
    tmpVal=BL_RD_REG(RF_BASE,RFCAL_CTRLEN);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_FCAL_EN);
    BL_WR_REG(RF_BASE,RFCAL_CTRLEN,tmpVal);

    cw = 1 << (bit_width-1);
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_VCO_FREQ_CW,cw);
    BL_WR_REG(RF_BASE,RF_VCO1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_LO_FBDV_SEL_FB_CLK,0);
    BL_WR_REG(RF_BASE,RF_FBDV,tmpVal);
#if XTAL24M
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_DIV,0x500);
    BL_WR_REG(RF_BASE,RF_VCO3,tmpVal);
    //0x500:24M		0x56B:26M		0x6AB:32M		0x800:38.4M		0x855:40M		0xAD5:52M	--Ling 2018/10/13
#endif
#if XTAL26M
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_DIV,0x56B);
    BL_WR_REG(RF_BASE,RF_VCO3,tmpVal);
    //0x500:24M		0x56B:26M		0x6AB:32M		0x800:38.4M		0x855:40M		0xAD5:52M	--Ling 2018/10/13
#endif
#if XTAL32M
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_DIV,0x6AB);
    BL_WR_REG(RF_BASE,RF_VCO3,tmpVal);
    //0x500:24M		0x56B:26M		0x6AB:32M		0x800:38.4M		0x855:40M		0xAD5:52M	--Ling 2018/10/13
#endif
#if XTAL38P4M
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_DIV,0x800);
    BL_WR_REG(RF_BASE,RF_VCO3,tmpVal);
    //0x500:24M		0x56B:26M		0x6AB:32M		0x800:38.4M		0x855:40M		0xAD5:52M	--Ling 2018/10/13
#endif
#if XTAL40M
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_DIV,0x855);
    BL_WR_REG(RF_BASE,RF_VCO3,tmpVal);
    //0x500:24M		0x56B:26M		0x6AB:32M		0x800:38.4M		0x855:40M		0xAD5:52M	--Ling 2018/10/13
#endif
#if XTAL52M
    tmpVal=BL_RD_REG(RF_BASE,RF_VCO3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_DIV,0xAD5);
    BL_WR_REG(RF_BASE,RF_VCO3,tmpVal);
    //0x500:24M		0x56B:26M		0x6AB:32M		0x800:38.4M		0x855:40M		0xAD5:52M	--Ling 2018/10/13
#endif
    BL_WR_REG(RF_BASE,RF_SDM2,0x1000000);
    tmpVal=BL_RD_REG(RF_BASE,RF_SDM1);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_SDM_BYPASS);
    BL_WR_REG(RF_BASE,RF_SDM1,tmpVal);

    tmpVal=BL_RD_REG(RF_BASE,RF_SDM1);
    tmpVal=BL_CLR_REG_BIT(tmpVal,RF_LO_SDM_RSTB);
    BL_WR_REG(RF_BASE,RF_SDM1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_FBDV_RST);
    BL_WR_REG(RF_BASE,RF_FBDV,tmpVal);
    rf_pri_wait_us(10);
    tmpVal=BL_RD_REG(RF_BASE,RF_SDM1);
    tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_SDM_RSTB);
    BL_WR_REG(RF_BASE,RF_SDM1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
    tmpVal=BL_CLR_REG_BIT(tmpVal,RF_LO_FBDV_RST);
    BL_WR_REG(RF_BASE,RF_FBDV,tmpVal); 
    rf_pri_wait_us(50);

    // Binary search to find initial channel
		while(1){
		    for (i = 1; i < bit_width; i++) {
				    cnt = rf_pri_fcal_meas(cw);
				    if (cnt >= count1 && cnt <= count2) {
						    break;
				    }
				    else if (cnt < count1) {
						    cw -= 1 << (bit_width-1-i);
				    }
				    else if (cnt > count2) {
						    cw += 1 << (bit_width-1-i);
				    }
		    }
				if (cw < 15) {
            bflb_platform_printf("Unexpected cw %d\r\n",(int)cw);
            cw = 1 << (bit_width-1);
            i = 1;
            tmpVal=BL_RD_REG(RF_BASE,RF_SDM1);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_LO_SDM_RSTB);
            BL_WR_REG(RF_BASE,RF_SDM1,tmpVal);
            tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
            tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_FBDV_RST);
            BL_WR_REG(RF_BASE,RF_FBDV,tmpVal);
            rf_pri_wait_us(50);
            tmpVal=BL_RD_REG(RF_BASE,RF_SDM1);
            tmpVal=BL_SET_REG_BIT(tmpVal,RF_LO_SDM_RSTB);
            BL_WR_REG(RF_BASE,RF_SDM1,tmpVal);
            tmpVal=BL_RD_REG(RF_BASE,RF_FBDV);
            tmpVal=BL_CLR_REG_BIT(tmpVal,RF_LO_FBDV_RST);
            BL_WR_REG(RF_BASE,RF_FBDV,tmpVal);
            rf_pri_wait_us(50);
        } else {
            break;
        }
		}
    cw = cw + 1;
    cw_binary = cw;
    cnt = rf_pri_fcal_meas(cw);

    // Store optim cnt table
    channel_cnt_opt_table[0] = cnt;
    for (i = 1; i < CNT_TIMES; i++) {
        cnt = rf_pri_fcal_meas(--cw);
        channel_cnt_opt_table[i] = cnt;
        if (cnt > count3) break;
    } 

    // Find cw for each channel
    for (i = 0,n = 0; i < NUM_CHANNELS; i++) {
        while (channel_cnt_table[i]>channel_cnt_opt_table[n]) n++;
        if ((channel_cnt_table[i]-channel_cnt_opt_table[n-1]) <
            (channel_cnt_opt_table[n]-channel_cnt_table[i])) {
            channel_cw_table[i] = cw_binary - (n-1);
        }
        else {
            channel_cw_table[i] = cw_binary - n;
        }
        n = n > 0 ? n-1 : 0;
    }
    
    // Set osmx register
    //mixed_reg->osmx.BF.osmx_cap = (channel_cw_table[6]>>3)&0b1111;
    
    // Restore state
    rf_pri_restore_state_for_cal();

    // Save calibration result
    for (i = 0; i < NUM_CHANNELS; i++) {
        rf_calib_data->lo[i].fcal = channel_cw_table[i];
    }
    tmpVal=BL_RD_REG(RF_BASE,RFCAL_STATUS);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_FCAL_STATUS,3);
    BL_WR_REG(RF_BASE,RFCAL_STATUS,tmpVal);
}

void rf_pri_full_cal(void)
{
    rf_pri_fcal();
    #if BL602_DBG
    rf_pri_lo_acal();
	
    rf_pri_roscal();
    rf_pri_rccal();
    
    rf_pri_config_channel(E_RF_CHANNEL_2442M);
    rf_pri_rxcal();    
    rf_pri_txcal();
    rf_pri_auto_gain();
    #endif
    rf_pri_config_channel(E_RF_CHANNEL_2442M);
    rf_pri_auto_gain();
}

void rf_pri_set_cal_reg(void)
{
    uint32_t tmpVal = 0;

    tmpVal=BL_RD_REG(RF_BASE,RF_ROSDAC_CTRL_HW1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_I_GC0,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_Q_GC0,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_I_GC1,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_Q_GC1,32);    
    BL_WR_REG(RF_BASE,RF_ROSDAC_CTRL_HW1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_ROSDAC_CTRL_HW2);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_I_GC2,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_Q_GC2,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_I_GC3,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_ROSDAC_Q_GC3,32);
    BL_WR_REG(RF_BASE,RF_ROSDAC_CTRL_HW2,tmpVal);
    
    tmpVal=BL_RD_REG(RF_BASE,RF_RXIQ_CTRL_HW1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_GAIN_COMP_GC0,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_PHASE_COMP_GC0,0);    
    BL_WR_REG(RF_BASE,RF_RXIQ_CTRL_HW1,tmpVal);  
    tmpVal=BL_RD_REG(RF_BASE,RF_RXIQ_CTRL_HW2);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_GAIN_COMP_GC1,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_PHASE_COMP_GC1,0);
    BL_WR_REG(RF_BASE,RF_RXIQ_CTRL_HW2,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_RXIQ_CTRL_HW3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_GAIN_COMP_GC2,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_PHASE_COMP_GC2,0);
    BL_WR_REG(RF_BASE,RF_RXIQ_CTRL_HW3,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_RXIQ_CTRL_HW4);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_GAIN_COMP_GC3,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RX_IQ_PHASE_COMP_GC3,0);
    BL_WR_REG(RF_BASE,RF_RXIQ_CTRL_HW4,tmpVal);                  

    // rf_calib_data->cal.rx_i = 0;
    // rf_calib_data->cal.rx_q = 0;
    tmpVal=BL_RD_REG(RF_BASE,RF_RBB2);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RBB_CAP1_FC_I,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RBB_CAP2_FC_I,32);    
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RBB_CAP1_FC_Q,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_RBB_CAP2_FC_Q,32);
    BL_WR_REG(RF_BASE,RF_RBB2,tmpVal);    

    tmpVal=BL_RD_REG(RF_BASE,RF_TOSDAC_CTRL_HW1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC0,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC0,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC1,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC1,32);
    BL_WR_REG(RF_BASE,RF_TOSDAC_CTRL_HW1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TOSDAC_CTRL_HW2);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC2,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC2,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC3,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC3,32);
    BL_WR_REG(RF_BASE,RF_TOSDAC_CTRL_HW2,tmpVal); 
    tmpVal=BL_RD_REG(RF_BASE,RF_TOSDAC_CTRL_HW3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC4,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC4,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC5,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC5,32);
    BL_WR_REG(RF_BASE,RF_TOSDAC_CTRL_HW3,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TOSDAC_CTRL_HW4);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC6,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC6,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_I_GC7,32);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TBB_TOSDAC_Q_GC7,32);
    BL_WR_REG(RF_BASE,RF_TOSDAC_CTRL_HW4,tmpVal);
    
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW0);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC0,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC0,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW0,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW1);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC1,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC1,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW1,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW2);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC2,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC2,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW2,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW3);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC3,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC3,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW3,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW4);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC4,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC4,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW4,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW5);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC5,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC5,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW5,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW6);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC6,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC6,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW6,tmpVal);
    tmpVal=BL_RD_REG(RF_BASE,RF_TX_IQ_GAIN_HW7);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_GAIN_COMP_GC7,1024);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,RF_TX_IQ_PHASE_COMP_GC7,0);    
    BL_WR_REG(RF_BASE,RF_TX_IQ_GAIN_HW7,tmpVal);
    // rf_calib_data->cal.tx_dc_comp_i = 0;
    // rf_calib_data->cal.tx_dc_comp_q = 0;
}

/*@} end of group L1C_Public_Functions */

/*@} end of group L1C */

/*@} end of group BL602_Peripheral_Driver */
