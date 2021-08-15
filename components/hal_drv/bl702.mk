# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl702_hal .

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := bl702_hal/bl_uart.c \
                  bl702_hal/bl_chip.c \
                  bl702_hal/bl_cks.c \
                  bl702_hal/bl_sys.c \
                  bl702_hal/bl_dma.c \
                  bl702_hal/bl_irq.c \
                  bl702_hal/bl_sec.c \
                  bl702_hal/bl_boot2.c \
                  bl702_hal/bl_timer.c \
                  bl702_hal/bl_gpio.c \
                  bl702_hal/bl_efuse.c \
                  bl702_hal/bl_flash.c \
                  bl702_hal/bl_sec_aes.c \
                  bl702_hal/bl_sec_sha.c \
                  bl702_hal/bl_wireless.c \
                  bl702_hal/bl_wdt.c \
                  bl702_hal/hal_uart.c \
                  bl702_hal/hal_gpio.c \
                  bl702_hal/hal_boot2.c \
                  bl702_hal/hal_sys.c \
                  bl702_hal/hal_board.c \
                  bl702_hal/bl_pds./*
 * hosal_dma.h
 *
 *  Created on: 2021年7月30日
 *      Author: yangyue
 */

#ifndef COMPONENTS_HAL_DRV_HOSAL_DMA_H_
#define COMPONENTS_HAL_DRV_HOSAL_DMA_H_



#endif /* COMPONENTS_HAL_DRV_HOSAL_DMA_H_ */
                  c \
                  bl702_hal/hal_pds.c \
                  bl702_hal/bl_rtc.c \
                  bl702_hal/hal_hwtimer.c \
                  bl702_hal/hosal_dma.c \

ifeq ($(CONFIG_USE_CAMERA),1)
COMPONENT_SRCS += bl702_hal/bl_cam.c
endif

COMPONENT_SRCDIRS := bl702_hal

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

##
CPPFLAGS += -DARCH_RISCV
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBFLB_USE_HAL_DRIVER
endif

ifeq ($(CONFIG_USE_XTAL32K),1)
CFLAGS += -DCFG_USE_XTAL32K
endif
