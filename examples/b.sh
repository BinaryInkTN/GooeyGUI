#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc plots.c -g -o plots -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./plots
