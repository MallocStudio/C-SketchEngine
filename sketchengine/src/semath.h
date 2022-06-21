// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h
// https://www.youtube.com/watch?v=WR0tAQj-og8
#ifndef SEMATH_H
#define SEMATH_H

#include "sedefines.h"
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
#define SEMATH_RAD2DEG(x) x * SEMATH_RAD2DEG_MULTIPLIER
#define SEMATH_DEG2RAD(x) x * SEMATH_DEG2RAD_MULTIPLIER

/// ----------------------
/// GENERAL MATH FUNCTIONS
/// ----------------------
f32 semath_sin(f32 x);
f32 semath_cos(f32 x);
f32 semath_tan(f32 x);
f32 semath_acos(f32 x);
f32 semath_asin(f32 x);
f32 semath_sqrt(f32 x);
f32 semath_abs(f32 x);
f32 semath_power(f32 base, f32 power);
f32 semath_remainder(f32 x, f32 denominator);

/// Indicates if the value is a power of 2. 0 is considered _not_ a power of 2
/// 'value' is the value to be interpreted
/// 'returns' true if a power of 2, otherwise false
b8 is_power_of_2(i32 value);

i32 random();
i32 random_in_range(i32 min, i32 max);
f32 frandom();
f32 frandom_in_range(f32 min, f32 max);

/// ----
/// Vec2
/// ----

/// returns a Vec2
 Vec2 vec2_create(f32 x, f32 y);

/// returns a Vec2 with x, y being zero
 Vec2 vec2_zero();

/// returns a Vec2 with x, y being one
 Vec2 vec2_one();

/// returns a Vec2 pointing up
 Vec2 vec2_up();

/// returns a Vec2 pointing down
 Vec2 vec2_down();

/// returns a Vec2 pointing left
 Vec2 vec2_left();

/// returns a Vec2 pointing right
 Vec2 vec2_right();

/// (v1 + v2)
 Vec2 vec2_add(Vec2 v1, Vec2 v2);

/// (v1 - v2)
 Vec2 vec2_sub(Vec2 v1, Vec2 v2);

/// (v1 * v2)
 Vec2 vec2_mul(Vec2 v1, Vec2 v2);

/// (v1 * scalar)
 Vec2 vec2_mul_scalar(Vec2 v1, f32 scalar);

/// (v1 / v2)
 Vec2 vec2_div(Vec2 v1, Vec2 v2);

 f32 vec2_dot(Vec2 v1, Vec2 v2);

 Vec2 vec2_average(Vec2 v1, Vec2 v2);

 Vec2 vec2_rotated(Vec2 v, f32 angle_radians);

/// returns the squared magnitude / length of the provided vector
/// use this one when comparing lengths of multiple vectors, because you don't
/// really need the sqrt version
 f32 vec2_magnitude_squared(Vec2 vec);

/// returns magnitude / length of the provided vector
 f32 vec2_magnitude(Vec2 vec);

/// normalises the provided vector in place to a unit vector
 void vec2_normalise(Vec2 *vec);

/// returns the normalised version of the provided vector
 Vec2 vec2_normalised(Vec2 vec);

/// find the angle of the given vec2
 f32 vec2_angle(Vec2 v);

 f32 vec2_angle_between(Vec2 v1, Vec2 v2);

/// compares all elements of v1 and v2 and unsures the difference is less than tolerance
/// tolerance is typically SEMATH_FLOAT_EPSILON or similiar
/// returns true if within tolerance
 b8 vec2_compare(Vec2 v1, Vec2 v2, f32 tolerance);

/// returns the distance between v1 and v2
 f32 vec2_distance(Vec2 v1, Vec2 v2);

/// ----
/// Vec3
/// ----

/// returns a Vec3
 Vec3 vec3_create(f32 x, f32 y, f32 z);

#define v3f(x, y, z) vec3_create(x, y, z)
#define v2f(x, y) vec2_create(x, y)

/// returns a Vec3 with x, y, z being zero
 Vec3 vec3_zero();

/// returns a Vec3 with x, y, z being one
 Vec3 vec3_one();

/// returns a Vec3 pointing up
 Vec3 vec3_up();

/// returns a Vec3 pointing down
 Vec3 vec3_down();

/// returns a Vec3 pointing left
 Vec3 vec3_left();

/// returns a Vec3 pointing right
 Vec3 vec3_right();

/// returns a Vec3 pointing forward (right handed coord system) (0, 0, -1)
 Vec3 vec3_forward();

/// returns a Vec3 pointing back (right handed coord system) (0, 0, +1)
 Vec3 vec3_backward();

/// v1 + v2 and returns a copy
 Vec3 vec3_add(Vec3 v1, Vec3 v2);

/// v1 - v2 and returns a copy
 Vec3 vec3_sub(Vec3 v1, Vec3 v2);

/// v1 * v2 and returns a copy
 Vec3 vec3_mul(Vec3 v1, Vec3 v2);

/// v1 * scalar and returns a copy
 Vec3 vec3_mul_scalar(Vec3 v1, f32 scalar);

/// v1 / v2 and returns a copy
 Vec3 vec3_div(Vec3 v1, Vec3 v2);

/// Returns the squared magnitude / length of the provided vector
 f32 vec3_magnitude_squared(Vec3 v);

/// Returns the magnitude / length of the provided vector
 f32 vec3_magnitude(Vec3 v);

/// Normalises the provided vector in place to a unit vector
 void vec3_normalise(Vec3 *v);

/// Returns a normalised copy of the supplied vector
 Vec3 vec3_normalised(Vec3 v);

/// Returns the dot product of v1 and v2
/// Typically used to calculat ethe difference in direction
 f32 vec3_dot(Vec3 v1, Vec3 v2);

/// Calculates and returns the cross product of the supplied vectors
/// The cross product is a new vector which is orthoganal to both provided vectors
 Vec3 vec3_cross(Vec3 v1, Vec3 v2);

/// Compares all elements of v1 and v2 and ensures the difference is less than tolerance
/// tolerance is typically SEMATH_FLOAT_EPSILON or similar
/// returns true if within tolerance
 b8 vec3_compare(Vec3 v1, Vec3 v2, f32 tolerance);

/// Returns the distance between v1 and v2
 f32 vec3_distance(Vec3 v1, Vec3 v2);

/// ----
/// Mat2
/// ----

/// init Mat2
 Mat2 mat2_create(f32 m0, f32 m1, f32 m2, f32 m3);

/// get a Mat2 identity
 Mat2 mat2_identity();

/// get a Mat2 from 2 Vec2
/// @param 'col1' column 1
/// @param 'col2' column 2
 Mat2 mat2_create_vec2(Vec2 col1, Vec2 col2);

/// returns true if the two provided mat2s are identitcal
 b8 mat2_compare(Mat2 m1, Mat2 m2, f32 tolerance);

// a b   e f   ae + bg, af + bh
// c d x g h = ce + dg, cf + dh
/// multiply m1 x m2 (column major)
 Mat2 mat2_mul(Mat2 m1, Mat2 m2);

/// multiply m by a v
 Vec2 mat2_mul_vec2(Mat2 m, Vec2 v);

/// --------
/// Matrix 4
/// --------
/// ported from:
/// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h

/// @brief creates a Mat4 based on the array passed in
/// @brief [0 - 3] is column 1, [4 - 7] is column 2 etc
/// @param 'v' is an array of 16 elements [0 - 15]
 Mat4 mat4_create(f32 v[16]);

/// returns an identity Mat4
 Mat4 mat4_identity();

/// returns the result of multiplying m1 and m2
 Mat4 mat4_mul(Mat4 m1, Mat4 m2);

/// creates and returns an orthographic projection Mat4.
/// Typically used to render flat or 2D scenes
/// @param left The left side of the view frustum.
/// @param right The right side of the view frustum.
/// @param bottom The bottom side of the view frustum.
/// @param top The top side of the view frustum.
/// @param near_clip The near clipping plane distance.
/// @param far_clip The far clipping plane distance.
 Mat4 mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_clip, f32 far_clip);

/// creates and returns a perspective Mat4. Typically used to render 3D scenes
/// @param fov_radians The field of view in radians.
/// @param aspect_ratio The aspect ratio.
/// @param near_clip The near clipping plane distance.
/// @param far_clip The far clipping plane distance.
 Mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip);

/// creates and returns a look-at Mat4, or a Mat4 looking at a target from the perspective of pos
 Mat4 mat4_lookat(Vec3 position, Vec3 target, Vec3 up);

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
 Mat4 mat4_transposed(Mat4 m);

/// creates and returns an inverse of the provided Mat4
 Mat4 mat4_inverse(Mat4 matrix);

/// get the standard ortho projection matrix from a viewport. near clip is set to -1, far clip is set to 1
 Mat4 viewport_to_ortho_projection_matrix (Rect viewport);

/// get the standard ortho projection matrix from a viewport with near clip and far clip adjustable
 Mat4 viewport_to_ortho_projection_matrix_extra (Rect viewport, f32 near_clip, f32 far_clip);

/// creates and returns a translation Mat4 from the given pos
 Mat4 mat4_translation(Vec3 pos);

/// creates and returns a scale Mat4
 Mat4 mat4_scale(Vec3 scale);

/// creates and returns a rotation Mat4 from the provided x angle
 Mat4 mat4_euler_x(f32 angle_radians);

/// creates and returns a rotation Mat4 from the provided y angle
 Mat4 mat4_euler_y(f32 angle_radians);

/// creates and returns a rotation Mat4 from the provided z angle
 Mat4 mat4_euler_z(f32 angle_radians);

/// creates a rot Mat4 from the provided x, y, z, axis rotation
 Mat4 mat4_euler_xyz(f32 x_radians, f32 y_radians, f32 z_radians);

/// returns a forward Vec3 relative to the provided Mat4
 Vec3 mat4_forward(Mat4 m);

/// returns a backward Vec3 relative to the provided Mat4
 Vec3 mat4_backward(Mat4 m);

/// returns a up Vec3 relative to the provided Mat4
 Vec3 mat4_up(Mat4 m);

/// returns a down Vec3 relative to the provided Mat4
 Vec3 mat4_down(Mat4 m);

/// returns a left Vec3 relative to the provided Mat4
 Vec3 mat4_left(Mat4 m);

/// returns a right Vec3 relative to the provided Mat4
 Vec3 mat4_right(Mat4 m);

/// creates and returns the result of the multiplication of m and v
 Vec4 mat4_mul_vec4 (Mat4 m, Vec4 v);

/// returns a Vec3 that represents translation
 Vec3 mat4_get_translation(Mat4 m);

/// ----------
/// Quaternion
/// ----------
/// ported from:
/// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h

/// Creates an identity quaternion
 Quat quat_identity();

/**
 * @brief Returns the normal of the provided quaternion.
 *
 * @param q The quaternion.
 * @return The normal of the provided quaternion.
 */
 f32 quat_normal(Quat q);

/**
 * @brief Returns a normalized copy of the provided quaternion.
 *
 * @param q The quaternion to normalize.
 * @return A normalized copy of the provided quaternion.
 */
 Quat quat_normalize(Quat q);

/**
 * @brief Returns the conjugate of the provided quaternion. That is,
 * The x, y and z elements are negated, but the w element is untouched.
 *
 * @param q The quaternion to obtain a conjugate of.
 * @return The conjugate quaternion.
 */
 Quat quat_conjugate(Quat q);

/**
 * @brief Returns an inverse copy of the provided quaternion.
 *
 * @param q The quaternion to invert.
 * @return An inverse copy of the provided quaternion.
 */
 Quat quat_inverse(Quat q);

 Quat quat_cross(Quat q1, Quat q2);

/**
 * @brief Multiplies the provided quaternions.
 *
 * @param q_0 The first quaternion.
 * @param q_1 The second quaternion.
 * @return The multiplied quaternion.
 */
 Quat quat_mul(Quat q_0, Quat q_1);

/// Multiply a Quat by a Vec3
 Vec3 quat_mul_vec3(Quat quat, Vec3 vec);

/**
 * @brief Calculates the dot product of the provided quaternions.
 *
 * @param q_0 The first quaternion.
 * @param q_1 The second quaternion.
 * @return The dot product of the provided quaternions.
 */
 f32 quat_dot(Quat q_0, Quat q_1);

/**
 * @brief Creates a rotation matrix from the given quaternion.
 *
 * @param q The quaternion to be used.
 * @return A rotation matrix.
 */
 Mat4 quat_to_mat4(Quat q);

/**
 * @brief Calculates a rotation matrix based on the quaternion and the passed in center point.
 *
 * @param q The quaternion.
 * @param center The center point.
 * @return A rotation matrix.
 */
 Mat4 quat_to_rotation_matrix(Quat q, Vec3 center);

/**
 * @brief Creates a quaternion from the given axis and angle.
 *
 * @param axis The axis of rotation.
 * @param radians The angle of rotation.
 * @param normalize Indicates if the quaternion should be normalized.
 * @return A new quaternion.
 */
 Quat quat_from_axis_angle(Vec3 axis, f32 radians, b8 normalize);

/**
 * @brief Calculates spherical linear interpolation of a given percentage
 * between two quaternions.
 *
 * @param q_0 The first quaternion.
 * @param q_1 The second quaternion.
 * @param percentage The percentage of interpolation, typically a value from 0.0f-1.0f.
 * @return An interpolated quaternion.
 */
 Quat quat_slerp(Quat q_0, Quat q_1, f32 percentage);

/// ----
/// RECT
/// ----

 Rect rect_create(Vec2 pos, Vec2 size);

 b8 rect_overlaps_rect(Rect a, Rect b);

 b8 rect_overlaps_point(Rect rect, Vec2 point);

 b8 point_overlaps_circle(Vec2 point, Vec2 center, f32 radius);

b8 ray_overlaps_sphere(Vec3 ray_origin, Vec3 ray_direction, f32 max_distance, Vec3 sphere_origin, f32 sphere_radius, f32 *hit_distance);

/// -----------
/// CONVERSIONS
/// -----------

/// Vec2i -> Vec2
 Vec2 vec2i_to_vec2(Vec2i v);

//.      value    |  result
//.    ---------- | --------
//.    range_from | range_to
//.
 f32 remapf(f32 value, f32 range_from, f32 range_to);
 i32 remap(i32 value, i32 range_from, i32 range_to);

/// amount is in the range of [0-1]
 f32 lerp(f32 value, f32 target, f32 amount);

/// amount is in the range of [0-1]
 Vec3 vec3_lerp(Vec3 v1, Vec3 v2, f32 amount);

/// the result's x, y, z correspond with the barycentric coordinates of the given 'pos'
 Vec3 cartesian_to_barycentric_coordinates(Vec2 cartesian, Vec2 point1, Vec2 point2, Vec2 point3);

#endif // SEMATH_H