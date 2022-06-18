#include "semath.h"
#include <math.h>

f32 semath_abs(f32 x) {
    return fabsf(x);
}

f32 semath_remainder(f32 x, f32 denominator) {
    return remainderf(x, denominator);
}

f32 semath_tan(f32 x) {
    return tanf(x);
}

f32 semath_sin(f32 x) {
    return sinf(x);
}

f32 semath_cos(f32 x) {
    return cosf(x);
}

f32 semath_acos(f32 x) {
    return acosf(x);
}

f32 semath_asin(f32 x) {
    return asinf(x);
}

f32 semath_sqrt(f32 x) {
    return sqrtf(x);
}

f32 semath_power(f32 base, f32 p) {
    return powf(base, p);
}

b8 is_power_of_2(i32 value) {
    return ((value != 0) && ((value) & (value - 1)) == 0);
}


/// ----
/// Vec2
/// ----

/// returns a Vec2
 Vec2 vec2_create(f32 x, f32 y) {
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

/// returns a Vec2 with x, y being zero
 Vec2 vec2_zero() {
    return (Vec2) {0.0f,0.0f};
}

/// returns a Vec2 with x, y being one
 Vec2 vec2_one() {
    return (Vec2) {1.0f, 1.0f};
}

/// returns a Vec2 pointing up
 Vec2 vec2_up() {
    return (Vec2) {0.0f, 1.0f};
}

/// returns a Vec2 pointing down
 Vec2 vec2_down() {
    return (Vec2) {0.0f, -1.0f};
}

/// returns a Vec2 pointing left
 Vec2 vec2_left() {
    return (Vec2) {-1.0f, 0.0f};
}

/// returns a Vec2 pointing right
 Vec2 vec2_right() {
    return (Vec2) {1.0f, 0.0f};
}

/// (v1 + v2)
 Vec2 vec2_add(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x + v2.x,
        v1.y + v2.y};
}

/// (v1 - v2)
 Vec2 vec2_sub(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x - v2.x,
        v1.y - v2.y};
}

/// (v1 * v2)
 Vec2 vec2_mul(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x * v2.x,
        v1.y * v2.y};
}

/// (v1 * scalar)
 Vec2 vec2_mul_scalar(Vec2 v1, f32 scalar) {
    return (Vec2) {
        v1.x * scalar,
        v1.y * scalar};
}

/// (v1 / v2)
 Vec2 vec2_div(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        v1.x / v2.x,
        v1.y / v2.y};
}

 f32 vec2_dot(Vec2 v1, Vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

 Vec2 vec2_average(Vec2 v1, Vec2 v2) {
    return (Vec2) {
        (v2.x + v1.x) * 0.5f,
        (v2.y + v1.y) * 0.5f,
    };
}

 Vec2 vec2_rotated(Vec2 v, f32 angle_radians) { // @check
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
 f32 vec2_magnitude_squared(Vec2 vec) {
    return vec.x * vec.x + vec.y * vec.y;
}

/// returns magnitude / length of the provided vector
 f32 vec2_magnitude(Vec2 vec) {
    return semath_sqrt(vec2_magnitude_squared(vec));
}

/// normalises the provided vector in place to a unit vector
 void vec2_normalise(Vec2 *vec) {
    const f32 mag = vec2_magnitude(*vec);
    if (mag > 1) {
        vec->x /= mag;
        vec->y /= mag;
    }
}

/// returns the normalised version of the provided vector
 Vec2 vec2_normalised(Vec2 vec) {
    vec2_normalise(&vec);
    return vec;
}

/// find the angle of the given vec2
 f32 vec2_angle(Vec2 v) {
    vec2_normalise(&v);
    return semath_acos(v.x);
}

 f32 vec2_angle_between(Vec2 v1, Vec2 v2) {
    f32 angle = semath_acos((vec2_dot(v1, v2)) / (vec2_magnitude(v1) * vec2_magnitude(v2)));
    return angle;
}

/// compares all elements of v1 and v2 and unsures the difference is less than tolerance
/// tolerance is typically SEMATH_FLOAT_EPSILON or similiar
/// returns true if within tolerance
 b8 vec2_compare(Vec2 v1, Vec2 v2, f32 tolerance) {
    if (semath_abs(v1.x - v2.x) > tolerance) {
        return false;
    }
    if (semath_abs(v1.y - v2.y) > tolerance) {
        return false;
    }
    return true;
}

/// returns the distance between v1 and v2
 f32 vec2_distance(Vec2 v1, Vec2 v2) {
    Vec2 d = (Vec2) {
        v1.x - v2.x,
        v1.y - v2.y};
    return vec2_magnitude(d);
}

/// ----
/// Vec3
/// ----

/// returns a Vec3
 Vec3 vec3_create(f32 x, f32 y, f32 z) {
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

#define v3f(x, y, z) vec3_create(x, y, z)
#define v2f(x, y) vec2_create(x, y)

/// returns a Vec3 with x, y, z being zero
 Vec3 vec3_zero() {
    return (Vec3) {0.0f,0.0f,0.0f};
}

/// returns a Vec3 with x, y, z being one
 Vec3 vec3_one() {
    return (Vec3) {1.0f, 1.0f, 1.0f};
}

/// returns a Vec3 pointing up
 Vec3 vec3_up() {
    return (Vec3) {0.0f, 1.0f, 0.0f};
}

/// returns a Vec3 pointing down
 Vec3 vec3_down() {
    return (Vec3) {0.0f, -1.0f, 0.0f};
}

/// returns a Vec3 pointing left
 Vec3 vec3_left() {
    return (Vec3) {-1.0f, 0.0f, 0.0f};
}

/// returns a Vec3 pointing right
 Vec3 vec3_right() {
    return (Vec3) {1.0f, 0.0f, 0.0f};
}

/// returns a Vec3 pointing forward (right handed coord system) (0, 0, -1)
 Vec3 vec3_forward() {
    return (Vec3) {0.0f, 0.0f, -1.0f};
}

/// returns a Vec3 pointing back (right handed coord system) (0, 0, +1)
 Vec3 vec3_backward() {
    return (Vec3) {0.0f, 0.0f, 1.0f};
}

/// v1 + v2 and returns a copy
 Vec3 vec3_add(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z};
}

/// v1 - v2 and returns a copy
 Vec3 vec3_sub(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z};
}

/// v1 * v2 and returns a copy
 Vec3 vec3_mul(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x * v2.x,
        v1.y * v2.y,
        v1.z * v2.z};
}

/// v1 * scalar and returns a copy
 Vec3 vec3_mul_scalar(Vec3 v1, f32 scalar) {
    return (Vec3) {
        v1.x * scalar,
        v1.y * scalar,
        v1.z * scalar};
}

/// v1 / v2 and returns a copy
 Vec3 vec3_div(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.x / v2.x,
        v1.y / v2.y,
        v1.z / v2.z};
}

/// Returns the squared magnitude / length of the provided vector
 f32 vec3_magnitude_squared(Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/// Returns the magnitude / length of the provided vector
 f32 vec3_magnitude(Vec3 v) {
    return semath_sqrt(vec3_magnitude_squared(v));
}

/// Normalises the provided vector in place to a unit vector
 void vec3_normalise(Vec3 *v) {
    const f32 mag = vec3_magnitude(*v);
    v->x /= mag;
    v->y /= mag;
    v->z /= mag;
}

/// Returns a normalised copy of the supplied vector
 Vec3 vec3_normalised(Vec3 v) {
    vec3_normalise(&v);
    return v;
}

/// Returns the dot product of v1 and v2
/// Typically used to calculat ethe difference in direction
 f32 vec3_dot(Vec3 v1, Vec3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/// Calculates and returns the cross product of the supplied vectors
/// The cross product is a new vector which is orthoganal to both provided vectors
 Vec3 vec3_cross(Vec3 v1, Vec3 v2) {
    return (Vec3) {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x};
}

/// Compares all elements of v1 and v2 and ensures the difference is less than tolerance
/// tolerance is typically SEMATH_FLOAT_EPSILON or similar
/// returns true if within tolerance
 b8 vec3_compare(Vec3 v1, Vec3 v2, f32 tolerance) {
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
 f32 vec3_distance(Vec3 v1, Vec3 v2) {
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
 Mat2 mat2_create(f32 m0, f32 m1, f32 m2, f32 m3) {
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
 Mat2 mat2_identity() {
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
 Mat2 mat2_create_vec2(Vec2 col1, Vec2 col2) {
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
 b8 mat2_compare(Mat2 m1, Mat2 m2, f32 tolerance) {
    if (semath_abs(m1.m0 - m2.m0) > tolerance) return false;
    if (semath_abs(m1.m1 - m2.m1) > tolerance) return false;
    if (semath_abs(m1.m2 - m2.m2) > tolerance) return false;
    if (semath_abs(m1.m3 - m2.m3) > tolerance) return false;
    return true;
}

// a b   e f   ae + bg, af + bh
// c d x g h = ce + dg, cf + dh
/// multiply m1 x m2 (column major)
 Mat2 mat2_mul(Mat2 m1, Mat2 m2) {
    Mat2 result;
    result.m0 = m1.m0 * m2.m0 + m1.m2 * m2.m1;
    result.m1 = m1.m1 * m2.m0 + m1.m3 * m2.m1;
    result.m2 = m1.m0 * m2.m2 + m1.m2 * m2.m3;
    result.m3 = m1.m1 * m2.m2 + m1.m3 * m2.m3;
    return result;
}

/// multiply m by a v
 Vec2 mat2_mul_vec2(Mat2 m, Vec2 v) {
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
//  Mat4 mat4_create() { // ! commented out because I don't think this will be useful
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
 Mat4 mat4_create(f32 v[16]) {
    return (Mat4) {
        v[0], v[4],  v[8], v[12],
        v[1], v[5],  v[9], v[13],
        v[2], v[6], v[10], v[14],
        v[3], v[7], v[11], v[15]
    };
}

/// returns an identity Mat4
 Mat4 mat4_identity() {
    return (Mat4) {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

/// returns the result of multiplying m1 and m2
 Mat4 mat4_mul(Mat4 m1, Mat4 m2) {
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
 Mat4 mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_clip, f32 far_clip) {
    // Mat4 result = mat4_identity();

    // f32 lr = 1.0f / (left - right);
    // f32 bt = 1.0f / (bottom - top);
    // f32 nf = 1.0f / (near_clip - far_clip);

    // result.data[0] = -2.0f * lr;
    // result.data[5] = -2.0f * bt;
    // result.data[10] = -2.0f * nf;

    // result.data[12] = (left + right) * lr;
    // result.data[13] = (top + bottom) * bt;
    // result.data[14] = (far_clip + near_clip) * nf;
    // return result;

    // Mat4 result = mat4_identity();

    // f32 rl = 1.0f / (right - left);
    // f32 tb = 1.0f / (top - bottom);
    // f32 fn = 1.0f / (far_clip - near_clip);

    // result.data[0] = 2.0f * rl;
    // result.data[5] = 2.0f * tb;
    // result.data[10] = -2.0f * fn;

    // result.data[12] = (left + right) * rl * -1;
    // result.data[13] = (top + bottom) * tb * -1;
    // result.data[14] = (far_clip + near_clip) * fn * -1;
    // return result;

    Mat4 out_matrix = mat4_identity();

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (near_clip - far_clip);

    out_matrix.data[0] = -2.0f * lr;
    out_matrix.data[5] = -2.0f * bt;
    out_matrix.data[10] = 2.0f * nf;

    out_matrix.data[12] = (left + right) * lr;
    out_matrix.data[13] = (top + bottom) * bt;
    out_matrix.data[14] = (far_clip + near_clip) * nf;
    return out_matrix;
}

/// creates and returns a perspective Mat4. Typically used to render 3D scenes
/// @param fov_radians The field of view in radians.
/// @param aspect_ratio The aspect ratio.
/// @param near_clip The near clipping plane distance.
/// @param far_clip The far clipping plane distance.
 Mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip) {
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
 Mat4 mat4_lookat(Vec3 position, Vec3 target, Vec3 up) {
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

// /// Creates a transform matrix combining the position and the oriantation. Oriantation is a normalised Vector that points to a
// /// certain direction
//  Mat4 mat4_create_transform(Vec3 position, Vec3 oriantation_direction, Vec3 up) {
//     Mat4 result;
//     Vec3 z_axis = oriantation_direction;

//     z_axis = vec3_normalised(z_axis);
//     Vec3 x_axis = vec3_normalised(vec3_cross(z_axis, up));
//     Vec3 y_axis = vec3_cross(x_axis, z_axis);

//     result.data[0] = x_axis.x;
//     result.data[1] = y_axis.x;
//     result.data[2] = -z_axis.x;
//     result.data[3] = 0;
//     result.data[4] = x_axis.y;
//     result.data[5] = y_axis.y;
//     result.data[6] = -z_axis.y;
//     result.data[7] = 0;
//     result.data[8] = x_axis.z;
//     result.data[9] = y_axis.z;
//     result.data[10] = -z_axis.z;
//     result.data[11] = 0;
//     result.data[12] = -vec3_dot(x_axis, position);
//     result.data[13] = -vec3_dot(y_axis, position);
//     result.data[14] = vec3_dot(z_axis, position);
//     result.data[15] = 1.0f;

//     return result;
// }

/// (rows -> columns) returns a transposed copy of the provided Mat4
 Mat4 mat4_transposed(Mat4 m) {
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
 Mat4 mat4_inverse(Mat4 matrix) {
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
 Mat4 viewport_to_ortho_projection_matrix (Rect viewport) {
    return mat4_ortho(viewport.x, viewport.w, viewport.y, viewport.h, -1.0f, 1.0f);
}

/// get the standard ortho projection matrix from a viewport with near clip and far clip adjustable
 Mat4 viewport_to_ortho_projection_matrix_extra (Rect viewport, f32 near_clip, f32 far_clip) {
    return mat4_ortho(viewport.x, viewport.w, viewport.y, viewport.h, near_clip, far_clip);
}

/// creates and returns a translation Mat4 from the given pos
 Mat4 mat4_translation(Vec3 pos) {
    Mat4 result = mat4_identity();
    result.data[12] = pos.x;
    result.data[13] = pos.y;
    result.data[14] = pos.z;
    return result;
}

/// creates and returns a scale Mat4
 Mat4 mat4_scale(Vec3 scale) {
    Mat4 result = mat4_identity();
    result.data[0] = scale.x;
    result.data[5] = scale.y;
    result.data[10] = scale.z;
    return result;
}

/// creates and returns a rotation Mat4 from the provided x angle
 Mat4 mat4_euler_x(f32 angle_radians) {
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
 Mat4 mat4_euler_y(f32 angle_radians) {
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
 Mat4 mat4_euler_z(f32 angle_radians) {
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
 Mat4 mat4_euler_xyz(f32 x_radians, f32 y_radians, f32 z_radians) {
    Mat4 rx = mat4_euler_x(x_radians); // x rotation
    Mat4 ry = mat4_euler_y(y_radians); // y rotation
    Mat4 rz = mat4_euler_z(z_radians); // z rotation
    Mat4 result = mat4_mul(rx, ry);
    result = mat4_mul(result, rz);
    return result;
}

/// returns a forward Vec3 relative to the provided Mat4
 Vec3 mat4_forward(Mat4 m) {
    Vec3 result;
    result.x = -m.data[2];
    result.y = -m.data[6];
    result.z = -m.data[10];
    vec3_normalise(&result);
    return result;
}

/// returns a backward Vec3 relative to the provided Mat4
 Vec3 mat4_backward(Mat4 m) {
    Vec3 result;
    result.x = m.data[2];
    result.y = m.data[6];
    result.z = m.data[10];
    vec3_normalise(&result);
    return result;
}

/// returns a up Vec3 relative to the provided Mat4
 Vec3 mat4_up(Mat4 m) {
    Vec3 result;
    result.x = m.data[1];
    result.y = m.data[5];
    result.z = m.data[9];
    vec3_normalise(&result);
    return result;
}

/// returns a down Vec3 relative to the provided Mat4
 Vec3 mat4_down(Mat4 m) {
    Vec3 result;
    result.x = -m.data[1];
    result.y = -m.data[5];
    result.z = -m.data[9];
    vec3_normalise(&result);
    return result;
}

/// returns a left Vec3 relative to the provided Mat4
 Vec3 mat4_left(Mat4 m) {
    Vec3 result;
    result.x = -m.data[0];
    result.y = -m.data[4];
    result.z = -m.data[8];
    vec3_normalise(&result);
    return result;
}

/// returns a right Vec3 relative to the provided Mat4
 Vec3 mat4_right(Mat4 m) {
    Vec3 result;
    result.x = m.data[0];
    result.y = m.data[4];
    result.z = m.data[8];
    vec3_normalise(&result);
    return result;
}

/// creates and returns the result of the multiplication of m and v
 Vec4 mat4_mul_vec4 (Mat4 m, Vec4 v) {
    // https://gamedev.stackexchange.com/questions/136573/multiply-matrix4x4-with-vec4
    // the v * M part (I think the other part is the same but transposed)
    Vec4 result;
    result.x = v.x * m.data[0] + v.y * m.data[4] + v.z * m.data[8]  + v.w * m.data[12];
    result.y = v.x * m.data[1] + v.y * m.data[5] + v.z * m.data[9]  + v.w * m.data[13];
    result.z = v.x * m.data[2] + v.y * m.data[6] + v.z * m.data[10] + v.w * m.data[14];
    result.w = v.x * m.data[3] + v.y * m.data[7] + v.z * m.data[11] + v.w * m.data[15];
    return result;
}

/// returns a Vec3 that represents translation
 Vec3 mat4_get_translation(Mat4 m) {
    return (Vec3) {m.data[12], m.data[13], m.data[14]};
}

/// ----------
/// Quaternion
/// ----------
/// ported from:
/// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h

/// Creates an identity quaternion
 Quat quat_identity() {
    return (Quat){0, 0, 0, 1.0f};
}

/**
 * @brief Returns the normal of the provided quaternion.
 *
 * @param q The quaternion.
 * @return The normal of the provided quaternion.
 */
 f32 quat_normal(Quat q) {
    return semath_sqrt(
        q.x * q.x +
        q.y * q.y +
        q.z * q.z +
        q.w * q.w);
}

/**
 * @brief Returns a normalized copy of the provided quaternion.
 *
 * @param q The quaternion to normalize.
 * @return A normalized copy of the provided quaternion.
 */
 Quat quat_normalize(Quat q) {
    f32 normal = quat_normal(q);
    return (Quat){
        q.x / normal,
        q.y / normal,
        q.z / normal,
        q.w / normal};
}

/**
 * @brief Returns the conjugate of the provided quaternion. That is,
 * The x, y and z elements are negated, but the w element is untouched.
 *
 * @param q The quaternion to obtain a conjugate of.
 * @return The conjugate quaternion.
 */
 Quat quat_conjugate(Quat q) {
    return (Quat){
        -q.x,
        -q.y,
        -q.z,
        q.w};
}

/**
 * @brief Returns an inverse copy of the provided quaternion.
 *
 * @param q The quaternion to invert.
 * @return An inverse copy of the provided quaternion.
 */
 Quat quat_inverse(Quat q) {
    return quat_normalize(quat_conjugate(q));
}

 Quat quat_cross(Quat q1, Quat q2) {
    Quat q;
    q.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    q.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    q.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
    q.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
    return q;
}

/**
 * @brief Multiplies the provided quaternions.
 *
 * @param q_0 The first quaternion.
 * @param q_1 The second quaternion.
 * @return The multiplied quaternion.
 */
 Quat quat_mul(Quat q_0, Quat q_1) {
    Quat out_quaternion;

    out_quaternion.x = q_0.x * q_1.w +
                       q_0.y * q_1.z -
                       q_0.z * q_1.y +
                       q_0.w * q_1.x;

    out_quaternion.y = -q_0.x * q_1.z +
                       q_0.y * q_1.w +
                       q_0.z * q_1.x +
                       q_0.w * q_1.y;

    out_quaternion.z = q_0.x * q_1.y -
                       q_0.y * q_1.x +
                       q_0.z * q_1.w +
                       q_0.w * q_1.z;

    out_quaternion.w = -q_0.x * q_1.x -
                       q_0.y * q_1.y -
                       q_0.z * q_1.z +
                       q_0.w * q_1.w;

    return out_quaternion;
}

/// Multiply a Quat by a Vec3
 Vec3 quat_mul_vec3(Quat quat, Vec3 vec) {
    // (Matin Kamali) implementation not Kohi as of writing this
    // https://answers.unity.com/questions/372371/multiply-quaternion-by-vector3-how-is-done.html
    f32 num1 = quat.x * 2;
    f32 num2 = quat.y * 2;
    f32 num3 = quat.z * 2;
    f32 num4 = quat.x * num1;
    f32 num5 = quat.y * num2;
    f32 num6 = quat.z * num3;
    f32 num7 = quat.x * num2;
    f32 num8 = quat.x * num3;
    f32 num9 = quat.y * num3;
    f32 num10 = quat.w * num1;
    f32 num11 = quat.w * num2;
    f32 num12 = quat.w * num3;
    Vec3 result;
    result.x = (1 - (num5 + num6)) * vec.x + (num7 - num12) * vec.y + (num8 + num11) * vec.z;
    result.y = (num7 + num12) * vec.x + (1 - (num4 + num6)) * vec.y + (num9 - num10) * vec.z;
    result.z = (num8 - num11) * vec.x + (num9 + num10) * vec.y + (1 - (num4 + num5)) * vec.z;
    return result;
}

/**
 * @brief Calculates the dot product of the provided quaternions.
 *
 * @param q_0 The first quaternion.
 * @param q_1 The second quaternion.
 * @return The dot product of the provided quaternions.
 */
 f32 quat_dot(Quat q_0, Quat q_1) {
    return q_0.x * q_1.x +
           q_0.y * q_1.y +
           q_0.z * q_1.z +
           q_0.w * q_1.w;
}

/**
 * @brief Creates a rotation matrix from the given quaternion.
 *
 * @param q The quaternion to be used.
 * @return A rotation matrix.
 */
 Mat4 quat_to_mat4(Quat q) {
    Mat4 out_matrix = mat4_identity();

    // https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix

    Quat n = quat_normalize(q);

    out_matrix.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    out_matrix.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    out_matrix.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

    out_matrix.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    out_matrix.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    out_matrix.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    out_matrix.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    out_matrix.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    out_matrix.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

    return out_matrix;
}

/**
 * @brief Calculates a rotation matrix based on the quaternion and the passed in center point.
 *
 * @param q The quaternion.
 * @param center The center point.
 * @return A rotation matrix.
 */
 Mat4 quat_to_rotation_matrix(Quat q, Vec3 center) {
    Mat4 out_matrix;

    f32* o = out_matrix.data;
    o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
    o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

    o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
    o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

    o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
    o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
    o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;
    return out_matrix;
}

/**
 * @brief Creates a quaternion from the given axis and angle.
 *
 * @param axis The axis of rotation.
 * @param radians The angle of rotation.
 * @param normalize Indicates if the quaternion should be normalized.
 * @return A new quaternion.
 */
 Quat quat_from_axis_angle(Vec3 axis, f32 radians, b8 normalize) {
    const f32 half_angle = 0.5f * radians;
    f32 s = semath_sin(half_angle);
    f32 c = semath_cos(half_angle);

    Quat q = (Quat){s * axis.x, s * axis.y, s * axis.z, c};
    if (normalize) {
        return quat_normalize(q);
    }
    return q;
}

/**
 * @brief Calculates spherical linear interpolation of a given percentage
 * between two quaternions.
 *
 * @param q_0 The first quaternion.
 * @param q_1 The second quaternion.
 * @param percentage The percentage of interpolation, typically a value from 0.0f-1.0f.
 * @return An interpolated quaternion.
 */
 Quat quat_slerp(Quat q_0, Quat q_1, f32 percentage) {
    Quat out_quaternion;
    // Source: https://en.wikipedia.org/wiki/Slerp
    // Only unit quaternions are valid rotations.
    // Normalize to avoid undefined behavior.
    Quat v0 = quat_normalize(q_0);
    Quat v1 = quat_normalize(q_1);

    // Compute the cosine of the angle between the two vectors.
    f32 dot = quat_dot(v0, v1);

    // If the dot product is negative, slerp won't take
    // the shorter path. Note that v1 and -v1 are equivalent when
    // the negation is applied to all four components. Fix by
    // reversing one quaternion.
    if (dot < 0.0f) {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }

    const f32 DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.
        out_quaternion = (Quat){
            v0.x + ((v1.x - v0.x) * percentage),
            v0.y + ((v1.y - v0.y) * percentage),
            v0.z + ((v1.z - v0.z) * percentage),
            v0.w + ((v1.w - v0.w) * percentage)};

        return quat_normalize(out_quaternion);
    }

    // Since dot is in range [0, DOT_THRESHOLD], acos is safe
    f32 theta_0 = semath_cos(dot);          // theta_0 = angle between input vectors
    f32 theta = theta_0 * percentage;  // theta = angle between v0 and result
    f32 sin_theta = semath_sin(theta);       // compute this value only once
    f32 sin_theta_0 = semath_sin(theta_0);   // compute this value only once

    f32 s0 = semath_cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
    f32 s1 = sin_theta / sin_theta_0;

    return (Quat){
        (v0.x * s0) + (v1.x * s1),
        (v0.y * s0) + (v1.y * s1),
        (v0.z * s0) + (v1.z * s1),
        (v0.w * s0) + (v1.w * s1)};
}

/// ----
/// RECT
/// ----

 Rect rect_create(Vec2 pos, Vec2 size) {
    return (Rect) {pos.x, pos.y, size.x, size.y};
}

 b8 rect_overlaps_rect(Rect a, Rect b) {
    // following Ericson, C, 2004. Real-Time Collision Detection. 1.  CRC Press.
    // page 79, AABB vs AABB
    f32 t;
    if ((t = a.x - b.x) > b.w || -t > a.w) return false;
    if ((t = a.y - b.y) > b.h || -t > a.h) return false;
    return true;
}

 b8 rect_overlaps_point(Rect rect, Vec2 point) {
    return (point.x > rect.x && point.x < rect.x + rect.w) && (point.y > rect.y && point.y < rect.y + rect.h);
}

 b8 point_overlaps_circle(Vec2 point, Vec2 center, f32 radius) {
    return vec2_distance(point, center) <= radius;
}

/// -----------
/// CONVERSIONS
/// -----------

/// Vec2i -> Vec2
 Vec2 vec2i_to_vec2(Vec2i v) {
    return (Vec2) {(f32)v.x, (f32)v.y};
}

//.      value    |  result
//.    ---------- | --------
//.    range_from | range_to
//.
 f32 remapf(f32 value, f32 range_from, f32 range_to) {
    return (value * range_to) / range_from;
}
 i32 remap(i32 value, i32 range_from, i32 range_to) {
    return (value * range_to) / range_from;
}

/// amount is in the range of [0-1]
 f32 lerp(f32 value, f32 target, f32 amount) {
    return value + amount * (target - value);
}

/// amount is in the range of [0-1]
 Vec3 vec3_lerp(Vec3 v1, Vec3 v2, f32 amount) {
    return (Vec3) {
        lerp(v1.x, v2.x, amount),
        lerp(v1.y, v2.y, amount),
        lerp(v1.z, v2.z, amount)
    };
}

/// the result's x, y, z correspond with the barycentric coordinates of the given 'pos'
 Vec3 cartesian_to_barycentric_coordinates(Vec2 cartesian, Vec2 point1, Vec2 point2, Vec2 point3) {
    Vec3 result;
    f32 x = cartesian.x;
    f32 y = cartesian.y;
    f32 x1 = point1.x;
    f32 x2 = point2.x;
    f32 x3 = point3.x;
    f32 y1 = point1.y;
    f32 y2 = point2.y;
    f32 y3 = point3.y;

    result.x = ( (y2 - y3) * (x - x3) + (x3 - x2) * (y - y3) ) / ( (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3) );
    result.y = ( (y3 - y1) * (x - x3) + (x1 - x3) * (y - y3) ) / ( (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3) );
    result.z = 1 - result.x - result.y;
    return result;
}