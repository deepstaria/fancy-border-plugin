all:
	$(CXX) -shared -fPIC --no-gnu-unique main.cpp borderDeco.cpp -o fancy-border.so -g `pkg-config --cflags pixman-1 libdrm hyprland` -std=c++2b -O2
clean:
	rm ./fancy-border.so
