#ifndef SEPHYSICS_DEFINES_H
#define SEPHYSICS_DEFINES_H

#include "defines.h"
#include "semath.h"
#include <float.h>

/// ----------
/// Primitives
/// ----------
typedef enum SE_SHAPES {
    SHAPE, AABB, CIRCLE,
    COUNT
} SE_SHAPES;

/// this way we can case whatever primitive we're using to (SE_Shape) 
/// to get some common data such as the type of the shape (polymorphism)
typedef struct SE_Shape {
    SE_SHAPES type;
} SE_Shape;

/// ----
/// AABB
/// ----
typedef struct SE_AABB {
    SE_Shape shape;
    f32 xmin, xmax, ymin, ymax;
} SE_AABB;
// * we init this to the following:
// * we use this approch because it better allows for updating the size of min and max boundaries
// * as in if xmax < point.x xmax = point.x
SEINLINE void init_aabb(SE_AABB *aabb) {
    aabb->shape.type = AABB;
    aabb->xmin = +FLT_MAX;
    aabb->xmax = -FLT_MAX;
    aabb->ymin = +FLT_MAX;
    aabb->ymax = -FLT_MAX;
}

/// ------
/// CIRCLE
/// ------
typedef struct SE_Circle {
    SE_Shape shape;
    Vec2 pos;
    f32 radius;
} SE_Circle;
SEINLINE void init_circle(SE_Circle *circle) {
    circle->shape.type = CIRCLE;
    circle->pos = (Vec2) {0, 0};
    circle->radius = 0.0f;
}

/// -----
/// PLANE
/// -----
typedef struct SE_Plane {
    Vec2 normal;
    f32 depth; // how far along the normal is this plane
} SE_Plane;
SEINLINE void init_plane(SE_Plane *plane) {
    plane->normal = vec2_create(0, 0);
    plane->depth = 0;
}

/// --------------
/// COLLISION DATA
/// --------------
typedef struct SE_Collision_Data {
    bool is_collided;
    f32 depth;   // HOW DEEP IS THE COLLISION (HOW FAR SHOULD WE MOVE THE PHYSICS OBJECTS)
    Vec2 normal; // THE DIRECTION OF DEPTH
    Vec2 world_pos; // POINT OF CONTACT
    
    SE_Shape *shape_a;
    SE_Shape *shape_b;
} SE_Collision_Data;
SEINLINE init_collision_data(SE_Collision_Data *cd, bool is_collided, Vec2 normal, Vec2 world_pos, f32 depth, SE_Shape *shape_a, SE_Shape* shape_b) {
    cd->is_collided = is_collided;
    cd->normal = normal;
    cd->world_pos = world_pos;
    cd->depth = depth;
    cd->shape_a = shape_a;
    cd->shape_b = shape_b;
}

#endif // SEPHYSICS_DEFINES_H