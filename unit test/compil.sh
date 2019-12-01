#!/bin/bash

if [ "$1" != "quick" ]; then
../setup.sh $1 $2
fi
gcc test.c libmlx.a -framework OpenGL -framework AppKit -lud_file -lud_image -o executable
#./executable
