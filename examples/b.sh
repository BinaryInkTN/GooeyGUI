#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc colormixer.c  -o colormixer -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./colormixer
