#include "seui_panel.h"
#include "seui_components.h"

static Rect expand_view_region(SE_UI *ctx, Rect normalised_rect) {
    Rect result = normalised_rect;
    result.x *= ctx->renderer.view_width;
    result.w *= ctx->renderer.view_width;
    result.y *= ctx->renderer.view_height;
    result.h *= ctx->renderer.view_height;
    return result;
}


/// -----------------------------------------
///                 PANEL
/// -----------------------------------------

/// Make a row
void seui_panel_row(SEUI_Panel *panel, u32 columns) {
    /* reset for the new row */
    panel->cursor.x = 0;
    panel->cursor.y -= panel->row_height;

    panel->fit_cursor.x = 0;
    panel->fit_cursor.y -= panel->row_height; // sense row_height is shrunk to min height
    panel->fit_size.y += panel->row_height;

    // panel->row_width = panel->row_width;     // keep the same width as the previous row
    panel->row_height = panel->min_item_height; // reset row height
    panel->row_width  = panel->cached_rect.w;
    panel->row_columns = columns;

    panel->next_item_height = panel->row_height;
}

void seui_panel_setup(SEUI_Panel *panel, Rect initial_rect, Vec2 min_size, bool minimised, f32 min_item_height, i32 docked_dir /* = 0*/) {
    seui_configure_panel_reset(panel);
    panel->calc_rect = initial_rect;
    panel->cached_rect = initial_rect;
    panel->minimised = minimised;
    panel->min_item_height = min_item_height;
    panel->is_embedded = false;

    if (docked_dir != 1 && docked_dir != 2) panel->docked_dir = 0; else panel->docked_dir = docked_dir;

    panel->row_height = panel->min_item_height; // default row height
    panel->row_width  = panel->min_item_height; // default row width
    panel->min_size = min_size;
}

/// Returns a rectangle that's suppose to be the rect
/// of the new item inside of the current panel.
Rect panel_put(SEUI_Panel *panel, f32 min_width, f32 min_height, bool expand) { // @remove expand parameter
    if (panel == NULL) {
        printf("ERROR: panel_put but panel was null\n");
        return (Rect) {0, 0, 32, 32};
    }
    // /* calculate the item rect based on panel cursor */
    // f32 margin = 16;
    // min_width += margin;

    // Rect item;
    // item.x = panel->cursor.x + panel->cached_rect.x; // it's ok if we lag behind one frame
    // item.y = panel->cursor.y + panel->cached_rect.y; // it's ok if we lag behind one frame
    // item.h = panel->next_item_height;
    // item.w = min_width;

    // /* advance cursor based on layout */
    // panel->cursor.x += item.w;
    // if (panel->cursor.x > panel->row_width) {
    //     panel->row_width = panel->cursor.x; // expand row width
    // }

    // if (min_height > panel->row_height) {
    //     // if we suddenly get an item whose height is larger than the current row height,
    //     // increase row height and min_size.y because we increase min_size.y based on the last row's height
    //     f32 difference = min_height - panel->row_height;
    //     panel->row_height += difference;     // expand row height
    //     panel->fit_size.y += difference;
    // }

    // /* update panel fit size */
    // panel->fit_cursor.x += min_width;
    // if (panel->fit_size.x < panel->fit_cursor.x) {
    //     panel->fit_size.x = panel->fit_cursor.x;
    // }
    // return item;
    Rect item;
    item.x = panel->cursor.x + panel->cached_rect.x;
    item.y = panel->cursor.y + panel->cached_rect.y;
    item.h = panel->next_item_height;

    item.w = panel->row_width / panel->row_columns;
    // if (item.w < min_width) item.w = min_width;

    /* advance the cursor based on row layout */
    // assuming default layout (to be changed)
    panel->cursor.x += item.w;
    if (panel->cursor.x > panel->row_width) {
        seui_panel_row(panel, panel->row_columns); // make a new row
    }

    /* update panel fit size */
    panel->fit_cursor.x += min_width;
    if (panel->fit_size.x < panel->fit_cursor.x) {
        panel->fit_size.x = panel->fit_cursor.x;
    }
    return item;
}

bool seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {
    ctx->current_panel = panel_data;
    bool *minimised   = &panel_data->minimised;
    bool is_minimised = *minimised;
    RGBA colour = ctx->theme.colour_bg;

    /* record previous frame's data */
    panel_data->cached_rect = panel_data->calc_rect; // record previous frame's rect
    /* reset panel for calculation again */
    panel_data->cursor = (Vec2) {
        0, // start from the top left
        panel_data->calc_rect.h
    };
    panel_data->fit_size = v2f(0, 0);
    panel_data->fit_cursor = panel_data->cursor;
    panel_data->next_item_height = panel_data->min_item_height;

    // draw a rectangle that represents the panel's dimensions
    if (!is_minimised && !panel_data->is_embedded) seui_render_rect(&ctx->renderer, panel_data->cached_rect, colour);

    { // panel widgets
        f32 minimise_button_size = panel_data->row_height;

        seui_panel_row(panel_data, 1); // make space for top bar

        Rect top_bar = panel_put(panel_data, panel_data->calc_rect.w, minimise_button_size, false);

        minimise_button_size = top_bar.h;

        Vec2 cursor = {
            top_bar.x, top_bar.y
        };

        UI_STATES drag_state = UI_STATE_DISABLED;
        if (panel_data->is_embedded == false) {
            /* drag button */
            Rect drag_button_rect = (Rect) {cursor.x, cursor.y, panel_data->calc_rect.w - minimise_button_size, minimise_button_size};
            Vec2 drag = seui_drag_button_at(ctx, drag_button_rect, &drag_state);
            setext_render_text_rect(&ctx->txt_renderer, title, drag_button_rect, v3f(1, 1, 1), true);
            panel_data->calc_rect.x += drag.x;
            panel_data->calc_rect.y += drag.y;
        }

        /* minimise button */
        Rect minimise_button_rect = (Rect) {cursor.x + panel_data->calc_rect.w - minimise_button_size, cursor.y, minimise_button_size, minimise_button_size};
        if (seui_button_at(ctx, "", minimise_button_rect)) {
            *minimised = !*minimised;
        }

        /* panel outline */
        seui_render_rect_outline(&ctx->renderer, panel_data->cached_rect, 1, RGBA_BLACK);

        /* resizeing */
        Vec2 min_size = panel_data->min_size;
        // Vec2 min_size = panel_data->fit_size; // @TODO fix this situation and // @remove  panel->min_size
        if (!is_minimised && !panel_data->is_embedded) {

            Rect resize_button = {
                panel_data->cached_rect.x + panel_data->cached_rect.w- 16, panel_data->cached_rect.y, 16, 16
            };
            Vec2 resize = seui_drag_button_at(ctx, resize_button, NULL);

            panel_data->calc_rect.w += resize.x;

            if (panel_data->calc_rect.h - resize.y > min_size.y) {
                panel_data->calc_rect.h -= resize.y;
                panel_data->calc_rect.y += resize.y;
            }
        }
        // clamp to min size
        if (panel_data->calc_rect.w < min_size.x) panel_data->calc_rect.w = min_size.x;
        if (panel_data->calc_rect.h < min_size.y) panel_data->calc_rect.h = min_size.y;

        // minimise button
        Vec2 index = is_minimised ? UI_ICON_INDEX_UNCOLLAPSE : UI_ICON_INDEX_COLLAPSE;
        seui_render_texture(&ctx->renderer, minimise_button_rect, index, RGBA_WHITE);

        if (!panel_data->is_embedded) { // -- docking
            RGBA dock_colour = {150, 0, 0, 100};
            Rect normalised_rect = panel_data->calc_rect;
            normalised_rect.x /= ctx->renderer.view_width;
            normalised_rect.w /= ctx->renderer.view_width;
            normalised_rect.y /= ctx->renderer.view_height;
            normalised_rect.h /= ctx->renderer.view_height;

            Vec2 normalised_cursor = ctx->input->mouse_screen_pos;
            normalised_cursor.x /= ctx->renderer.view_width;
            normalised_cursor.y /= ctx->renderer.view_height;

            // @note UI_STATE_ACTIVE means that the button was just released
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_RIGHT, normalised_cursor)) { // right
                if (drag_state == UI_STATE_HOT) {
                    seui_render_rect(&ctx->renderer, expand_view_region(ctx, SEUI_VIEW_REGION_RIGHT), dock_colour);
                }
                if (drag_state == UI_STATE_ACTIVE && !ctx->input->is_mouse_left_down) { // mouse released so dock
                    panel_data->docked_dir = 2;
                }
            } else
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_LEFT, normalised_cursor)) { // left
                if (drag_state == UI_STATE_HOT) {
                    seui_render_rect(&ctx->renderer, expand_view_region(ctx, SEUI_VIEW_REGION_LEFT), dock_colour);
                }
                if (drag_state == UI_STATE_ACTIVE && !ctx->input->is_mouse_left_down) { // mouse released so dock
                    panel_data->docked_dir = 1;
                }
            } else { // NOT IN DOCKING BAY
                if (panel_data->docked_dir > 0 && drag_state == UI_STATE_HOT) { // mouse is pressing so undock
                    panel_data->docked_dir = 0;
                }
            }

            if (panel_data->docked_dir > 0) {
                if (panel_data->docked_dir == 1) { // left
                    normalised_rect = expand_view_region(ctx, SEUI_VIEW_REGION_LEFT);
                    panel_data->calc_rect = normalised_rect;
                } else
                if (panel_data->docked_dir == 2) { // right
                    normalised_rect = expand_view_region(ctx, SEUI_VIEW_REGION_RIGHT);
                    panel_data->calc_rect = normalised_rect;
                }
            }
        }
    }

    return !is_minimised;
}

bool seui_panel(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {
    panel_data->is_embedded = true;
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, panel_data->calc_rect.w, panel_data->calc_rect.h, true);
        panel_data->calc_rect = rect;
    }
    return seui_panel_at(ctx, title, panel_data);
}