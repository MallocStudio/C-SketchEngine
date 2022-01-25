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
// #define DEFAULT_FONT_PATH "assets/fonts/Ya'ahowu/Yaahowu.ttf"
#define DEFAULT_FONT_PATH "assets/fonts/josefin-sans-font/JosefinSansRegular-x3LYV.ttf"
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

///
bool point_in_circle(Vec2i point, Vec2i circle_pos, f32 radius);

// /// ------------- ///
// /// Dynamic Array ///
// /// ------------- ///

// typedef enum TYPES {
//     // we currently don't support TYPE_UNDEFINED, as that would require us
//     // to handle resizing and redefining the type after we use an appropriate
//     // procedure that defines the type we'd like to use (matink 24.1.2022)
//     // ! TYPE_UNDEFINED is currently unsupported
//     TYPE_UNDEFINED, // we're not sure what data type we're gonna use
//     TYPE_VOID,      // this is a void pointer, so any data
//     TYPE_I32,       // i32
//     TYPE_F32,       // f32
//     TYPES_COUNT, // * this entry must be the last one * //
// } TYPES;

// typedef struct Dynamic_Array {
//     void *data;
//     u32 num_of_entries;
//     u32 capacity;
//     TYPES type;
// } Dynamic_Array;
// /// initialise the memory for dynamic array based on the given "capacity"
// /// and the data type we would like to store
// /// "capacity" refers to the number of entries we thing we would require
// void dynamic_array_init(Dynamic_Array *array, i32 capacity, TYPES type);
// void dynamic_array_add_int(Dynamic_Array *array, i32 data);

#endif //CORE_H