#include "SEGL_Math.h"

vec4 mat4_mul_vec4 (const mat4 *m, const vec4 *v) {
    vec4 result;
    result.x = m->m0  * v->x + m->m1  * v->y + m->m2  * v->z + m->m3  * v->w;
    result.y = m->m4  * v->x + m->m5  * v->y + m->m6  * v->z + m->m7  * v->w;
    result.z = m->m8  * v->x + m->m9  * v->y + m->m10 * v->z + m->m11 * v->w;
    result.w = m->m12 * v->x + m->m13 * v->y + m->m14 * v->z + m->m15 * v->w;
    return result;
}