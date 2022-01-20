#include "ui.h"
#include "renderer.h"
#include "input.h"

///
void ui_update_context(UI_Context *context) {
    context->mouse_pos = get_mouse_pos(&context->is_mouse_left_pressed, &context->is_mouse_right_pressed);
    // if (context->is_mouse_busy) {
    //     // if we release the mouse button, the mouse is no longer considererd busy
    //     if (context->is_mouse_right_pressed == false) context->is_mouse_busy = false;
    // }
}

///
void ui_init_theme (UI_Theme *theme) {
    theme->color_base     = (RGBA){1.f, 0.2f, 0.2f, 1.f};
    theme->color_pressed  = (RGBA){0.5f, 0.2f, 0.2f, 1.f};
    theme->color_selected = (RGBA){1.f, 0.4f, 0.4f, 1.f};
    theme->color_disabled = (RGBA){0.4f, 0.4f, 0.4f, 1.f};
    theme->color_panel_base = (RGBA){0.4f, 0.4f, 0.4f, 1.f};
    theme->color_transition_amount = 0.025f;
    theme->padding = (Rect) {2, 2, 2, 2};
}

// /// render a rearrangable rect
// void ui_render_floating_rect(UI_Context *context, Rect *rect) {
//     bool is_pressed = context->is_mouse_left_pressed;
//     Vec2i mouse_pos = context->mouse_pos;
//     f32 button_radius = 16.f;
//     Vec2i move_button = {rect->x, rect->y};
//     Vec2i resize_button = {rect->x + rect->w, rect->y + rect->h};
//     bool is_move = false, is_resize = false;
    
//     if (is_pressed) {
//         if (point_in_circle(mouse_pos, move_button, button_radius)) {
//             is_move = true;
//         }
//         if (point_in_circle(mouse_pos, resize_button, button_radius)) {
//             is_resize = true;
//         }
//     }

//     if (is_move) {
//         rect->x = mouse_pos.x;
//         rect->y = mouse_pos.y;
//     }
//     if (is_resize) {
//         rect->w = mouse_pos.x - rect->x;
//         rect->h = mouse_pos.y - rect->y;
//         rect->w = SDL_clamp(rect->w, button_radius, 9999999);
//         rect->h = SDL_clamp(rect->h, button_radius, 9999999);
//     }
//     render_rect_color(context->renderer->sdl_renderer, *rect, (RGBA) {1, 0, 0, 1});
//     set_render_draw_color_rgba(context->renderer->sdl_renderer, (RGBA) {1, 0, 0, 1});
//     render_circle(context->renderer->sdl_renderer, move_button.x, move_button.y, button_radius);
//     render_circle(context->renderer->sdl_renderer, resize_button.x, resize_button.y, button_radius);
//     reset_render_draw_color(context->renderer->sdl_renderer);
// }

/// render a panel for other widgets
void ui_begin(UI_Context *ctx, Rect *rect) {
    ctx->current_max_id = 0;
    ctx->window_rect = *rect;
    ctx->view_rect = ctx->window_rect;
    ctx->used_rect = (Rect) {rect->x, rect->y, 0, 0};
    ctx->at_x = rect->x;
    ctx->at_y = rect->y;
    ctx->at_w = 0;
    ctx->at_h = 0;

    // -- render background
    render_rect_filled_color(ctx->renderer->sdl_renderer, *rect, ctx->theme->color_panel_base);
    // -- move grab button
    ui_row(ctx, 1, 16);
    if (ui_button_grab(ctx, (Rect) {0})) {
        rect->x = ctx->mouse_pos.x - ctx->mouse_pos_grabbed_offset.x;
        rect->y = ctx->mouse_pos.y - ctx->mouse_pos_grabbed_offset.y;
    }
    // -- resize grab button
    if (ui_button_grab(ctx, (Rect) {ctx->window_rect.x + ctx->window_rect.w - 8, ctx->window_rect.y + ctx->window_rect.h - 8, 16, 16})) {
        rect->w = ctx->mouse_pos.x - ctx->mouse_pos_grabbed_offset.x - rect->x;
        rect->h = ctx->mouse_pos.y - ctx->mouse_pos_grabbed_offset.y - rect->y;
    }
}

void ui_row(UI_Context *ctx, i32 number_of_items, i32 height) {
    ctx->at_x = ctx->window_rect.x; // reset x
    ctx->at_y = ctx->window_rect.y + ctx->used_rect.h; // advance down
    ctx->at_w = ctx->window_rect.w / number_of_items;
    ctx->at_h = height;
    ctx->used_rect.h += height;
}

// /// render a panel
// void ui_panel(UI_Context *ctx, i32 number_of_items, UI_LAYOUTS layout) {
//     Rect rect = ctx->current_new_item_rect;
//     ui_context_increase_advance_by(ctx);
// }

/// render a button using context
bool ui_button(UI_Context *ctx, const char *string) {
    bool result = false;
    UI_ID id = ctx->current_max_id;
    ctx->current_max_id++;
    Rect padding = ctx->theme->padding;
    Rect rect = {
        ctx->at_x + padding.x, ctx->at_y + padding.y, ctx->at_w - padding.w, ctx->at_h - padding.h
    };

    ctx->at_x += ctx->at_w;
    // ui_context_increase_advance_by(ctx);
    
    bool mouse_up        = !ctx->is_mouse_left_pressed;
    bool mouse_down      = !mouse_up;
    bool mouse_is_inside = SDL_PointInRect(&ctx->mouse_pos, &rect);
    RGBA color = ctx->theme->color_base;

    if (ctx->active == id) {
        if (mouse_up) {
            if (ctx->hot == id) result = true; // mouse up while hovering over button
            ctx->active = -1; // we're no longer active
        }
    } else if (ctx->hot == id) {
        if (mouse_down) ctx->active = id; // we're now active
    }
    if (mouse_is_inside) {
        // if no other item is active, make us hot
        if (ctx->active == -1) ctx->hot = id;
    }
    else if (ctx->hot == id) ctx->hot = -1;

    if (ctx->hot    == id) color = ctx->theme->color_selected;
    if (ctx->active == id) color = ctx->theme->color_pressed;

    // -- base
    render_rect_filled_color(ctx->renderer->sdl_renderer, rect, color);
    // -- text
    if (string != NULL) {
        render_string(ctx->renderer, string, rect, true);
    }

    return result;
}

/// render a button using context
bool ui_button_grab(UI_Context *ctx, Rect rect) {
    bool result = false;
    UI_ID id = ctx->current_max_id;
    ctx->current_max_id++;
    if (rect.x == 0 && rect.y == 0 && rect.w == 0 && rect.h == 0) {
        Rect padding = ctx->theme->padding;
        rect = (Rect) {
            ctx->at_x + padding.x, ctx->at_y + padding.y, ctx->at_w - padding.w, ctx->at_h - padding.h
        };
    }

    ctx->at_x += ctx->at_w;
    // ui_context_increase_advance_by(ctx);
    
    bool mouse_up        = !ctx->is_mouse_left_pressed;
    bool mouse_down      = !mouse_up;
    bool mouse_is_inside = SDL_PointInRect(&ctx->mouse_pos, &rect);
    RGBA color = ctx->theme->color_base;

    if (ctx->active == id) {
        result = true; // mouse up while hovering over button
        if (mouse_up) ctx->active = -1; // we're no longer active
    } else if (ctx->hot == id) {
        if (mouse_down) {
            ctx->mouse_pos_grabbed_offset.x = ctx->mouse_pos.x - rect.x;
            ctx->mouse_pos_grabbed_offset.y = ctx->mouse_pos.y - rect.y;
            ctx->active = id; // we're now active
        }
    }
    if (mouse_is_inside) {
        // if no other item is active, make us hot
        if (ctx->active == -1) ctx->hot = id;
    }
    else if (ctx->hot == id) ctx->hot = -1;

    if (ctx->hot    == id) color = ctx->theme->color_selected;
    if (ctx->active == id) color = ctx->theme->color_pressed;

    // -- base
    render_rect_filled_color(ctx->renderer->sdl_renderer, rect, color);

    return result;
}

// /// render a label using context.
// void ui_label(UI_Context *ctx, const char *title) { // todo add back
//     Rect rect = ctx->current_new_item_rect;
//     ui_context_increase_advance_by(ctx);

//     render_string(ctx->renderer, title, rect, true);
// }

// /// Advance the UI_Context.current_new_item_rect attributes based on the latest widget rect
// void ui_context_increase_advance_by(UI_Context *ctx) {
//     if (ctx->layout == UI_LAYOUT_VERTICAL) {
//         ctx->current_new_item_rect.y += ctx->advance_by;
//     } else
//     if (ctx->layout == UI_LAYOUT_HORIZONTAL) {
//         ctx->current_new_item_rect.x += ctx->advance_by;
//     }
// }