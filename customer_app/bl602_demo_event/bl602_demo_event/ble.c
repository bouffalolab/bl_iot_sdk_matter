#include <FreeRTOS.h>
#include <task.h>

#include "bluetooth.h"
#include "stack_cli_cmds.h"
#if defined(CONFIG_BT_MESH)
#include "mesh_cli_cmds.h"
#endif
#include "hci_driver.h"
#include "ble_lib_api.h"
#if defined(CONFIG_BT_WIFIPROV_SERVER)
#include "wifi_prov.h"
#endif
#include "log.h"
#include "wifi_prov_api.h"
#if defined(CONFIG_BLE_TP_SERVER)
#include "ble_tp_svc.h"
#endif

#if defined(CONFIG_BT_WIFIPROV_SERVER)
static void wifiprov_connect_ap_ind(void)
{
    printf("Recevied indication to connect to AP\r\n");    
    wifi_prov_api_event_trigger_connect();
}

static void wifiprov_disc_from_ap_ind(void)
{
    printf("Recevied indication to disconnect to AP\r\n");
    wifi_prov_api_event_trigger_disconnect();
}

static void wifiprov_ssid_ind(void *buf,size_t size)
{
    printf("Recevied ssid : %s \r\n", bt_hex(buf, size));
    wifi_prov_api_event_trigger_ssid(buf, size);
}

static void wifiprov_bssid_ind(void *buf,size_t size)
{
    
    printf("Recevied bssid: %s \r\n", bt_hex(buf, size));
}

static void wifiprov_password_ind(void *buf,size_t size)
{
    printf("Recevied password: %s \r\n", bt_hex(buf, size));
    wifi_prov_api_event_trigger_password(buf, size);
}

struct conn_callback WifiProv_conn_callback = {
	.local_connect_remote_ap = wifiprov_connect_ap_ind,
	.local_disconnect_remote_ap = wifiprov_disc_from_ap_ind,
	.get_remote_ap_ssid = wifiprov_ssid_ind,
	.get_remote_ap_bssid = wifiprov_bssid_ind,
	.get_remote_password = wifiprov_password_ind,
};
#endif

void bt_enable_cb(int err)
{
    if (!err) {       
        blestack_cli_register();
        #if defined(CONFIG_BT_MESH)
        blemesh_cli_register();
        #endif
        #if defined(CONFIG_BT_WIFIPROV_SERVER)
        WifiProv_init(&WifiProv_conn_callback);
        #endif
        #if defined(CONFIG_BLE_TP_SERVER)
        ble_tp_init();
        #endif
    }
}

void ble_stack_start(void)
{
     // Initialize BLE controller
    ble_controller_init(configMAX_PRIORITIES - 1);
    // Initialize BLE Host stack
    hci_driver_init();
    bt_enable(bt_enable_cb);
}
