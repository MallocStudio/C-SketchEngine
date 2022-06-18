// checkout wikipedia list of moments on inertia
#ifndef SEPHYSICS_H
#define SEPHYSICS_H

#include "sephysics_defines.h"

/// -------------------------------------------------------------------
/// Collision function table functions
/// -------------------------------------------------------------------

SE_Collision_Data se_phys_check_plane_plane(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_plane_circle(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_plane_box(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_circle_plane(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_circle_circle(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_circle_box(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_box_plane(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_box_circle(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_box_box(SE_Shape *a, SE_Shape *b);

SE_Collision_Data se_phys_check_polygon_box(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_box_polygon(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_polygon_circle(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_circle_polygon(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_polygon_plane(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_plane_polygon(SE_Shape *a, SE_Shape *b);
SE_Collision_Data se_phys_check_polygon_polygon(SE_Shape *a, SE_Shape *b);

/// ------------------------
/// COLLISION FUNCTION TABLE
/// ------------------------

/// note that we use a function pointer table to resolve collision between shapes
typedef SE_Collision_Data (*se_phys_check)(SE_Shape*, SE_Shape*);

static se_phys_check collision_function_array[] = {
    se_phys_check_plane_plane,   se_phys_check_plane_circle,   se_phys_check_plane_box,   se_phys_check_plane_polygon,
    se_phys_check_circle_plane,  se_phys_check_circle_circle,  se_phys_check_circle_box,  se_phys_check_circle_polygon,
    se_phys_check_box_plane,     se_phys_check_box_circle,     se_phys_check_box_box,     se_phys_check_box_polygon,
    se_phys_check_polygon_plane, se_phys_check_polygon_circle, se_phys_check_polygon_box, se_phys_check_polygon_polygon,
};

/// ---------------------------
/// RENDERER RELATED TO PHYSICS
/// ---------------------------
#include "serenderer.h"

typedef struct SE_Physics_Global {
    b8 active;
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
    if (global_physics_debug->active) segl_lines_update_frame(&global_physics_debug->lines);
    else segl_lines_clear(&global_physics_debug->lines);
}
void se_phys_render_shape(SEGL_Line_Renderer *renderer, SE_Shape *shape);
void render_collision_data(const SE_Collision_Data *collision_data);
/// ---------------
/// HANDLING SHAPES
/// ---------------

/// move the given shape by translation
void se_phys_set_shape_transform(SE_Shape *shape, Vec2 pos, Vec2 normal);
void se_phys_get_shape_transform(SE_Shape *shape, Vec2 *pos, Vec2 *normal);
#endif // SEPHYSICS_H