#ifndef DEFINES_H
#define DEFINES_H

/// inlining
#ifdef _MSC_VER
#define SEINLINE __forceinline
#define SENOINLINE __declspec(noinline)
#else
#define SEINLINE static inline
#define SENOINLINE
#endif // inline

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

// ///
// /// "dynamic" array type
// ///
// typedef enum ARRAY_TYPES {
//     I32, F32, U32,
//     VEC2, VEC3, VEC4, MAT3, MAT4, QUATERNION,
//     COUNT
// } ARRAY_TYPES;

// typedef struct Array {
//     u32 count;
//     u32 capacity;
//     ARRAY_TYPES type;
//     void *data;
// } Array;

// SEINLINE void array_init(Array *array, ARRAY_TYPES type, u32 capacity) {
//     u32 type_size = 0;
//     switch (type) {
//         case I32: {

//         } break;
//         case F32: {

//         } break;
//         case U32: {

//         } break;
//         case VEC2: {

//         } break;
//         case VEC3: {

//         } break;
//         default : {
//             SDL_assert("Given a type that's not defines here mate");
//         }
//     }
//     array->data = new(
// }

#endif // DEFINES_H