#ifndef CORE_H
#define CORE_H

#include "SDL.h"
#include "SDL_ttf.h"
#include "types.h"
#include "math.h"
#include <stdio.h>
#include <memory.h>

#define new(type) ( type *) malloc (sizeof( type ))
#define ERROR_ON_NOTZERO_SDL(x, additional_message) if( x != 0) {printf("(%s)\n", additional_message); print_sdl_error();}
#define ERROR_ON_NULL_SDL(x, additional_message) if( x == NULL) {printf("(%s)\n", additional_message); print_sdl_error();}
#define DEFAULT_FONT_PATH "assets/fonts/Ya'ahowu/Yaahowu.ttf"
///
typedef struct {
    SDL_Window   *window;   // not owned
    // SDL_Renderer *sdl_renderer; // not owned
    Renderer *renderer; // ! owned
    i32 window_width;
    i32 window_height;
    i32 keyboard[];
} App;
void init_app   (App *app);
void deinit_app (App *app);

// App *global_app;
static f32 delta_time;

/// debugging for SDL2
void print_sdl_error();
void print_ttf_error();

bool point_in_circle(Vec2i point, Vec2i circle_pos, f32 radius);

#endif //CORE_H