## import basic make rule from Makefile.rule
include ./Makefile.rule

## set dependences for the final exacutable file
DEPS= example pal/src

## target for this Makefile
all: ${DEPS} test

## rules for make clean
clean: ${DEPS}

## rules for make distclean
distclean: ${DEPS}
	${RM} ${TARGET_LIB}/*
	${RM} ${TARGET_BIN}/*

## rules for make DEPS
${DEPS}:
	make -C $@ $(MAKECMDGOALS)

## rules for make test that is the final target
test:
	make test -C example

.PHONY:all clean distclean ${DEPS} test
