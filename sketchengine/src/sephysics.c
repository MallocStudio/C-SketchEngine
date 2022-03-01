#include "sephysics.h"

SE_Collision_Data se_phys_check_plane_plane(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_PLANE && b->type == SE_SHAPES_PLANE);
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;
    // @incomplete but I guess it doesn't matter because it's plane vs plane

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_plane_circle(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_PLANE && b->type == SE_SHAPES_CIRCLE);
    return se_phys_check_circle_plane(b, a);
}

SE_Collision_Data se_phys_check_plane_box(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_PLANE && b->type == SE_SHAPES_BOX);
    return se_phys_check_box_plane(b, a);
}

SE_Collision_Data se_phys_check_circle_plane(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_CIRCLE && b->type == SE_SHAPES_PLANE);
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    SE_Circle *c = (SE_Circle*) a;
    SE_Plane *p = (SE_Plane*) b;
    Vec2 circle_pos = c->pos;

    Vec2 plane_pos = vec2_mul_scalar(p->normal, p->depth);
    // project circle_pos onto plane's normal vector
    f32 normal_projection = vec2_dot(circle_pos, p->normal);
    Vec2 c_pos_on_plane_normal = vec2_mul_scalar(p->normal, normal_projection);
    f32 _depth = c->radius + p->depth - normal_projection;

    // -- collided
    if (_depth > 0) {
        collided = true;
        normal = p->normal;
        depth = _depth;


        Vec2 plane_vec = vec2_create(p->normal.y, -p->normal.x);
        f32 np = vec2_dot(circle_pos, plane_vec);
        Vec2 c_pos_on_plane_vec = vec2_mul_scalar(plane_vec, np);
        world_pos = vec2_add(plane_pos, c_pos_on_plane_vec);

        // -- debug rendering
        if (global_physics_debug->active) {
            // -- normal
            global_physics_debug->lines.current_colour = vec3_create(1, 1, 0);
            segl_lines_draw_line_segment(&global_physics_debug->lines, world_pos, vec2_add(world_pos, normal));
            // -- depth
            global_physics_debug->lines.current_colour = vec3_create(0, 1, 1);
            segl_lines_draw_line_segment(&global_physics_debug->lines, circle_pos, vec2_add(circle_pos, vec2_mul_scalar(normal, depth)));
            // -- world pos
            global_physics_debug->lines.current_colour = vec3_create(1, 0, 0);
            segl_lines_draw_cross(&global_physics_debug->lines, world_pos, 0.1f);
            
            global_physics_debug->lines.current_colour = vec3_create(1, 1, 1);
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_circle_circle(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;
    
    SE_Circle *c1 = (SE_Circle*) a;
    SE_Circle *c2 = (SE_Circle*) b;
    Vec2 c1_pos = c1->pos;
    Vec2 c2_pos = c2->pos;

    // penetration
    f32 combined_radius = c1->radius + c2->radius;
    f32 distance = vec2_distance(c1_pos, c2_pos);
    // -- if distance is less than the maximum radius of these 2 circles, we're colliding
    if (distance < combined_radius) {
        collided = true;
        normal = vec2_sub(c2_pos, c1_pos);
        if (vec2_magnitude_squared(normal) == 0) normal = vec2_up();
        vec2_normalise(&normal);
        depth = combined_radius - distance;
        world_pos = vec2_add(c1_pos, vec2_mul_scalar(normal, c1->radius - depth));

        if (global_physics_debug->active) { // -- debug rendering
            // world pos
            segl_lines_draw_cross(&global_physics_debug->lines, world_pos, 0.1f);
            // penetration
            segl_lines_draw_arrow(&global_physics_debug->lines, 
                world_pos, 
                vec2_add(world_pos, vec2_mul_scalar(normal, depth)));
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_circle_box(SE_Shape *a, SE_Shape *b) {
    return se_phys_check_box_circle(b, a);
}

SE_Collision_Data se_phys_check_box_plane(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    SE_Box *box = (SE_Box*) a;
    SE_Plane *plane = (SE_Plane*) b;

    // loop through each of the points and figure out
    // which one has penetrated, and if multiple have
    // figure out which one is deeper
    Vec2 box_points[4] = {
        {box->x,          box->y         },
        {box->x + box->w, box->y         },
        {box->x,          box->y + box->h},
        {box->x + box->w, box->y + box->h}
    };
    for (i32 i = 0; i < 4; ++i) {
        Vec2 point = box_points[i];
        // -- is point inside of the plane?
        Vec2 plane_pos             = vec2_mul_scalar(plane->normal, plane->depth);
        f32 normal_projection      = vec2_dot(point, plane->normal);
        Vec2 point_on_plane_normal = vec2_mul_scalar(plane->normal, normal_projection);
        f32 _depth = plane->depth - normal_projection;
        if (_depth > 0) {
            collided = true;
            if (depth < _depth) {
                depth = _depth;
                normal = plane->normal;

                Vec2 plane_vec = vec2_create(plane->normal.y, -plane->normal.x);
                f32 np = vec2_dot(point, plane_vec);
                Vec2 point_on_plane_vec = vec2_mul_scalar(plane->normal, np);
                world_pos = vec2_add(plane_pos, point_on_plane_vec);
            }
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_box_circle(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    SE_Circle *circle = (SE_Circle*) b;
    SE_Box *box = (SE_Box*) a;

    Vec2 circle_pos = circle->pos;
    Vec2 clamped_pos = circle_pos;
    f32 xmin, ymin, xmax, ymax;
    box_get_xmin_ymin_xmax_ymax(box, &xmin, &ymin, &xmax, &ymax);

    // -- find the closest point from the box to the circle
    if (clamped_pos.x < xmin) clamped_pos.x = xmin;
    if (clamped_pos.x > xmax) clamped_pos.x = xmax;
    if (clamped_pos.y < ymin) clamped_pos.y = ymin;
    if (clamped_pos.y > ymax) clamped_pos.y = ymax;

    // -- if the closes point is less than the circle's radius, we're colliding
    if (vec2_distance(clamped_pos, circle_pos) < circle->radius) {
        collided = true;
        world_pos = clamped_pos;
        normal = vec2_sub(circle_pos, clamped_pos);
        if (vec2_magnitude_squared(normal) == 0) normal = vec2_up();
        vec2_normalise(&normal);
        depth = circle->radius - vec2_distance(clamped_pos, circle_pos);

        { // -- debug rendering
            if (global_physics_debug->active) {
                Vec2 normal_p1 = clamped_pos;
                Vec2 normal_p2 = vec2_mul_scalar(normal, depth);
                segl_lines_draw_arrow(&global_physics_debug->lines, normal_p1, vec2_add(clamped_pos, normal_p2));
                segl_lines_draw_cross(&global_physics_debug->lines, clamped_pos, 0.1f);
            }
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_box_box(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    SE_Box *box1 = (SE_Box*) a;
    SE_Box *box2 = (SE_Box*) b;

    f32 xmin1, ymin1, xmax1, ymax1;
    f32 xmin2, ymin2, xmax2, ymax2;
    box_get_xmin_ymin_xmax_ymax(box1, &xmin1, &ymin1, &xmax1, &ymax1);
    box_get_xmin_ymin_xmax_ymax(box2, &xmin2, &ymin2, &xmax2, &ymax2);

    Vec2 box1_points[4] = {
        {xmin1, ymin1},
        {xmin1, ymax1},
        {xmax1, ymin1},
        {xmax1, ymax1}
    };

    Vec2 box2_points[4] = {
        {xmin2, ymin2},
        {xmin2, ymax2},
        {xmax2, ymin2},
        {xmax2, ymax2}
    };

    Vec2 deepest_point1 = vec2_zero();
    Vec2 deepest_point2 = vec2_zero();

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            Vec2 point1 = box1_points[i];
            Vec2 point2 = box2_points[i];
            if (depth < vec2_distance(point1, point2)) {
                // @incomplete
            }
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

void se_phys_render_shape(SEGL_Line_Renderer *renderer, SE_Shape *shape) {
    switch (shape->type) {
        case SE_SHAPES_CIRCLE: {
            SE_Circle *circle = (SE_Circle*) shape;
            segl_lines_draw_circle(renderer, circle->pos, circle->radius, 32);
        } break;
        case SE_SHAPES_BOX: {
            SE_Box *box = (SE_Box*) shape;
            f32 xmin, xmax, ymin, ymax;
            xmin = box->x;
            xmax = box->x + box->w;
            ymin = box->y;
            ymax = box->y + box->h;
            segl_lines_draw_line_segment(renderer, (Vec2) {xmin, ymin}, (Vec2) {xmax, ymin});
            segl_lines_draw_line_segment(renderer, (Vec2) {xmax, ymin}, (Vec2) {xmax, ymax});
            segl_lines_draw_line_segment(renderer, (Vec2) {xmax, ymax}, (Vec2) {xmin, ymax});
            segl_lines_draw_line_segment(renderer, (Vec2) {xmin, ymax}, (Vec2) {xmin, ymin});
        } break;
        case SE_SHAPES_PLANE: {
            SE_Plane *p = (SE_Plane*) shape;

            Vec2 plane_pos = vec2_mul_scalar(p->normal, p->depth);
            Vec2 plane_vec = vec2_create(p->normal.y, -p->normal.x);
            // -- the line segment
            renderer->current_colour = (Vec3) {1, 0, 0};
            segl_lines_draw_line_segment(renderer,
                vec2_add(plane_pos, vec2_mul_scalar(plane_vec, -10.0f)), 
                vec2_add(plane_pos, vec2_mul_scalar(plane_vec, +10.0f)));
            // -- the normal arrow
            renderer->current_colour = (Vec3) {0.5f, 0, 0.5f};
            segl_lines_draw_line_segment(renderer,
                plane_pos,
                vec2_add(plane_pos, p->normal)
                );
            renderer->current_colour = (Vec3) {1, 1, 1};
        } break;
    }
};

void se_phys_set_shape_transform(SE_Shape *shape, Vec2 pos, Vec2 normal) {
    switch (shape->type) {
        case SE_SHAPES_CIRCLE: {
            SE_Circle *circle = (SE_Circle*) shape;
            circle->pos = pos;
        } break;
        case SE_SHAPES_BOX: {
            SE_Box *box = (SE_Box*) shape;
            box->x = pos.x;
            box->y = pos.y;
        } break;
        case SE_SHAPES_PLANE: {
            SE_Plane *plane = (SE_Plane*) shape;
            plane->normal = normal;
            plane->depth = vec2_dot(pos, normal);
        } break;
    }
}

void se_phys_get_shape_transform(SE_Shape *shape, Vec2 *pos, Vec2 *normal) {
    switch (shape->type) {
        case SE_SHAPES_CIRCLE: {
            SE_Circle *circle = (SE_Circle*) shape;
            if (pos != NULL) *pos = circle->pos;
        } break;
        case SE_SHAPES_BOX: {
            SE_Box *box = (SE_Box*) shape;
            if (pos != NULL) {
                pos->x = box->x;
                pos->y = box->y;
            }
        } break;
        case SE_SHAPES_PLANE: {
            SE_Plane *plane = (SE_Plane*) shape;
            if (normal != NULL) *normal = plane->normal;
            if (pos != NULL) {
                *pos = vec2_mul_scalar(plane->normal, plane->depth);
            }
        } break;
    }
}