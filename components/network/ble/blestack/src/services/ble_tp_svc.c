/****************************************************************************
FILE NAME
    ble_tp_svc.c

DESCRIPTION
NOTES
*/
/****************************************************************************/

#include <errno.h>
#include <stdbool.h>

#include "bluetooth.h"
#include "conn.h"
#include "gatt.h"
#include "uuid.h"
#include "ble_tp_svc.h"


static void ble_tp_connected(struct bt_conn *conn, u8_t err);
static void ble_tp_disconnected(struct bt_conn *conn, u8_t reason);


struct bt_conn *ble_tp_conn;


static struct bt_conn_cb ble_tp_conn_callbacks = {
	.connected	=   ble_tp_connected,
	.disconnected	=   ble_tp_disconnected,
};

/*************************************************************************
NAME    
    ble_tp_connected
*/
static void ble_tp_connected(struct bt_conn *conn, u8_t err)
{   
	printf("%s\n",__func__);

	ble_tp_conn = conn;
}

/*************************************************************************
NAME    
    ble_tp_disconnected
*/
static void ble_tp_disconnected(struct bt_conn *conn, u8_t reason)
{ 
	printf("%s\n",__func__);
	
	ble_tp_conn = NULL;
}

/*************************************************************************
NAME    
    ble_tp_notify
*/ 

int ble_tp_notify(void *data, u16_t len)
{
	int err = -1;
	u8_t *send_data = (u8_t*)data;
	u16_t slen = len;

	if(ble_tp_conn)
	{
		err = bt_gatt_notify(ble_tp_conn, get_attr(BT_CHAR_BLE_TP_TX_ATTR_INDEX), send_data, slen);
	}

	return err;
}

/*************************************************************************
NAME    
    ble_tp_ccc_cfg_changed
*/ 
static void ble_tp_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	ARG_UNUSED(attr);

	bool enotify = (value == BT_GATT_CCC_NOTIFY);
	printf("%s, Notify enable? %d\n", __func__, enotify);
	    
	char data[256] = {};
	for(u8_t i = 0; i < 0xff; i++)
	{
		data[i] = i;
	}

	if(enotify)
	{
		while(1)
		{
			ble_tp_notify(data, 244);
		}
	}

}

/*************************************************************************
NAME    
    ble_tp_recv
*/ 
static int ble_tp_recv(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr, const void *buf,
			  u16_t len, u16_t offset, u8_t flags)
{
	printf("%s, Receive write data len : %d bytes\n", __func__, len);
	return 0;
}

/*************************************************************************
*  DEFINE : attrs 
*/
static struct bt_gatt_attr attrs[]= {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_SVC_BLE_TP),

	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_TX,
							BT_GATT_CHRC_NOTIFY,
							BT_GATT_PERM_READ, 
							NULL, 
							NULL,
							NULL),

	BT_GATT_CCC(ble_tp_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

	BT_GATT_CHARACTERISTIC(BT_UUID_CHAR_BLE_TP_RX,
							BT_GATT_CHRC_WRITE_WITHOUT_RESP,
							BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
							NULL, 
							ble_tp_recv,
							NULL)
};

/*************************************************************************
NAME    
    get_attr
*/
struct bt_gatt_attr *get_attr(u8_t index)
{
	return &attrs[index];
}


struct bt_gatt_service ble_tp_server = BT_GATT_SERVICE(attrs);


/*************************************************************************
NAME    
    ble_tp_init
*/
void ble_tp_init()
{
	bt_conn_cb_register(&ble_tp_conn_callbacks);
	bt_gatt_service_register(&ble_tp_server);
}



