#include "serenderer2D.h"


void serender2d_init (SE_Renderer2D *renderer, Rect viewport) {
    renderer->initialised = true;
        // init shaders
    seshader_init_from(&renderer->shader, "shaders/2D.vsd", "shaders/2D.fsd");
    seshader_init_from(&renderer->shader_textured, "shaders/2D_Textured.vsd", "shaders/2D_Textured.fsd");
        // if shaders are not loaded successfully do not continue
    if (!renderer->shader.loaded_successfully || !renderer->shader_textured.loaded_successfully) {
        renderer->initialised = false;
        return;
    }
        // set all shapes to zero
    memset(renderer->shape_lines,    0, sizeof(renderer->shape_lines));
    memset(renderer->shape_rects,    0, sizeof(renderer->shape_rects));
    memset(renderer->shape_polygons, 0, sizeof(renderer->shape_polygons));
    serender2d_clear_shapes(renderer);
        // opengl stuff
    glGenBuffers(1,      &renderer->vbo_dynamic);
    glGenVertexArrays(1, &renderer->vao_dynamic);

    glBindVertexArray(                    renderer->vao_dynamic);
    glBindBuffer(GL_ARRAY_BUFFER,         renderer->vbo_dynamic);

    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Vertex2D) * SE_SHAPE_POLYGON_VERTEX_MAX_SIZE, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex2D), (void*)offsetof(SE_Vertex2D, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex2D), (void*)offsetof(SE_Vertex2D, depth));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SE_Vertex2D), (void*)offsetof(SE_Vertex2D, colour));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex2D), (void*)offsetof(SE_Vertex2D, uv));

    glBindVertexArray(                    0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);

        // viewport
    serender2d_resize(renderer, viewport);
}

void serender2d_deinit (SE_Renderer2D *renderer) {
    if (renderer->initialised) {
        renderer->initialised = false;
            // shaders
        seshader_deinit(&renderer->shader);
        seshader_deinit(&renderer->shader_textured);
            // opengl
        glDeleteBuffers(1,      &renderer->vbo_dynamic);
        glDeleteVertexArrays(1, &renderer->vao_dynamic);
    }
}

void serender2d_resize (SE_Renderer2D *renderer, Rect viewport) {
    renderer->viewport = viewport;
    renderer->view_projection = viewport_to_ortho_projection_matrix_extra(viewport, -100, 100);
}

void serender2d_upload_to_gpu (SE_Renderer2D *renderer) {
    // Instead of this approach, I calculate the vbo and ibo one shape at a time while rendering them
}

void serender2d_clear_shapes (SE_Renderer2D *renderer) {
    renderer->shape_line_count = 0;
    renderer->shape_rect_count = 0;
    renderer->shape_textured_rect_count = 0;
    renderer->shape_polygon_count = 0;
}

void serender2d_render_uploaded_shapes (SE_Renderer2D *renderer) {
        // gl config
    glEnable(GL_BLEND);
        /// untextured shapes
    seshader_use(&renderer->shader);
    seshader_set_uniform_mat4(&renderer->shader, "view_projection", renderer->view_projection);

    glBindVertexArray(renderer->vao_dynamic);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_dynamic);
        // rects
    for (u32 i = 0; i < renderer->shape_rect_count; ++i) {
        SE_Shape_Rect shape = renderer->shape_rects[i];
        f32 x = shape.rect.x;
        f32 y = shape.rect.y;
        f32 w = shape.rect.w;
        f32 h = shape.rect.h;
            // vertices (no need for indices sense this is a quad)
        SE_Vertex2D vertices[6];
            // pos
        vertices[0].pos = v2f(x, y + h);
        vertices[1].pos = v2f(x, y);
        vertices[2].pos = v2f(x + w, y);
        vertices[3].pos = v2f(x, y + h);
        vertices[4].pos = v2f(x + w, y);
        vertices[5].pos = v2f(x + w, y + h);
            // depth
        vertices[0].depth = shape.depth;
        vertices[1].depth = shape.depth;
        vertices[2].depth = shape.depth;
        vertices[3].depth = shape.depth;
        vertices[4].depth = shape.depth;
        vertices[5].depth = shape.depth;
            // colour
        vertices[0].colour = shape.colour;
        vertices[1].colour = shape.colour;
        vertices[2].colour = shape.colour;
        vertices[3].colour = shape.colour;
        vertices[4].colour = shape.colour;
        vertices[5].colour = shape.colour;
            // update the content of vbo
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
        // lines
    for (u32 i = 0; i < renderer->shape_line_count; ++i) {
        SE_Shape_Line shape = renderer->shape_lines[i];
            // vertices
        SE_Vertex2D vertices[2];
            // pos
        vertices[0].pos = shape.pos1;
        vertices[1].pos = shape.pos2;
            // depth
        vertices[0].depth = shape.depth;
        vertices[1].depth = shape.depth;
            // colour
        vertices[0].colour = shape.colour;
        vertices[1].colour = shape.colour;
            // update the content of vbo
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glLineWidth(shape.width);
        glDrawArrays(GL_LINES, 0, 2);
        glLineWidth(1);
    }
        // polygons
    for (u32 i = 0; i < renderer->shape_polygon_count; ++i) {
        SE_Shape_Polygon shape = renderer->shape_polygons[i];
            // update the content of vbo
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SE_Vertex2D) * shape.vertex_count, shape.vertices);
        glDrawArrays(GL_TRIANGLES, 0, shape.vertex_count);
    }
        /// textured shapes
    seshader_use(&renderer->shader_textured);
    seshader_set_uniform_mat4(&renderer->shader_textured, "view_projection", renderer->view_projection);
    seshader_set_uniform_i32(&renderer->shader_textured, "diffuse", 0);

    for (u32 i = 0; i < renderer->shape_textured_rect_count; ++i) {
        SE_Shape_Textured_Rect shape = renderer->shape_textured_rects[i];
        f32 x = shape.rect_shape.rect.x;
        f32 y = shape.rect_shape.rect.y;
        f32 w = shape.rect_shape.rect.w;
        f32 h = shape.rect_shape.rect.h;
        f32 depth = shape.rect_shape.depth;
        RGBA colour = shape.rect_shape.colour;
        Vec2 uv_min = shape.uv_min;
        Vec2 uv_max = shape.uv_max;
            // vertices (no need for indices sense this is a quad)
        SE_Vertex2D vertices[6];
            // pos
        vertices[0].pos = v2f(x, y + h);
        vertices[1].pos = v2f(x, y);
        vertices[2].pos = v2f(x + w, y);
        vertices[3].pos = v2f(x, y + h);
        vertices[4].pos = v2f(x + w, y);
        vertices[5].pos = v2f(x + w, y + h);
            // depth
        vertices[0].depth = depth;
        vertices[1].depth = depth;
        vertices[2].depth = depth;
        vertices[3].depth = depth;
        vertices[4].depth = depth;
        vertices[5].depth = depth;
            // colour
        vertices[0].colour = colour;
        vertices[1].colour = colour;
        vertices[2].colour = colour;
        vertices[3].colour = colour;
        vertices[4].colour = colour;
        vertices[5].colour = colour;
            // uv
        vertices[0].uv = v2f(uv_min.x, uv_min.y);
        vertices[1].uv = v2f(uv_min.x, uv_max.y);
        vertices[2].uv = v2f(uv_max.x, uv_max.y);
        vertices[3].uv = v2f(uv_min.x, uv_min.y);
        vertices[4].uv = v2f(uv_max.x, uv_max.y);
        vertices[5].uv = v2f(uv_max.x, uv_min.y);
            // texture id
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shape.texture_id);
            // update the content of vbo
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void serender2d_add_rect (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour) {
    renderer->shape_rects[renderer->shape_rect_count].rect       = rect;
    renderer->shape_rects[renderer->shape_rect_count].depth      = depth;
    renderer->shape_rects[renderer->shape_rect_count].colour     = colour;
    renderer->shape_rect_count++;
}

void serender2d_add_line (SE_Renderer2D *renderer, Vec2 pos1, Vec2 pos2, f32 depth, RGBA colour, f32 width) {
    renderer->shape_lines[renderer->shape_line_count].pos1   = pos1;
    renderer->shape_lines[renderer->shape_line_count].pos2   = pos2;
    renderer->shape_lines[renderer->shape_line_count].depth  = depth;
    renderer->shape_lines[renderer->shape_line_count].colour = colour;
    renderer->shape_lines[renderer->shape_line_count].width  = width;
    renderer->shape_line_count++;
}

static void add_vertex(SE_Shape_Polygon *shape, Vec2 pos, f32 depth, RGBA colour) {
    shape->vertices[shape->vertex_count].pos    = pos;
    shape->vertices[shape->vertex_count].depth  = depth;
    shape->vertices[shape->vertex_count].colour = colour;
    shape->vertices[shape->vertex_count].uv     = v2f(0,0);
    shape->vertex_count++;
}

void serender2d_add_circle (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour) {
    serender2d_add_circle_outline(renderer, center, radius, depth, segment_count, colour, 3);
}

void serender2d_add_rect_textured_atlas (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA tint, const SE_Texture_Atlas *atlas, Vec2 atlas_index) {
    Vec2 uv_min = v2f(
        (atlas->texture.width / atlas->columns) * atlas_index.x / atlas->texture.width,
        (atlas->texture.height / atlas->rows)   * atlas_index.y / atlas->texture.height
    );
    Vec2 uv_max = v2f(
        (atlas->texture.width / atlas->columns) * (atlas_index.x + 1) / atlas->texture.width,
        (atlas->texture.height / atlas->rows)   * (atlas_index.y + 1) / atlas->texture.height
    );
    renderer->shape_textured_rects[renderer->shape_textured_rect_count].rect_shape.rect   = rect;
    renderer->shape_textured_rects[renderer->shape_textured_rect_count].rect_shape.depth  = depth;
    renderer->shape_textured_rects[renderer->shape_textured_rect_count].rect_shape.colour = tint;
    renderer->shape_textured_rects[renderer->shape_textured_rect_count].texture_id        = atlas->texture.id;
    renderer->shape_textured_rects[renderer->shape_textured_rect_count].uv_min            = uv_min;
    renderer->shape_textured_rects[renderer->shape_textured_rect_count].uv_max            = uv_max;
    renderer->shape_textured_rect_count++;
}

void serender2d_add_rect_outline (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour, f32 width) {
    serender2d_add_line(renderer, v2f(rect.x, rect.y), v2f(rect.x + rect.w, rect.y), depth, colour, width);
    serender2d_add_line(renderer, v2f(rect.x + rect.w, rect.y), v2f(rect.x + rect.w, rect.y + rect.h), depth, colour, width);
    serender2d_add_line(renderer, v2f(rect.x, rect.y + rect.h), v2f(rect.x + rect.w, rect.y + rect.h), depth, colour, width);
    serender2d_add_line(renderer, v2f(rect.x, rect.y), v2f(rect.x, rect.y + rect.h), depth, colour, width);
}

void serender2d_add_circle_outline (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour, f32 width) {
    f32 angle_increment_amount = SEMATH_PI_2 / segment_count;
    f32 angle = 0;
    for (u32 i = 0; i < segment_count; ++i) {
        SE_Shape_Line *shape = &renderer->shape_lines[renderer->shape_line_count];
        renderer->shape_line_count++;

        f32 x = semath_cos(angle) * radius + center.x;
        f32 y = semath_sin(angle) * radius + center.y;
        shape->pos1 = v2f(x, y);

        angle += angle_increment_amount;
        x = semath_cos(angle) * radius + center.x;
        y = semath_sin(angle) * radius + center.y;
        shape->pos2 = v2f(x, y);

        shape->depth = depth;
        shape->colour = colour;
        shape->width = width;
    }
}

void serender2d_add_hsv_wheel (SE_Renderer2D *renderer, Vec2 center, f32 inner_radius, f32 width, f32 depth) {
    const u32 segment_count = 32;
    f32 angle_increment_amount = SEMATH_PI_2 / segment_count;
    f32 angle = 0;
    for (u32 i = 0; i < segment_count; ++i) {
        f32 outer_radius = inner_radius + width;
        SE_Shape_Polygon *shape = &renderer->shape_polygons[renderer->shape_polygon_count];
        renderer->shape_polygon_count++;

        shape->vertex_count = 0;
        if (SEMATH_RAD2DEG(angle) < 0 || SEMATH_RAD2DEG(angle) > 359) angle = 0;
        Vec2 pos1, pos2, pos3, pos4; // a quad
        RGBA colour1, colour2;
        colour1.a = 255; colour2.a = 255;

            // outer
        pos1.x = semath_cos(angle) * outer_radius + center.x;
        pos1.y = semath_sin(angle) * outer_radius + center.y;
        hsv_to_rgba(SEMATH_RAD2DEG(angle), 1, 1, &colour1);
            // inner
        pos2.x = semath_cos(angle) * inner_radius + center.x;
        pos2.y = semath_sin(angle) * inner_radius + center.y;

        angle += angle_increment_amount;
        if (SEMATH_RAD2DEG(angle) < 0 || SEMATH_RAD2DEG(angle) > 359) angle = 0;

            // outer
        pos3.x = semath_cos(angle) * outer_radius + center.x;
        pos3.y = semath_sin(angle) * outer_radius + center.y;
        hsv_to_rgba(SEMATH_RAD2DEG(angle), 1, 1, &colour2);
            // inner
        pos4.x = semath_cos(angle) * inner_radius + center.x;
        pos4.y = semath_sin(angle) * inner_radius + center.y;

            // vertices
        add_vertex(shape, pos2, depth, colour1);
        add_vertex(shape, pos1, depth, colour1);
        add_vertex(shape, pos3, depth, colour2);
        add_vertex(shape, pos3, depth, colour2);
        add_vertex(shape, pos4, depth, colour2);
        add_vertex(shape, pos2, depth, colour1);
    }
}
void serender2d_add_hsv_triangle (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, f32 angle) {
    SE_Shape_Polygon *shape = &renderer->shape_polygons[renderer->shape_polygon_count];
    renderer->shape_polygon_count++;

    shape->vertex_count = 0;

    Vec2 p1 = {
        semath_cos(angle) * radius + center.x,
        semath_sin(angle) * radius + center.y,
    };
    Vec2 p2 = {
        semath_cos(angle + 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.x,
        semath_sin(angle + 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.y,
    };
    Vec2 p3 = {
        semath_cos(angle - 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.x,
        semath_sin(angle - 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.y,
    };

    RGBA colour_tip;
    RGBA colour_tip_white = RGBA_WHITE;
    RGBA colour_tip_black = RGBA_BLACK;
    colour_tip.a = 255;
    hsv_to_rgba(angle * SEMATH_RAD2DEG_MULTIPLIER, 1, 1, &colour_tip);

    add_vertex(shape, p1, depth, colour_tip);
    add_vertex(shape, p2, depth, colour_tip_white);
    add_vertex(shape, p3, depth, colour_tip_black);
}