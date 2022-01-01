#include "math.h"

///
f32 lerp_f32 (f32 v, f32 to, f32 amount) {
    return v + (to - v) * amount;
}

///
void lerp_rgba (RGBA *color, RGBA destination, f32 amount) {
    color->r = lerp_f32(color->r, destination.r, amount);
    color->g = lerp_f32(color->g, destination.r, amount);
    color->b = lerp_f32(color->b, destination.r, amount);
    color->a = lerp_f32(color->a, destination.r, amount);
}