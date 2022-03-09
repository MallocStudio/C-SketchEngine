#ifndef SEPHYSICS_DEFINES_H
#define SEPHYSICS_DEFINES_H

#include "defines.h"
#include "semath.h"
#include <float.h>


/// ----------
/// Primitives
/// ----------

/// The types of collision shapes
typedef enum SE_SHAPES {
    SE_SHAPES_NONE  = -2, // future proof
    SE_SHAPES_JOINT = -1, // future proof 
    SE_SHAPES_PLANE = 0,
    SE_SHAPES_CIRCLE, SE_SHAPES_BOX, SE_SHAPES_POLYGON,
    // SE_SHAPES_AABB,
    SE_SHAPES_COUNT
} SE_SHAPES;

/// this way we can case whatever primitive we're using to (SE_Shape)
/// to get some common data such as the type of the shape (polymorphism)
typedef struct SE_Shape {
    Vec2 velocity;
    Vec2 acceleration;
    f32 inverse_mass;
    SE_SHAPES type;
    // f32 rotation;
} SE_Shape;

// /// ----
// /// AABB
// /// ----
// typedef struct SE_AABB {
//     SE_Shape shape;
//     f32 xmin, xmax, ymin, ymax;
// } SE_AABB;
// // * we init this to the following:
// // * we use this approch because it better allows for updating the size of min and max boundaries
// // * as in if xmax < point.x xmax = point.x
// SEINLINE void init_aabb(SE_AABB *aabb) {
//     aabb->shape.type = SE_SHAPES_AABB;
//     // aabb->xmin = +FLT_MAX;
//     // aabb->xmax = -FLT_MAX;
//     // aabb->ymin = +FLT_MAX;
//     // aabb->ymax = -FLT_MAX;
//     aabb->xmin = -0.5f;
//     aabb->xmax = +0.5f;
//     aabb->ymin = -0.5f;
//     aabb->ymax = +0.5f;
// }

/// ---
/// BOX
/// ---
typedef struct SE_Box {
    SE_Shape shape;
    f32 x, y, w, h;
} SE_Box;

/// -----------
/// SAT Polygon
/// -----------
#define SE_POLYGON_POINTS_COUNT_MAX 10
typedef struct SE_Polygon {
    Vec2 points[SE_POLYGON_POINTS_COUNT_MAX];
    u32 points_count;
} SE_Polygon;

/// ------
/// CIRCLE
/// ------
typedef struct SE_Circle {
    SE_Shape shape;
    f32 radius;
    Vec2 pos;
} SE_Circle;

/// -----
/// PLANE
/// -----
typedef struct SE_Plane {
    SE_Shape shape;
    Vec2 normal;
    f32 depth; // how far along the normal is this plane
} SE_Plane;

/// --------------
/// COLLISION DATA
/// --------------
typedef struct SE_Collision_Data {
    bool is_collided;
    f32 depth;   // HOW DEEP IS THE COLLISION (HOW FAR SHOULD WE MOVE THE PHYSICS OBJECTS)
    Vec2 normal; // THE DIRECTION OF DEPTH (from shape_a towards shape_b)
    Vec2 world_pos; // POINT OF CONTACT

    SE_Shape *shape_a;
    SE_Shape *shape_b;
} SE_Collision_Data;

/// -------------------------
/// INITIALIZATION PROCEDURES
/// -------------------------

SEINLINE void init_shape(SE_Shape *shape, SE_SHAPES type) {
    shape->type = type;
    shape->velocity = vec2_zero();
    shape->acceleration = vec2_zero();
    shape->inverse_mass = 1; // ! plane's inverse mass is set to 0 in its own constructor
    // shape->collision_data = new(struct SE_Collision_Data);
}
SEINLINE void deinit_shape(SE_Shape *shape) {
    // free(shape->collision_data);
}

SEINLINE void init_box(SE_Box *box) {
    init_shape(&box->shape, SE_SHAPES_BOX);
    box->x = -0.5f;
    box->y = -0.5f;
    box->w = 1;
    box->h = 1;
}
SEINLINE void box_get_xmin_ymin_xmax_ymax(SE_Box *box, f32 *xmin, f32 *ymin, f32 *xmax, f32 *ymax) {
    if (xmin != NULL) *xmin = box->x;
    if (ymin != NULL) *ymin = box->y;
    if (xmax != NULL) *xmax = box->x + box->w;
    if (ymax != NULL) *ymax = box->y + box->h;
}
SEINLINE void box_get_x_y_w_h(SE_Box *box, f32 *x, f32 *y, f32 *w, f32 *h) {
    if (x != NULL) *x = box->x;
    if (y != NULL) *y = box->y;
    if (w != NULL) *w = box->w;
    if (h != NULL) *h = box->h;
}

SEINLINE void init_circle(SE_Circle *circle) {
    init_shape(&circle->shape, SE_SHAPES_CIRCLE);
    circle->pos = vec2_zero();
    circle->radius = 1.0f;
}

SEINLINE void init_plane(SE_Plane *plane) {
    init_shape(&plane->shape, SE_SHAPES_PLANE);
    plane->shape.inverse_mass = 0;
    plane->normal = vec2_create(0, 1);
    plane->depth = 1;
}

SEINLINE void init_polygon(SE_Polygon *polygon) {
    polygon->points_count = 0;
}
SEINLINE void se_polygon_add_point(SE_Polygon *polygon, Vec2 point) {
    polygon->points[polygon->points_count] = point;
    polygon->points_count++;
}

SEINLINE init_collision_data(SE_Collision_Data *cd, bool is_collided, Vec2 normal, Vec2 world_pos, f32 depth, SE_Shape *shape_a, SE_Shape* shape_b) {
    cd->is_collided = is_collided;
    cd->normal = normal;
    cd->world_pos = world_pos;
    cd->depth = depth;
    cd->shape_a = shape_a;
    cd->shape_b = shape_b;
}
SEINLINE void collision_data_deep_copy(const SE_Collision_Data *src, SE_Collision_Data *destination) {
    destination->is_collided = src->is_collided;
    destination->depth = src->depth;
    destination->normal = src->normal;
    destination->world_pos = src->world_pos;
    destination->shape_a = src->shape_a;
    destination->shape_b = src->shape_b;
}

#endif // SEPHYSICS_DEFINES_H