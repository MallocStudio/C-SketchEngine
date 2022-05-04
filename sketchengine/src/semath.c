#include "semath.h"
#include <math.h>

f32 semath_abs(f32 x) {
    return fabsf(x);
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

f32 semath_sqrt(f32 x) {
    return sqrtf(x);
}

f32 semath_power(f32 base, f32 p) {
    return powf(base, p);
}