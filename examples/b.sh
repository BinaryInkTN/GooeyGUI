#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc todolist.c -g -o todolist -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./todolist
