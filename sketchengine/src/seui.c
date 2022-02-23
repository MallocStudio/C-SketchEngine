#include "seui.h"
#include "seinput.h"
#include <memory.h>

/// --------
/// INTERNAL
/// --------

/// reset the context values such as rects, at_*, etc
/// this way we can do it in ui_init_context and ui_begin
/// also allows us to delay the reset in ui_begin so we can have access
/// to the infromation from the previous frame
void ui_context_reset(UI_Context *ctx, Rect *rect) {
    ctx->current_max_id = UI_ID_NULL;
    ctx->window_rect = *rect;
    ctx->view_rect = ctx->window_rect;
    ctx->min_rect = (Rect) {0, 0, 0, 0};
    ctx->prev_item_rect = (Rect) {0};
    ctx->at_x = rect->x;
    ctx->at_y = rect->y;
    ctx->at_w = 0;
    ctx->at_h = 0;
}

/// call this after rendering a grabable item becomes active
void ui_update_mouse_grab_pos(UI_Context *ctx) {
    // i32 x_offset = ctx->prev_item_rect.x - ctx->input->mouse_pressed_pos.x;
    // i32 y_offset = ctx->prev_item_rect.y - ctx->input->mouse_pressed_pos.y;
    // ctx->input->mouse_grab_offset.x = x_offset;
    // ctx->input->mouse_grab_offset.y = y_offset;

    i32 x_offset = ctx->prev_item_rect.x - ctx->input->mouse_screen_pressed_pos.x;
    i32 y_offset = ctx->prev_item_rect.y - ctx->input->mouse_screen_pressed_pos.y;
    ctx->mouse_grab_offset.x = x_offset;
    ctx->mouse_grab_offset.y = y_offset;
}

/// gives a unique id for this frame
UI_ID ui_generate_id(UI_Context *ctx) {
    ctx->current_max_id++;
    return ctx->current_max_id;
}

void clamp_rgb(RGB *rgb) {
    if (rgb->r > 1) rgb->r = 1;
    if (rgb->g > 1) rgb->g = 1;
    if (rgb->b > 1) rgb->b = 1;

    if (rgb->r < 0) rgb->r = 0;
    if (rgb->g < 0) rgb->g = 0;
    if (rgb->b < 0) rgb->b = 0;
}

/// -------------------------
/// DEFINITIONS OF THE HEADER
/// -------------------------

void ui_init_theme (UI_Theme *theme) {
    RGB primary   = (RGB) {1, 0.5f, 0.2f};
    RGB secondary = (RGB) {0.3f, 0.3f, 0.3f};
    theme->color_interactive_normal = primary;
    theme->color_interactive_hot    = (RGB) { primary.r + 0.2f, 
                                               primary.g + 0.1f, 
                                               primary.b + 0.1f};
    theme->color_interactive_active = (RGB) { primary.r - 0.2f, 
                                               primary.g - 0.1f, 
                                               primary.b - 0.1f};
    clamp_rgb(&theme->color_interactive_hot);
    clamp_rgb(&theme->color_interactive_active);
    theme->color_panel_base = secondary;
    theme->color_transition_amount = 0.025f;
    theme->padding = (Rect) {2, 2, 2, 2};
}


void ui_init_context(UI_Context *ctx, SE_Input *input) {
    // -- theme
    ctx->theme = new(UI_Theme);
    ui_init_theme(ctx->theme);
    ctx->active = UI_ID_NULL;
    ctx->hot    = UI_ID_NULL;

    // -- renderer
    segl_render_2d_init(&ctx->renderer, "Simple.vsd", "Simple.fsd");

    // -- text renderer
    setext_init(&ctx->txt_renderer, (Rect){0, 0, 0, 0}); // ! we set the projection matrix of txt_renderer in update()

    // -- set the rest of context's values to their default
    ui_context_reset(ctx, &((Rect) {0}));

    // -- input
    ctx->input = input;
}

void ui_deinit_context(UI_Context *ctx) {
    free(ctx->theme);
    segl_render_2d_deinit(&ctx->renderer);
    setext_deinit(&ctx->txt_renderer);
}

void ui_context_set_theme(UI_Context *ctx, UI_Theme *theme) {
    if (ctx->theme != NULL) free(ctx->theme);
    ctx->theme = theme;
}

void ui_update_context(UI_Context *ctx, Rect viewport) {
    // -- update ortho for txt_renderer
    // ctx->txt_renderer.shader_projection_matrix = ctx->projection_matrix; // directly set this
    setext_set_viewport(&ctx->txt_renderer, viewport);

    // -- update ortho for renderer
    Mat4 projection_matrix = mat4_ortho(viewport.x, viewport.w, viewport.y, viewport.h, 1, -1);
    segl_shader_program_use_shader(&ctx->renderer.shader_program);
    segl_shader_program_set_uniform_mat4(&ctx->renderer.shader_program, "vpMatrix", projection_matrix);
}

void ui_render(UI_Context *ctx) {
    // * note that the projection matrices are updated in update()    
    // -- render ui cursor
    segl_render_2d_rect(&ctx->renderer, (Rect) {
        ctx->input->mouse_screen_pos.x - 8, ctx->input->mouse_screen_pos.y - 8,
        16, 16
    });

    segl_render_2d_update_frame(&ctx->renderer);
    // segl_render_2d_clear(&ctx->renderer);
    
    setext_render(&ctx->txt_renderer);
}

void ui_begin(UI_Context *ctx, Rect *rect) {
    // save what happened the previous frame
    ctx->min_rect_prev_frame = ctx->min_rect;

    // -- make sure rect is at least the minimum amount it should be
    if (rect->w < ctx->min_rect.w) rect->w = ctx->min_rect.w;
    if (rect->h < ctx->min_rect.h) rect->h = ctx->min_rect.h;
    
    // reset for this frame
    ui_context_reset(ctx, rect);


    // -- render background
    // render_rect_filled_color(ctx->renderer->sdl_renderer, *rect, ctx->theme->color_panel_base);
    { // @temp fix this bull shit
        RGB c = ctx->theme->color_panel_base;
        ctx->renderer.current_colour = (RGB) {c.r, c.g, c.b};
        segl_render_2d_rect(&ctx->renderer, *rect);
        ctx->renderer.current_colour = ctx->renderer.default_colour;
    }
    // -- move grab button
    ui_row(ctx, 1, 16, 200);
    // ui_put(ctx);
    // ui_put(ctx);
    // ui_put(ctx);
    if (ui_button_grab(ctx, (Rect) {0})) {
        rect->x = ctx->input->mouse_screen_pos.x + ctx->mouse_grab_offset.x;
        rect->y = ctx->input->mouse_screen_pos.y + ctx->mouse_grab_offset.y;
    }
    // -- resize grab button
    if (ui_button_grab(ctx,
        (Rect) {ctx->window_rect.x + ctx->window_rect.w - 16, 
        ctx->window_rect.y + ctx->window_rect.h - 16, 16, 16})) {
            rect->w = ctx->input->mouse_screen_pos.x - rect->x + 8;
            rect->h = ctx->input->mouse_screen_pos.y - rect->y + 8;
            if (rect->w < ctx->min_rect_prev_frame.w) rect->w = ctx->min_rect_prev_frame.w;
            if (rect->h < ctx->min_rect_prev_frame.h) rect->h = ctx->min_rect_prev_frame.h;
    }

}

void ui_row(UI_Context *ctx, i32 number_of_items, i32 height, i32 min_width) {
    // -- setup the layout based on the given parameters
    ctx->at_x = ctx->window_rect.x;                    // reset x
    ctx->at_y = ctx->window_rect.y + ctx->min_rect.h; // advance down
    // ctx->at_y = ctx->y_advance_by; // advance down
    
    // -- update the min rect
    ctx->min_rect.h += height;
    if (ctx->min_rect.w < min_width) ctx->min_rect.w = min_width; // @incomplete what happens if we are arranging horizontally? shouldn't min_rect.w be increased like ctx->min_rect.h += height; is?

    // -- describe the size of each item within this row
    ctx->at_w = ctx->window_rect.w / number_of_items;
    ctx->at_h = height;

    // -- this layout goes horizontally to the right
    ctx->x_advance_by = ctx->at_w;
    ctx->y_advance_by = 0;
}

SEINLINE bool point_in_rect(Vec2 p, Rect r) {
    return ( (p.x >= r.x) && (p.x < (r.x + r.w)) &&
             (p.y >= r.y) && (p.y < (r.y + r.h)) ) ? true : false;
}

bool ui_button(UI_Context *ctx, const char *string) {
    bool result = false;
    UI_ID id = ui_generate_id(ctx);
    Rect padding = ctx->theme->padding;
    Rect rect = {
        ctx->at_x + padding.x, ctx->at_y + padding.y, ctx->at_w - padding.w, ctx->at_h - padding.h
    };

    ui_put(ctx);

    bool mouse_up        = !ctx->input->is_mouse_left_down;
    bool mouse_down      = !mouse_up;
    bool mouse_is_inside = point_in_rect(ctx->input->mouse_screen_pos, rect);
    RGB color = ctx->theme->color_interactive_normal;

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

        if (ctx->hot    == id) color = ctx->theme->color_interactive_hot;
        if (ctx->active == id) color = ctx->theme->color_interactive_active;
    }

    { // -- rendering
        // -- base
        ctx->renderer.current_colour = (RGB) {color.r, color.g, color.b};
        segl_render_2d_rect(&ctx->renderer, rect);
        ctx->renderer.current_colour = ctx->renderer.default_colour;
        // -- text
        if (string != NULL) {
            Vec2 string_size = setext_size_string(&ctx->txt_renderer, string);
            Vec2 centered_pos;
            centered_pos.x = (rect.w - string_size.x) * 0.5f;
            centered_pos.y = (rect.h - string_size.y) * 0.5f;
            setext_render_text(&ctx->txt_renderer, string, rect.x + centered_pos.x, rect.y + centered_pos.y, 1, (Vec3) {1, 1, 1});
        }
    }

    return result;
}

bool ui_button_grab(UI_Context *ctx, Rect rect) {
    bool result = false;
    UI_ID id = ui_generate_id(ctx);
    if (rect.x == 0 && rect.y == 0 && rect.w == 0 && rect.h == 0) {
        Rect padding = ctx->theme->padding;
        rect = (Rect) {
            ctx->at_x + padding.x, ctx->at_y + padding.y, ctx->at_w - padding.w, ctx->at_h - padding.h
        };
    }

    ui_put(ctx);
    
    bool mouse_up        = !ctx->input->is_mouse_left_down;
    bool mouse_down      = !mouse_up;
    bool mouse_is_inside = point_in_rect(ctx->input->mouse_screen_pos, rect);
    RGB color = ctx->theme->color_interactive_normal;

    { // -- input
        if (ctx->active == id) {
            result = true; // mouse up while hovering over button
            if (mouse_up) ctx->active = UI_ID_NULL; // we're no longer active
        } else if (ctx->hot == id) {
            if (mouse_down) {
                ctx->input->is_mouse_left_handled = true; // tell the input system that we've handled this mouse event
                ctx->active = id; // we're now active
                // -- update the current mouse grab offset
                ui_update_mouse_grab_pos(ctx);
            }
        }
        if (mouse_is_inside) {
            // if no other item is active, make us hot
            if (ctx->active == UI_ID_NULL) ctx->hot = id;
        }
        else if (ctx->hot == id) ctx->hot = UI_ID_NULL;

        if (ctx->hot    == id) color = ctx->theme->color_interactive_hot;
        if (ctx->active == id) color = ctx->theme->color_interactive_active;
    }

    { // -- rendering
        // base
        ctx->renderer.current_colour = (RGB) {color.r, color.g, color.b};
        segl_render_2d_rect(&ctx->renderer, rect);
        ctx->renderer.current_colour = ctx->renderer.default_colour;
    }

    return result;
}

void ui_label(UI_Context *ctx, const char *title) { // todo add back
    UI_ID id = ui_generate_id(ctx);

    Rect rect = {
        ctx->at_x, ctx->at_y, ctx->at_w, ctx->at_h
    };
    ui_put(ctx);
    // render_string(ctx->renderer, title, rect, STRING_STYLE_ALIGN_CENTER);
    // @TODO render text
    ctx->renderer.current_colour = (RGB) {1, 0, 0};
    segl_render_2d_rect(&ctx->renderer, rect);
    ctx->renderer.current_colour = ctx->renderer.default_colour;
}

void ui_put (UI_Context *ctx) {
    ctx->prev_item_rect = (Rect) {
        ctx->at_x, ctx->at_y, ctx->at_w, ctx->at_h
    };
    ctx->at_x += ctx->x_advance_by;
    ctx->at_y += ctx->y_advance_by;
}

void ui_margin(UI_Context *ctx, i32 amount) {
    // @incomplete
}