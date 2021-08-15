#ifndef __BL60x_FW_API_H__
#define __BL60x_FW_API_H__

//#include <bl_ro_params_mgt.h>
#include <stdint.h>

/*--------------------------------------------------------------------*/
/* Reason Codes - these codes are used in bouffalolab fw actions      */
/* when an error action is taking place the reason code can indicate  */
/* what the reason code.                                              */
/*--------------------------------------------------------------------*/
#define WLAN_FW_SUCCESSFUL                                        0 
#define WLAN_FW_TX_AUTH_FRAME_ALLOCATE_FAIILURE                   1 
#define WLAN_FW_AUTHENTICATION_FAIILURE                           2
#define WLAN_FW_AUTH_ALGO_FAIILURE                                3
#define WLAN_FW_TX_ASSOC_FRAME_ALLOCATE_FAIILURE                  4 
#define WLAN_FW_ASSOCIATE_FAIILURE                                5
#define WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION                  6
#define WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION                      7
#define WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE          8
#define WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE   9
#define WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE 10
#define WLAN_FW_TX_AUTH_OR_ASSOC_FRAME_TRANSMIT_FAILURE          11
#define WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL                        12
#define WLAN_FW_CREATE_CHANNEL_CTX_FAILURE_WHEN_JOIN_NETWORK     13
#define WLAN_FW_JOIN_NETWORK_FAILURE                             14
#define WLAN_FW_ADD_STA_FAILURE                                  15
#define WLAN_FW_BEACON_LOSS                                      16

typedef struct
{
    uint32_t gpadc_oscode      : 12;

    uint32_t rx_offset_i       : 10;
    uint32_t rx_offset_q       : 10;

    uint32_t rbb_cap1_fc_i     : 6;
    uint32_t rbb_cap1_fc_q     : 6;
    uint32_t rbb_cap2_fc_i     : 6;
    uint32_t rbb_cap2_fc_q     : 6;

    uint32_t tx_dc_comp_i      : 12;
    uint32_t tx_dc_comp_q      : 12;

    uint32_t tmx_cs            : 3;
    uint32_t txpwr_att_rec     : 3;
    uint32_t pa_pwrmx_osdac    : 4;
    uint32_t tmx_csh           : 3;
    uint32_t tmx_csl           : 3;

    uint32_t tsen_refcode_rfcal : 12;
    uint32_t tsen_refcode_corner : 12;

    uint32_t rc32k_code_fr_ext : 10;
    uint32_t rc32m_code_fr_ext : 8;
    uint32_t saradc_oscode     : 10;
    uint16_t fcal_4osmx        : 4;

} fw_rf_calib1_tag;

/// Definitions of LO dependent calibration structure #pragma pack (2)
typedef struct
{
    uint16_t fcal       : 8;
    uint16_t acal       : 5;
} fw_rf_calib2_tag;

//
typedef struct
{
    uint32_t rosdac_i          : 6;
    uint32_t rosdac_q          : 6;
    uint32_t rx_iq_gain_comp   : 11;
    uint32_t rx_iq_phase_comp  : 10;
} fw_rf_calib3_tag;

//
typedef struct
{
    uint32_t tosdac_i          : 6;
    uint32_t tosdac_q          : 6;
    uint32_t tx_iq_gain_comp   : 11;
    uint32_t tx_iq_phase_comp  : 10;
} fw_rf_calib4_tag;

/// Definitions of the RF calibration result structure
//typedef volatile struct
typedef struct
{
    uint32_t        inited;
    fw_rf_calib1_tag   cal;
    fw_rf_calib2_tag   lo[21];
    fw_rf_calib3_tag   rxcal[4];
    fw_rf_calib4_tag   txcal[8];
} fw_rf_calib_data_tag;

void fw_rf_get_cal_data(fw_rf_calib_data_tag *fw_rf_data);
void fw_rf_set_cal_data(fw_rf_calib_data_tag *fw_rf_data);
void bl60x_fw_xtal_capcode_set(uint8_t cap_in, uint8_t cap_out, uint8_t enable, uint8_t cap_in_max, uint8_t cap_out_max);
void bl60x_fw_xtal_capcode_update(uint8_t cap_in, uint8_t cap_out);
void bl60x_fw_xtal_capcode_restore(void);
void bl60x_fw_xtal_capcode_autofit(void);
void bl60x_fw_xtal_capcode_get(uint8_t *cap_in, uint8_t *cap_out);
//void bl60x_fw_rf_tx_power_table_set(bl_tx_pwr_tbl_t* pwr_table);
int bl60x_fw_password_hash(char *password, char *ssid, int ssidlength, char *output);
void bl60x_fw_rf_table_set(uint32_t* channel_div_table_in, uint16_t* channel_cnt_table_in,
                                                uint16_t lo_fcal_div);

void bl60x_fw_dump_data(void);
void bl60x_fw_dump_statistic(int forced);
void bl60x_current_time_us(long long *time_now);

int bl60x_check_mac_status(int *is_ok);

enum {
    /// Background
    API_AC_BK = 0,
    /// Best-effort
    API_AC_BE,
    /// Video
    API_AC_VI,
    /// Voice
    API_AC_VO,
    /// Number of access categories
    API_AC_MAX
};

int bl60x_edca_get(int ac, uint8_t *aifs, uint8_t *cwmin, uint8_t *cwmax, uint16_t *txop);

/*Wi-Fi Firmware Entry*/
void wifi_main(void *param);

void bl_tpc_update_power_table(int8_t power_table_config[38]);
void bl_tpc_update_power_table_rate(int8_t power_table[24]);
void bl_tpc_update_power_table_channel_offset(int8_t power_table[14]);
void bl_tpc_update_power_rate_11b(int8_t power_rate_table[4]);
void bl_tpc_update_power_rate_11g(int8_t power_rate_table[8]);
void bl_tpc_update_power_rate_11n(int8_t power_rate_table[8]);
void bl_tpc_power_table_get(int8_t power_table_config[38]);

void phy_cli_register(void);



enum task_mm_cfg {
    TASK_MM_CFG_KEEP_ALIVE_STATUS_ENABLED,
    TASK_MM_CFG_KEEP_ALIVE_TIME_LAST_RECEIVED,
    TASK_MM_CFG_KEEP_ALIVE_PACKET_COUNTER,
};
#endif /*__BL60x_FW_API_H__*/
