#ifndef SEDEFINES_H
#define SEDEFINES_H

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
typedef unsigned char ubyte;
// typedef enum bool {
//     false, true
// } bool;
typedef unsigned short bool;
#define false 0
#define true 1

typedef struct RGBA {
    ubyte r, g, b, a;   // 0 - 255
} RGBA;
#define TYPEOF_RGBA_OPENGL GL_UNSIGNED_BYTE
typedef struct RGB {
    ubyte r, g, b; // 0 - 255
} RGB;
#define TYPEOF_RGB_OPENGL GL_UNSIGNED_BYTE
typedef struct HSV {
    i32 h; // in degrees [0-360]
    f32 s; // [0-1]
    f32 v; // [0-1]
} HSV;

SEINLINE void rgba_normalise(RGBA *rgb) {
    rgb->r = rgb->r / 255;
    rgb->g = rgb->g / 255;
    rgb->b = rgb->b / 255;
    rgb->a = rgb->a / 255;
}

SEINLINE bool rgba_compare(RGBA v1, RGBA v2/*, f32 tolerance*/) {
    // if (semath_abs(v1.r - v2.r) > tolerance) {
    //     return false;
    // }
    // if (semath_abs(v1.g - v2.g) > tolerance) {
    //     return false;
    // }
    // if (semath_abs(v1.b - v2.b) > tolerance) {
    //     return false;
    // }
    // if (semath_abs(v1.a - v2.a) > tolerance) {
    //     return false;
    // }
    // return true;
    return (v1.r == v2.r && v1.b == v2.b && v1.g == v2.g && v1.a == v2.a);
}

void rgb_to_hsv(RGB rgb, i32 *hue, f32 *saturation, f32 *value);
void hsv_to_rgb(i32 hue, f32 saturation, f32 value, RGB *rgb);
void hsv_to_rgba(i32 hue, f32 saturation, f32 value, RGBA *rgb);

#define RGBA_RED   (RGBA){255, 0, 0, 255}
#define RGBA_BLUE  (RGBA){0, 0, 255, 255}
#define RGBA_GREEN (RGBA){0, 255, 0, 255}
#define RGBA_WHITE (RGBA){255, 255, 255, 255}
#define RGBA_BLACK (RGBA){0, 0, 0, 255}

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
//             SDL_assert("Given a type that's not defined here mate");
//         }
//     }
//     array->data = new(
// }

#endif // SEDEFINES_H