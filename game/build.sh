#x86_64-w64-mingw32-gcc -o SketchEngine ../src/*.c -I "../lib/include/SDL2/" -L "../lib/lib/" -lmingw32 -lSDL2_ttf -lSDL2 -lSDL2main -g
gcc -o SketchEngine ../src/*.c -I "../lib/include/SDL2/" -L "../lib/lib/" -lSDL2_ttf -lSDL2 -lSDL2main -g
