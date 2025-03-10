#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc drop_surface.c  -o drop_surface -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/  -lm -fsanitize=address,undefined && ./drop_surface
