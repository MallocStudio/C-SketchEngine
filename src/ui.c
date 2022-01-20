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
bool ui_begin(UI_Context *ctx, Rect rect) {
    bool result = true; // return true by default
    ctx->window_rect = rect;
    ctx->view_rect = ctx->window_rect;
    ctx->used_area = (Vec2i) {0};
    ctx->at_x = 0;
    ctx->at_y = 0;
    ctx->at_w = 0;
    ctx->at_h = 0;

    // -- render background
    render_rect_filled_color(ctx->renderer->sdl_renderer, rect, ctx->theme->color_panel_base);
    return result;
}

void ui_row(UI_Context *ctx, i32 number_of_items, i32 height) {
    ctx->at_x = ctx->window_rect.x; // reset x
    ctx->at_y = ctx->used_area.y; // advance down
    ctx->at_w = ctx->window_rect.w / number_of_items;
    ctx->at_h = height;
    ctx->used_area.y += height;
}

// /// render a panel
// void ui_panel(UI_Context *ctx, i32 number_of_items, UI_LAYOUTS layout) {
//     Rect rect = ctx->current_new_item_rect;
//     ui_context_increase_advance_by(ctx);
// }

/// render a button using context
bool ui_button(UI_Context *ctx, UI_ID id, const char *string) {
    bool result = false;
    
    Rect rect = {
        ctx->at_x, ctx->at_y, ctx->at_w, ctx->at_h
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