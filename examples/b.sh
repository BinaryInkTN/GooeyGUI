#!/bin/bash

cd .. && cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cd build && make -j 12 && sudo make install && cd .. && cd examples && gcc IoT_dashboard.c  -o bin/IoT_dashboard -L/usr/local/lib/ -lfreetype -lGooeyGUI -lpaho-mqtt3cs -I/usr/local/include/ -I/usr/local/include/Gooey/ -I/usr/local/include/GLPS/ -lGLPS  -lm && ./bin/IoT_dashboard
