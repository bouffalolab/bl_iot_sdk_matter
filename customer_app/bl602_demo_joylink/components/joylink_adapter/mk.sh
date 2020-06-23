#!/bin/bash

EXCLUDE_DIR=(
test
target
lua
bin
lib
)

EXCLUDE_NUM=${#EXCLUDE_DIR[@]}

SUBDIRS=$(ls -Rl | grep ^d | awk '{print $9}')
#SUBDIRS1=$(echo $SUBDIRS | sed 's/test//g')
for((i=0; i<$EXCLUDE_NUM; i++));
do 
	#echo "$i. exclude directory \"${EXCLUDE_DIR[$i]}\""
    SUBDIRS=$(echo $SUBDIRS | sed "s/${EXCLUDE_DIR[$i]}//g")
    #echo $SUBDIRS
done

echo $SUBDIRS
#find . \( -path ./test -o -path ./test1 \) -prune -o -type f -name *.c -print


