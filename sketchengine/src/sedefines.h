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

SEINLINE void hsv_clamp(HSV *hsv) {
    if (hsv->h > 359) hsv->h = 0;
    if (hsv->h < 0) hsv->h = 359;
    if (hsv->s > 1) hsv->s = 1;
    if (hsv->s < 0) hsv->s = 0;
    if (hsv->v > 1) hsv->v = 1;
    if (hsv->v < 0) hsv->v = 0;
}

SEINLINE void rgba_normalise(RGBA *rgb) {
    rgb->r = rgb->r / 255;
    rgb->g = rgb->g / 255;
    rgb->b = rgb->b / 255;
    rgb->a = rgb->a / 255;
}

SEINLINE bool rgba_compare(RGBA v1, RGBA v2/*, f32 tolerance*/) {
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

typedef struct SE_Array_F32 {
    u32 count;
    u32 capacity;
    f32 *data;
} SE_Array_F32;

SEINLINE void searray_f32_init(SE_Array_F32 *array, u32 initial_capacity) {
    array->count = 0;
    array->capacity = initial_capacity;
    array->data = malloc(sizeof(f32) * array->capacity);
}

SEINLINE void searray_f32_clear(SE_Array_F32 *array) {
    array->count = 0;
}

SEINLINE void searray_f32_resize(SE_Array_F32 *array, u32 new_capacity) {
    if (new_capacity > array->capacity) {
        array->capacity = new_capacity;
        array->data = realloc(array->data, sizeof(f32) * array->capacity);
    }
}

SEINLINE void searray_f32_add(SE_Array_F32 *array, f32 value) {
    if (array->count >= array->capacity) {
        searray_f32_resize(array, array->capacity + array->capacity * 0.5f);
    }
    array->data[array->count] = value;
    array->count++;
}

SEINLINE f32 searray_f32_get(SE_Array_F32 *array, u32 index) {
    return array->data[index];
}

SEINLINE void searray_f32_deinit(SE_Array_F32 *array) {
    free(array->data);
    array->count = 0;
    array->capacity = 0;
}

typedef struct SE_Array_I32 {
    u32 count;
    u32 capacity;
    i32 *data;
} SE_Array_I32;

SEINLINE void searray_i32_init(SE_Array_I32 *array, u32 initial_capacity) {
    array->count = 0;
    array->capacity = initial_capacity;
    array->data = malloc(sizeof(i32) * array->capacity);
}

SEINLINE void searray_i32_deinit(SE_Array_I32 *array) {
    free(array->data);
    array->count = 0;
    array->capacity = 0;
}

SEINLINE void searray_i32_clear(SE_Array_I32 *array) {
    array->count = 0;
}

SEINLINE void searray_i32_resize(SE_Array_I32 *array, u32 new_capacity) {
    if (new_capacity > array->capacity) {
        array->capacity = new_capacity;
        array->data = realloc(array->data, sizeof(i32) * array->capacity);
    }
}

SEINLINE void searray_i32_add(SE_Array_I32 *array, i32 value) {
    if (array->count >= array->capacity) {
        searray_i32_resize(array, array->capacity + array->capacity * 0.5f);
    }
    array->data[array->count] = value;
    array->count++;
}

SEINLINE i32 searray_i32_get(SE_Array_I32 *array, u32 index) {
    return array->data[index];
}

#endif // SEDEFINES_H