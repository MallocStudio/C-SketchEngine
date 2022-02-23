// #include "sephysics_render.h"

// void se_render_aabb(SEGL_Line_Renderer *lines, const SE_AABB *aabb) {
//     Vec2 p1 = {aabb->xmin, aabb->ymin};
//     Vec2 p2 = {aabb->xmax, aabb->ymin};
//     Vec2 p3 = {aabb->xmax, aabb->ymax};
//     Vec2 p4 = {aabb->xmin, aabb->ymax};
//     segl_lines_draw_line_segment(lines, p1, p2);
//     segl_lines_draw_line_segment(lines, p2, p3);
//     segl_lines_draw_line_segment(lines, p3, p4);
//     segl_lines_draw_line_segment(lines, p4, p1);
// }

// void se_render_circle(SEGL_Line_Renderer *lines, const SE_Circle *circle) {
//     segl_lines_draw_circle(lines, circle->pos, circle->radius, 32);
// }