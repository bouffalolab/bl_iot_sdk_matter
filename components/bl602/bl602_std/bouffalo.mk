# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += StdDriver/Inc \
							 CMSIS/Device/Bouffalo/BL602/Peripherals \
							 RISCV/Device/Bouffalo/BL602/Startup \
							 RISCV/Core/Include \
							 CMSIS/Include \
							 Common/platform_print \
							 Common/soft_crc \

							 
## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=							 


## This component's src 
COMPONENT_SRCS := StdDriver/Src/bl602_uart.c \
                  StdDriver/Src/bl602_sec_eng.c \
                  StdDriver/Src/bl602_dma.c \
                  StdDriver/Src/bl602_common.c \
                  StdDriver/Src/bl602_glb.c \
                  StdDriver/Src/bl602_hbn.c \
                  StdDriver/Src/bl602_timer.c \
                  StdDriver/Src/bl602_aon.c \
                  StdDriver/Src/bl602_pds.c \
                  StdDriver/Src/bl602_l1c.c \
                  StdDriver/Src/bl602_ef_ctrl.c \
                  StdDriver/Src/bl602_sf_ctrl.c \
                  Common/soft_crc/softcrc.c \


COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := . StdDriver/Src Common/soft_crc

##
CPPFLAGS += -DARCH_RISCV -DSTDDRV_VERSION=096d971a96c12b5857abc7606bfd5ac1bf371a41
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBL602_USE_HAL_DRIVER
endif
