// checkout wikipedia list of moments on inertia
#ifndef SEPHYSICS_H
#define SEPHYSICS_H

/// This is not a complete physics solution yet
/// It is just a testbed and an attempt at an assignment
#include "sephysics_defines.h"
#include "serenderer.h"

typedef struct SE_Physics_Global {
    bool active;
    SEGL_Line_Renderer lines; // meant for debugging and rendering debug shapes
} SE_Physics_Global;
SE_Physics_Global *global_physics_debug;
SEINLINE void se_physics_global_init() {
    global_physics_debug->active = false;
    segl_lines_init(&global_physics_debug->lines);
}
SEINLINE void se_physics_global_deinit() {
    segl_lines_deinit(&global_physics_debug->lines);
}
SEINLINE void se_physics_global_render() {
    segl_lines_update_frame(&global_physics_debug->lines);
}

/// check for collision between an AABB and a circle
SE_Collision_Data se_phys_check_aabb_circle(SE_AABB *aabb, SE_Circle *circle);
/// check for collision between a circle and a circle
SE_Collision_Data se_phys_check_circle_circle(SE_Circle *c1, SE_Circle *c2);
/// check for collision between a circle and a plane
SE_Collision_Data se_phys_check_circle_plane(SE_Circle* c, SE_Plane *p);
#endif // SEPHYSICS_H