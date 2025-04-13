#!/bin/bash

cd .. && cd build && make && sudo make install && cd .. && cd examples && gcc IoT_dashboard.c  -o bin/IoT_dashboard -L/usr/local/lib/ -lfreetype -lGooeyGUI -lpaho-mqtt3c -I/usr/local/include/ -I/usr/local/include/Gooey/ -I/usr/local/include/GLPS/ -lGLPS  -lm -fsanitize=address,undefined && ./bin/IoT_dashboard
