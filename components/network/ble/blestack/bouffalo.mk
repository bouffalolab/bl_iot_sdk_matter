##################################################################################
#
# Component Makefile
#
##################################################################################
ble_stack_srcs_dirs := src/bl_hci_wrapper \
					src/port \
					src/common \
					src/common/tinycrypt/source  \
					src/hci_onchip   \
					src/host   \
					src/host_cmdproc   \
					src/services

ifeq ($(CONFIG_BT_OAD_SERVER),1)
ble_stack_srcs_dirs+= src/profiles/oad
endif

ifeq ($(CONFIG_BT_STACK_CLI),1)
ble_stack_srcs_dirs+= src/cli_cmds
endif

ble_stack_srcs_include_dirs    += src/port/include \
								src/common/include \
								src/common/include/zephyr  \
								src/common/include/misc  \
								src/common/include/common  \
								src/common/tinycrypt/include/tinycrypt  \
								src/hci_onchip   \
								src/bl_hci_wrapper \
								src/host   \
								src/include/bluetooth  \
								src/include/drivers/bluetooth  \
								src/profiles \
								src/host_cmdproc \
								src/cli_cmds \
								src/services

ifneq ($(CONFIG_BT_OAD_SERVER)_$(CONFIG_BT_OAD_CLIENT),0_0)
ble_stack_srcs_include_dirs    += src/profiles/oad
endif

# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS := $(ble_stack_srcs_include_dirs)

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS   :=

## This component's src 
ble_stack_srcs  := src/port/bl_port.c \
					src/common/atomic_c.c \
					src/common/buf.c \
					src/common/log.c \
					src/common/poll.c \
					src/common/rpa.c \
					src/common/work_q.c \
					src/common/utils.c \
					src/common/tinycrypt/source/aes_decrypt.c \
					src/common/tinycrypt/source/aes_encrypt.c \
					src/common/tinycrypt/source/cbc_mode.c \
					src/common/tinycrypt/source/ccm_mode.c \
					src/common/tinycrypt/source/cmac_mode.c \
					src/common/tinycrypt/source/ctr_mode.c \
					src/common/tinycrypt/source/ctr_prng.c \
					src/common/tinycrypt/source/ecc.c \
					src/common/tinycrypt/source/ecc_dh.c \
					src/common/tinycrypt/source/ecc_dsa.c \
					src/common/tinycrypt/source/ecc_platform_specific.c \
					src/common/tinycrypt/source/hmac.c \
					src/common/tinycrypt/source/hmac_prng.c \
					src/common/tinycrypt/source/sha256.c \
					src/common/tinycrypt/source/utils.c \
					src/bl_hci_wrapper/bl_hci_wrapper.c \
					src/hci_onchip/hci_driver.c \
					src/host/att.c \
					src/host/conn.c \
					src/host/crypto.c \
					src/host/gatt.c \
					src/host/hci_core.c \
					src/host/hci_ecc.c \
					src/host/keys.c \
					src/host/l2cap.c \
					src/host/smp.c \
					src/host/settings.c \
					src/host/uuid.c \
					
ifeq ($(CONFIG_BT_OAD_CLIENT),1)
ble_stack_srcs   += src/host_cmdproc/oadc_cmdproc.c
endif

ifeq ($(CONFIG_BT_OAD_SERVER),1)
ble_stack_srcs   += src/profiles/oad/oad_api.c \
					src/profiles/oad/oad_service.c
endif

ifeq ($(CONFIG_BT_STACK_CLI),1)
ble_stack_srcs   += src/cli_cmds/stack_cli_cmds.c
endif

ifeq ($(CONFIG_BT_HOG_SERVER),1)
ble_stack_srcs   += src/services/hog.c
endif

ifeq ($(CONFIG_BT_BAS_SERVER),1)
ble_stack_srcs   += src/services/bas.c
endif

ifeq ($(CONFIG_BT_SCPS_SERVER),1)
ble_stack_srcs   += src/services/scps.c
endif

ifeq ($(CONFIG_BT_DIS_SERVER),1)
ble_stack_srcs   += src/services/dis.c
endif

ifeq ($(CONFIG_BT_WIFIPROV_SERVER),1)
ble_stack_srcs   += src/services/wifi_prov.c
endif

COMPONENT_SRCS := $(ble_stack_srcs)

COMPONENT_OBJS   := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := $(ble_stack_srcs_dirs)

CFLAGS   += -DCONFIG_BT_SMP \
			-DCONFIG_BT_SIGNING \
			-DCONFIG_BT_L2CAP_DYNAMIC_CHANNEL \
			-DCONFIG_BT_GATT_CLIENT \
			-DCONFIG_BT_CONN \
 			-DCONFIG_BT_GATT_DIS_PNP \
 			-DCONFIG_BT_GATT_DIS_SERIAL_NUMBER \
 			-DCONFIG_BT_GATT_DIS_FW_REV \
 			-DCONFIG_BT_GATT_DIS_HW_REV \
 			-DCONFIG_BT_GATT_DIS_SW_REV
#CFLAGS += -DCONFIG_BT_SETTINGS

ifeq ($(CONFIG_BT_CENTRAL),1)
CFLAGS += -DCONFIG_BT_CENTRAL
endif
ifeq ($(CONFIG_BT_OBSERVER),1)
CFLAGS += -DCONFIG_BT_OBSERVER
endif
ifeq ($(CONFIG_BT_PERIPHERAL),1)
CFLAGS += -DCONFIG_BT_PERIPHERAL
endif
ifeq ($(CONFIG_BLE_STACK_DBG_PRINT),1)
CFLAGS += -DCFG_BLE_STACK_DBG_PRINT
endif
ifeq ($(CONFIG_BT_OAD_SERVER),1)
CFLAGS += -DCONFIG_BT_OAD_SERVER
endif
ifeq ($(CONFIG_BT_OAD_CLIENT),1)
CFLAGS += -DCONFIG_BT_OAD_CLIENT
endif
ifeq ($(CONFIG_BT_REMOTE_CONTROL),1)
CFLAGS += -DCONFIG_BT_REMOTE_CONTROL
endif
ifneq ($(CONFIG_BT_REMOTE_CONTROL),1)
ifneq ($(CONFIG_BT_MESH),1)
CFLAGS += -DCONFIG_BT_PRIVACY
endif
endif
ifeq ($(CONFIG_BT_MESH),1)
CFLAGS += -DCONFIG_BT_MESH
endif

CFLAGS   += -Wno-unused-const-variable  \
            -Wno-unused-but-set-variable \
            -Wno-format

include $(COMPONENT_PATH)/../ble_common.mk