# Software-Raytracer
Originally a single threaded software raytracer in C expanded to be multithreaded using C++

This is compiled using mingw64 with libraries freeglut and freeimage

Compiling:

g++ raytracer.cpp -o raytrace -fpermissive -O2 -lfreeimage -lopengl32 -lglu32 -lfreeglut

Originally written in August 2023
