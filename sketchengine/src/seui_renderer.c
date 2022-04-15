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
        /* filled shapes */
        glGenBuffers(1, &renderer->vbo);
        glGenBuffers(1, &renderer->ibo);
        glGenVertexArrays(1, &renderer->vao);

        /* lines */
        glGenBuffers(1, &renderer->vbo_lines);
        glGenBuffers(1, &renderer->ibo_lines);
        glGenVertexArrays(1, &renderer->vao_lines);

        renderer->view_width = window_w;
        renderer->view_height = window_h;
        renderer->view_projection = mat4_ortho(0, renderer->view_width, 0, renderer->view_height, -1.0f, 1000);

        setexture_atlas_load(&renderer->icons, "assets/UI/icons/ui_icons_atlas.png", 4, 4);
    } else {
        renderer->initialised = false;
    }
}

void seui_renderer_deinit(UI_Renderer *renderer) {
    if (renderer->initialised) {
        seshader_deinit(&renderer->shader);

        /* filled shapes */
        glDeleteVertexArrays(1, &renderer->vao);
        glDeleteBuffers     (1, &renderer->vbo);
        glDeleteBuffers     (1, &renderer->ibo);

        /* lines */
        glDeleteVertexArrays(1, &renderer->vao_lines);
        glDeleteBuffers     (1, &renderer->vbo_lines);
        glDeleteBuffers     (1, &renderer->ibo_lines);

        renderer->initialised = false;
        renderer->shape_count = 0;
        setexture_atlas_unload(&renderer->icons);
    }
}

void seui_renderer_clear(UI_Renderer *renderer) {
    renderer->shape_count = 0;
}

void seui_renderer_upload(UI_Renderer *renderer) {
    // -- get the data from shapes
    u32 vertex_count = 0;
    u32 index_count  = 0;
    u32 vertex_count_lines = 0;
    u32 index_count_lines = 0;
    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {
        if (renderer->shapes[shape_index].vertex_count == 2) {
            // lines
            vertex_count_lines += renderer->shapes[shape_index].vertex_count;
            index_count_lines  += renderer->shapes[shape_index].index_count;
        } else {
            // filled shapes
            vertex_count += renderer->shapes[shape_index].vertex_count;
            index_count  += renderer->shapes[shape_index].index_count;
        }
    }

    renderer->vertex_count = vertex_count;
    renderer->index_count  = index_count;

    renderer->vertex_count_lines = vertex_count_lines;
    renderer->index_count_lines  = index_count_lines;

    UI_Vertex *verts = malloc(sizeof(UI_Vertex) * vertex_count);
    u32     *indices = malloc(sizeof(u32)       * index_count);
    UI_Vertex *verts_lines = malloc(sizeof(UI_Vertex) * vertex_count_lines);
    u32     *indices_lines = malloc(sizeof(u32)       * index_count_lines);

    u32 vertex_index = 0;
    u32 index_index = 0;
    u32 vertex_index_lines = 0;
    u32 index_index_lines = 0;

    u32 shape_index_filled = 0; // used to calculate the offset of vertices
    u32 previous_shape_vertex_count = 0;
    u32 shape_index_lines = 0;
    u32 previous_shape_vertex_count_lines = 0;

    /* loop through every shape and copy their vertices and indices to renderer's vertex and index buffer */
    for (u32 shape_index = 0; shape_index < renderer->shape_count; ++shape_index) {
        /* vertices */
        for (u32 i = 0; i < renderer->shapes[shape_index].vertex_count; ++i) {
            if (renderer->shapes[shape_index].vertex_count == 2) {
                // lines
                verts_lines[vertex_index_lines].colour = renderer->shapes[shape_index].vertices[i].colour;
                verts_lines[vertex_index_lines].pos = renderer->shapes[shape_index].vertices[i].pos;
                verts_lines[vertex_index_lines].texture_uv = renderer->shapes[shape_index].vertices[i].texture_uv;
                vertex_index_lines++;
            } else {
                // filled shapes
                verts[vertex_index].colour = renderer->shapes[shape_index].vertices[i].colour;
                verts[vertex_index].pos = renderer->shapes[shape_index].vertices[i].pos;
                verts[vertex_index].texture_uv = renderer->shapes[shape_index].vertices[i].texture_uv;
                vertex_index++;
            }
        }

        /* indices */
        for (u32 i = 0; i < renderer->shapes[shape_index].index_count; ++i) {
            if (renderer->shapes[shape_index].vertex_count == 2) {
                // lines
                indices_lines[index_index_lines] = renderer->shapes[shape_index].indices[i] + (shape_index_lines) * previous_shape_vertex_count_lines;
                index_index_lines++;
            } else {
                // filled shapes
                indices[index_index] = renderer->shapes[shape_index].indices[i] + (shape_index_filled) * previous_shape_vertex_count;
                index_index++;
            }
        }

        if (renderer->shapes[shape_index].vertex_count == 2) {
            // lines
            previous_shape_vertex_count_lines = renderer->shapes[shape_index].vertex_count;
            shape_index_lines++;
        } else {
            // filled shapes
            previous_shape_vertex_count = renderer->shapes[shape_index].vertex_count;
            shape_index_filled++;
        }
    }

    // SDL_assert_always(vertex_index == vertex_count); // make sure we've looped through every shape
    // SDL_assert_always(index_index == index_count);


    // debug_print_verts(verts, vertex_count);
    // debug_print_indices(indices, index_count);
    { // -- filled shapes
        // Bind our array object once here so later
        // we can just bind the array object and the
        // rest of the stuff gets bind with it.
        glBindVertexArray(renderer->vao);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);


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

        // unselect
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    { // -- lines
        // Bind our array object once here
        glBindVertexArray(renderer->vao_lines);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_lines);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo_lines);

        // fill data
        glBufferData(GL_ARRAY_BUFFER, sizeof(UI_Vertex) *  vertex_count_lines,   verts_lines, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count_lines, indices_lines, GL_STATIC_DRAW);

        // -- enable position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, pos));
        // -- enable color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, TYPEOF_RGBA_OPENGL, GL_TRUE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, colour));
        // -- enable texture_uv
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(UI_Vertex), (void*)offsetof(UI_Vertex, texture_uv));

        // unselect
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    free(verts);
    free(indices);
    free(verts_lines);
    free(indices_lines);
}

void seui_renderer_draw(UI_Renderer *renderer) {
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    seshader_use(&renderer->shader);
    seshader_set_uniform_mat4(&renderer->shader, "projection_view", renderer->view_projection);

    seshader_set_uniform_i32(&renderer->shader, "icons_texture", 0);
    setexture_atlas_bind(&renderer->icons);

    { // -- lines
        glBindVertexArray(renderer->vao_lines);
        glDrawElements(GL_LINES, renderer->index_count_lines, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    { // -- filled shapes
        glBindVertexArray(renderer->vao);
        glDrawElements(GL_TRIANGLES, renderer->index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    setexture_atlas_unbind();
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
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

static void seui_shape_add_vertex_textured(UI_Shape *shape, Vec2 pos, Vec2 uv, RGBA tint) {
    shape->vertices[shape->vertex_count].pos        = pos;
    shape->vertices[shape->vertex_count].colour     = tint;
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

static void seui_shape_rect_textured(UI_Shape *shape, Rect rect, Vec2 cell_index, Vec2 cell_size, Vec2 texture_size, RGBA tint) {
    Vec2 pos1, pos2, pos3, pos4;
    Vec2 uv1, uv2, uv3, uv4;
    uv1 = (Vec2) {0};
    uv2 = (Vec2) {0};
    uv3 = (Vec2) {0};
    uv4 = (Vec2) {0};

    pos1 = (Vec2) {rect.x         , rect.y         };
    pos2 = (Vec2) {rect.x         , rect.y + rect.h};
    pos3 = (Vec2) {rect.x + rect.w, rect.y + rect.h};
    pos4 = (Vec2) {rect.x + rect.w, rect.y         };

    /* calculate uvs based on cell size and index */
    Vec2 pixel_pos1 = vec2_mul(cell_size, cell_index); // a pixel pos on texture (not 0 - 1)
    // (in case of index {1, 0} -> pixel_pos1 = {cell_size.x, 0}
         pixel_pos1 = vec2_add(pixel_pos1, (Vec2) {0, 0});
    Vec2 pixel_pos2 = vec2_add(pixel_pos1, (Vec2) {0, cell_size.x});
    Vec2 pixel_pos3 = vec2_add(pixel_pos1, (Vec2) {cell_size.x, cell_size.y});
    Vec2 pixel_pos4 = vec2_add(pixel_pos1, (Vec2) {cell_size.x, 0});

    uv1 = vec2_div(pixel_pos2, texture_size);
    uv2 = vec2_div(pixel_pos1, texture_size);
    uv3 = vec2_div(pixel_pos4, texture_size);
    uv4 = vec2_div(pixel_pos3, texture_size);

    shape->vertex_count = 0;
    seui_shape_add_vertex_textured(shape, pos1, uv1, tint);
    seui_shape_add_vertex_textured(shape, pos2, uv2, tint);
    seui_shape_add_vertex_textured(shape, pos3, uv3, tint);
    seui_shape_add_vertex_textured(shape, pos4, uv4, tint);
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

static void seui_shape_circle(UI_Shape *shape, Vec2 center, f32 radius, RGBA colour, Vec2 cell_size, Vec2 texture_size) {
    Vec2 cell_index = UI_ICON_INDEX_CIRCLE_FILLED;

    Vec2 pos1, pos2, pos3, pos4;
    Vec2 uv1, uv2, uv3, uv4;
    uv1 = (Vec2) {0};
    uv2 = (Vec2) {0};
    uv3 = (Vec2) {0};
    uv4 = (Vec2) {0};

    pos1 = (Vec2) {center.x - radius * 0.5f, center.y - radius * 0.5f};
    pos2 = (Vec2) {center.x - radius * 0.5f, center.y + radius * 0.5f};
    pos3 = (Vec2) {center.x + radius * 0.5f, center.y + radius * 0.5f};
    pos4 = (Vec2) {center.x + radius * 0.5f, center.y - radius * 0.5f};

    /* calculate uvs based on cell size and index */
    Vec2 pixel_pos1 = vec2_mul(cell_size, cell_index); // a pixel pos on texture (not 0 - 1)
    // (in case of index {1, 0} -> pixel_pos1 = {cell_size.x, 0}
         pixel_pos1 = vec2_add(pixel_pos1, (Vec2) {0, 0});
    Vec2 pixel_pos2 = vec2_add(pixel_pos1, (Vec2) {0, cell_size.x});
    Vec2 pixel_pos3 = vec2_add(pixel_pos1, (Vec2) {cell_size.x, cell_size.y});
    Vec2 pixel_pos4 = vec2_add(pixel_pos1, (Vec2) {cell_size.x, 0});

    uv1 = vec2_div(pixel_pos2, texture_size);
    uv2 = vec2_div(pixel_pos1, texture_size);
    uv3 = vec2_div(pixel_pos4, texture_size);
    uv4 = vec2_div(pixel_pos3, texture_size);

    shape->vertex_count = 0;
    seui_shape_add_vertex_textured(shape, pos1, uv1, colour);
    seui_shape_add_vertex_textured(shape, pos2, uv2, colour);
    seui_shape_add_vertex_textured(shape, pos3, uv3, colour);
    seui_shape_add_vertex_textured(shape, pos4, uv4, colour);
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

void static seui_shape_line(UI_Shape *shape, Vec2 pos1, Vec2 pos2, f32 width) {
    RGBA colour = RGBA_WHITE;
    /* vertices */
    shape->vertex_count = 0;

    seui_shape_add_vertex(shape, pos1, colour);
    seui_shape_add_vertex(shape, pos2, colour);

    /* indices */
    shape->index_count = 0;
    seui_shape_add_index(shape, 0);
    seui_shape_add_index(shape, 1);
}

void seui_render_rect(UI_Renderer *renderer, Rect rect, RGBA colour) {
    seui_shape_rect(&renderer->shapes[renderer->shape_count], rect, colour);
    renderer->shape_count++;
}

void seui_render_texture(UI_Renderer *renderer, Rect rect, Vec2 cell_index, RGBA tint) {
    Vec2 texture_size = {
        renderer->icons.texture.width,
        renderer->icons.texture.height,
    };
    Vec2 cell_size = {
        texture_size.x / renderer->icons.columns,
        texture_size.y / renderer->icons.rows,
    };
    seui_shape_rect_textured(&renderer->shapes[renderer->shape_count], rect, cell_index, cell_size, texture_size, tint);
    renderer->shape_count++;
}

void seui_render_line(UI_Renderer *renderer, Vec2 pos1, Vec2 pos2, f32 width) {
    seui_shape_line(&renderer->shapes[renderer->shape_count], pos1, pos2, width);
    renderer->shape_count++;
}

void seui_render_circle(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour) {
    Vec2 texture_size = {
        renderer->icons.texture.width,
        renderer->icons.texture.height,
    };
    Vec2 cell_size = {
        texture_size.x / renderer->icons.columns,
        texture_size.y / renderer->icons.rows,
    };
    seui_shape_circle(&renderer->shapes[renderer->shape_count], center, radius, colour, cell_size, texture_size);
    renderer->shape_count++;
}