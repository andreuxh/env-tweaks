all: ../lib/libenvtweaks.so

../lib/libenvtweaks.so: envtweaks.cc
	g++ -shared -fPIC -std=c++0x -o $@ $^ -ldl
