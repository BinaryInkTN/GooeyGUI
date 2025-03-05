#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc hello_world.c -g -o hello_world -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./hello_world
