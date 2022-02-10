#include "sephysics.h"

SE_Collision_Data se_phys_check_aabb_circle(SE_AABB *box, SE_Circle *circle) {
    SE_Collision_Data result;
    bool collided = false;
    vec2 world_pos = {0};
    vec2 normal = {0};
    f32 depth = 0;

    { // -- the actual collision check
        vec2 clamped_pos = circle->pos;
        if (clamped_pos.x < box->xmin) clamped_pos.x = box->xmin;
        if (clamped_pos.x > box->xmax) clamped_pos.x = box->xmax;
        if (clamped_pos.y < box->ymin) clamped_pos.y = box->ymin;
        if (clamped_pos.y > box->ymax) clamped_pos.y = box->ymax;

        if (vec2_distance(clamped_pos, circle->pos) < circle->radius) {
            collided = true;
            world_pos = clamped_pos;
            normal = vec2_sub(circle->pos, clamped_pos);
            vec2_normalise(&normal);
            depth = vec2_distance(clamped_pos, circle->pos);

            // -- rendering
            physics_global->lines.current_colour = (vec3) {1, 0, 0};
            segl_lines_draw_line_segment(&physics_global->lines, clamped_pos, normal);
        }

        segl_lines_draw_circle(&physics_global->lines, clamped_pos, 0.1f, 16); // @nocheckin

        // -- reset rendering colour
        if (collided) physics_global->lines.current_colour = (vec3) {1, 1, 1};
    }

    init_collision_data(&result, collided, normal, world_pos, depth, (SE_Shape*) box, (SE_Shape*) circle);
    return result;
}