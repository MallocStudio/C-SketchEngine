#ifndef CORE_H
#define CORE_H

#include "SDL.h"
#include "SDL_ttf.h"
#include "types.h"
#include "math.h"
#include <stdio.h>
#include <memory.h>

#define new(x) ( x *) malloc (sizeof( x ))

///
typedef struct {
    SDL_Window   *window;   // not owned
    SDL_Renderer *renderer; // not owned
    i32 window_width;
    i32 window_height;
    i32 keyboard[];
} App;

App *global_app;
f32  delta_time;

/// debugging for SDL2
void print_sdl_error();
void print_ttf_error();

#endif //CORE_H