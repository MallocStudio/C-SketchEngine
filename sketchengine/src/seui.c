#include "seui.h"

///
/// RENDERER
///

void seui_renderer_init(UI_Renderer *renderer, const char *vsd, const char *fsd) {
    renderer->initialised = true;
    renderer->shape_count = 0;
    seshader_init_from(&renderer->shader, vsd, fsd);
    if (renderer->shader.loaded_successfully) {
        glGenBuffers(1, &renderer->vbo);
        glGenVertexArrays(1, &renderer->vao);
    } else {
        renderer->initialised = false;
    }
}

void seui_renderer_deinit(UI_Renderer *renderer) {
    if (renderer->initialised) {
        seshader_deinit(&renderer->shader);
        glDeleteVertexArrays(1, &renderer->vao);
        glDeleteBuffers(1, &renderer->vbo);
        renderer->initialised = false;
        renderer->shape_count = 0;
    }
}

void seui_renderer_upload(UI_Renderer *renderer) {
    // Bind our array object once here so later
    // we can just bind the array object and the
    // rest of the stuff gets bind with it.
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

    // get the data from shapes
    u32 vertex_count = 0;
    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {
        vertex_count += renderer->shapes[shape_index].vertex_count;
    }
    renderer->vertex_count = vertex_count;
    UI_Vertex *verts = malloc(sizeof(UI_Vertex) * vertex_count);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(UI_Vertex) * vertex_count, verts, GL_STATIC_DRAW);

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
}

void seui_renderer_draw(UI_Renderer *renderer) {
    glBindVertexArray(renderer->vao);
    glDrawArrays(GL_TRIANGLES, 0, renderer->vertex_count);
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
    seui_shape_add_vertex(shape, (Vec2) {rect.x         , rect.y    }, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x         , rect.y + h}, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x + rect.w, rect.y + h}, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x + rect.w, rect.y    }, colour);
}

void seui_render_rect(UI_Renderer *renderer, Rect rect, RGBA colour) {
    UI_Shape *shape = &renderer->shapes[renderer->shape_count];
    seui_shape_rect(shape, rect, colour);
    renderer->shape_count++;
}

///
/// THEME
///

// void seui_theme_init(UI_Theme *theme) {

// }

///
/// PANEL
///

///
/// CONTEXT
///