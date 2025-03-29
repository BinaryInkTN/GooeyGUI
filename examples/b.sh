#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc dropdown_example.c  -o dropdown_example -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./dropdown_example
