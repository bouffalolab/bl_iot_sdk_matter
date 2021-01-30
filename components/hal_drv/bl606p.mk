# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl606p_hal

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := bl606p_hal/bl_uart.c \
	bl606p_hal/bl_irq.c \
	bl606p_hal/bl_sec.c \
	bl606p_hal/bl_chip.c	\
	bl606p_hal/bl_timer.c 	\
	bl606p_hal/hal_sys.c	\
	bl606p_hal/hal_uart.c	

ifeq ($(CONFIG_WIFI),1)
COMPONENT_SRCS += bl606p_hal/bl_wifi.c \
		  bl606p_hal/hal_wifi.c 
endif
COMPONENT_SRCDIRS := bl606p_hal

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

##
ifeq ($(CONFIG_BT),1)
CPPFLAGS += -DCFG_BLE_ENABLE
else
endif

CPPFLAGS += -DARCH_RISCV
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBL606P_USE_HAL_DRIVER
endif

ifeq ($(CONFIG_USE_XTAL32K),1)
CFLAGS += -DCFG_USE_XTAL32K
endif
CFLAGS += -DCPU_M1
