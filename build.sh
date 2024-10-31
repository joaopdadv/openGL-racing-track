set -xe

mkdir -p build/Debug

# arch64
clang++ -g -std=c++17 -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib src/main.cpp src/Shader.cpp src/glad.c -lglfw -framework OpenGL -o build/Debug/outDebug

./build/Debug/outDebug