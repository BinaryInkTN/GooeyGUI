#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc counter.c -g -o counter -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./counter
