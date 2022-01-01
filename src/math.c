#include "math.h"

///
f32 lerp_f32 (f32 v, f32 to, f32 amount) {
    return v + (to - v) * amount;
}

///
void clamp (f32 *v, f32 min, f32 max) {
    if (*v < min) *v = min;
    else if (*v > max) *v = max;
}

///
void lerp_rgba (RGBA *color, RGBA destination, f32 amount) {
    color->r = lerp_f32(color->r, destination.r, amount);
    color->g = lerp_f32(color->g, destination.g, amount);
    color->b = lerp_f32(color->b, destination.b, amount);
    color->a = lerp_f32(color->a, destination.a, amount);
    clamp_rgba(color);
}

///
void clamp_rgba (RGBA *color) {
    clamp(&color->r, 0, 1);
    clamp(&color->g, 0, 1);
    clamp(&color->b, 0, 1);
    clamp(&color->a, 0, 1);
}