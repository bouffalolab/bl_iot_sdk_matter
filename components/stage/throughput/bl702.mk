# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl702/inc
							 
## This component's src 
COMPONENT_SRCS := bl702/src/tp_spi_master.c \

				  
COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := bl702/src

##
#CPPFLAGS += 
