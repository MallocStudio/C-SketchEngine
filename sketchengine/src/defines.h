#ifndef DEFINES_H
#define DEFINES_H

#include "SDL2/SDL.h"

/// debugging for SDL2
void print_sdl_error();

#define new(type) ( type *) malloc (sizeof( type ))
#define ERROR_ON_NOTZERO_SDL(x, additional_message) if( x != 0) {printf("(%s)\n", additional_message); print_sdl_error();}
#define ERROR_ON_NULL_SDL(x, additional_message) if( x == NULL) {printf("(%s)\n", additional_message); print_sdl_error();}

typedef int i32;
typedef unsigned int u32;
typedef uint64_t u64;
typedef short i16;
typedef char u8;
typedef float f32;
typedef long long f64;

typedef enum bool {
    false, true
} bool;

typedef struct RGBA {
    f32 r, g, b, a;
} RGBA;
typedef struct RGB {
    f32 r, g, b;
} RGB;

/// inlining
#ifdef _MSC_VER
#define SEINLINE __forceinline
#define SENOINLINE __declspec(noinline)
#else
#define SEINLINE static inline
#define SENOINLINE
#endif // inline

#endif // DEFINES_H