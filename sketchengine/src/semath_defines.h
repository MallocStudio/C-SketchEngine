#ifndef SEMATH_TYPES
#define SEMATH_TYPES
/// note: this math library was made while following the Kohi game engine tutorial series
/// MATH TYPES
/// ----------
#include "sedefines.h"

#ifndef se_math_max
    #define se_math_max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef se_math_min
    #define se_math_min(a,b) (((a) < (b)) ? (a) : (b))
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

typedef struct Vec3i {
    i32 x, y, z;
} Vec3i;

typedef struct Vec4 {
    f32 x, y, z, w;
} Vec4;

/// quaternion
typedef Vec4 Quat;

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

typedef struct AABB3D {
    Vec3 min;
    Vec3 max;
} AABB3D;

se_array_struct(Vec2);
se_array_struct(Vec2i);
se_array_struct(Vec3);
se_array_struct(Vec3i);
se_array_struct(Vec4);
se_array_struct(Quat);
se_array_struct(Rect);
se_array_struct(Mat2);
se_array_struct(Mat4);
se_array_struct(AABB3D);

#endif // SEMATH_TYPES