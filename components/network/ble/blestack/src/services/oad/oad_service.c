#include "byteorder.h"
#include "oad_service.h"
#include "oad.h"
 
oad_upper_recv_cb upper_recv_cb;

static int oad_recv(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr, const void *buf,
			  u16_t len, u16_t offset, u8_t flags)
{
    (upper_recv_cb)(conn, buf, len);
    
    return 0;
}

static void oad_ccc_changed(const struct bt_gatt_attr *attr, u16_t value)
{
    printf("oad ccc:value=[%d]\r\n",value);
    
}


static struct bt_gatt_attr oad_attrs[] = {
    BT_GATT_PRIMARY_SERVICE(BT_UUID_OAD),
    BT_GATT_CHARACTERISTIC(BT_UUID_OAD_DATA_IN,
            BT_GATT_CHRC_WRITE_WITHOUT_RESP,
            BT_GATT_PERM_WRITE, NULL, oad_recv, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_OAD_DATA_OUT,
            BT_GATT_CHRC_NOTIFY,BT_GATT_PERM_NONE,
            NULL, NULL, NULL),
    BT_GATT_CCC(oad_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

};

static struct bt_gatt_service oad_svc = BT_GATT_SERVICE(oad_attrs);

void bt_oad_notify(struct bt_conn *conn, const void *data, u16_t len)  
{
    bt_gatt_notify(conn, &oad_attrs[3], data, len);
}

void bt_oad_register_callback(oad_upper_recv_cb cb)
{
    upper_recv_cb = cb;
}

void bt_oad_service_enable(void)
{
    bt_gatt_service_register(&oad_svc);
}

void bt_oad_service_disable(void)
{
    bt_gatt_service_unregister(&oad_svc);
}
