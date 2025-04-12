#! /bin/bash
make
cp lib/* ~/.wine/drive_c/windows/system32/
x86_64-w64-mingw32-gcc test.c -L./lib -I./internal -I./include -lGLPS
wine a
