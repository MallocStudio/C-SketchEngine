#include "seui_renderer.h"
#include <stdio.h>
///
/// RENDERER
///
static void debug_print_verts(UI_Vertex *verts, u32 count) {
    printf ("==verts==\n");
    for (u32 i = 0; i < count; ++i) {
        printf("vertex (%i) pos {%f, %f} colour {%i, %i, %i, %i} uv {%f, %f}\n", i, verts[i].pos.x, verts[i].pos.y, verts[i].colour.r, verts[i].colour.g, verts[i].colour.b, verts[i].colour.a, verts[i].texture_uv.x, verts[i].texture_uv.y);
    }
    printf ("==end==\n");
}

static void debug_print_indices(u32 *indices, u32 count) {
    printf ("==indices==\n");
    for (u32 i = 0; i < count; ++i) {
        printf("indices (%i) : %i\n", i, indices[i]);
    }
    printf ("==end==\n");
}

void seui_renderer_init(UI_Renderer *renderer, const char *vsd, const char *fsd, u32 window_w, u32 window_h) {
    renderer->initialised = true;
    renderer->shape_count = 0;
    renderer->vertex_count = 0;
    renderer->index_count = 0;
    seshader_init_from(&renderer->shader, vsd, fsd);
    if (renderer->shader.loaded_successfully) {
        glGenBuffers(1, &renderer->vbo);
        glGenBuffers(1, &renderer->ibo);
        glGenVertexArrays(1, &renderer->vao);

        renderer->view_projection = mat4_ortho(0, window_w, 0, window_h, -1.0f, 1000);

        setexture_atlas_load(&renderer->icons, "assets/UI/icons/ui_icons_atlas.png", 11, 10);
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
        setexture_atlas_unload(&renderer->icons);
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);

    // get the data from shapes
    u32 vertex_count = 0;
    u32 index_count = 0;

    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {
        vertex_count += renderer->shapes[shape_index].vertex_count;
        index_count  += renderer->shapes[shape_index].index_count;
    }

    renderer->vertex_count = vertex_count;
    renderer->index_count  = index_count;

    UI_Vertex *verts = malloc(sizeof(UI_Vertex) * vertex_count);
    u32     *indices = malloc(sizeof(u32)       * index_count);

    u32 vertex_index = 0;
    u32 index_index = 0;
    u32 previous_shape_vertex_count = 0;

    /* loop through every shape and copy their vertices and indices to renderer's vertex and index buffer */
    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {

        /* vertices */
        for (u32 i = 0; i < renderer->shapes[shape_index].vertex_count; ++i) {
            verts[vertex_index].colour = renderer->shapes[shape_index].vertices[i].colour;
            verts[vertex_index].pos = renderer->shapes[shape_index].vertices[i].pos;
            verts[vertex_index].texture_uv = renderer->shapes[shape_index].vertices[i].texture_uv;
            vertex_index++;
        }

        /* indices */
        for (u32 i = 0; i < renderer->shapes[shape_index].index_count; ++i) {
            indices[index_index] = renderer->shapes[shape_index].indices[i] + (shape_index) * previous_shape_vertex_count;
            index_index++;
        }

        previous_shape_vertex_count = renderer->shapes[shape_index].vertex_count;
    }

    // SDL_assert_always(vertex_index == vertex_count); // make sure we've looped through every shape
    // SDL_assert_always(index_index == index_count);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(UI_Vertex) * vertex_count, verts,    GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, indices, GL_STATIC_DRAW);

    // -- enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, pos));
    // -- enable color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, TYPEOF_RGBA_OPENGL, GL_TRUE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, colour));
    // -- enable texture_uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, texture_uv));

    // debug_print_verts(verts, vertex_count);
    // debug_print_indices(indices, index_count);

    free(verts);
    free(indices);

    // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void seui_renderer_draw(UI_Renderer *renderer) {
    seshader_use(&renderer->shader);
    seshader_set_uniform_mat4(&renderer->shader, "projection_view", renderer->view_projection);

    seshader_set_uniform_i32(&renderer->shader, "icons_texture", 0);
    setexture_atlas_bind(&renderer->icons);

    glBindVertexArray(renderer->vao);

    // the indexed way
    glDrawElements(GL_TRIANGLES, renderer->index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    setexture_atlas_unbind();
}

static bool vertex_equals(UI_Vertex v1, UI_Vertex v2) {
    return (vec2_compare(v1.pos, v2.pos, SEMATH_FLOAT_EPSILON) && rgba_compare(v1.colour, v2.colour) && vec2_compare(v1.texture_uv, v2.texture_uv, SEMATH_FLOAT_EPSILON));
}

/// Add a vertex with the given pos and colour to the given shape
static void seui_shape_add_vertex(UI_Shape *shape, Vec2 pos, RGBA colour) {
    shape->vertices[shape->vertex_count].pos      = pos;
    shape->vertices[shape->vertex_count].colour   = colour;
    shape->vertices[shape->vertex_count].texture_uv = (Vec2) {0};
    shape->vertex_count++;
}

static void seui_shape_add_vertex_textured(UI_Shape *shape, Vec2 pos, Vec2 uv) {
    shape->vertices[shape->vertex_count].pos        = pos;
    shape->vertices[shape->vertex_count].colour     = RGBA_BLACK;
    shape->vertices[shape->vertex_count].texture_uv = uv;
    shape->vertex_count++;
}

static void seui_shape_add_index(UI_Shape *shape, u32 index) {
    shape->indices[shape->index_count] = index;
    shape->index_count++;
}

/// Create a rectangle
static void seui_shape_rect(UI_Shape *shape, Rect rect, RGBA colour) {
    /* add the vertices */
    shape->vertex_count = 0;
    seui_shape_add_vertex(shape, (Vec2) {rect.x         , rect.y         }, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x         , rect.y + rect.h}, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x + rect.w, rect.y + rect.h}, colour);
    seui_shape_add_vertex(shape, (Vec2) {rect.x + rect.w, rect.y         }, colour);
    SDL_assert_always(shape->vertex_count == 4);

    /* add the indices */
    shape->index_count = 0;
    seui_shape_add_index(shape, 0);
    seui_shape_add_index(shape, 1);
    seui_shape_add_index(shape, 2);
    seui_shape_add_index(shape, 2);
    seui_shape_add_index(shape, 3);
    seui_shape_add_index(shape, 0);
    SDL_assert_always(shape->index_count == 6);
}

static void seui_shape_rect_textured(UI_Shape *shape, Rect rect, Vec2 cell_index, Vec2 cell_size) {
    Vec2 pos1, pos2, pos3, pos4;
    Vec2 uv1, uv2, uv3, uv4;
    uv1 = (Vec2) {0};
    uv2 = (Vec2) {0};
    uv3 = (Vec2) {0};
    uv4 = (Vec2) {0};
    cell_size = (Vec2) {16, 16};

    pos1 = (Vec2) {rect.x         , rect.y         };
    pos2 = (Vec2) {rect.x         , rect.y + rect.h};
    pos3 = (Vec2) {rect.x + rect.w, rect.y + rect.h};
    pos4 = (Vec2) {rect.x + rect.w, rect.y         };

    /* set uvs to the index */
    uv1 = vec2_mul(cell_index, cell_size);
    uv2 = vec2_mul(cell_index, cell_size);
    uv3 = vec2_mul(cell_index, cell_size);
    uv4 = vec2_mul(cell_index, cell_size);

    /* shape the rect uv */
    uv1.x += 0;           uv1.y += 0;
    uv2.x += 0;           uv2.y += cell_size.y;
    uv3.x += cell_size.x; uv3.y += cell_size.y;
    uv4.x += cell_size.x; uv4.y += 0;

    /* remap uvs to 0 - 1 */
    uv1.x = uv1.x / 16;
    uv1.y = uv1.y / 16;
    uv2.x = uv2.x / 16;
    uv2.y = uv2.y / 16;
    uv3.x = uv3.x / 16;
    uv3.y = uv3.y / 16;
    uv4.x = uv4.x / 16;
    uv4.y = uv4.y / 16;

    uv1.x = 0;
    uv1.y = 0;
    uv2.x = 0;
    uv2.y = 1;
    uv3.x = 0.5f;
    uv3.y = 1;
    uv4.x = 0.5f;
    uv4.y = 0;

    /* @nocheckin */
    // uv1.x = 0; uv1.y = 0;
    // uv2.x = 0; uv2.y = 1;
    // uv3.x = 1; uv3.y = 1;
    // uv4.x = 1; uv4.y = 0;

    shape->vertex_count = 0;
    seui_shape_add_vertex_textured(shape, pos1, uv1);
    seui_shape_add_vertex_textured(shape, pos2, uv2);
    seui_shape_add_vertex_textured(shape, pos3, uv3);
    seui_shape_add_vertex_textured(shape, pos4, uv4);
    SDL_assert_always(shape->vertex_count == 4);

    /* add the indices */
    shape->index_count = 0;
    seui_shape_add_index(shape, 0);
    seui_shape_add_index(shape, 1);
    seui_shape_add_index(shape, 2);
    seui_shape_add_index(shape, 2);
    seui_shape_add_index(shape, 3);
    seui_shape_add_index(shape, 0);
    SDL_assert_always(shape->index_count == 6);
}

void seui_render_rect(UI_Renderer *renderer, Rect rect, RGBA colour) {
    seui_shape_rect(&renderer->shapes[renderer->shape_count], rect, colour);
    renderer->shape_count++;
}

void seui_render_texture(UI_Renderer *renderer, Rect rect, Vec2 cell_index) {
    Vec2 cell_size;
    cell_size.x = renderer->icons.texture.width  / (f32) renderer->icons.columns;
    cell_size.y = renderer->icons.texture.height / (f32) renderer->icons.rows;
    seui_shape_rect_textured(&renderer->shapes[renderer->shape_count], rect, cell_index, cell_size);
    renderer->shape_count++;
}
