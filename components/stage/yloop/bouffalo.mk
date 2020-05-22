# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += include
							 
## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS := include

## This component's src 
COMPONENT_SRCS := src/yloop.c \
	              src/select.c \
                  src/aos_freertos.c \
                  src/device.c \
                  src/local_event.c \

				  
COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := src


##
#CPPFLAGS += 
