#ifndef CORE_H
#define CORE_H

#include "SDL.h"
#include "types.h"
#include "math.h"
#include <stdio.h>

///
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    i32 window_width;
    i32 window_height;
    i32 keyboard[];
} App;

App *global_app;
f32  delta_time;

/// debugging for SDL2
void print_sdl_error();

#endif //CORE_H