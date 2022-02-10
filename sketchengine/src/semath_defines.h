#ifndef SEMATH_TYPES
#define SEMATH_TYPES
/// note: this math library was made while following the Kohi game engine tutorial series
/// MATH TYPES
/// ----------
#include "defines.h"

// * note: we use right handed coordinate system

typedef struct vec2 {
    f32 x, y;
} vec2;

typedef struct vec3 {
    f32 x, y, z;
} vec3;

typedef struct vec4 {
    f32 x, y, z, w;
} vec4;

// typedef vec4 quat; // think of quaternions as oriantation instead of rotations

/// column major mat2
typedef struct mat2 {
    f32 m0, m2; // row 1
    f32 m1, m3; // row 2
} mat2;

/// column major mat4
typedef struct mat4 {
    // f32 m0, m4, m8, m12;  // row 1
    // f32 m1, m5, m9, m13;  // row 2
    // f32 m2, m6, m10, m14; // row 3
    // f32 m3, m7, m11, m15; // row 4
    f32 data[16];
} mat4;

#endif // SEMATH_TYPES