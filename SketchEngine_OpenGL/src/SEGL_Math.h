#ifndef SEGL_MATH_H
#define SEGL_MATH_H

#include "raymath.h"
typedef Matrix  mat4;
typedef Vector2 vec2;
typedef Vector3 vec3;
typedef Vector4 vec4;

#include "defines.h"

vec4 mat4_mul_vec4 (const mat4 *m, const vec4 *v);

#endif // SEGL_MATH_H