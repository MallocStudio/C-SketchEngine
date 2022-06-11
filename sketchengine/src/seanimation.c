#include "seanimation.h"

f32 seanimation_update(SE_Animation *animation, f32 delta_time) {
    se_assert(animation != NULL);
    animation->current_frame += delta_time * animation->speed;
    if (animation->current_frame > animation->duration) animation->current_frame = 0;
    return animation->current_frame;
}