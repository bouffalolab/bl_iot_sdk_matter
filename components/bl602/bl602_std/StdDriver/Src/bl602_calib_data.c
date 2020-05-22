#include <string.h>
#include "bl602_rf_calib_data.h"

#if 0
#define RF_CALIB_DATA_ADDR      (0x40020000)

rf_calib_data_tag* rf_calib_data = (rf_calib_data_tag *)RF_CALIB_DATA_ADDR;
#else
static rf_calib_data_tag data;
rf_calib_data_tag* rf_calib_data = &data;
#endif


void rf_pri_init_calib_mem(void)
{
    memset((void*)rf_calib_data, 0, sizeof(rf_calib_data_tag));
    uint8_t i;
    for(i=0;i<4;i++){
        rf_calib_data->rxcal[i].rosdac_i = 32;
        rf_calib_data->rxcal[i].rosdac_q = 32;   
        rf_calib_data->rxcal[i].rx_iq_gain_comp = 1024;
        rf_calib_data->rxcal[i].rx_iq_phase_comp = 0;             
    }
    rf_calib_data->cal.rx_offset_i = 0;
    rf_calib_data->cal.rx_offset_q = 0;
    rf_calib_data->cal.rbb_cap1_fc_i = 32;
    rf_calib_data->cal.rbb_cap1_fc_q = 32;
    rf_calib_data->cal.rbb_cap2_fc_i = 32;
    rf_calib_data->cal.rbb_cap2_fc_q = 32;
    for(i=0;i<8;i++){
        rf_calib_data->txcal[i].tosdac_i = 32;
        rf_calib_data->txcal[i].tosdac_q = 32;   
        rf_calib_data->txcal[i].tx_iq_gain_comp = 1024;
        rf_calib_data->txcal[i].tx_iq_phase_comp = 0;             
    }    
    rf_calib_data->cal.tx_dc_comp_i = 0;
    rf_calib_data->cal.tx_dc_comp_q = 0;

    rf_calib_data->lo[E_RF_CHANNEL_2412M-1].fcal = 0x7c;
    rf_calib_data->lo[E_RF_CHANNEL_2417M-1].fcal = 0x7b;
    rf_calib_data->lo[E_RF_CHANNEL_2422M-1].fcal = 0x7a;
    rf_calib_data->lo[E_RF_CHANNEL_2427M-1].fcal = 0x79;
    rf_calib_data->lo[E_RF_CHANNEL_2432M-1].fcal = 0x78;
    rf_calib_data->lo[E_RF_CHANNEL_2437M-1].fcal = 0x78;
    rf_calib_data->lo[E_RF_CHANNEL_2442M-1].fcal = 0x77;
    rf_calib_data->lo[E_RF_CHANNEL_2447M-1].fcal = 0x76;
    rf_calib_data->lo[E_RF_CHANNEL_2452M-1].fcal = 0x75;
    rf_calib_data->lo[E_RF_CHANNEL_2457M-1].fcal = 0x74;
    rf_calib_data->lo[E_RF_CHANNEL_2462M-1].fcal = 0x74;
    rf_calib_data->lo[E_RF_CHANNEL_2467M-1].fcal = 0x73;
    rf_calib_data->lo[E_RF_CHANNEL_2472M-1].fcal = 0x72;
    rf_calib_data->lo[E_RF_CHANNEL_2484M-1].fcal = 0x70;
    rf_calib_data->lo[E_RF_CHANNEL_2505P6M-1].fcal = 0x67;

    rf_calib_data->lo[E_RF_CHANNEL_2412M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2417M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2422M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2427M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2432M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2437M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2442M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2447M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2452M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2457M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2462M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2467M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2472M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2484M-1].acal = 0xf;
    rf_calib_data->lo[E_RF_CHANNEL_2505P6M-1].acal = 0xf;
}
