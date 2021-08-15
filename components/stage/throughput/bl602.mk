# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl602/inc .
							 
## This component's src 
COMPONENT_SRCS := bl602/src/tp_flow.c \
                  bl602/src/tp_spi_slave.c \
                  bl602/src/ringbuf.c \
                  tp_io.c \
				  
COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := bl602/src .

##
#CPPFLAGS += 
