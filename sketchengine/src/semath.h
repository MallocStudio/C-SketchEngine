// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h
// https://www.youtube.com/watch?v=WR0tAQj-og8
#ifndef SEMATH_H
#define SEMATH_H

#include "defines.h"
#include "semath_defines.h"
#include <memory.h>

// An approximate representation of PI.
#define SEMATH_PI 3.14159265358979323846f
// An approximate representation of PI multiplied by 2.
#define SEMATH_PI_2 2.0f * SEMATH_PI
// An approximate representation of PI divided by 2.
#define SEMATH_HALF_PI 0.5f * SEMATH_PI
// An approximate representation of PI divided by 4.
#define SEMATH_QUARTER_PI 0.25f * SEMATH_PI
// One divided by an approximate representation of PI.
#define SEMATH_ONE_OVER_PI 1.0f / SEMATH_PI
// One divided by half of an approximate representation of PI.
#define SEMATH_ONE_OVER_TWO_PI 1.0f / SEMATH_PI_2
// An approximation of the square root of 2.
#define SEMATH_SQRT_TWO 1.41421356237309504880f
// An approximation of the square root of 3.
#define SEMATH_SQRT_THREE 1.73205080756887729352f
// One divided by an approximation of the square root of 2.
#define SEMATH_SQRT_ONE_OVER_TWO 0.70710678118654752440f
// One divided by an approximation of the square root of 3.
#define SEMATH_SQRT_ONE_OVER_THREE 0.57735026918962576450f
// A multiplier used to convert degrees to radians.
#define SEMATH_DEG2RAD_MULTIPLIER SEMATH_PI / 180.0f
// A multiplier used to convert radians to degrees.
#define SEMATH_RAD2DEG_MULTIPLIER 180.0f / SEMATH_PI
// The multiplier to convert seconds to milliseconds.
#define SEMATH_SEC_TO_MS_MULTIPLIER 1000.0f
// The multiplier to convert milliseconds to seconds.
#define SEMATH_MS_TO_SEC_MULTIPLIER 0.001f
// A huge number that should be larger than any valid number used.
#define SEMATH_INFINITY 1e30f
// Smallest positive number where 1.0 + FLOAT_EPSILON != 0
#define SEMATH_FLOAT_EPSILON 1.192092896e-07f

/// ----------------------
/// GENERAL MATH FUNCTIONS
/// ----------------------
f32 semath_sin(f32 x);
f32 semath_cos(f32 x);
f32 semath_tan(f32 x);
f32 semath_acos(f32 x);
f32 semath_sqrt(f32 x);
f32 semath_abs(f32 x);

/// Indicates if the value is a power of 2. 0 is considered _not_ a power of 2
/// 'value' is the value to be interpreted
/// 'returns' true if a power of 2, otherwise false
SEINLINE bool is_power_of_2(i32 value) {
    return ((value != 0) && ((value) & (value - 1)) == 0);
}

i32 random();
i32 random_in_range(i32 min, i32 max);
f32 frandom();
f32 frandom_in_range(f32 min, f32 max);

/// ----
/// Vec2
/// ----

/// returns a Vec2
SEINLINE Vec2 vec2_create(f32 x, f32 y) {
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

/// returns a Vec2 with x, y being zero
SEINLINE Vec2 vec2_zero() {
    return (Vec2) {0.0f,0.0f};
}

/// returns a Vec2 with x, y being one
SEINLINE Vec2 vec2_one() {
    return (Vec2) {1.0f, 1.0f};
}

/// returns a Vec2 pointing up
SEINLINE Vec2 vec2_up() {
    return (Vec2) {0.0f, 1.0f};
}

/// returns a Vec2 pointing down
SEINLINE Vec2 vec2_down() {
    return (Vec2) {0.0f, -1.0f};
}

/// returns a Vec2 pointing left
SEINLINE Vec2 vec2_left() {
    return (Vec2) {-1.0f, 0.0f};
}

/// returns a Vec2 pointing right
SEINLINE Vec2 vec2_right() {
    return (Vec2) {1.0f, 0.0f};
}

/// (v1 + v2)
SEINLINE Vec2 vec2_add(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x + v2.x,
        v1.y + v2.y};
}

/// (v1 - v2)
SEINLINE Vec2 vec2_sub(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x - v2.x,
        v1.y - v2.y};
}

/// (v1 * v2)
SEINLINE Vec2 vec2_mul(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x * v2.x,
        v1.y * v2.y};
}

/// (v1 * scalar)
SEINLINE Vec2 vec2_mul_scalar(Vec2 v1, f32 scalar) {
    return (Vec2) {
        v1.x * scalar,
        v1.y * scalar};
}

/// (v1 / v2)
SEINLINE Vec2 vec2_div(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x / v2.x,
        v1.y / v2.y};
}

SEINLINE f32 vec2_dot(Vec2 v1, Vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

SEINLINE Vec2 vec2_rotated(Vec2 v, f32 angle_radians) { // @check
    f32 cos = semath_cos(angle_radians);
    f32 sin = semath_sin(angle_radians);
    f32 tx = v.x;
    f32 ty = v.y;
    return (Vec2) {
        (cos * tx) - (sin * ty),
        (sin * tx) + (cos * ty),
    };
}

/// returns the squared magnitude / length of the provided vector
/// use this one when comparing lengths of multiple vectors, because you don't
/// really need the sqrt version
SEINLINE f32 vec2_magnitude_squared(Vec2 vec) {
    return vec.x * vec.x + vec.y * vec.y;
}

/// returns magnitude / length of the provided vector
SEINLINE f32 vec2_magnitude(Vec2 vec) {
    return semath_sqrt(vec2_magnitude_squared(vec));
}

/// normalises the provided vector in place to a unit vector
SEINLINE void vec2_normalise(Vec2 *vec) {
    const f32 mag = vec2_magnitude(*vec);
    vec->x /= mag;
    vec->y /= mag;
}

/// returns the normalised version of the provided vector
SEINLINE Vec2 vec2_normalised(Vec2 vec) {
    vec2_normalise(&vec);
    return vec;
}

/// compares all elements of v1 and v2 and unsures the difference is less than tolerance
/// tolerance is typically SEMATH_FLOAT_EPSILON or similiar
/// returns true if within tolerance
SEINLINE bool vec2_compare(Vec2 v1, Vec2 v2, f32 tolerance) {
    if (semath_abs(v1.x - v2.x) > tolerance) {
        return false;
    }
    if (semath_abs(v1.y - v2.y) > tolerance) {
        return false;
    }
    return true;
}

/// returns the distance between v1 and v2
SEINLINE f32 vec2_distance(Vec2 v1, Vec2 v2) {
    Vec2 d = (Vec2) {
        v1.x - v2.x,
        v1.y - v2.y};
    return vec2_magnitude(d);
}

/// ----
/// Vec3
/// ----

/// returns a Vec3
SEINLINE Vec3 vec3_create(f32 x, f32 y, f32 z) {
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

/// returns a Vec3 with x, y, z being zero
SEINLINE Vec3 vec3_zero() {
    return (Vec3) {0.0f,0.0f,0.0f};
}

/// returns a Vec3 with x, y, z being one
SEINLINE Vec3 vec3_one() {
    return (Vec3) {1.0f, 1.0f, 1.0f};
}

/// returns a Vec3 pointing up
SEINLINE Vec3 vec3_up() {
    return (Vec3) {0.0f, 1.0f, 0.0f};
}

/// returns a Vec3 pointing down
SEINLINE Vec3 vec3_down() {
    return (Vec3) {0.0f, -1.0f, 0.0f};
}

/// returns a Vec3 pointing left
SEINLINE Vec3 vec3_left() {
    return (Vec3) {-1.0f, 0.0f, 0.0f};
}

/// returns a Vec3 pointing right
SEINLINE Vec3 vec3_right() {
    return (Vec3) {1.0f, 0.0f, 0.0f};
}

/// returns a Vec3 pointing forward (right handed coord system) (0, 0, -1)
SEINLINE Vec3 vec3_forward() {
    return (Vec3) {0.0f, 0.0f, -1.0f};
}

/// returns a Vec3 pointing back (right handed coord system) (0, 0, +1)
SEINLINE Vec3 vec3_back() {
    return (Vec3) {0.0f, 0.0f, 1.0f};
}

/// v1 + v2 and returns a copy
SEINLINE Vec3 vec3_add(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z};
}

/// v1 - v2 and returns a copy
SEINLINE Vec3 vec3_sub(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z};
}

/// v1 * v2 and returns a copy
SEINLINE Vec3 vec3_mul(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x * v2.x,
        v1.y * v2.y,
        v1.z * v2.z};
}

/// v1 * scalar and returns a copy
SEINLINE Vec3 vec3_mul_scalar(Vec3 v1, f32 scalar) {
    return (Vec3) {
        v1.x * scalar,
        v1.y * scalar,
        v1.z * scalar};
}

/// v1 / v2 and returns a copy
SEINLINE Vec3 vec3_div(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x / v2.x,
        v1.y / v2.y,
        v1.z / v2.z};
}

/// Returns the squared magnitude / length of the provided vector
SEINLINE f32 vec3_magnitude_squared(Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/// Returns the magnitude / length of the provided vector
SEINLINE f32 vec3_magnitude(Vec3 v) {
    return semath_sqrt(vec3_magnitude_squared(v));
}

/// Normalises the provided vector in place to a unit vector
SEINLINE void vec3_normalise(Vec3 *v) {
    const f32 mag = vec3_magnitude(*v);
    v->x /= mag;
    v->y /= mag;
    v->z /= mag;
}

/// Returns a normalised copy of the supplied vector
SEINLINE Vec3 vec3_normalised(Vec3 v) {
    vec3_normalise(&v);
    return v;
}

/// Returns the dot product of v1 and v2
/// Typically used to calculat ethe difference in direction
SEINLINE f32 vec3_dot(Vec3 v1, Vec3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/// Calculates and returns the cross product of the supplied vectors
/// The cross product is a new vector which is orthoganal to both provided vectors
SEINLINE Vec3 vec3_cross(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x};
}

/// Compares all elements of v1 and v2 and ensures the difference is less than tolerance
/// tolerance is typically SEMATH_FLOAT_EPSILON or similar
/// returns true if within tolerance
SEINLINE bool vec3_compare(Vec3 v1, Vec3 v2, f32 tolerance) {
    if (semath_abs(v1.x - v2.x) > tolerance) {
        return false;
    }
    if (semath_abs(v1.y - v2.y) > tolerance) {
        return false;
    }
    if (semath_abs(v1.z - v2.z) > tolerance) {
        return false;
    }
    return true;
}

/// Returns the distance between v1 and v2
SEINLINE f32 vec3_distance(Vec3 v1, Vec3 v2) {
    Vec3 d = (Vec3) {
        v1.x - v2.x,
        v1.x - v2.y,
        v1.z - v2.z};
    return vec3_magnitude(d);
}

/// ----
/// Mat2
/// ----

/// init Mat2
SEINLINE Mat2 mat2_create(f32 m0, f32 m1, f32 m2, f32 m3) {
    // Mat2 r;
    // r.m0 = m0; r.m2 = m2;
    // r.m1 = m1; r.m3 = m3;
    // return r;
    return (Mat2) {
        m0, m2,
        m1, m3,
    };
}

/// get a Mat2 identity
SEINLINE Mat2 mat2_identity() {
    // Mat2 r;
    // r.m0 = 1; r.m2 = 0;
    // r.m1 = 0; r.m3 = 1;
    // return r;
    return (Mat2) {
        1, 0,
        0, 1
    };
}

/// get a Mat2 from 2 Vec2
/// @param 'col1' column 1
/// @param 'col2' column 2
SEINLINE Mat2 mat2_create_vec2(Vec2 col1, Vec2 col2) {
    // Mat2 r;
    // r.m0 = col1.x; r.m2 = col2.x;
    // r.m1 = col1.y; r.m3 = col2.y;
    // return r;
    return (Mat2) {
        col1.x, col2.x,
        col1.y, col2.y
    };
}

/// returns true if the two provided mat2s are identitcal
SEINLINE bool mat2_compare(Mat2 m1, Mat2 m2, f32 tolerance) {
    if (semath_abs(m1.m0 - m2.m0) > tolerance) return false;
    if (semath_abs(m1.m1 - m2.m1) > tolerance) return false;
    if (semath_abs(m1.m2 - m2.m2) > tolerance) return false;
    if (semath_abs(m1.m3 - m2.m3) > tolerance) return false;
    return true;
}

// a b   e f   ae + bg, af + bh
// c d x g h = ce + dg, cf + dh
/// multiply m1 x m2 (column major)
SEINLINE Mat2 mat2_mul(Mat2 m1, Mat2 m2) {
    Mat2 result;
    result.m0 = m1.m0 * m2.m0 + m1.m2 * m2.m1;
    result.m1 = m1.m1 * m2.m0 + m1.m3 * m2.m1;
    result.m2 = m1.m0 * m2.m2 + m1.m2 * m2.m3;
    result.m3 = m1.m1 * m2.m2 + m1.m3 * m2.m3;
    return result;
}

/// multiply m by a v
SEINLINE Vec2 mat2_mul_vec2(Mat2 m, Vec2 v) {
    Vec2 result;
    result.x = m.m0 * v.x + m.m2 * v.y;
    result.y = m.m1 * v.x + m.m3 * v.y;
    return result;
}

/// --------
/// Matrix 4
/// --------
/// ported from:
/// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h

// /// returns a Mat4 set to zero (NOT IDENTITY)
// SEINLINE Mat4 mat4_create() { // ! commented out because I don't think this will be useful
//     return (Mat4) {
//         0, 0, 0, 0,
//         0, 0, 0, 0,
//         0, 0, 0, 0,
//         0, 0, 0, 0
//     };
// }

/// @brief creates a Mat4 based on the array passed in
/// @brief [0 - 3] is column 1, [4 - 7] is column 2 etc
/// @param 'v' is an array of 16 elements [0 - 15]
SEINLINE Mat4 mat4_create(f32 v[16]) {
    return (Mat4) {
        v[0], v[4],  v[8], v[12],
        v[1], v[5],  v[9], v[13],
        v[2], v[6], v[10], v[14],
        v[3], v[7], v[11], v[15]
    };
}

/// returns an identity Mat4
SEINLINE Mat4 mat4_identity() {
    return (Mat4) {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

/// returns the result of multiplying m1 and m2
SEINLINE Mat4 mat4_mul(Mat4 m1, Mat4 m2) {
    Mat4 result = mat4_identity();

    const f32 *m1_ptr = m1.data;
    const f32 *m2_ptr = m2.data;
    f32 *dst_ptr = result.data;

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            *dst_ptr =
                m1_ptr[0] * m2_ptr[0 + j] +
                m1_ptr[1] * m2_ptr[4 + j] +
                m1_ptr[2] * m2_ptr[8 + j] +
                m1_ptr[3] * m2_ptr[12 + j];
            dst_ptr++;
        }
        m1_ptr += 4;
    }
    return result;
}

/// creates and returns an orthographic projection Mat4.
/// Typically used to render flat or 2D scenes
/// @param left The left side of the view frustum.
/// @param right The right side of the view frustum.
/// @param bottom The bottom side of the view frustum.
/// @param top The top side of the view frustum.
/// @param near_clip The near clipping plane distance.
/// @param far_clip The far clipping plane distance.
SEINLINE Mat4 mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_clip, f32 far_clip) {
    Mat4 result = mat4_identity();

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (near_clip - far_clip);

    result.data[0] = -2.0f * lr;
    result.data[5] = -2.0f * bt;
    result.data[10] = 2.0f * nf;

    result.data[12] = (left + right) * lr;
    result.data[13] = (top + bottom) * bt;
    result.data[14] = (far_clip + near_clip) * nf;
    return result;
}

/// creates and returns a perspective Mat4. Typically used to render 3D scenes
/// @param fov_radians The field of view in radians.
/// @param aspect_ratio The aspect ratio.
/// @param near_clip The near clipping plane distance.
/// @param far_clip The far clipping plane distance.
SEINLINE Mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip) {
    f32 half_tan_fov = semath_tan(fov_radians * 0.5f);
    Mat4 result;
    memset(result.data, 0, sizeof(f32) * 16);
    result.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
    result.data[5] = 1.0f / half_tan_fov;
    result.data[10] = -((far_clip + near_clip) / (far_clip - near_clip));
    result.data[11] = -1.0f;
    result.data[14] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
    return result;
}

/// creates and returns a look-at Mat4, or a Mat4 looking at a target from the perspective of pos
SEINLINE Mat4 mat4_lookat(Vec3 position, Vec3 target, Vec3 up) {
    Mat4 result;
    Vec3 z_axis;
    z_axis.x = target.x - position.x;
    z_axis.y = target.y - position.y;
    z_axis.z = target.z - position.z;

    z_axis = vec3_normalised(z_axis);
    Vec3 x_axis = vec3_normalised(vec3_cross(z_axis, up));
    Vec3 y_axis = vec3_cross(x_axis, z_axis);

    result.data[0] = x_axis.x;
    result.data[1] = y_axis.x;
    result.data[2] = -z_axis.x;
    result.data[3] = 0;
    result.data[4] = x_axis.y;
    result.data[5] = y_axis.y;
    result.data[6] = -z_axis.y;
    result.data[7] = 0;
    result.data[8] = x_axis.z;
    result.data[9] = y_axis.z;
    result.data[10] = -z_axis.z;
    result.data[11] = 0;
    result.data[12] = -vec3_dot(x_axis, position);
    result.data[13] = -vec3_dot(y_axis, position);
    result.data[14] = vec3_dot(z_axis, position);
    result.data[15] = 1.0f;

    return result;
}

/// (rows -> columns) returns a transposed copy of the provided Mat4
SEINLINE Mat4 mat4_transposed(Mat4 m) {
    Mat4 result = mat4_identity();
    result.data[0] = m.data[0];
    result.data[1] = m.data[4];
    result.data[2] = m.data[8];
    result.data[3] = m.data[12];
    result.data[4] = m.data[1];
    result.data[5] = m.data[5];
    result.data[6] = m.data[9];
    result.data[7] = m.data[13];
    result.data[8] = m.data[2];
    result.data[9] = m.data[6];
    result.data[10] = m.data[10];
    result.data[11] = m.data[14];
    result.data[12] = m.data[3];
    result.data[13] = m.data[7];
    result.data[14] = m.data[11];
    result.data[15] = m.data[15];
    return result;
}

/// creates and returns an inverse of the provided Mat4
SEINLINE Mat4 mat4_inverse(Mat4 matrix) {
    const f32* m = matrix.data;

    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    Mat4 result;
    f32* o = result.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return result;
}

/// get the standard ortho projection matrix from a viewport. near clip is set to -1, far clip is set to 1
SEINLINE Mat4 viewport_to_ortho_projection_matrix (Rect viewport) {
    return mat4_ortho(viewport.x, viewport.w, viewport.y, viewport.h, -1, 1);
}

/// get the standard ortho projection matrix from a viewport with near clip and far clip adjustable
SEINLINE Mat4 viewport_to_ortho_projection_matrix_extra (Rect viewport, f32 near_clip, f32 far_clip) {
    return mat4_ortho(viewport.x, viewport.w, viewport.y, viewport.h, near_clip, far_clip);
}

/// creates and returns a translation Mat4 from the given pos
SEINLINE Mat4 mat4_translation(Vec3 pos) {
    Mat4 result = mat4_identity();
    result.data[12] = pos.x;
    result.data[13] = pos.y;
    result.data[14] = pos.z;
    return result;
}

/// creates and returns a scale Mat4
SEINLINE Mat4 mat4_scale(Vec3 scale) {
    Mat4 result = mat4_identity();
    result.data[0] = scale.x;
    result.data[5] = scale.y;
    result.data[10] = scale.z;
    return result;
}

/// creates and returns a rotation Mat4 from the provided x angle
SEINLINE Mat4 mat4_euler_x(f32 angle_radians) {
    Mat4 result = mat4_identity();
    f32 c = semath_cos(angle_radians);
    f32 s = semath_sin(angle_radians);
    result.data[5] = c;
    result.data[6] = s;
    result.data[9] = -s;
    result.data[10] = c;
    return result;
}

/// creates and returns a rotation Mat4 from the provided y angle
SEINLINE Mat4 mat4_euler_y(f32 angle_radians) {
    Mat4 result = mat4_identity();
    f32 c = semath_cos(angle_radians);
    f32 s = semath_sin(angle_radians);
    result.data[0] = c;
    result.data[2] = -s;
    result.data[8] = s;
    result.data[10] = c;
    return result;
}

/// creates and returns a rotation Mat4 from the provided z angle
SEINLINE Mat4 mat4_euler_z(f32 angle_radians) {
    Mat4 result = mat4_identity();
    f32 c = semath_cos(angle_radians);
    f32 s = semath_sin(angle_radians);
    result.data[0] = c;
    result.data[1] = s;
    result.data[4] = -s;
    result.data[5] = c;
    return result;
}

/// creates a rot Mat4 from the provided x, y, z, axis rotation
SEINLINE Mat4 mat4_euler_xyz(f32 x_radians, f32 y_radians, f32 z_radians) {
    Mat4 rx = mat4_euler_x(x_radians); // x rotation
    Mat4 ry = mat4_euler_y(y_radians); // y rotation
    Mat4 rz = mat4_euler_z(z_radians); // z rotation
    Mat4 result = mat4_mul(rx, ry);
    result = mat4_mul(result, rz);
    return result;
}

/// returns a forward Vec3 relative to the provided Mat4
SEINLINE Vec3 mat4_forward(Mat4 m) {
    Vec3 result;
    result.x = -m.data[2];
    result.y = -m.data[6];
    result.z = -m.data[10];
    vec3_normalise(&result);
    return result;
}

/// returns a backward Vec3 relative to the provided Mat4
SEINLINE Vec3 mat4_backward(Mat4 m) {
    Vec3 result;
    result.x = m.data[2];
    result.y = m.data[6];
    result.z = m.data[10];
    vec3_normalise(&result);
    return result;
}

/// returns a up Vec3 relative to the provided Mat4
SEINLINE Vec3 mat4_up(Mat4 m) {
    Vec3 result;
    result.x = m.data[1];
    result.y = m.data[5];
    result.z = m.data[9];
    vec3_normalise(&result);
    return result;
}

/// returns a down Vec3 relative to the provided Mat4
SEINLINE Vec3 mat4_down(Mat4 m) {
    Vec3 result;
    result.x = -m.data[1];
    result.y = -m.data[5];
    result.z = -m.data[9];
    vec3_normalise(&result);
    return result;
}

/// returns a left Vec3 relative to the provided Mat4
SEINLINE Vec3 mat4_left(Mat4 m) {
    Vec3 result;
    result.x = -m.data[0];
    result.y = -m.data[4];
    result.z = -m.data[8];
    vec3_normalise(&result);
    return result;
}

/// returns a right Vec3 relative to the provided Mat4
SEINLINE Vec3 mat4_right(Mat4 m) {
    Vec3 result;
    result.x = m.data[0];
    result.y = m.data[4];
    result.z = m.data[8];
    vec3_normalise(&result);
    return result;
}

/// creates and returns the result of the multiplication of m and v
SEINLINE Vec4 mat4_mul_vec4 (const Mat4 *m, const Vec4 *v) {
    Vec4 result;
    result.x = m->data[0 ] * v->x + m->data[1 ] * v->y + m->data[2 ] * v->z + m->data[3 ] * v->w;
    result.y = m->data[4 ] * v->x + m->data[5 ] * v->y + m->data[6 ] * v->z + m->data[7 ] * v->w;
    result.z = m->data[8 ] * v->x + m->data[9 ] * v->y + m->data[10] * v->z + m->data[11] * v->w;
    result.w = m->data[12] * v->x + m->data[13] * v->y + m->data[14] * v->z + m->data[15] * v->w;
    return result;
}

/// returns a Vec3 that represents translation
SEINLINE Vec3 mat4_get_translation(Mat4 m) {
    return (Vec3) {m.data[12], m.data[13], m.data[14]};
}

/// -----
/// TESTS
/// -----

/// tests all Mat2 functions and returns true if all succeed
SEINLINE bool mat2_test() {
    bool result = true;
    { // -- mat2_create

    }
    { // -- mat2_create_vec2

    }
    { // -- mat2_mul 1
        Mat2 m1 = mat2_identity();
        Mat2 m2 = mat2_identity();
        if (mat2_compare(mat2_mul(m1, m2), mat2_identity(), SEMATH_FLOAT_EPSILON)) {
            printf("mat2_test : mat2_mul 1: ok\n");
        } else {
            result = false;
            printf("mat2_test : mat2_mul 1: failed\n");
        }
    }
    { // -- mat2_mul 2
        Mat2 m1 = mat2_create(2, 1, 0, 2);
        Mat2 m2 = mat2_create(1, 3, 2, 4);
        if (mat2_compare(mat2_mul(m1, m2), mat2_create(2, 7, 4, 10), SEMATH_FLOAT_EPSILON)) {
            printf("mat2_test : mat2_mul 2: ok\n");
        } else {
            result = false;
            printf("mat2_test : mat2_mul 2: failed\n");
        }
    }
    { // -- mat2_mul_vec2
        Mat2 m1 = mat2_create(3, 8, 4, 2);
        Vec2 v1 = vec2_create(12, 15);
        Vec2 v = mat2_mul_vec2(m1, v1);
        Vec2 test_against = vec2_create(96, 126);
        if (vec2_compare(v, test_against, SEMATH_FLOAT_EPSILON)) {
            printf("mat2_test : mat2_mul_vec2: ok\n");
        } else {
            result = false;
            printf("mat2_test : mat2_mul_vec2: failed\n");
        }
    }
    return result;
};

/// -----------
/// CONVERSIONS
/// -----------

/// Vec2i -> Vec2
SEINLINE Vec2 vec2i_to_vec2(Vec2i v) {
    return (Vec2) {(f32)v.x, (f32)v.y};
}

#endif // SEMATH_H