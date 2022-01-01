#ifndef MATH_H
#define MATH_H

#include "core.h"

///
/// general math
///

/// lerp f32 from 'v' to 'to' by 'amount'
f32 lerp_f32 (f32 v, f32 to, f32 amount);

///
/// color math
///

/// lerps 'color' towards 'destination' by 'amount'. Does not return anything, the result is
/// stored in 'color'
void lerp_rgba (RGBA *color, RGBA destination, f32 amount);

#endif // MATH_H