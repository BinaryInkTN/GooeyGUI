#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc tabs_example.c  -o tabs_example -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./tabs_example
