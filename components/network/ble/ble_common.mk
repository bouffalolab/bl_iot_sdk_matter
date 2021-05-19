CFLAGS   += -DCFG_FREERTOS
CFLAGS   += -DARCH_RISCV

ifeq ($(CONFIG_CHIP_NAME),BL602)
CFLAGS   += -DBL602
CFLAGS   += -DCONFIG_SET_TX_PWR
endif

ifeq ($(CONFIG_CHIP_NAME),BL702)
CFLAGS   += -DBL702
endif

ifeq ($(CONFIG_CHIP_NAME),BL606p)
CFLAGS   += -DBL606p
endif

ifeq ($(CONFIG_DBG_RUN_ON_FPGA), 1)
CFLAGS   += -DCFG_DBG_RUN_ON_FPGA
endif

ifeq ($(CONFIG_BUILD_ROM_CODE),1)
CFLAGS += -DBUILD_ROM_CODE
endif

CFLAGS   += -DCFG_BLE_ENABLE
CPPFLAGS += -DCFG_BLE_ENABLE

CFLAGS   += -DBFLB_BLE
CFLAGS   += -DCFG_BLE
CFLAGS   += -DCFG_SLEEP
CFLAGS   += -DOPTIMIZE_DATA_EVT_FLOW_FROM_CONTROLLER
CFLAGS   += -DCFG_BT_RESET

CONFIG_BT_AUDIO?=0
ifeq ($(CONFIG_BT_AUDIO),1)
CFLAGS += -DCONFIG_BT_AUDIO
endif

CONFIG_BT_BREDR?=0
ifeq ($(CONFIG_BT_BREDR),1)
CFLAGS += -DCONFIG_BT_BREDR
CFLAGS += -DCONFIG_MAX_SCO=2
CFLAGS += -DSBC_DEC_INCLUDED
CFLAGS += -DSBC_ENC_INCLUDED
CFLAGS += -DCONFIG_BT_AVDTP
CFLAGS += -DCONFIG_BT_A2DP
CFLAGS += -DCONFIG_BT_RFCOMM
CFLAGS += -DCONFIG_BT_HFP
endif

ifeq ($(CONFIG_BT_TL),1)
CFLAGS += -DCONFIG_BT_TL
CONFIG_BLE_HOST_DISABLE:=1
endif

CONFIG_BT_CONN?=2
CFLAGS += -DCFG_CON=$(CONFIG_BT_CONN)
CONFIG_BLE_TX_BUFF_DATA?=$(CONFIG_BT_CONN)
CFLAGS += -DCFG_BLE_TX_BUFF_DATA=$(CONFIG_BLE_TX_BUFF_DATA)
CONFIG_BT_ALLROLES?=1
CONFIG_BT_CENTRAL?=1
CONFIG_BT_OBSERVER?=1
CONFIG_BT_PERIPHERAL?=1
CONFIG_BT_BROADCASTER?=1
CONFIG_BT_SETTINGS?=0
CONFIG_BLE_TP_SERVER?=0
CONFIG_BLE_MULTI_ADV?=0
CONFIG_BLE_RECONNECT_TEST?=0
CONFIG_BT_STACK_CLI?=1
CONFIG_BLE_STACK_DBG_PRINT?=1
CONFIG_BT_STACK_PTS?=0
CONFIG_BLE_TP_TEST ?= 0
CONFIG_BT_GEN_RANDOM_BY_SW?=0 #If BLE host generate random value by software
CONFIG_DISABLE_BT_SMP ?= 0
CONFIG_DISABLE_BT_HOST_PRIVACY ?= 1
CFG_BLE_PDS?=0
CONFIG_BT_MESH?=0
CONFIG_BT_MESH_MODEL?=0
ifeq ($(CONFIG_BT_MESH),1)
CONFIG_BT_MESH_PB_ADV?=1
CONFIG_BT_MESH_PB_GATT?=1
CONFIG_BT_MESH_FRIEND?=1
CONFIG_BT_MESH_LOW_POWER?=1
CONFIG_BT_MESH_PROXY?=1
CONFIG_BT_MESH_GATT_PROXY?=1
ifeq ($(CONFIG_BT_MESH_MODEL), 1)
CONFIG_BT_MESH_MODEL_GEN_SRV?=1
CONFIG_BT_MESH_MODEL_GEN_CLI?=1
CONFIG_BT_MESH_MODEL_LIGHT_SRV?=1
CONFIG_BT_MESH_MODEL_LIGHT_CLI?=1
else
CONFIG_BT_MESH_MODEL_GEN_SRV?=1
endif
CONFIG_BT_MESH_PROVISIONER?=0
ifeq ($(CONFIG_BT_MESH_PROVISIONER),1)
CONFIG_BT_MESH_CDB?=1
else
CONFIG_BT_MESH_CDB?=0
endif
CONFIG_BT_MESH_SYNC?=0
CONFIG_BT_MESH_NODE_SEND_CFGCLI_MSG?=0 # Config mesh normal node can send configure client message or not.
endif

##########################################
############## BLE STACK #################
##########################################

ifeq ($(CONFIG_BT_ALLROLES),1)

CFLAGS += -DCONFIG_BT_ALLROLES
CFLAGS += -DCONFIG_BT_CENTRAL
CFLAGS += -DCONFIG_BT_OBSERVER
CFLAGS += -DCONFIG_BT_PERIPHERAL
CFLAGS += -DCONFIG_BT_BROADCASTER

else

ifeq ($(CONFIG_BT_CENTRAL),1)
CFLAGS += -DCONFIG_BT_CENTRAL
endif
ifeq ($(CONFIG_BT_OBSERVER),1)
CFLAGS += -DCONFIG_BT_OBSERVER
endif
ifeq ($(CONFIG_BT_PERIPHERAL),1)
CFLAGS += -DCONFIG_BT_PERIPHERAL
endif
ifeq ($(CONFIG_BT_BROADCASTER),1)
CFLAGS += -DCONFIG_BT_BROADCASTER
endif

endif

ifneq ($(CONFIG_DBG_RUN_ON_FPGA), 1)
ifeq ($(CONFIG_BT_SETTINGS), 1)
CFLAGS += -DCONFIG_BT_SETTINGS
endif
endif

ifeq ($(CONFIG_BLE_MFG),1)
CFLAGS += -DCONFIG_BLE_MFG
ifeq ($(CONFIG_BLE_MFG_HCI_CMD),1)
CFLAGS   += -DCONFIG_BLE_MFG_HCI_CMD
endif
endif

ifeq ($(CONFIG_BT_GEN_RANDOM_BY_SW),1)
CFLAGS += -DCONFIG_BT_GEN_RANDOM_BY_SW
endif

ifeq ($(CFG_BLE_PDS),1)
CFLAGS += -DCFG_BLE_PDS
endif

CFLAGS   += -DCONFIG_BT_L2CAP_DYNAMIC_CHANNEL \
			-DCONFIG_BT_GATT_CLIENT \
			-DCONFIG_BT_CONN \
 			-DCONFIG_BT_GATT_DIS_PNP \
 			-DCONFIG_BT_GATT_DIS_SERIAL_NUMBER \
 			-DCONFIG_BT_GATT_DIS_FW_REV \
 			-DCONFIG_BT_GATT_DIS_HW_REV \
 			-DCONFIG_BT_GATT_DIS_SW_REV \
 			-DCONFIG_BT_ECC \
 			-DCONFIG_BT_GATT_DYNAMIC_DB \
 			-DCONFIG_BT_GATT_SERVICE_CHANGED \
 			-DCONFIG_BT_KEYS_OVERWRITE_OLDEST \
 			-DCONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING \
 			-DCONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS \
 			-DCONFIG_BT_BONDABLE \
 			-DCONFIG_BT_HCI_VS_EVT_USER \
 			-DCONFIG_BT_ASSERT 

ifneq ($(CONFIG_DISABLE_BT_SMP), 1)
CFLAGS += -DCONFIG_BT_SMP \
 			-DCONFIG_BT_SIGNING
endif

ifneq ($(CONFIG_DBG_RUN_ON_FPGA), 1)
CFLAGS += -DCONFIG_BT_SETTINGS_CCC_LAZY_LOADING \
 			-DCONFIG_BT_SETTINGS_USE_PRINTK
endif

ifeq ($(CONFIG_BLE_STACK_DBG_PRINT),1)
CFLAGS += -DCFG_BLE_STACK_DBG_PRINT
endif

ifeq ($(CONFIG_BT_DEBUG_MONITOR),1)
CFLAGS += -DCONFIG_BT_DEBUG_MONITOR
endif
ifeq ($(CONFIG_BLE_AT_CMD),1)
CFLAGS += -DCONFIG_BLE_AT_CMD
endif

ifeq ($(CONFIG_BT_OAD_SERVER),1)
CFLAGS += -DCONFIG_BT_OAD_SERVER
CFLAGS += -DCONFIG_BT_SETTINGS
endif
ifeq ($(CONFIG_BT_OAD_CLIENT),1)
CFLAGS += -DCONFIG_BT_OAD_CLIENT
endif
ifeq ($(CONFIG_HOGP_SERVER),1)
CFLAGS += -DCONFIG_HOGP_SERVER
endif
ifeq ($(CONFIG_BT_BAS_SERVER),1)
CFLAGS += -DCONFIG_BT_BAS_SERVER
endif
ifeq ($(CONFIG_BT_SCPS_SERVER),1)
CFLAGS += -DCONFIG_BT_SCPS_SERVER
endif
ifeq ($(CONFIG_BT_DIS_SERVER),1)
CFLAGS += -DCONFIG_BT_DIS_SERVER
endif

ifeq ($(CONFIG_BT_REMOTE_CONTROL),1)
CFLAGS += -DCONFIG_BT_REMOTE_CONTROL
endif

ifneq ($(CONFIG_DISABLE_BT_HOST_PRIVACY), 1)
CFLAGS += -DCONFIG_BT_PRIVACY
endif

#ifneq ($(CONFIG_BT_REMOTE_CONTROL),1)
#ifneq ($(CONFIG_BT_MESH),1)
#CFLAGS += -DCONFIG_BT_PRIVACY
#endif
#endif

ifeq ($(CONFIG_BLE_TP_SERVER),1)
CFLAGS += -DCONFIG_BLE_TP_SERVER
ifeq ($(CONFIG_BLE_TP_TEST),1)
CFLAGS += -DCONFIG_BLE_TP_TEST
endif
endif

ifeq ($(CONFIG_BLE_MULTI_ADV),1)
CFLAGS += -DCONFIG_BLE_MULTI_ADV
endif

ifeq ($(CONFIG_BLE_RECONNECT_TEST),1)
CFLAGS += -DCONFIG_BLE_RECONNECT_TEST
endif

ifeq ($(CONFIG_BT_STACK_PTS),1)
CFLAGS += -DCONFIG_BT_STACK_PTS
endif

ifeq ($(PTS_TEST_CASE_INSUFFICIENT_KEY),1)
CFLAGS += -DPTS_TEST_CASE_INSUFFICIENT_KEY
endif
ifeq ($(PTS_GAP_SLAVER_CONFIG_INDICATE_CHARC),1)
CFLAGS += -DPTS_GAP_SLAVER_CONFIG_INDICATE_CHARC
endif

ifeq ($(CONFIG_ZIGBEE_PROV),1)
CFLAGS += -DCONFIG_ZIGBEE_PROV
endif

ifeq ($(CONFIG_AUTO_PTS),1)
CFLAGS += -DCONFIG_AUTO_PTS
endif

##########################################
############## BLE MESH ##################
##########################################

ifeq ($(CONFIG_BT_MESH),1)

CFLAGS += -DCONFIG_BT_MESH
CFLAGS += -DCONFIG_BT_MESH_PROV
CFLAGS += -DCONFIG_BT_MESH_RELAY
#CFLAGS += -DCONFIG_BT_SETTINGS
ifeq ($(CONFIG_BT_MESH_PB_ADV),1)
CFLAGS += -DCONFIG_BT_MESH_PB_ADV
endif
ifeq ($(CONFIG_BT_MESH_PB_GATT),1)
CFLAGS += -DCONFIG_BT_MESH_PB_GATT
endif
ifeq ($(CONFIG_BT_MESH_FRIEND),1)
CFLAGS += -DCONFIG_BT_MESH_FRIEND
endif
ifeq ($(CONFIG_BT_MESH_LOW_POWER),1)
CFLAGS += -DCONFIG_BT_MESH_LOW_POWER
endif
ifeq ($(CONFIG_BT_MESH_PROXY),1)
CFLAGS += -DCONFIG_BT_MESH_PROXY
endif
ifeq ($(CONFIG_BT_MESH_GATT_PROXY),1)
CFLAGS += -DCONFIG_BT_MESH_GATT_PROXY
endif
ifeq ($(CONFIG_BLE_STACK_DBG_PRINT),1)
CFLAGS += -DCFG_BLE_STACK_DBG_PRINT
endif
ifeq ($(CONFIG_BT_MESH_SYNC),1)
CFLAGS += -DCONFIG_BT_MESH_SYNC
endif
ifeq ($(CONFIG_BT_MESH_NODE_SEND_CFGCLI_MSG),1)
CFLAGS += -DCONFIG_BT_MESH_NODE_SEND_CFGCLI_MSG
endif
ifeq ($(CONFIG_BT_MESH_CDB),1)
CFLAGS += -DCONFIG_BT_MESH_CDB
CFLAGS += -DCONFIG_BT_MESH_CDB_NODE_COUNT=64
CFLAGS += -DCONFIG_BT_MESH_CDB_SUBNET_COUNT=2
CFLAGS += -DCONFIG_BT_MESH_CDB_APP_KEY_COUNT=2
endif 
ifeq ($(CONFIG_BT_MESH_PROVISIONER),1)
CFLAGS += -DCONFIG_BT_MESH_PROVISIONER
CFLAGS += -DCONFIG_BT_MESH_CFG_CLI
CFLAGS += -DCONFIG_BT_MESH_HEALTH_CLI
endif
ifeq ($(CONFIG_BT_MESH_MODEL),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL
CFLAGS += -DCONFIG_BT_MESH_TEST
ifeq ($(CONFIG_BT_MESH_MODEL_GEN_SRV),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL_GEN_SRV
endif 
ifeq ($(CONFIG_BT_MESH_MODEL_GEN_CLI),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL_GEN_CLI
endif 
ifeq ($(CONFIG_BT_MESH_MODEL_LIGHT_SRV),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL_LIGHT_SRV
endif 
ifeq ($(CONFIG_BT_MESH_MODEL_LIGHT_CLI),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL_LIGHT_CLI
endif
ifeq ($(CONFIG_BT_MESH_MODEL_VENDOR_CLI),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL_VENDOR_CLI
endif
else
ifeq ($(CONFIG_BT_MESH_MODEL_GEN_SRV),1)
CFLAGS += -DCONFIG_BT_MESH_MODEL_GEN_SRV
endif 
endif
endif

ifeq ($(CONFIG_USE_XTAL32K),1)
CFLAGS += -DCFG_USE_XTAL32K
endif

CFLAGS   += -Wno-unused-const-variable  \
            -Wno-unused-but-set-variable \
            -Wno-format