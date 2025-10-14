#!/bin/bash
set -e
file_name=$1 
if [[ -z ${file_name} ]];then
	echo "Error Usage: ./b.sh <file_name>" 
	exit 1
fi
# remove extention
file_name=$(echo "${file_name%.*}")

cd .. && cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -D BUILD_TESTING=OFF && cd build && make -j 12 && sudo make install && cd .. && cd examples &&  gcc $file_name.c  -o $file_name -L/usr/local/lib/ -lcjson -lfreetype -lGooeyGUI-1 -I/usr/local/include/ -g  -I/usr/local/include/GLPS/ -lGLPS  -lm && ./$file_name
