#if 0
#include "seui.h"

/// ----------
/// File scope
/// ----------

/// Update ctx cursor position for placing items
static void seui_put (UI_Context *ctx) {
    ctx->prev_item_rect = (Rect) {
        ctx->at_x, ctx->at_y, ctx->at_w, ctx->at_h
    };
    ctx->at_x += ctx->x_advance_by;
    ctx->at_y += ctx->y_advance_by;
}

/// gives a unique id for this frame
static UI_ID ui_generate_id(UI_Context *ctx) {
    ctx->current_max_id++;
    return ctx->current_max_id;
}

/// reset the context values such as rects, at_*, etc
/// this way we can do it in ui_init_context and ui_begin
/// also allows us to delay the reset in ui_begin so we can have access
/// to the infromation from the previous frame
static void seui_context_reset(UI_Context *ctx, Rect rect) {
    ctx->current_panel = NULL;
    ctx->panel_count = 0;
    seui_renderer_clear(&ctx->renderer);

    ctx->current_max_id = UI_ID_NULL;
    ctx->view_rect = rect;
    ctx->current_panel->min_rect = (Rect) {0, 0, 0, 0};
    ctx->current_panel->prev_item_rect = (Rect) {0};
    ctx->current_panel->at_x = rect.x;
    ctx->current_panel->at_y = rect.y;
    ctx->current_panel->at_w = 0;
    ctx->current_panel->at_h = 0;
}

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

///
/// THEME
///

void seui_theme_init(UI_Theme *theme) {
    theme->colour_primary   = (RGBA) {150, 60, 80, 255};
    theme->colour_bg        = (RGBA) {15, 60, 80, 255};
    theme->colour_secondary = (RGBA) {40, 20, 60, 255};
}

///
/// WIDGETS
///

void seui_panel_begin(SEUI_Context *ctx, Rect initial_size) {
    // -- initialise a new panel
    ctx->current_panel = &ctx->panels[ctx->panel_count];
    ctx->panel_count++;

    ctx->current_panel->rect = initial_size;
    ctx->current_panel->theme = &ctx->theme;

    // -- border
    seui_render_rect(&ctx->renderer, ctx->current_panel->rect, ctx->theme.colour_bg);
}

bool seui_button_grab(SEUI_Context *ctx, Vec2 pos) {
    bool result = false;
    UI_ID id = ui_generate_id(ctx);
    Rect rect = {
        pos.x, pos.y, 16.0f, 16.0f
    }

    seui_put(ctx);

    bool mouse_up        = !ctx->input->is_mouse_left_down;
    bool mouse_down      = !mouse_up;
    bool mouse_is_inside = point_in_rect(ctx->input->mouse_screen_pos, rect);
    RGBA colour = ctx->theme.colour_secondary;

    { // -- input
        if (ctx->active == id) {
            if (mouse_up) {
                if (ctx->hot == id) result = true; // mouse up while hovering over button
                ctx->active = UI_ID_NULL; // we're no longer active
            }
        } else if (ctx->hot == id) {
            if (mouse_down) {
                ctx->input->is_mouse_left_handled = true; // tell the input system that we've handled this mouse event
                ctx->active = id; // we're now active
            }
        }
        if (mouse_is_inside) {
            // if no other item is active, make us hot
            if (ctx->active == UI_ID_NULL) ctx->hot = id;
        }
        else if (ctx->hot == id) ctx->hot = UI_ID_NULL;

        if (ctx->hot    == id) colour = ctx->theme.colour_hot;
        if (ctx->active == id) colour = ctx->theme.colour_active;
    }

    { // -- rendering
        // -- base
        seui_render_rect(&ctx->renderer, rect, colour);
        /*
        // -- text
        if (string != NULL) {
            Vec2 string_size = setext_size_string(&ctx->txt_renderer, string);
            Vec2 centered_pos;
            centered_pos.x = (rect.w - string_size.x) * 0.5f;
            centered_pos.y = (rect.h - string_size.y) * 0.5f;
            setext_render_text(&ctx->txt_renderer, string, rect.x + centered_pos.x, rect.y + centered_pos.y, 1, (Vec3) {1, 1, 1});
        }
        */
    }

    return result;
}

///
/// CONTEXT
///

void seui_init(SEUI_Context *ctx, Rect viewport, const char *vsd, const char *fsd, SE_Input *input) {
    seui_context_reset(ctx, viewport);

    // -- renderer
    seui_renderer_init(&ctx->renderer, vsd, fsd, viewport.w, viewport.h);

    // -- panels
    ctx->panel_count = 0;
    memset(ctx->panels, 0, sizeof(ctx->panels));

    // -- input
    ctx->input = input;

    seui_theme_init(&ctx->theme);
}

void seui_deinit(SEUI_Context *ctx) {
    seui_renderer_deinit(&ctx->renderer);
}

void seui_begin(SEUI_Context *ctx) {
    seui_context_reset(ctx, ctx->view_rect);
}

void seui_render(SEUI_Context *ctx) {
    seui_renderer_upload(&ctx->renderer);
    seui_renderer_draw(&ctx->renderer);
}

#endif