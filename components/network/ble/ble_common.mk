CFLAGS   += -DBFLB_BLE
CFLAGS   += -DCFG_BLE
CFLAGS   += -DCFG_CON=2
CFLAGS   += -DCFG_SLEEP

CFLAGS   += -DCFG_FREERTOS
CFLAGS   += -DARCH_RISCV

ifeq ($(CONFIG_CHIP_NAME),BL602)
CFLAGS   += -DBL602
endif

ifeq ($(CONFIG_CHIP_NAME),BL702)
CFLAGS   += -DBL702
endif

ifeq ($(CONFIG_DBG_RUN_ON_FPGA), 1)
CFLAGS   += -DCFG_DBG_RUN_ON_FPGA
endif

ifeq ($(CONFIG_BT_CENTRAL),1)
CFLAGS += -DCONFIG_BT_CENTRAL
endif
ifeq ($(CONFIG_BT_OBSERVER),1)
CFLAGS += -DCONFIG_BT_OBSERVER
endif
ifeq ($(CONFIG_BT_PERIPHERAL),1)
CFLAGS += -DCONFIG_BT_PERIPHERAL
endif

ifneq ($(CONFIG_DBG_RUN_ON_FPGA), 1)
CFLAGS += -DCONFIG_BT_SETTINGS
endif