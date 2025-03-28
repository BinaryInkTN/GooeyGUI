#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc builder_example.c  -o builder_example -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./builder_example
