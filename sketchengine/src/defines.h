// #define DEFINES_IMPL to implement some of the functionalitites (required to do this once)
#ifndef DEFINES_H
#define DEFINES_H

#include <stdio.h> // ! required for printf
#include "SDL2/SDL.h"

/// debugging for SDL2
void print_sdl_error();

#ifdef DEFINES_IMPL
    void print_sdl_error() {
        const char *error = SDL_GetError();
        if (strlen(error) <= 0) {
            int x = 0; // dummy assignment for breakpoints
        }
        printf("ERROR: %s\n", error);
    }
#endif

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

/// inlining
#ifdef _MSC_VER
#define SEINLINE __forceinline
#define SENOINLINE __declspec(noinline)
#else
#define SEINLINE static inline
#define SENOINLINE
#endif // inline

#endif // DEFINES_H