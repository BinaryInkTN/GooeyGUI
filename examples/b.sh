#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc amp.c  -o amp -L/usr/local/lib -lfreetype -lGooeyGUI -I/usr/local/include/Gooey/ -I/usr/local/include/GLPS/ -lGLPS  -lm -fsanitize=address,undefined && ./amp
