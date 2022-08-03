#!/bin/sh
# Ignore first argument
pydir=$(dirname $0)
shift
for arg in $@; do
    case $arg in
        --prefix|--exec-prefix)
            echo "${pydir}"
            ;;
        --includes)
            echo "-I${pydir}/include/python3.7m"
            ;;
        --libs|--ldflags)
            echo "-L${pydir}/lib/python3.7/config-3.7m-darwin -L${pydir}/lib -lpython3.7m -ldl -framework CoreFoundation"
            ;;
    esac
    shift
done
