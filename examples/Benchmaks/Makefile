


buildLib: 
	cd ../.. && cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cd build && make -j 12 && sudo make install


stress_test_grid: buildLib
	gcc $@.c  -o $@ -L/usr/local/lib/ -lfreetype -lGooeyGUI -I/usr/local/include/ -I/usr/local/include/Gooey/ -I/usr/local/include/GLPS/ -lGLPS  -lm 
