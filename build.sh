set -xe

g++ -o main main.cpp \
  -lglfw -lGL -lGLEW -lm \
  -I"./include/irrKlang" \
  ./lib/bin/linux-gcc-64/libIrrKlang.so \
  && ./main