#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := bl602_demo_sdiowifi
PROJECT_PATH := $(abspath .)
PROJECT_BOARD := evb
export PROJECT_PATH PROJECT_BOARD
#CONFIG_TOOLPREFIX :=

-include ./proj_config.mk

ifeq ($(origin BL60X_SDK_PATH), undefined)
BL60X_SDK_PATH_GUESS ?= $(shell pwd)
BL60X_SDK_PATH ?= $(BL60X_SDK_PATH_GUESS)/../..
$(info ****** Please SET BL60X_SDK_PATH ******)
$(info ****** Trying SDK PATH [$(BL60X_SDK_PATH)])
endif

COMPONENTS_NETWORK := sntp
COMPONENTS_BLSYS   := bltime blfdt blmtd blota bloop loopadc looprt loopset
COMPONENTS_VFS     := romfs

INCLUDE_COMPONENTS += freertos_riscv_ram bl602 bl602_std bl602_wifi bl602_wifidrv hal_drv lwip lwip_dhcpd vfs yloop utils cli netutils blog dns_server
INCLUDE_COMPONENTS += throughput
INCLUDE_COMPONENTS += easyflash4
INCLUDE_COMPONENTS += newlibc
INCLUDE_COMPONENTS += $(COMPONENTS_NETWORK)
INCLUDE_COMPONENTS += $(COMPONENTS_BLSYS)
INCLUDE_COMPONENTS += $(COMPONENTS_VFS)
INCLUDE_COMPONENTS += netbus_mgmr
INCLUDE_COMPONENTS += $(PROJECT_NAME)
ifeq ($(CONFIG_SDIO_BLE_ENABLE),1)
INCLUDE_COMPONENTS += blecontroller
endif
include $(BL60X_SDK_PATH)/make_scripts_riscv/project.mk

CPPFLAGS += -DGLOBAL_NETBUS_WIFI_APP_ENABLE
