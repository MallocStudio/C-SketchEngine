#ifndef SEMATH_TYPES
#define SEMATH_TYPES
/// note: this math library was made while following the Kohi game engine tutorial series
/// MATH TYPES
/// ----------
#include "defines.h"

#ifndef semath_max
    #define semath_max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef semath_min
    #define semath_min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// * note: we use right handed coordinate system

typedef struct Vec2 {
    f32 x, y;
} Vec2;

typedef struct Vec2i {
    i32 x, y;
} Vec2i;

typedef struct Vec3 {
    f32 x, y, z;
} Vec3;

typedef struct Vec4 {
    f32 x, y, z, w;
} Vec4;

typedef struct Rect {
    f32 x, y, w, h;
} Rect;

// typedef Vec4 quat; // think of quaternions as oriantation instead of rotations

/// column major Mat2
typedef struct Mat2 {
    f32 m0, m2; // row 1
    f32 m1, m3; // row 2
} Mat2;

/// column major Mat4
typedef struct Mat4 {
    // f32 m0, m4, m8, m12;  // row 1
    // f32 m1, m5, m9, m13;  // row 2
    // f32 m2, m6, m10, m14; // row 3
    // f32 m3, m7, m11, m15; // row 4
    f32 data[16];
} Mat4;

#endif // SEMATH_TYPES