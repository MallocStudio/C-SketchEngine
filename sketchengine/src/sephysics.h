#ifndef SEPHYSICS_H
#define SEPHYSICS_H

/// This is not a complete physics solution yet
/// It is just a testbed and an attempt at an assignment
#include "sephysics_defines.h"
#include "serenderer.h" // @nocheckin
typedef struct SE_Physics_Global { // @nocheckin
    Line_Renderer lines; // meant for debugging and rendering debug shapes
} SE_Physics_Global;
SE_Physics_Global *physics_global;
SEINLINE void se_physics_global_init(SE_Physics_Global *pg) {
    segl_lines_init(&pg->lines);
}
SEINLINE void se_physics_global_deinit(SE_Physics_Global *pg) {
    segl_lines_deinit(&pg->lines);
}
SEINLINE void se_physics_global_render(SE_Physics_Global *pg) {
    segl_lines_update_frame(&pg->lines);
}

SE_Collision_Data se_phys_check_aabb_circle(SE_AABB *aabb, SE_Circle *circle);

#endif // SEPHYSICS_H