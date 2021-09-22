# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl602u_hal platform_hal

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := bl602u_hal/bl_uart.c \
                  bl602u_hal/bl_chip.c \
                  bl602u_hal/bl_cks.c \
                  bl602u_hal/bl_sys.c \
                  bl602u_hal/bl_sys_cli.c \
                  bl602u_hal/bl_dma.c \
                  bl602u_hal/bl_irq.c \
                  bl602u_hal/bl_sec.c \
                  bl602u_hal/bl_boot2.c \
                  bl602u_hal/bl_timer.c \
                  bl602u_hal/bl_gpio.c \
                  bl602u_hal/bl_gpio_cli.c \
                  bl602u_hal/bl_hbn.c \
                  bl602u_hal/bl_flash.c \
                  bl602u_hal/bl_pwm.c \
                  bl602u_hal/bl_sec_aes.c \
                  bl602u_hal/bl_sec_sha.c \
                  bl602u_hal/bl_wifi.c \
                  bl602u_hal/bl_wdt.c \
                  bl602u_hal/bl_wdt_cli.c \
                  bl602u_hal/hal_uart.c \
                  bl602u_hal/hal_gpio.c \
                  bl602u_hal/hal_hbn.c \
                  bl602u_hal/hal_pwm.c \
                  bl602u_hal/hal_boot2.c \
                  bl602u_hal/hal_sys.c \
                  bl602u_hal/hal_board.c \
                  bl602u_hal/hal_ir.c \
                  bl602u_hal/hal_button.c \
                  bl602u_hal/hal_hbnram.c \
                  bl602u_hal/bl_rtc.c \


COMPONENT_SRCDIRS := bl602u_hal platform_hal

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

##
ifeq ($(CONFIG_BT),1)
CPPFLAGS += -DCFG_BLE_ENABLE
else
endif

CPPFLAGS += -DARCH_RISCV
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBL602_USE_HAL_DRIVER
endif

ifeq ($(CONFIG_USE_XTAL32K),1)
CFLAGS += -DCFG_USE_XTAL32K
endif
