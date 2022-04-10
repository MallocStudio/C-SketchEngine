#include "seui_renderer.h"
///
/// RENDERER
///

void seui_renderer_init(UI_Renderer *renderer, const char *vsd, const char *fsd, u32 window_w, u32 window_h) {
    renderer->initialised = true;
    renderer->shape_count = 0;
    renderer->vertex_count = 0;
    seshader_init_from(&renderer->shader, vsd, fsd);
    if (renderer->shader.loaded_successfully) {
        glGenBuffers(1, &renderer->vbo);
        glGenBuffers(1, &renderer->ibo);
        glGenVertexArrays(1, &renderer->vao);

        renderer->view_projection = mat4_ortho(0, window_w, 0, window_h, -1.0f, 1000);
    } else {
        renderer->initialised = false;
    }
}

void seui_renderer_deinit(UI_Renderer *renderer) {
    if (renderer->initialised) {
        seshader_deinit(&renderer->shader);
        glDeleteVertexArrays(1, &renderer->vao);
        glDeleteBuffers(1, &renderer->vbo);
        glDeleteBuffers(1, &renderer->ibo);
        renderer->initialised = false;
        renderer->shape_count = 0;
    }
}

void seui_renderer_clear(UI_Renderer *renderer) {
    renderer->shape_count = 0;
}

void seui_renderer_upload(UI_Renderer *renderer) {
    // Bind our array object once here so later
    // we can just bind the array object and the
    // rest of the stuff gets bind with it.
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);

    // get the data from shapes
    u32 vertex_count = 0;
    // u32 index_count = 0;

    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {
        vertex_count += renderer->shapes[shape_index].vertex_count;
    }
    renderer->vertex_count = vertex_count;
    UI_Vertex *verts = malloc(sizeof(UI_Vertex) * vertex_count);
    // u32 *indices = malloc(sizeof(u32) * index_count);

    u32 vertex_index = 0;
    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {
        u32 shape_vertex_count = renderer->shapes[shape_index].vertex_count;

        for (u32 shape_vertex_index = 0; shape_vertex_index < shape_vertex_count; ++shape_vertex_index) {
            verts[vertex_index].colour = renderer->shapes[shape_index].vertices[shape_vertex_index].colour;
            verts[vertex_index].pos = renderer->shapes[shape_index].vertices[shape_vertex_index].pos;
            vertex_index++;
        }
    }

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(UI_Vertex) * vertex_count, verts,    GL_STATIC_DRAW);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, indices, GL_STATIC_DRAW);

    // -- enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, pos));
    // -- enable color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, TYPEOF_RGBA_OPENGL, GL_TRUE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, colour));

    free(verts);

    // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void seui_renderer_draw(UI_Renderer *renderer) {
    seshader_use(&renderer->shader);
    seshader_set_uniform_mat4(&renderer->shader, "projection_view", renderer->view_projection);
    glBindVertexArray(renderer->vao);
    // glDrawArrays(GL_TRIANGLES, 0, renderer->vertex_count);
    for (u32 i = 0; i < renderer->shape_count; ++i) {
        u32 shape_vertex_count = renderer->shapes[i].vertex_count;
        glDrawArrays(GL_TRIANGLE_FAN, i * shape_vertex_count, shape_vertex_count);
    }
    glBindVertexArray(0);
}

/// Add a vertex with the given pos and colour to the given shape
static void seui_shape_add_vertex(UI_Shape *shape, Vec2 pos, RGBA colour) {
    shape->vertices[shape->vertex_count].pos    = pos;
    shape->vertices[shape->vertex_count].colour = colour;
    shape->vertex_count++;
}

/// Create a rectangle
static void seui_shape_rect(UI_Shape *shape, Rect rect, RGBA colour) {
    shape->vertex_count = 0;
    seui_shape_add_vertex(shape, (Vec2) {rect.x         , rect.y         }, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x         , rect.y + rect.h}, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x + rect.w, rect.y + rect.h}, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x + rect.w, rect.y         }, colour);
}

void seui_render_rect(UI_Renderer *renderer, Rect rect, RGBA colour) {
    seui_shape_rect(&renderer->shapes[renderer->shape_count], rect, colour);
    renderer->shape_count++;
}
