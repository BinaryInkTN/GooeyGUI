#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc todolist.c  -o todolist -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./todolist
