/****************************************************************************
FILE NAME
    ble_tp_svc.h

DESCRIPTION
NOTES
*/
/****************************************************************************/

#ifndef _BLE_TP_SVC_H_
#define _BLE_TP_SVC_H_

#include "config.h"

#define BT_UUID_SVC_BLE_TP				BT_UUID_DECLARE_16(0x9527)
#define BT_UUID_CHAR_BLE_TP_TX		BT_UUID_DECLARE_16(0x9528)
#define BT_UUID_CHAR_BLE_TP_RX		BT_UUID_DECLARE_16(0x9529)


#define BT_CHAR_BLE_TP_TX_ATTR_INDEX	(2)
#define BT_CHAR_BLE_TP_RX_ATTR_INDEX	(4)

void ble_tp_init();
struct bt_gatt_attr *get_attr(u8_t index);

#endif 

