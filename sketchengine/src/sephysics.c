#include "sephysics.h"

SE_Collision_Data se_phys_check_aabb_circle(SE_AABB *box, SE_Circle *circle) {
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    // -- find the closest point from the box to the circle
    Vec2 clamped_pos = circle->pos;
    if (clamped_pos.x < box->xmin) clamped_pos.x = box->xmin;
    if (clamped_pos.x > box->xmax) clamped_pos.x = box->xmax;
    if (clamped_pos.y < box->ymin) clamped_pos.y = box->ymin;
    if (clamped_pos.y > box->ymax) clamped_pos.y = box->ymax;

    // -- if the closest point is less than the circle's radius, we're colliding
    if (vec2_distance(clamped_pos, circle->pos) < circle->radius) {
        collided = true;
        world_pos = clamped_pos;
        normal = vec2_sub(clamped_pos, circle->pos);
        vec2_normalise(&normal);
        depth = circle->radius - vec2_distance(clamped_pos, circle->pos);

        // -- debug rendering
        if (global_physics_debug->active) {
            Vec2 normal_p1 = clamped_pos;
            Vec2 normal_p2 = vec2_mul_scalar(normal, depth);
            segl_lines_draw_arrow(&global_physics_debug->lines, normal_p1, vec2_add(clamped_pos, normal_p2));
            segl_lines_draw_cross(&global_physics_debug->lines, clamped_pos, 0.1f);
        }
    }

    init_collision_data(&result, collided, normal, world_pos, depth, (SE_Shape*) box, (SE_Shape*) circle);
    return result;
}

SE_Collision_Data se_phys_check_circle_circle(SE_Circle *c1, SE_Circle *c2) {
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    // penetration
    f32 combined_radius = c1->radius + c2->radius;
    f32 distance = vec2_distance(c1->pos, c2->pos);
    // -- if distance is less than the maximum radius of these 2 circles, we're colliding
    if (distance < combined_radius) {
        collided = true;
        normal = vec2_sub(c2->pos, c1->pos);
        vec2_normalise(&normal);
        depth = combined_radius - distance;
        world_pos = vec2_add(c1->pos, vec2_mul_scalar(normal, c1->radius - depth));

        if (global_physics_debug->active) { // -- debug rendering
            // world pos
            segl_lines_draw_cross(&global_physics_debug->lines, world_pos, 0.1f);
            // penetration
            segl_lines_draw_arrow(&global_physics_debug->lines, 
                world_pos, 
                vec2_add(world_pos, vec2_mul_scalar(normal, depth)));
        }
    }

    init_collision_data(&result, collided, normal, world_pos, depth, (SE_Shape*) c1, (SE_Shape*) c2);
    return result;
}

SE_Collision_Data se_phys_check_circle_plane(SE_Circle* c, SE_Plane *p) { // @incomplete
    SE_Collision_Data result;
    bool collided = false;
    Vec2 world_pos = {0};
    Vec2 normal = {0};
    f32 depth = 0;

    // -- if the distance between the circle and the plane is less than the circle's radius, we're colliding
    f32 normal_projection = vec2_dot(c->pos, p->normal);
    Vec2 c_pos_on_plane_normal = vec2_mul_scalar(p->normal, normal_projection);
    if (normal_projection < c->radius) { // c->pos is behind the plane (considering the radius)
        collided = true;
        depth = normal_projection;
        normal = p->normal;
        world_pos = vec2_add(c_pos_on_plane_normal, c->pos);

        if (global_physics_debug->active) { // -- debug rendering
            // world pos
            segl_lines_draw_cross(&global_physics_debug->lines, world_pos, 0.1f);
            // penetration
            segl_lines_draw_arrow(&global_physics_debug->lines, 
                p->pos, 
                vec2_add(p->pos, vec2_mul_scalar(normal, depth)));
        }
    }

    init_collision_data(&result, collided, normal, world_pos, depth, (SE_Shape*) c, (SE_Shape*) p);
    return result;
}