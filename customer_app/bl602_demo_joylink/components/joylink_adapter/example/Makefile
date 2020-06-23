## import basic make rule from Makefile.rule
include ../Makefile.rule

## set header and source files
HEADERS = $(wildcard *.h)
ORG_SOURCES = $(wildcard *.c)
OUT_SRC = test.c joylink_porting_layer.c

# Please comment this line if you need the OTA example for Linux compiled
OUT_SRC += joylink_extern_ota.c joylink_extern_user.c
## set library names
SOURCES=$(filter-out ${OUT_SRC}, ${ORG_SOURCES})
OBJS = $(patsubst %.c, %.o, $(SOURCES))
LIBNAME = $(strip ${shell pwd |xargs basename})

## set include path
INCLUDES += -I${PROJECT_ROOT_PATH}/joylink/inc
INCLUDES += -I${PROJECT_ROOT_PATH}/joylink/inc/json
INCLUDES += -I${PROJECT_ROOT_PATH}/joylink/inc/auth
INCLUDES += -I${PROJECT_ROOT_PATH}/joylink/inc/list
INCLUDES += -I${PROJECT_ROOT_PATH}/joylink/inc/softap
INCLUDES += -I${PROJECT_ROOT_PATH}/pal/inc

## set static libraries
STATIC_LIBS += ${PROJECT_ROOT_PATH}/joylink/lib/${PLATFORM}/libjoylink.a
STATIC_LIBS += ${TARGET_LIB}/lib${PLATFORM}.a
STATIC_LIBS += ${TARGET_LIB}/libexample.a

## set library of math
LIBS += -lm

ifeq (${ARCH}, x86)
all:${OBJS} liba libso
else
all:${OBJS} liba
endif

## rules to make *.o
.SUFFIXES: .c .o
.c.o:
	${CC} ${CFLAGS} -c $(INCLUDES) $(LIBS) $*.c

## rules to make static libarary
liba:
	${AR} -crs lib${LIBNAME}.a ${OBJS}
	${MV} lib${LIBNAME}.a ${TARGET_LIB}

## rules to make shared libarary
libso:
	${CC}  ${OBJS} -shared -fPIC -o lib${LIBNAME}.so
	${MV} lib${LIBNAME}.so ${TARGET_LIB}

## rules to make binery file for test
test:
	${CC} -D_TEST_ test.c ${OBJS} -o $@ ${CFLAGS} ${INCLUDES} ${STATIC_LIBS} ${LIBS}
	${MV} test ${TARGET_BIN}

## rules to make clean
clean:
	${RM} *.o *.so *.a test

## rules to make distclean
distclean:clean
	${RM} ./*.a ./*.so ${TARGET_LIB}/lib${LIBNAME}.*

.PHONY:all clean test
