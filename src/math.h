#ifndef MATH_H
#define MATH_H

#include "core.h"

///
/// general math
///

/// lerp f32 from 'v' to 'to' by 'amount'
f32 lerp_f32 (f32 v, f32 to, f32 amount);

void clamp (f32 *v, f32 min, f32 max);

///
/// color math
///

/// lerps 'color' towards 'destination' by 'amount'. Does not return anything, the result is
/// stored in 'color'
void lerp_rgba (RGBA *color, RGBA destination, f32 amount);

/// Clamps RGBA [0 1]
void clamp_rgba (RGBA *color);
#endif // MATH_H