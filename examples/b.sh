#!/bin/bash

cd .. && cd build && make -j 12 && sudo make install && cd .. && cd examples && gcc IoT_dashboard.c  -o bin/tabs_example -L/usr/local/lib/ -lfreetype -lGooeyGUI -lpaho-mqtt3c -I/usr/local/include/ -I/usr/local/include/Gooey/ -I/usr/local/include/GLPS/ -lGLPS  -lm -fsanitize=address,undefined && ./bin/IoT_dashboard
