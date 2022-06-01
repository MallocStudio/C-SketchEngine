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
    renderer->shape_polygon_count = 0;
}

void serender2d_render_uploaded_shapes (SE_Renderer2D *renderer) {
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void serender2d_add_rect (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour) {
    renderer->shape_rects[renderer->shape_rect_count].rect       = rect;
    renderer->shape_rects[renderer->shape_rect_count].depth      = depth;
    renderer->shape_rects[renderer->shape_rect_count].colour     = colour;
    renderer->shape_rects[renderer->shape_rect_count].texture_id = 0;
    renderer->shape_rect_count++;
}

void serender2d_add_line (SE_Renderer2D *renderer, Vec2 pos1, Vec2 pos2, f32 depth, RGBA colour) {
    renderer->shape_lines[renderer->shape_line_count].pos1   = pos1;
    renderer->shape_lines[renderer->shape_line_count].pos2   = pos2;
    renderer->shape_lines[renderer->shape_line_count].depth  = depth;
    renderer->shape_lines[renderer->shape_line_count].colour = colour;
    renderer->shape_lines[renderer->shape_line_count].width  = 2;
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
    serender2d_add_circle_outline(renderer, center, radius, depth, segment_count, colour);
}

void serender2d_add_rect_textured_atlas (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA tint, const SE_Texture_Atlas *atlas, Vec2 atlas_index) {

}

void serender2d_add_rect_outline (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour) {

}

void serender2d_add_line_outline (SE_Renderer2D *renderer, Vec2 pos1, Vec2 pos2, f32 depth, RGBA colour) {

}

void serender2d_add_circle_outline (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour) {
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
        shape->width = 1;
    }
}
