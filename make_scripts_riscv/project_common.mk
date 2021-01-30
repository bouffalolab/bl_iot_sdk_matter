#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

ifeq ("$(CONFIG_CHIP_NAME)", "BL606p")
CFLAGS += -DBFLB_USE_HAL_DRIVER -DCPU_M1 -DARCH_RISCV
include $(BL60X_SDK_PATH)/make_scripts_thead_riscv/project.mk
else
ifeq ("$(CONFIG_CHIP_NAME)", "BL602")
include $(BL60X_SDK_PATH)/make_scripts_riscv/project.mk
endif
endif

