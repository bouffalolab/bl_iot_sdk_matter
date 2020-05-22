#include <stdlib.h>
#include "conn.h"
#include "gatt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli.h"
#include "stack_cli_cmds.h"

static u8_t selected_id = BT_ID_DEFAULT;
bool ble_inited = false;
#define PASSKEY_MAX  0xF423F
#define NAME_LEN 30
#define CHAR_SIZE_MAX           512

#if defined(CONFIG_BT_CONN)
struct bt_conn *default_conn = NULL;
#endif

struct bt_data ad_discov[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    #if defined(BL602)
    BT_DATA(BT_DATA_NAME_COMPLETE, "BL602-BLE-DEV", 13),
    #else
    BT_DATA(BT_DATA_NAME_COMPLETE, "BL70X-BLE-DEV", 13),
    #endif
};

#define vOutputString(...)  printf(__VA_ARGS__)

static void cmd_init(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_start_scan(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_stop_scan(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_start_advertise(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_stop_advertise(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_connect_le(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_disconnect(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_select_conn(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_unpair(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_conn_update(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_security(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_auth(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_auth_cancel(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_auth_passkey_confirm(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_auth_pairing_confirm(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_auth_passkey(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_exchange_mtu(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_discover(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_read(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_write(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_write_without_rsp(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_subscribe(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
static void cmd_unsubscribe(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);

const struct cli_command btStackCmdSet[] STATIC_CLI_CMD_ATTRIBUTE = {
    /*1.The cmd string to type, 2.Cmd description, 3.The function to run, 4.Number of parameters*/
    {"cmd_init", "\r\ncmd_init:[Initialize]\r\n Parameter[Null]\r\n", cmd_init},
    #if defined(CONFIG_BT_OBSERVER)
    #if defined(CONFIG_BT_STACK_PTS)
    {"cmd_start_scan", "\r\ncmd_start_scan:\r\n\
     [Scan type, 0:passive scan, 1:active scan]\r\n\
     [Duplicate filtering, 0:Disable duplicate filtering, 1:Enable duplicate filtering]\r\n\
     [Scan interval, 0x0004-4000,e.g.0080]\r\n\
     [Scan window, 0x0004-4000,e.g.0050]\r\n\
     [Is_RPA, 0:non-rpa, 1:rpa]\r\n", cmd_start_scan},
    #else
    {"cmd_start_scan", "\r\ncmd_start_scan:\r\n\
     [Scan type, 0:passive scan, 1:active scan]\r\n\
     [Duplicate filtering, 0:Disable duplicate filtering, 1:Enable duplicate filtering]\r\n\
     [Scan interval, 0x0004-4000,e.g.0080]\r\n\
     [Scan window, 0x0004-4000,e.g.0050]\r\n", cmd_start_scan},
    #endif//CONFIG_BT_STACK_PTS
    {"cmd_stop_scan", "\r\ncmd_stop_scan:[Stop scan]\r\n\
      Parameter[Null]\r\n", cmd_stop_scan},
    #endif

    #if defined(CONFIG_BT_PERIPHERAL)
    #if defined(CONFIG_BT_STACK_PTS)
    {"cmd_start_adv", "\r\ncmd_start_adv:\r\n\
     [Adv type,0:adv_ind,1:adv_scan_ind,2:adv_nonconn_ind]\r\n\
     [Mode, 0:discov, 1:non-discov]\r\n\
     [Is_RPA, 0:non-rpa, 1:rpa]\r\n\
     [Adv Interval Min,0x0020-4000,e.g.0030]\r\n\
     [Adv Interval Max,0x0020-4000,e.g.0060]\r\n", cmd_start_advertise},
    #else
    {"cmd_start_adv", "\r\ncmd_start_adv:\r\n\
     [Adv type,0:adv_ind,1:adv_scan_ind,2:adv_nonconn_ind]\r\n\
     [Mode, 0:discov, 1:non-discov]\r\n\
     [Adv Interval Min,0x0020-4000,e.g.0030]\r\n\
     [Adv Interval Max,0x0020-4000,e.g.0060]\r\n", cmd_start_advertise},
    #endif //CONFIG_BT_STACK_PTS
     
    {"cmd_stop_adv", "\r\ncmd_stop_adv:[Stop advertising]\r\n\
     Parameter[Null]\r\n", cmd_stop_advertise},
    #endif //#if defined(CONFIG_BT_PERIPHERAL)

    #if defined(CONFIG_BT_CONN)
    #if defined(CONFIG_BT_CENTRAL)
    {"cmd_connect_le", "\r\ncmd_connect_le:[Connect remote device]\r\n\
     [Address type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND]\r\n\
     [Address value, e.g.112233AABBCC]\r\n", cmd_connect_le},
    #endif //#if defined(CONFIG_BT_CONN)
    
    {"cmd_disconnect", "\r\ncmd_disconnect:[Disconnect remote device]\r\n\
     [Address type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND]\r\n\
     [Address value,e.g.112233AABBCC]\r\n", cmd_disconnect},
     
    {"cmd_select_conn", "\r\ncmd_select_conn:[Select a specific connection]\r\n\
     [Address type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND]\r\n\
     [Address value, e.g.112233AABBCC]\r\n", cmd_select_conn},
     
    {"cmd_unpair", "\r\ncmd_unpair:[Unpair connection]\r\n\
     [Address type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND]\r\n\
     [Address value, all 0: unpair all connection, otherwise:unpair specific connection]\r\n", cmd_unpair},

    {"cmd_conn_update", "\r\ncmd_conn_update:\r\n\
     [Conn Interval Min,0x0006-0C80,e.g.0030]\r\n\
     [Conn Interval Max,0x0006-0C80,e.g.0030]\r\n\
     [Conn Latency,0x0000-01f3,e.g.0004]\r\n\
     [Supervision Timeout,0x000A-0C80,e.g.0010]\r\n", cmd_conn_update},
    #endif //#if defined(CONFIG_BT_CONN)
    
    #if defined(CONFIG_BT_SMP)
    {"cmd_security", "\r\ncmd_security:[Start security]\r\n\
     [Security level, Default value 4, 2:BT_SECURITY_MEDIUM, 3:BT_SECURITY_HIGH, 4:BT_SECURITY_FIPS]\r\n", cmd_security},
    {"cmd_auth", "\r\ncmd_auth:[Register auth callback]\r\n", cmd_auth},
    {"cmd_auth_cancel", "\r\ncmd_auth_cancel:[Register auth callback]\r\n", cmd_auth_cancel},
    {"cmd_auth_passkey_confirm", "\r\ncmd_auth_passkey_confirm:[Confirm passkey]\r\n", cmd_auth_passkey_confirm},
    {"cmd_auth_pairing_confirm", "\r\ncmd_auth_pairing_confirm:[Confirm pairing in secure connection]\r\n", cmd_auth_pairing_confirm},
    {"cmd_auth_passkey", "\r\ncmd_auth_passkey:[Input passkey]\r\n\
     [Passkey, 00000000-000F423F]", cmd_auth_passkey},
    #endif //#if defined(CONFIG_BT_SMP)

    #if defined(CONFIG_BT_GATT_CLIENT)
    {"cmd_exchange_mtu", "\r\ncmd_exchange_mtu:[Exchange mtu]\r\n Parameter[Null]\r\n", cmd_exchange_mtu},
    {"cmd_discover", "\r\ncmd_discover:[Gatt discovery]\r\n\
     [Discovery type, 0:Primary, 1:Secondary, 2:Include, 3:Characteristic, 4:Descriptor]\r\n\
     [Uuid value, 2 Octets, e.g.1800]\r\n\
     [Start handle, 2 Octets, e.g.0001]\r\n\
     [End handle, 2 Octets, e.g.ffff]\r\n", cmd_discover},
    {"cmd_read", "\r\ncmd_read:[Gatt Read]\r\n\
     [Attribute handle, 2 Octets]\r\n\
     [Value offset, 2 Octets]\r\n", cmd_read},
    {"cmd_write", "\r\ncmd_write:[Gatt write]\r\n\
     [Attribute handle, 2 Octets]\r\n\
     [Value offset, 2 Octets]\r\n\
     [Value length, 2 Octets]\r\n\
     [Value data]\r\n", cmd_write},
    {"cmd_write_without_rsp", "\r\ncmd_write_without_rsp:[Gatt write without response]\r\n\
     [Sign, 0: No need signed, 1:Signed write cmd if no smp]\r\n\
     [Attribute handle, 2 Octets]\r\n\
     [Value length, 2 Octets]\r\n\
     [Value data]\r\n", cmd_write_without_rsp},
    {"cmd_subscribe", "\r\ncmd_subscribe:[Gatt subscribe]\r\n\
     [CCC handle, 2 Octets]\r\n\
     [Value handle, 2 Octets]\r\n\
     [Value, 1:notify, 2:indicate]\r\n", cmd_subscribe},
     {"cmd_unsubscribe", "\r\ncmd_unsubscribe:[Gatt unsubscribe]\r\n Parameter[Null]\r\n", cmd_unsubscribe},
    #endif
};

#if defined(CONFIG_BT_CONN)
static void connected(struct bt_conn *conn, u8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		vOutputString("Failed to connect to %s (%u) \r\n", addr,
			     err);
		return;
	}

	vOutputString("Connected: %s \r\n", addr);

	if (!default_conn) {
		default_conn = conn;
	}
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	vOutputString("Disconnected: %s (reason %u) \r\n", addr, reason);

	if (default_conn == conn) {
		default_conn = NULL;
	}
}

static void le_param_updated(struct bt_conn *conn, u16_t interval,
			     u16_t latency, u16_t timeout)
{
	vOutputString("LE conn param updated: int 0x%04x lat %d to %d \r\n", interval, latency, timeout);
}

#if defined(CONFIG_BT_SMP)
static void identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa,
			      const bt_addr_le_t *identity)
{
	char addr_identity[BT_ADDR_LE_STR_LEN];
	char addr_rpa[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
	bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

	vOutputString("Identity resolved %s -> %s \r\n", addr_rpa, addr_identity);
}

static void security_changed(struct bt_conn *conn, bt_security_t level)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	vOutputString("Security changed: %s level %u \r\n", addr, level);
}
#endif

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_updated = le_param_updated,
#if defined(CONFIG_BT_SMP)
	.identity_resolved = identity_resolved,
	.security_changed = security_changed,
#endif
};
#endif //CONFIG_BT_CONN

static void cmd_init(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if(ble_inited){
        vOutputString("Has initialized \r\n");
        return;
    }

    #if defined(CONFIG_BT_CONN)
    default_conn = NULL;
    bt_conn_cb_register(&conn_callbacks);
    #endif
    ble_inited = true;
    vOutputString("Init successfully \r\n");
}

#if defined(CONFIG_BT_OBSERVER)
static bool data_cb(struct bt_data *data, void *user_data)
{
	char *name = user_data;
    u8_t len;

	switch (data->type) {
	case BT_DATA_NAME_SHORTENED:
	case BT_DATA_NAME_COMPLETE:
        len = (data->data_len > NAME_LEN - 1)?(NAME_LEN - 1):(data->data_len);
		memcpy(name, data->data, len);
		return false;
	default:
		return true;
	}
}

#if defined(CONFIG_BT_STACK_PTS)
char *pts_cmplt_name = "PTS-GAP-224B";
char *pts_short_name = "PTS-GAP";
bt_addr_le_t pts_addr;
#endif
static void device_found(const bt_addr_le_t *addr, s8_t rssi, u8_t evtype,
			 struct net_buf_simple *buf)
{
	char le_addr[BT_ADDR_LE_STR_LEN];
	char name[NAME_LEN];

	(void)memset(name, 0, sizeof(name));

	bt_data_parse(buf, data_cb, name);

	bt_addr_le_to_str(addr, le_addr, sizeof(le_addr));
    #if defined(CONFIG_BT_STACK_PTS)
    if(!memcmp(&pts_addr, addr, sizeof(bt_addr_le_t)) ||
       !memcmp(name,pts_cmplt_name, sizeof(*pts_cmplt_name)) ||
       !memcmp(name,pts_short_name, sizeof(*pts_short_name))){
    if(memcmp(&pts_addr, addr, sizeof(bt_addr_le_t)))
        memcpy(&pts_addr, addr, sizeof(pts_addr));  
    #endif
	vOutputString("[DEVICE]: %s, AD evt type %u, RSSI %i %s \r\n",le_addr, evtype, rssi, name);

    #if defined(CONFIG_BT_STACK_PTS)
    }
    #endif
}

static void cmd_start_scan(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    struct bt_le_scan_param scan_param;
    int err;

    (void)err;

    #if defined(CONFIG_BT_STACK_PTS)
    bool is_rpa;
    if(argc != 6){
    #else
    if(argc != 5){
    #endif
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }

    co_get_bytearray_from_string(&argv[1], &scan_param.type, 1);
    
    co_get_bytearray_from_string(&argv[2], &scan_param.filter_dup, 1);
    
    co_get_uint16_from_string(&argv[3], &scan_param.interval);
    
    co_get_uint16_from_string(&argv[4], &scan_param.window);

    #if defined(CONFIG_BT_STACK_PTS)
    co_get_bytearray_from_string(&argv[5], (uint8_t *)&is_rpa, 1);
    err = bt_le_scan_start(&scan_param, device_found, is_rpa);
    #else
    err = bt_le_scan_start(&scan_param, device_found);
    #endif
    if(err){
        vOutputString("Failed to start scan (err %d) \r\n", err);
    }else{
        vOutputString("Start scan successfully \r\n");
    }
}

static void cmd_stop_scan(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;
    
	err = bt_le_scan_stop();
	if (err) {
		vOutputString("Stopping scanning failed (err %d)\r\n", err);
	} else {
		vOutputString("Scan successfully stopped \r\n");
	}
}
#endif //#if defined(CONFIG_BT_OBSERVER)

#if defined(CONFIG_BT_PERIPHERAL)
static void cmd_start_advertise(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    struct bt_le_adv_param param;
	const struct bt_data *ad;
	size_t ad_len;
	int err;
    uint8_t adv_type, tmp;
    #if defined(CONFIG_BT_STACK_PTS)
    bool is_rpa = false;
    #endif

    #if defined(CONFIG_BT_STACK_PTS)
    if(argc != 4 && argc != 6){
    #else
    if(argc != 3 && argc != 5){
    #endif
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
        
    param.id = selected_id;
    param.interval_min = BT_GAP_ADV_FAST_INT_MIN_2;
    param.interval_max = BT_GAP_ADV_FAST_INT_MAX_2;

    /*Get adv type, 0:adv_ind,  1:adv_scan_ind, 2:adv_nonconn_ind*/
    co_get_bytearray_from_string(&argv[1], &adv_type, 1);
    
    if(adv_type == 0){
        param.options = (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME | BT_LE_ADV_OPT_ONE_TIME);
    }else if(adv_type == 1){
        param.options = BT_LE_ADV_OPT_USE_NAME;
    }else if(adv_type == 2){
        param.options = 0;
    }else{
        vOutputString("Arg1 is invalid\r\n");
        return;
    }

    /*Get mode, 0:discoverable,  1:non discoverable*/
    co_get_bytearray_from_string(&argv[2], &tmp, 1);

    if(tmp == 0 || tmp == 1){
        if(tmp)
            ad_discov[0].data = 0;
        ad = ad_discov;
        ad_len = ARRAY_SIZE(ad_discov);
    }else{
        vOutputString("Arg2 is invalid\r\n");
        return;
    }

    #if defined(CONFIG_BT_STACK_PTS)
    /*use resolvable private address or non-resolvable private address
       1:resolvable private address,0:non-resolvable private address*/
    co_get_bytearray_from_string(&argv[3], (uint8_t *)&is_rpa, 1);
    if(argc == 6){
        co_get_uint16_from_string(&argv[4], &param.interval_min);
        co_get_uint16_from_string(&argv[5], &param.interval_max);
    }  
    #else
    if(argc == 5){
        co_get_uint16_from_string(&argv[3], &param.interval_min);
        co_get_uint16_from_string(&argv[4], &param.interval_max);
    }
    #endif//CONFIG_BT_STACK_PTS
    
    if(adv_type == 1){
        #if defined(CONFIG_BT_STACK_PTS)
        err = bt_le_adv_start(&param, ad, ad_len, &ad_discov[0], 1, is_rpa);
        #else
        err = bt_le_adv_start(&param, ad, ad_len, &ad_discov[0], 1);
        #endif
    }else{
        #if defined(CONFIG_BT_STACK_PTS)
        err = bt_le_adv_start(&param, ad, ad_len, NULL, 0, is_rpa);
        #else
        err = bt_le_adv_start(&param, ad, ad_len, NULL, 0);
        #endif
    }
    
    if(err){
        vOutputString("Failed to start advertising\r\n");
    }else{
        vOutputString("Advertising started\r\n");
    }
}

static void cmd_stop_advertise(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if(bt_le_adv_stop()){
        vOutputString("Failed to stop advertising\r\n");
    }else{
        vOutputString("Advertising stopped\r\n");
    }
}
#endif //#if defined(CONFIG_BT_PERIPHERAL)

#if defined(CONFIG_BT_CONN)
#if defined(CONFIG_BT_CENTRAL)
static void cmd_connect_le(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    bt_addr_le_t addr;
    struct bt_conn *conn;
    u8_t  addr_val[6];

    if(argc != 3){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
       
    /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    co_get_bytearray_from_string(&argv[1], &addr.type, 1);

    co_get_bytearray_from_string(&argv[2], addr_val, 6);

    co_reverse_bytearray(addr_val, addr.a.val, 6);
    
    conn = bt_conn_create_le(&addr, BT_LE_CONN_PARAM_DEFAULT);

    if(!conn){
        vOutputString("Connection failed\r\n");
    }else{
        vOutputString("Connection pending\r\n");
    }
}
#endif //#if defined(CONFIG_BT_CENTRAL)

static void cmd_disconnect(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    bt_addr_le_t addr;
    u8_t  addr_val[6];
    struct bt_conn *conn;

    if(argc != 3){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }

    /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    co_get_bytearray_from_string(&argv[1], &addr.type, 1);
    co_get_bytearray_from_string(&argv[2], addr_val, 6);
    co_reverse_bytearray(addr_val, addr.a.val, 6);
    
    conn = bt_conn_lookup_addr_le(selected_id, &addr);

    if(!conn){
        vOutputString("Not connected\r\n");
        return;
    }

    if(bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN)){
        vOutputString("Disconnection failed\r\n");
    }else{
        vOutputString("Disconnect successfully\r\n");
    }
    bt_conn_unref(conn);
}

static void cmd_select_conn(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    bt_addr_le_t addr;
    struct bt_conn *conn;
    u8_t  addr_val[6];

    if(argc != 3){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }

    /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    co_get_bytearray_from_string(&argv[1], &addr.type, 1);

    co_get_bytearray_from_string(&argv[2], addr_val, 6);

    co_reverse_bytearray(addr_val, addr.a.val, 6);
    
    conn = bt_conn_lookup_addr_le(selected_id, &addr);
           
    if(!conn){
        vOutputString("No matching connection found\r\n");
        return;
    }

    if(default_conn){
        bt_conn_unref(default_conn);
    }

    default_conn = conn;
}

static void cmd_unpair(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    bt_addr_le_t addr;
    u8_t  addr_val[6];
    int err;

    if(argc != 3){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }

    /*Get addr type, 0:ADDR_PUBLIC, 1:ADDR_RAND, 2:ADDR_RPA_OR_PUBLIC, 3:ADDR_RPA_OR_RAND*/
    co_get_bytearray_from_string(&argv[1], &addr.type, 1);
    
    co_get_bytearray_from_string(&argv[2], addr_val, 6);

    co_reverse_bytearray(addr_val, addr.a.val, 6);
        
    err = bt_unpair(selected_id, &addr);

    if(err){
        vOutputString("Failed to unpair\r\n");
    }else{
        vOutputString("Unpair successfully\r\n");
    }
}

static void cmd_conn_update(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	struct bt_le_conn_param param;
	int err;

    if(argc != 5){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    co_get_uint16_from_string(&argv[1], &param.interval_min);
    co_get_uint16_from_string(&argv[2], &param.interval_max);
    co_get_uint16_from_string(&argv[3], &param.latency);
    co_get_uint16_from_string(&argv[4], &param.timeout);

	err = bt_conn_le_param_update(default_conn, &param);
	if (err) {
		vOutputString("conn update failed (err %d)\r\n", err);
	} else {
		vOutputString("conn update initiated\r\n");
	}
}
#endif //#if defined(CONFIG_BT_CONN)

#if defined(CONFIG_BT_SMP)
static void cmd_security(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int err;

    u8_t sec_level = BT_SECURITY_FIPS;

    if(!default_conn){
        vOutputString("Please firstly choose the connection using cmd_select_conn\r\n");
        return;
    }

    if(argc == 2)
        co_get_bytearray_from_string(&argv[1], &sec_level, 1);
    
    err = bt_conn_security(default_conn, sec_level);

    if(err){
        vOutputString("Failed to start security, (err %d) \r\n", err);
    }else{
        vOutputString("Start security successfully\r\n");
    }
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));    

    vOutputString("passkey_str is: %06u\r\n", passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	vOutputString("Confirm passkey for %s: %06u\r\n", addr, passkey);
}

static void auth_passkey_entry(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	vOutputString("Enter passkey for %s\r\n", addr);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	
	vOutputString("Pairing cancelled: %s\r\n", addr);
}

static void auth_pairing_confirm(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	vOutputString("Confirm pairing for %s\r\n", addr);
}

static void auth_pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	vOutputString("%s with %s\r\n", bonded ? "Bonded" : "Paired", addr);
}

static void auth_pairing_failed(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	vOutputString("Pairing failed with %s\r\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = auth_passkey_entry,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
	.pairing_confirm = auth_pairing_confirm,
	.pairing_failed = auth_pairing_failed,
	.pairing_complete = auth_pairing_complete,
};

static void cmd_auth(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int err;

    err = bt_conn_auth_cb_register(&auth_cb_display);

    if(err){
        vOutputString("Auth callback has already been registered\r\n");
    }else{
        vOutputString("Register auth callback successfully\r\n");
    }
}

static void cmd_auth_cancel(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	struct bt_conn *conn;
    
	if (default_conn) {
		conn = default_conn;
	}else {
		conn = NULL;
	}

	if (!conn) {
        vOutputString("Not connected\r\n");
		return;
	}

	bt_conn_auth_cancel(conn);
}

static void cmd_auth_passkey_confirm(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    
	if (!default_conn) {
        vOutputString("Not connected\r\n");
		return;
	}

	bt_conn_auth_passkey_confirm(default_conn);
}

static void cmd_auth_pairing_confirm(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
   
	if (!default_conn) {
        vOutputString("Not connected\r\n");
		return;
	}

	bt_conn_auth_pairing_confirm(default_conn);
}

static void cmd_auth_passkey(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	uint32_t passkey;

    if(argc != 2){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    
	if (!default_conn) {
        vOutputString("Not connected\r\n");
		return;
	}

    passkey = atoi(argv[1]);
	if (passkey > PASSKEY_MAX) {
        vOutputString("Passkey should be between 0-999999\r\n");
		return;
	}

	bt_conn_auth_passkey_entry(default_conn, passkey);
}
#endif //#if defined(CONFIG_BT_SMP)

#if defined(CONFIG_BT_GATT_CLIENT)
static void exchange_func(struct bt_conn *conn, u8_t err,
			  struct bt_gatt_exchange_params *params)
{
	vOutputString("Exchange %s\r\n", err == 0U ? "successful" : "failed");
}

static struct bt_gatt_exchange_params exchange_params;

static void cmd_exchange_mtu(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;
    
	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

	exchange_params.func = exchange_func;

	err = bt_gatt_exchange_mtu(default_conn, &exchange_params);
	if (err) {
		vOutputString("Exchange failed (err %d)\r\n", err);
	} else {
		vOutputString("Exchange pending\r\n");
	}
}

static struct bt_gatt_discover_params discover_params;
static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);

static void print_chrc_props(u8_t properties)
{
	vOutputString("Properties: ");

	if (properties & BT_GATT_CHRC_BROADCAST) {
		vOutputString("[bcast]\r\n");
	}

	if (properties & BT_GATT_CHRC_READ) {
		vOutputString("[read]\r\n");
	}

	if (properties & BT_GATT_CHRC_WRITE) {
		vOutputString("[write]\r\n");
	}

	if (properties & BT_GATT_CHRC_WRITE_WITHOUT_RESP) {
		vOutputString("[write w/w rsp]\r\n");
	}

	if (properties & BT_GATT_CHRC_NOTIFY) {
		vOutputString("[notify]\r\n");
	}

	if (properties & BT_GATT_CHRC_INDICATE) {
		vOutputString("[indicate]");
	}

	if (properties & BT_GATT_CHRC_AUTH) {
		vOutputString("[auth]\r\n");
	}

	if (properties & BT_GATT_CHRC_EXT_PROP) {
		vOutputString("[ext prop]\r\n");
	}
}

static u8_t discover_func(struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params)
{
	struct bt_gatt_service_val *gatt_service;
	struct bt_gatt_chrc *gatt_chrc;
	struct bt_gatt_include *gatt_include;
	char str[37];

	if (!attr) {
		vOutputString( "Discover complete\r\n");
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	switch (params->type) {
	case BT_GATT_DISCOVER_SECONDARY:
	case BT_GATT_DISCOVER_PRIMARY:
		gatt_service = attr->user_data;
		bt_uuid_to_str(gatt_service->uuid, str, sizeof(str));
		vOutputString("Service %s found: start handle %x, end_handle %x\r\n", str, attr->handle, gatt_service->end_handle);
		break;
	case BT_GATT_DISCOVER_CHARACTERISTIC:
		gatt_chrc = attr->user_data;
		bt_uuid_to_str(gatt_chrc->uuid, str, sizeof(str));
		vOutputString("Characteristic %s found: handle %x\r\n", str, attr->handle);
		print_chrc_props(gatt_chrc->properties);
		break;
	case BT_GATT_DISCOVER_INCLUDE:
		gatt_include = attr->user_data;
		bt_uuid_to_str(gatt_include->uuid, str, sizeof(str));
		vOutputString("Include %s found: handle %x, start %x, end %x\r\n", str, attr->handle,
			    gatt_include->start_handle, gatt_include->end_handle);
		break;
	default:
		bt_uuid_to_str(attr->uuid, str, sizeof(str));
		vOutputString("Descriptor %s found: handle %x\r\n", str, attr->handle);
		break;
	}

	return BT_GATT_ITER_CONTINUE;
}

static void cmd_discover(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;
    u8_t disc_type;

    if(argc != 5){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    
	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

	discover_params.func = discover_func;
	discover_params.start_handle = 0x0001;
	discover_params.end_handle = 0xffff;

    co_get_bytearray_from_string(&argv[1], &disc_type, 1);
    if(disc_type == 0){
        discover_params.type = BT_GATT_DISCOVER_PRIMARY;
    }else if(disc_type == 1){
        discover_params.type = BT_GATT_DISCOVER_SECONDARY;
    }else if(disc_type == 2){
        discover_params.type = BT_GATT_DISCOVER_INCLUDE;
    }else if(disc_type == 3){
        discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
    }else if(disc_type == 4){
        discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
    }else{
        vOutputString("Invalid discovery type\r\n");
        return;
    }
    co_get_uint16_from_string(&argv[2], &uuid.val);
    if(uuid.val)
        discover_params.uuid = &uuid.uuid;
    else
        discover_params.uuid = NULL;

    co_get_uint16_from_string(&argv[3], &discover_params.start_handle);
    co_get_uint16_from_string(&argv[4], &discover_params.end_handle);

	err = bt_gatt_discover(default_conn, &discover_params);
	if (err) {
		vOutputString("Discover failed (err %d)\r\n", err);
	} else {
		vOutputString("Discover pending\r\n");
	}
}

static struct bt_gatt_read_params read_params;

static u8_t read_func(struct bt_conn *conn, u8_t err, struct bt_gatt_read_params *params, const void *data, u16_t length)
{
	vOutputString("Read complete: err %u length %u \r\n", err, length);

	if (!data) {
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	return BT_GATT_ITER_CONTINUE;
}

static void cmd_read(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;

    if(argc != 3){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    
	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

    co_get_uint16_from_string(&argv[1], &read_params.single.handle);
    co_get_uint16_from_string(&argv[2], &read_params.single.offset);

    read_params.func = read_func;
	read_params.handle_count = 1;

	err = bt_gatt_read(default_conn, &read_params);
	if (err) {
		vOutputString("Read failed (err %d)\r\n", err);
	} else {
		vOutputString("Read pending\r\n");
	}
}

static struct bt_gatt_write_params write_params;
static u8_t gatt_write_buf[CHAR_SIZE_MAX];

static void write_func(struct bt_conn *conn, u8_t err,
		       struct bt_gatt_write_params *params)
{
	vOutputString("Write complete: err %u \r\n", err);

	(void)memset(&write_params, 0, sizeof(write_params));
}

static void cmd_write(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;
    uint16_t data_len;

    if(argc != 5){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    
	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

	if (write_params.func) {
		vOutputString("Write ongoing\r\n");
		return;
	}

    co_get_uint16_from_string(&argv[1], &write_params.handle);
    co_get_uint16_from_string(&argv[2], &write_params.offset);
    co_get_uint16_from_string(&argv[3], &write_params.length);
    data_len = write_params.length > sizeof(gatt_write_buf)? (sizeof(gatt_write_buf)):(write_params.length);
    co_get_bytearray_from_string(&argv[4], gatt_write_buf, data_len);
    
	write_params.data = gatt_write_buf;
	write_params.length = data_len;
	write_params.func = write_func;

	err = bt_gatt_write(default_conn, &write_params);
    
	if (err) {
		vOutputString("Write failed (err %d)\r\n", err);
	} else {
		vOutputString("Write pending\r\n");
	}
}

static void cmd_write_without_rsp(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	u16_t handle;
	int err;
	u16_t len;
	bool sign;

    if(argc != 5){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    
	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

    co_get_bytearray_from_string(&argv[1], (uint8_t *)&sign, 1);
    co_get_uint16_from_string(&argv[2], &handle);
	co_get_uint16_from_string(&argv[3], &len);
    len = len > sizeof(gatt_write_buf)? (sizeof(gatt_write_buf)):(len);
	co_get_bytearray_from_string(&argv[4], gatt_write_buf, len);

	err = bt_gatt_write_without_response(default_conn, handle, gatt_write_buf, len, sign);

	vOutputString("Write Complete (err %d)\r\n", err);
}

static struct bt_gatt_subscribe_params subscribe_params;

static u8_t notify_func(struct bt_conn *conn,
			struct bt_gatt_subscribe_params *params,
			const void *data, u16_t length)
{
	if (!params->value) {
		vOutputString("Unsubscribed\r\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}

	vOutputString("Notification: data %p length %u\r\n", data, length);

	return BT_GATT_ITER_CONTINUE;
}

static void cmd_subscribe(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;

    (void)err;

    if(argc != 4){
        vOutputString("Number of Parameters is not correct\r\n");
        return;
    }
    
	if (subscribe_params.value_handle) {
		vOutputString( "Cannot subscribe: subscription to %x already exists\r\n", subscribe_params.value_handle);
		return;
	}

	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

    co_get_uint16_from_string(&argv[1], &subscribe_params.ccc_handle);
    co_get_uint16_from_string(&argv[2], &subscribe_params.value_handle);
    co_get_uint16_from_string(&argv[3], &subscribe_params.value);
	subscribe_params.notify = notify_func;

	err = bt_gatt_subscribe(default_conn, &subscribe_params);
	if (err) {
		vOutputString("Subscribe failed (err %d)\r\n", err);
	} else {
		vOutputString("Subscribed\r\n");
	}

}

static void cmd_unsubscribe(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int err;

    (void)err;
    
	if (!default_conn) {
		vOutputString("Not connected\r\n");
		return;
	}

	if (!subscribe_params.value_handle) {
		vOutputString("No subscription found\r\n");
		return;
	}

	err = bt_gatt_unsubscribe(default_conn, &subscribe_params);
	if (err) {
		vOutputString("Unsubscribe failed (err %d)\r\n", err);
	} else {
		vOutputString("Unsubscribe success\r\n");
	}
}
#endif /* CONFIG_BT_GATT_CLIENT */

int blestack_cli_register(void)
{
    #if defined(CONFIG_BT_STACK_PTS)
    memcpy(&pts_addr.a, BT_ADDR_NONE, sizeof(pts_addr.a));
    #endif
    //aos_cli_register_commands(btStackCmdSet, sizeof(btStackCmdSet)/sizeof(btStackCmdSet[0]));
    return 0;
}
