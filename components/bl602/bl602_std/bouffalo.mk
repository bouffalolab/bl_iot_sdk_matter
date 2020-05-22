# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl602_std/StdDriver/Inc \
							 bl602_std/Device/Bouffalo/BL602/Peripherals \
							 bl602_std/Device/Bouffalo/BL602/Startup \
							 bl602_std/RISCV/Core/Include \
							 bl602_std/Include \
							 bl602_std/Common/platform_print \
							 bl602_std/Common/soft_crc \

							 
## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=							 


## This component's src 
COMPONENT_SRCS := bl602_std/StdDriver/Src/bl602_uart.c \
                  bl602_std/StdDriver/Src/bl602_sec_eng.c \
                  bl602_std/StdDriver/Src/bl602_dma.c \
                  bl602_std/StdDriver/Src/bl602_common.c \
                  bl602_std/StdDriver/Src/bl602_glb.c \
                  bl602_std/StdDriver/Src/bl602_hbn.c \
                  bl602_std/StdDriver/Src/bl602_timer.c \
                  bl602_std/StdDriver/Src/bl602_aon.c \
                  bl602_std/StdDriver/Src/bl602_pds.c \
                  bl602_std/StdDriver/Src/bl602_l1c.c \
                  bl602_std/StdDriver/Src/bl602_ef_ctrl.c \
                  bl602_std/StdDriver/Src/bl602_sf_ctrl.c \
                  bl602_std/StdDriver/Src/bl602_sflash.c \
                  bl602_std/StdDriver/Src/bl602_sf_cfg.c  \
                  bl602_std/StdDriver/Src/bl602_xip_sflash.c \
                  bl602_std/Common/soft_crc/softcrc.c \


COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := bl602_std bl602_std/StdDriver/Src bl602_std/Common/soft_crc

##
CPPFLAGS += -DARCH_RISCV -DSTDDRV_VERSION=096d971a96c12b5857abc7606bfd5ac1bf371a41
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBL602_USE_HAL_DRIVER
endif
