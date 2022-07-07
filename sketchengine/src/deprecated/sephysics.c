#include "sephysics.h"

void render_collision_data(const SE_Collision_Data *collision_data) {
    if (!global_physics_debug->active) return;
    // world pos
    segl_lines_draw_cross(&global_physics_debug->lines,
        collision_data->world_pos, 0.1f);
    // normal
    segl_lines_draw_arrow(&global_physics_debug->lines,
        collision_data->world_pos,
        vec2_add(collision_data->world_pos, collision_data->normal));
    // normal depth
    segl_lines_draw_arrow(&global_physics_debug->lines,
        collision_data->world_pos,
        vec2_add(collision_data->world_pos,
            vec2_mul_scalar(collision_data->normal, collision_data->depth)));
}

SE_Collision_Data se_phys_check_plane_plane(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_PLANE && b->type == SE_SHAPES_PLANE);
    SE_Collision_Data result;
    b8 collided = false;
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
    SE_Collision_Data result = se_phys_check_circle_plane(b, a);
    result.normal = vec2_mul_scalar(result.normal, -1);
    // @question the thing that's happening is that, in collision depentration we're also flipping the normal, so we actually depenetrate correctly, but the issue is having the above code results in us rendering the collision normal in the wrong direction. And perhaps the collision normal is wrong and should be the other way around. Talk to Finn about this and ask for his opinion.

    // result.shape_a = b;
    // result.shape_b = a;
    return result;
}

SE_Collision_Data se_phys_check_plane_box(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_PLANE && b->type == SE_SHAPES_BOX);
    SE_Collision_Data result = se_phys_check_box_plane(b, a);
    result.normal = vec2_mul_scalar(result.normal, -1);
    // result.shape_a = b;
    // result.shape_b = a;
    return result;
}

SE_Collision_Data se_phys_check_circle_plane(SE_Shape *a, SE_Shape *b) {
    SDL_assert(a->type == SE_SHAPES_CIRCLE && b->type == SE_SHAPES_PLANE);
    SE_Collision_Data result;
    b8 collided = false;
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
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_circle_circle(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
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
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_circle_box(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result = se_phys_check_box_circle(b, a);
    // result.normal = vec2_mul_scalar(result.normal, -1);
    // result.shape_a = b;
    // result.shape_b = a;
    return result;
}

SE_Collision_Data se_phys_check_box_plane(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
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

                // Vec2 plane_vec = vec2_create(plane->normal.y, -plane->normal.x);
                // f32 np = vec2_dot(point, plane_vec);
                // Vec2 point_on_plane_vec = vec2_mul_scalar(plane->normal, np);
                // world_pos = vec2_add(point, point_on_plane_vec);
                world_pos = point;
            }
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_box_circle(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
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
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

b8 rect_overlaps_rect(Rect a, Rect b) {
    // following Ericson, C, 2004. Real-Time Collision Detection. 1.  CRC Press.
    // page 79, AABB vs AABB
    f32 t;
    if ((t = a.x - b.x) > b.w || -t > a.w) return false;
    if ((t = a.y - b.y) > b.h || -t > a.h) return false;
    return true;
}

SE_Collision_Data se_phys_check_box_box(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    SE_Box *box1 = (SE_Box*) a;
    SE_Box *box2 = (SE_Box*) b;

    f32 xmin1, ymin1, xmax1, ymax1;
    f32 xmin2, ymin2, xmax2, ymax2;
    box_get_xmin_ymin_xmax_ymax(box1, &xmin1, &ymin1, &xmax1, &ymax1);
    box_get_xmin_ymin_xmax_ymax(box2, &xmin2, &ymin2, &xmax2, &ymax2);

    {
        f32 right_penetration = xmax2 - xmin1;
        f32 down_penetration  = ymax2 - ymin1;
        f32 left_penetration  = xmax1 - xmin2;
        f32 up_penetration    = ymax1 - ymin2;

        if (right_penetration < down_penetration && right_penetration < left_penetration && right_penetration < up_penetration) {
            // the smallest is 'right'
            if (right_penetration > 0) {
                collided = true;
                // normal = vec2_right();
                normal = vec2_left();
                depth = right_penetration;
                world_pos = vec2_create(xmax1, (ymin1 + ymax2) * 0.5f);
            }
        } else if (down_penetration < left_penetration && down_penetration < up_penetration) {
            // the smallest is 'down'
            if (down_penetration > 0) {
                collided = true;
                normal = vec2_down();
                depth = down_penetration;
            }
        } else if (left_penetration < up_penetration) {
            // the smallest is 'left'
            if (left_penetration > 0) {
                collided = true;
                // normal = vec2_left();
                normal = vec2_right();
                depth = left_penetration;
                world_pos = vec2_create(xmin1, (ymin1 + ymax2) * 0.5f);
            }
        } else {
            // the smallest is 'up'
            if (up_penetration > 0) {
                collided = true;
                normal = vec2_up();
                depth = up_penetration;
            }
        }
    }

    // -- fill out the collision data and return it
    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

/// ---
/// SAT
/// ---

SE_Collision_Data se_phys_check_polygon_box(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;
    // @incomplete

    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_polygon_circle(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;
    // @incomplete

    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_polygon_plane(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;
    // @incomplete

    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_polygon_polygon(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result;
    b8 collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    SE_Polygon *polygon1 = (SE_Polygon*)a;
    SE_Polygon *polygon2 = (SE_Polygon*)b;

    // -- figure out the axis of each edge

    Vec2 axis[SE_POLYGON_POINTS_COUNT_MAX - 1];
    u32 axis_count = 0;

    for (i32 i = 0; i < polygon1->points_count - 1; --i) {
        Vec2 p1 = polygon1->points[i];
        Vec2 p2 = polygon1->points[i+1];
        axis[axis_count] = vec2_sub(p1, p2);
        axis_count++;
    }

    for (i32 i = 0; i < polygon2->points_count - 1; --i) {
        Vec2 p1 = polygon2->points[i];
        Vec2 p2 = polygon2->points[i+1];
        axis[axis_count] = vec2_sub(p1, p2);
        axis_count++;
    }

    // -- go through each axis and calculate if the shapes are not overlapping
    collided = true; // assume we're colliding unless proven otherwise
    f32 min_peneteration = 0;
    for (i32 i = 0; i < axis_count; ++i) {
        Vec2 current_axis = axis[i];
        f32 min1 = 0;
        f32 max1 = 0;
        f32 min2 = 0;
        f32 max2 = 0;

        // f32 projected_points_1[SE_POLYGON_POINTS_COUNT_MAX];
        // u32 projected_points_1_count = 0;
        for (i32 p_index = 0; p_index < polygon1->points_count; ++p_index) {
            f32 projected_point = vec2_dot(polygon1->points[p_index], current_axis);
            // projected_points_1[projected_points_1_count] = projected_point;
            // projected_points_1_count++;

            if (projected_point < min1) min1 = projected_point;
            if (projected_point > max1) max1 = projected_point;
        }

        // f32 projected_points_2[SE_POLYGON_POINTS_COUNT_MAX];
        // u32 projected_points_2_count = 0;
        for (i32 p_index = 0; p_index < polygon2->points_count; ++p_index) {
            f32 projected_point = vec2_dot(polygon2->points[p_index], current_axis);
            // projected_points_2[projected_points_2_count] = projected_point;
            // projected_points_2_count++;

            if (projected_point < min2) min2 = projected_point;
            if (projected_point > max2) max2 = projected_point;
        }

        if (!(min2 <= max1 && max2 >= min1)) {
            collided = false;
            break;
        }

        if (se_math_abs(min2 - max1) < min_peneteration) { // update min penetration
            collided = true;
            normal = vec2_create(current_axis.y, -current_axis.x);
            depth = se_math_abs(min2 - max1);
            // world_pos =
        }
    }

    init_collision_data(&result, collided, normal, world_pos, depth, a, b);
    return result;
}

SE_Collision_Data se_phys_check_plane_polygon(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result = se_phys_check_polygon_plane(b, a);
    result.normal = vec2_mul_scalar(result.normal, -1);
    return result;
}

SE_Collision_Data se_phys_check_circle_polygon(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result = se_phys_check_polygon_circle(b, a);
    result.normal = vec2_mul_scalar(result.normal, -1);
    return result;
}

SE_Collision_Data se_phys_check_box_polygon(SE_Shape *a, SE_Shape *b) {
    SE_Collision_Data result = se_phys_check_polygon_box(b, a);
    result.normal = vec2_mul_scalar(result.normal, -1);
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
            segl_lines_draw_arrow(renderer,
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