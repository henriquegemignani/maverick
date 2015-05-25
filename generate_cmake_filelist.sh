#!/bin/bash

OUTPUT_FILE=src_list.cmake

cd src
FILES=`find */ -name "*.h" -o -name "*.cc" | sort`
echo "SET(GAME_SRCS " > $OUTPUT_FILE
for f in $FILES; do
	echo "    $f " >> $OUTPUT_FILE
done
echo ")" >> ./src_list.cmake
echo "File $OUTPUT_FILE generated."
