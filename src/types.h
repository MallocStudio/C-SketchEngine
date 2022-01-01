#ifndef TYPES_H
#define TYPES_H

/// sdl2 types
#include "SDL_types.h"

/// primitive types
typedef float     f32;
typedef int       i32;
typedef uint32_t  u32;
typedef Uint8     u8;
typedef enum {
    false = 0,
    true  = 1,
} bool;
typedef SDL_Rect  Rect;
typedef SDL_FRect RectF;
typedef SDL_Point Vec2i;

/// color types
typedef struct {
    f32 r, g, b, a; // from 0 - 1
} RGBA;

#endif // TYPES_H