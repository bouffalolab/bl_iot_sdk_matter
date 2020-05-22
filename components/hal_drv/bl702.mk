# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl702_hal

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := bl702_hal/bl_uart.c \
                  bl702_hal/bl_chip.c \
                  bl702_hal/bl_irq.c \
                  bl702_hal/bl_sec.c \
                  bl702_hal/hal_uart.c \
                  bl702_hal/hal_sys.c \

COMPONENT_SRCDIRS := bl702_hal

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

##
CPPFLAGS += -DARCH_RISCV
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBL702_USE_HAL_DRIVER
endif
