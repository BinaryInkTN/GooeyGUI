#!/bin/bash

cd .. && cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cd build && make -j 12 && sudo make install && cd .. && cd examples && gcc robot_dashboard.c  -o bin/robot_dashboard -L/usr/local/lib/ -lfreetype -lGooeyGUI -lpaho-mqtt3c -I/usr/local/include/ -I/usr/local/include/Gooey/ -I/usr/local/include/GLPS/ -lGLPS  -lm && ./bin/robot_dashboard
