all:
	$(CXX) -shared -fPIC --no-gnu-unique main.cpp borderDeco.cpp OpenGL.cpp -o fancy-borders.so -g `pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon` -std=c++2b -O2
clean:
	rm ./fancy-borders.so
