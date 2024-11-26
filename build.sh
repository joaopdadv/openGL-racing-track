set -xe

clear

mkdir -p build/Debug

# arch64
#clang++ -g -std=c++17 -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib src/main.cpp src/Shader.cpp src/glad.c -lglfw -framework OpenGL -o build/Debug/outDebug
#./build/Debug/outDebug

# linux
#g++ -o main src/main.cpp src/Shader.cpp -lglfw -lGL -lGLEW -lm -I"./include/" -I"./include/irrKlang" ./irrKlang-64bit-1.6.0/bin/linux-gcc-64/libIrrKlang.so -pthread
#./main
