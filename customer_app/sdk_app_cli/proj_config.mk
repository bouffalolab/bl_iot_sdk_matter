#
#compiler flag config domain
#
#CONFIG_TOOLPREFIX :=
#CONFIG_OPTIMIZATION_LEVEL_RELEASE := 1
#CONFIG_M4_SOFTFP := 1

#
#board config domain
#
CONFIG_BOARD_FLASH_SIZE := 2

#firmware config domain
#

#set CONFIG_ENABLE_ACP to 1 to enable ACP, set to 0 or comment this line to disable
#CONFIG_ENABLE_ACP:=1
CONFIG_BL_IOT_FW_AP:=1
CONFIG_BL_IOT_FW_AMPDU:=0
CONFIG_BL_IOT_FW_AMSDU:=0
CONFIG_BL_IOT_FW_P2P:=0
CONFIG_ENABLE_PSM_RAM:=1
#CONFIG_ENABLE_CAMERA:=1
#CONFIG_ENABLE_BLSYNC:=1
#CONFIG_ENABLE_VFS_SPI:=1
CONFIG_ENABLE_VFS_ROMFS:=1

# set easyflash env psm size, only support 4K、8K、16K options
CONFIG_ENABLE_PSM_EF_SIZE:=16K

CONFIG_FREERTOS_TICKLESS_MODE:=0
CONFIG_BL602_USE_ROM_DRIVER:=0

CONFIG_BT_CENTRAL:=1
CONFIG_BT_OBSERVER:=1
CONFIG_BT_PERIPHERAL:=1
CONFIG_BT_STACK_CLI:=1

CONFIG_WIFI:=0
