color
cls
gcc -o SketchEngine ../src/*.c -I "include/" -L "lib/" -lSDL2_ttf -lmingw32 -lSDL2 -lSDL2main -lglew32 -lglu32 -lopengl32 -g