# Component Makefile
#
## These include paths would be exported to project level
ifneq ($(CONFIG_CHIP_NAME),BL606)
COMPONENT_ADD_INCLUDEDIRS += bl60x_hal platform_hal
else
COMPONENT_ADD_INCLUDEDIRS += bl602_hal
endif

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
ifneq ($(CONFIG_CHIP_NAME),BL606)
COMPONENT_SRCS := bl60x_hal/bl_adc.c \
                  bl60x_hal/bl_cam.c \
                  bl60x_hal/bl_dma.c \
                  bl60x_hal/bl_efuse.c \
                  bl60x_hal/bl_gpio.c \
                  bl60x_hal/bl_i2s.c \
                  bl60x_hal/bl_cpu.c \
                  bl60x_hal/bl_psram.c \
                  bl60x_hal/bl_sdh.c \
                  bl60x_hal/bl_spi.c \
                  bl60x_hal/bl_sys.c \
                  bl60x_hal/bl_uart.c \
                  bl60x_hal/bl_boot2.c \
                  bl60x_hal/bl_sec.c \
                  bl60x_hal/bl_hbn.c \
                  bl60x_hal/bl_timer.c \
                  bl60x_hal/hal_adc.c \
                  bl60x_hal/hal_sdh.c \
                  bl60x_hal/hal_spi.c \
                  bl60x_hal/hal_sys.c \
                  bl60x_hal/hal_uart.c \
                  bl60x_hal/hal_gpio.c \
                  bl60x_hal/hal_boot2.c \
                  bl60x_hal/softcrc.c \
                  bl60x_hal/hal_sec.c \
                  bl60x_hal/hal_hbn.c \
                  bl60x_hal/hal_timer.c \

ifneq ($(CONFIG_MCU_ONLY),1)
COMPONENT_SRCS += bl60x_hal/bl_wifi.c
endif

COMPONENT_SRCDIRS := bl60x_hal platform_hal

else
COMPONENT_SRCS := bl602_hal/
endif

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

##
#CPPFLAGS +=
