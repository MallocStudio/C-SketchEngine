#ifndef SE_ANIMATION_H
#define SE_ANIMATION_H

#include "sedefines.h"
#include "semath.h"
#include "sestring.h"

typedef struct SE_Animation {
    f32 duration;
    f32 current_frame;
    f32 speed;
} SE_Animation;

    /// Updates current frame and returns its value.
    /// If current frame exceeds duration, it gets set to zero.
f32 seanimation_update(SE_Animation *animation, f32 delta_time);

#endif // SE_ANIMATION_H