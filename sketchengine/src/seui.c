#include "seui.h"

/// -----------------------------------------
///               UTILITIES
/// -----------------------------------------

static u32 generate_ui_id(SE_UI *ctx) {
    ctx->max_id++; // we start with zero. So we increase first.
    u32 id = ctx->max_id;
    return id;
}

/// note that stay_active_on_mouse_leave is used for dragging ui items
static UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, bool stay_active_on_mouse_leave /* = false */) {
    UI_STATES result = UI_STATE_IDLE;
    SE_Input *input = ctx->input;
    bool mouse_down   = input->is_mouse_left_down;
    bool mouse_up     = !mouse_down;
    bool mouse_inside = rect_overlaps_point(rect, input->mouse_screen_pos);

    if (ctx->hot == id) { // pressing down
        if (mouse_up) { // make active
            if (ctx->hot == id) result = UI_STATE_ACTIVE; // mouse up while pressing down
            ctx->hot = SEUI_ID_NULL; // no longer hot
        }
    } else if (ctx->warm == id) { // hover
        if (mouse_down) { // make hot
            input->is_mouse_left_handled = true; // tell input that we've used up this input
            ctx->hot = id;
        }
        // else remain warm
    }

    if (mouse_inside) {
        // if no other item is hot, make us warm
        if (ctx->hot == SEUI_ID_NULL) {
            ctx->warm = id;
        }
    } else if (ctx->warm == id) {
        ctx->warm = SEUI_ID_NULL;
    } else if (ctx->hot == id && !stay_active_on_mouse_leave) {
        ctx->hot = SEUI_ID_NULL;
    }

    // figure out result
    if (result != UI_STATE_ACTIVE) {
        if (ctx->hot == id) {
            result = UI_STATE_HOT;
        } else if (ctx->warm == id) {
            result = UI_STATE_WARM;
        } else {
            result = UI_STATE_IDLE;
        }
    }
    return result;
}

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
void seui_panel_row(SEUI_Panel *panel, f32 num_of_columns) {
    panel->cursor.x = panel->config_row_left_margin;
    panel->item_height = panel->min_item_height; // reset row height
    panel->row_columns = num_of_columns;
    panel->row_item_count = 0;
    panel->row_expanded_item_width = (panel->rect.w - panel->config_row_left_margin - panel->config_row_right_margin) / num_of_columns;
}

void seui_panel_configure(SEUI_Panel *panel, Rect initial_rect, bool minimised, f32 min_item_height, i32 docked_dir /* = 0*/) {
    seui_configure_panel_reset(panel);
    panel->rect = initial_rect;
    panel->minimised = minimised;
    panel->row_columns = 1;
    panel->min_item_height = min_item_height;
    panel->is_embedded = false;

    if (docked_dir != 1 && docked_dir != 2) panel->docked_dir = 0; else panel->docked_dir = docked_dir;
}

/// Returns a rectangle that's suppose to be the rect
/// of the new item inside of the current panel.
static Rect panel_put(SEUI_Panel *panel, f32 min_width, f32 min_height, bool expand) {
    if (panel == NULL) {
        printf("ERROR: panel_put but panel was null\n");
        return (Rect) {0, 0, 32, 32};
    }

    Rect panel_rect = panel->final_rect;
    Vec2 cursor = panel->cursor; // relative space
    cursor = vec2_add(cursor, (Vec2) {panel_rect.x, panel_rect.y}); // global space

    Rect result = {0};
    {
        f32 height = min_height; // this item's height

        if (height <= 0) {
            height = panel->item_height;
        }

        if (min_height > panel->item_height) {
            panel->item_height = min_height; // update panel item height so other widgets in the same row will have expanded size
        }

        if (!expand && panel->row_item_count == 0) { // if this is the first item of the panel make it min width
            result.w = min_width;
        } else {
            result.w = panel_rect.w / panel->row_columns;
            // result.w = panel->row_expanded_item_width;
        }
        if (expand) {
            result.h = panel->item_height;
        } else {
            result.h = min_height;
        }
        result.x = cursor.x;
        result.y = cursor.y - result.h;
    }

    // increment min width
    // @incomplete panel->config_row_right_margin is not taken into account here.
    // if (panel->cursor.x > min_width) {
    //     panel->min_size.x += min_width;
    // }
    if (panel->cursor.x > panel->min_size.x) {
        panel->min_size.x = panel->cursor.x;
    }

    // Increment the cursor
    panel->cursor.x += result.w;
    if (panel->cursor.x > panel_rect.w - 1) { // -1 gets rid of floating point error while comparing
        seui_panel_row(panel, panel->row_columns);
        panel->cursor.y -= result.h;
        panel->min_size.y += result.h;
    }

    // ! note that this is just a remind to redo how we figure out the min size of a panel or figure out if
    // ! the current method is good enough
    // if (panel->cursor.x > panel->min_size.x) {
    //     panel->min_size.x = panel->cursor.x;
    // }

    // if (panel->cursor.y > panel->min_size.y) {
    //     panel->min_size.y = panel->cursor.y;
    // }

    panel->row_item_count++;
    return result;
}

bool seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {
    bool *minimised   = &panel_data->minimised;
    bool is_minimised = *minimised;
    RGBA colour = ctx->theme.colour_bg;

    panel_data->final_rect = panel_data->rect; // record previous frame's rect

    ctx->current_panel = panel_data;
    panel_data->cursor = (Vec2) {
        0, // start from the top left
        panel_data->rect.h
    };
    panel_data->item_height = panel_data->min_item_height;

    // draw a rectangle that represents the panel's dimensions
    if (!is_minimised && !panel_data->is_embedded) seui_render_rect(&ctx->renderer, panel_data->final_rect, colour);

    { // panel widgets
        f32 minimise_button_size = 32;
        seui_panel_row(panel_data, 1);
        Rect top_bar = panel_put(panel_data, panel_data->rect.w, minimise_button_size, false);

        minimise_button_size = top_bar.h;

        Vec2 cursor = vec2_add(panel_data->cursor, (Vec2) {panel_data->rect.x, panel_data->rect.y});

        UI_STATES drag_state = UI_STATE_DISABLED;
        if (panel_data->is_embedded == false) {
            /* drag button */
            Rect drag_button_rect = (Rect) {cursor.x, cursor.y, panel_data->rect.w - minimise_button_size, minimise_button_size};
            Vec2 drag = seui_drag_button_at(ctx, drag_button_rect, &drag_state);
            setext_render_text_rect(&ctx->txt_renderer, title, drag_button_rect, v3f(1, 1, 1), true);
            panel_data->rect.x += drag.x;
            panel_data->rect.y += drag.y;
        }

        /* minimise button */
        Rect minimise_button_rect = (Rect) {cursor.x + panel_data->rect.w - minimise_button_size, cursor.y, minimise_button_size, minimise_button_size};
        if (seui_button_at(ctx, "", minimise_button_rect)) {
            *minimised = !*minimised;
        }

        /* panel outline */
        seui_render_rect_outline(&ctx->renderer, panel_data->final_rect, 1, RGBA_BLACK);

        /* resizeing */
        Vec2 min_size = panel_data->min_size;
        if (!is_minimised && !panel_data->is_embedded) {

            Rect resize_button = {
                panel_data->final_rect.x + panel_data->final_rect.w- 16, panel_data->final_rect.y, 16, 16
            };
            Vec2 resize = seui_drag_button_at(ctx, resize_button, NULL);

            panel_data->rect.w += resize.x;

            if (panel_data->rect.h - resize.y > min_size.y) {
                panel_data->rect.h -= resize.y;
                panel_data->rect.y += resize.y;
            }
        }
        // clamp to min size
        if (panel_data->rect.w < min_size.x) panel_data->rect.w = min_size.x;
        if (panel_data->rect.h < min_size.y) panel_data->rect.h = min_size.y;

        // minimise button
        Vec2 index = is_minimised ? UI_ICON_INDEX_UNCOLLAPSE : UI_ICON_INDEX_COLLAPSE;
        seui_render_texture(&ctx->renderer, minimise_button_rect, index, RGBA_WHITE);

        if (!panel_data->is_embedded) { // -- docking
            RGBA dock_colour = {150, 0, 0, 100};
            Rect normalised_rect = panel_data->rect;
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
                    // printf("docked\n");
                    panel_data->docked_dir = 2;
                }
            } else
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_LEFT, normalised_cursor)) { // left
                if (drag_state == UI_STATE_HOT) {
                    seui_render_rect(&ctx->renderer, expand_view_region(ctx, SEUI_VIEW_REGION_LEFT), dock_colour);
                }
                if (drag_state == UI_STATE_ACTIVE && !ctx->input->is_mouse_left_down) { // mouse released so dock
                    // printf("docked\n");
                    panel_data->docked_dir = 1;
                }
            } else { // NOT IN DOCKING BAY
                if (panel_data->docked_dir > 0 && drag_state == UI_STATE_HOT) { // mouse is pressing so undock
                    // printf("undocked\n");
                    panel_data->docked_dir = 0;
                    // panel_data->rect.w = panel_data->min_size.x;
                    // panel_data->rect.h = panel_data->min_size.y;
                }
            }

            if (panel_data->docked_dir > 0) {
                if (panel_data->docked_dir == 1) { // left
                    normalised_rect = expand_view_region(ctx, SEUI_VIEW_REGION_LEFT);
                    panel_data->rect = normalised_rect;
                } else
                if (panel_data->docked_dir == 2) { // right
                    normalised_rect = expand_view_region(ctx, SEUI_VIEW_REGION_RIGHT);
                    panel_data->rect = normalised_rect;
                }
            }
        }
    }

    // reset min size
    panel_data->min_size = (Vec2) {0};
    panel_data->min_size.x = panel_data->row_columns * 64; // 64 is default item width
    panel_data->min_size.y = panel_data->item_height;

    return !is_minimised;
}

bool seui_panel(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {
    panel_data->is_embedded = true;
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, panel_data->rect.w, panel_data->rect.h, true);
        panel_data->rect = rect;
    }
    return seui_panel_at(ctx, title, panel_data);
}

/// -----------------------------------------
///                WIDGETS
/// -----------------------------------------

bool seui_button(SE_UI *ctx, const char *text) {
    Vec2 text_size = setext_size_string(&ctx->txt_renderer, text);
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, text_size.x, text_size.y, true);
    }
    return seui_button_at(ctx, text, rect);
}

void seui_label(SE_UI *ctx, const char *text) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        Vec2 text_size = setext_size_string(&ctx->txt_renderer, text);
        rect = panel_put(ctx->current_panel, text_size.x, text_size.y, true);
        // rect = panel_put(ctx->current_panel, 100, 32, true);
    }
    seui_label_at(ctx, text, rect);
}

void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, bool editable) {
    char label_buffer[255];
    seui_panel_row(ctx->current_panel, 1);
    seui_label(ctx, title);

    if (!editable) {
        seui_panel_row(ctx->current_panel, 3);
        sprintf(label_buffer, "x: %.2f", value->x);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "y: %.2f", value->y);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "z: %.2f", value->z);
        seui_label(ctx, label_buffer);
    } else {
        ctx->text_input_only_numerical = true;
        seui_panel_row(ctx->current_panel, 6);

        sprintf(label_buffer, "x:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%.2f", value->x);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->x = sestring_as_f32(&ctx->text_input_cache);

        sprintf(label_buffer, "y:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%.2f", value->y);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->y = sestring_as_f32(&ctx->text_input_cache);

        sprintf(label_buffer, "z:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%.2f", value->z);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->z = sestring_as_f32(&ctx->text_input_cache);

        seui_configure_text_input_reset(ctx); // reset configurations
    }
}

void seui_slider(SE_UI *ctx, f32 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 24, 24, true);
    }
    Vec2 pos1 = {rect.x, (rect.y + rect.y + rect.h) * 0.5f};
    Vec2 pos2 = {rect.x + rect.w, (rect.y + rect.y + rect.h) * 0.5f};
    seui_slider_at(ctx, pos1, pos2, value);
}

void seui_slider2d(SE_UI *ctx, Vec2 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32, true);
    }
    Vec2 center = {
        (rect.x + rect.x + rect.w) * 0.5f,
        (rect.y + rect.y + rect.h) * 0.5f
    };
    f32 radius = rect.h * 0.8f;
    seui_slider2d_at(ctx, center, radius, value);
}

void seui_colour_picker(SE_UI *ctx, RGBA *value) {
    Rect rect = {0, 0, 128, 128}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, rect.w, rect.h, true);
    }
    seui_colour_picker_at(ctx, rect, value);
}

void seui_input_text(SE_UI *ctx, SE_String *text) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32, true);
    }
    seui_input_text_at(ctx, text, rect);
}

bool seui_button_at(SE_UI *ctx, const char *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);
    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_normal;
        } break;
        case UI_STATE_WARM: {
            colour = colour_hover;
        } break;
        case UI_STATE_HOT: {
            colour = colour_pressed;
        } break;
   }

    seui_render_rect(renderer, rect, colour);
    setext_render_text_rect(&ctx->txt_renderer, text, rect, vec3_create(1, 1, 1), true);

    return ui_state == UI_STATE_ACTIVE;
}

bool seui_button_textured_at(SE_UI *ctx, Vec2 texture_index, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);

    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_normal;
        } break;
        case UI_STATE_WARM: {
            colour = colour_hover;
        } break;
        case UI_STATE_HOT: {
            colour = colour_pressed;
        } break;
    }

    if (texture_index.x == 0 && texture_index.y == 0) {
        seui_render_rect(renderer, rect, colour);
    } else {
        seui_render_texture(renderer, rect, texture_index, colour);
    }

    return ui_state == UI_STATE_ACTIVE;
}

Vec2 seui_drag_button_textured_at(SE_UI *ctx, Rect rect, Vec2 texture_index, UI_STATES *state) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    Vec2 drag = {0};
    UI_STATES ui_state = get_ui_state(ctx, id, rect, true);

    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_normal;
        } break;
        case UI_STATE_WARM: {
            colour = colour_hover;
        } break;
        case UI_STATE_HOT: {
            colour = colour_pressed;
            drag = input->mouse_screen_pos_delta;
        } break;
    }
    if (state != NULL) *state = ui_state;

    if (texture_index.x == 0 && texture_index.y == 0) {
        seui_render_rect(renderer, rect, colour);
    } else {
        seui_render_texture(renderer, rect, texture_index, colour);
    }

    return drag;
}

Vec2 seui_drag_button_at(SE_UI *ctx, Rect rect, UI_STATES *state) {
    return seui_drag_button_textured_at(ctx, rect, UI_ICON_INDEX_NULL, state);
}

void seui_label_at(SE_UI *ctx, const char *text, Rect rect) {
    RGBA colour = RGBA_WHITE;
    setext_render_text_rect(&ctx->txt_renderer, text, rect, vec3_create(1, 1, 1), true);
    seui_render_rect(&ctx->renderer, rect, (RGBA) {10, 10, 10, 100});
}

void seui_slider_at(SE_UI *ctx, Vec2 pos1, Vec2 pos2, f32 *value) {
    // clamp before doing anything
    if (*value < 0) *value = 0;
    if (*value > 1) *value = 1;

    // apply padding to pos
    f32 padding = 16;
    pos1.x += padding;
    pos2.x -= padding;

    Vec2 button_size = {24, 24};
    Vec2 pos_rel1 = vec2_zero();
    Vec2 pos_rel2 = vec2_sub(pos2, pos1);
    Vec2 pos_offset = vec2_mul_scalar(button_size, -0.5f);
    Vec2 button_pos = vec2_add(vec2_add(vec2_mul_scalar(vec2_average(pos_rel1, pos_rel2), *value * 2), pos1), pos_offset);

    /* draw the line */
    seui_render_line(&ctx->renderer, pos1, pos2, 3, RGBA_BLACK);

    /* draw the slider button */
    Rect button_rect = rect_create(button_pos, button_size);

    UI_STATES ui_state = UI_STATE_IDLE;
    Vec2 drag = seui_drag_button_textured_at(ctx, button_rect, UI_ICON_INDEX_SLIDER, &ui_state);

    if (ui_state == UI_STATE_WARM) {
        *value += ctx->input->mouse_wheel * 0.01f;
    }

    // clamp before concluding
    *value += drag.x * 0.01f;
    if (*value < 0) *value = 0;
    if (*value > 1) *value = 1;
}

void seui_slider2d_at(SE_UI *ctx, Vec2 center, f32 radius, Vec2 *value) {
    // normalise before doing anything
    vec2_normalise(value);

    Vec2 button_size = {24, 24};
    Vec2 button_pos = center;

    // value
    button_pos = vec2_add(button_pos, vec2_mul_scalar(*value, radius * 0.5f));

    // size
    button_pos = vec2_sub(button_pos, vec2_mul_scalar(button_size, 0.5f));

    /* draw the border */
    seui_render_circle(&ctx->renderer, center, radius, (RGBA) {0, 0, 0, 50});

    /* draw the button */
    Rect button_rect = rect_create(button_pos, button_size);
    Vec2 drag = seui_drag_button_textured_at(ctx, button_rect, UI_ICON_INDEX_SLIDER, NULL);

    // clamp before concluding
    // *value += drag.x * 0.01f;
    *value = vec2_add(*value, vec2_mul_scalar(drag, 0.05f));
    vec2_normalise(value);
}

/// Draws the Saturation Value Box that's used to pick those values. Updates saturation and value if the user clicks on the box
static void colour_picker_saturation_value_box(SE_UI *ctx, Rect rect, i32 hue, i32 *saturation, i32 *value) {
    Rect cursor = {0};
    cursor.w = 16;
    cursor.h = 16;
    cursor.x = (*saturation) * rect.w;
    cursor.y = (*value)      * rect.h;

    cursor.x += rect.x - cursor.w * 0.5f;
    cursor.y += rect.y - cursor.h * 0.5f;

    u32 id = generate_ui_id(ctx);
    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);

    if (ui_state == UI_STATE_ACTIVE) { // select saturation and value
        Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
        i32 window_w;
        i32 window_h;
        SDL_GetWindowSize(ctx->input->window, &window_w, &window_h);
        mouse_pos.y = window_h - mouse_pos.y;
        mouse_pos.x -= rect.x;
        mouse_pos.y -= rect.y;
        mouse_pos.x /= rect.w;
        mouse_pos.y /= rect.h;

        (*saturation) = (i32)(mouse_pos.x * 100);
        (*value) = (i32)(mouse_pos.y * 100);
        printf("sat : %i\n", *saturation);
        printf("value : %i\n", *value);
        printf("mouse_pos : %f, %f\n", mouse_pos.x, mouse_pos.y);
    }

    seui_render_colour_box(&ctx->renderer, rect, hue);
    seui_drag_button_textured_at(ctx, cursor, UI_ICON_INDEX_CIRCLE_FILLED, NULL);
}

static colour_picker_hue_box(SE_UI *ctx, Rect rect, i32 *hue) {
    // Rect cursor = {0};
    // cursor.w = 16;
    // cursor.h = 16;
    // cursor.x = (*hue) *
    // u32 id = generate_ui_id(ctx);
    // UI_STATES ui_state = get_ui_state(ctx, id, hue_box, false);

    // if (ui_state == UI_STATE_WARM) {
    //     // select hue
    //     if (ctx->input->mouse_wheel != 0) {
    //         hue += ctx->input->mouse_wheel;
    //         if (hue > 359) hue = 0;
    //         if (hue < 0) hue = 359;
    //         printf("hue: %f\n", hue);
    //     }
    // }
    i32 hue_rounded = *hue;
    f32 value = hue_rounded / 360.0f;
    // printf("hue: %f\n", *hue);
    // printf("value: %f\n", value);
    seui_slider_at(ctx, v2f(rect.x, rect.y + rect.h / 2), v2f(rect.x + rect.w, rect.y + rect.h / 2), &value);
    hue_rounded = value * 360;
    (*hue) = hue_rounded;
}

void seui_colour_picker_at(SE_UI *ctx, Rect rect, RGBA *colour) {
#if 1 // the pretty gui way is broken atm
    // colour box
    i32 hue;
    i32 saturation;
    i32 value;
    rgb_to_hsv((RGB) {colour->r, colour->g, colour->b}, &hue, &saturation, &value);

    Rect hue_box = {
        rect.x, rect.y, rect.w, 16
    };
    Rect saturation_value_box = {
        rect.x, rect.y + hue_box.h,
        rect.w, rect.h - hue_box.h
    };
    colour_picker_saturation_value_box(ctx, saturation_value_box, hue, &saturation, &value);
    colour_picker_hue_box(ctx, hue_box, &hue);
    hsv_to_rgba(hue, saturation, value, colour);
#else
    // f32 r,g,b,a;
    // r = colour->r / 255;
    // g = colour->g / 255;
    // b = colour->b / 255;
    // a = colour->a / 255;

    // f32 h = rect.h / 6;
    // f32 y = rect.y + rect.h - h;

    // seui_render_rect(&ctx->renderer, rect, *colour);

    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &r);
    // y -= h;
    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &g);
    // y -= h;
    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &b);
    // y -= h;
    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &a);

    // colour->r = r * 255;
    // colour->g = g * 255;
    // colour->b = b * 255;
    // colour->a = a * 255;
#endif
}

bool seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max) {
    u32 id = generate_ui_id(ctx);

    RGBA colour_bg = (RGBA) {10, 10, 10, 100}; // same colour as label bg
    RGBA colour_highlight = (RGBA) {80, 90, 150, 255};
    Vec3 colour_text = (Vec3) {255, 255, 255};
    RGBA colour = colour_bg;

    f32 button_size = rect.h;
    Rect label = {
        rect.x + button_size,
        rect.y,
        rect.w - button_size * 2,
        rect.h
    };
    UI_STATES ui_state = get_ui_state(ctx, id, label, false);
    switch (ui_state) {
        case UI_STATE_IDLE: {
        } break;
        case UI_STATE_WARM: {
        } break;
        case UI_STATE_ACTIVE: {
        } break;
    }

    /* background */
    seui_render_rect(&ctx->renderer, rect, colour_bg);
    /* left button */
    Rect button = {
        rect.x,
        rect.y,
        button_size,
        button_size
    };
    bool decrease = seui_button_textured_at(ctx, UI_ICON_INDEX_ARROW_LEFT, button);
    /* right button */
    button = (Rect) {
        rect.x + rect.w - button_size,
        rect.y + rect.h - button_size,
        button_size,
        button_size
    };
    bool increase = seui_button_textured_at(ctx, UI_ICON_INDEX_ARROW_RIGHT, button);

    /* value change */
    bool changed = false;
    if (increase) {
        (*value)++;
        changed = true;
    }
    if (decrease) {
        (*value)--;
        changed = true;
    }
    if (min != 0 || max != 0) {
        if (*value > max) *value = max;
        if (*value < min) *value = min;
    }

    /* display value */
    char buffer[SESTRING_MAX_NUM_OF_DIGITS];
    sprintf(buffer, "%i", *value);
    setext_render_text_rect(&ctx->txt_renderer, buffer, label, colour_text, true);

    return changed;
}

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_bg = (RGBA) {50, 60, 120, 255};
    RGBA colour_highlight = (RGBA) {80, 90, 150, 255};
    Vec3 colour_text = (Vec3) {255, 255, 255};
    Vec3 colour_text_hint = (Vec3) {100, 100, 100};
    RGBA colour = colour_bg;

    SE_String *display_text = text;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);
    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_bg;
        } break;
        case UI_STATE_WARM: {
            colour = colour_highlight;

            // allow for increasing and decreasing numerical values while hovering
            if (ctx->text_input_only_numerical) {
                f32 numerical_value = sestring_as_f32(text);
                if (ctx->input->mouse_wheel != 0) {
                    char value_as_string[SESTRING_MAX_NUM_OF_DIGITS];
                    numerical_value += ctx->input->mouse_wheel;
                    sprintf(value_as_string, "%.2f", numerical_value);
                    sestring_clear(text);
                    sestring_append(text, value_as_string);
                }
            }

        } break;
        case UI_STATE_ACTIVE: {
            if (ctx->active != id) {
                // make this the current active widget
                ctx->active = id;
                sestring_duplicate(text, &ctx->text_input); // prepare the cache for new data
                seinput_text_input_activate(input, &ctx->text_input, ctx->text_input_only_numerical);
            }
        } break;
    }

    if (ctx->active == id) {
        display_text = &ctx->text_input;
        colour = RGBA_BLACK;

        // allow for deleting characters
        if (seinput_is_key_pressed(input, SDL_SCANCODE_BACKSPACE)) {
            sestring_delete_from_end(&ctx->text_input, 1);
        }

        // clear when double clicked on
        if (ctx->input->is_mouse_left_down && rect_overlaps_point(rect, ctx->input->mouse_screen_pos)) {
            sestring_clear(&ctx->text_input);
        }

        /* deactivate accept */
        if ((seinput_is_mouse_left_pressed(ctx->input) || seinput_is_mouse_right_pressed(input)) && ui_state != UI_STATE_WARM
            || seinput_is_key_pressed(ctx->input, SDL_SCANCODE_RETURN)) {
            // copy the input text data over to text
            sestring_clear(text); // get rid of what was there and put in the new edited text
            sestring_duplicate(&ctx->text_input, text);
            ctx->active = SEUI_ID_NULL;
            seinput_text_input_deactivate(input);
        } else
        /* deactivate cancel */
        if (seinput_is_key_pressed(ctx->input, SDL_SCANCODE_ESCAPE)) {
            // DON'T copy the input text data over to text
            ctx->active = SEUI_ID_NULL;
            seinput_text_input_deactivate(input);
        }
    }

    seui_render_rect(renderer, rect, colour);
    if (display_text->size > 0) {
        setext_render_text_rect(&ctx->txt_renderer, display_text->buffer, rect, colour_text, true);
    } else if (!ctx->text_input_only_numerical) {
        setext_render_text_rect(&ctx->txt_renderer, "press to type", rect, colour_text_hint, true);
    } else {
        setext_render_text_rect(&ctx->txt_renderer, "...", rect, colour_text_hint, true);
    }
}

bool seui_button_textured(SE_UI *ctx, Vec2 texture_index) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, rect.x, rect.y, true);
    }
    return seui_button_textured_at(ctx, texture_index, rect);
}

bool seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max) {
    Rect rect = {0, 0, 100, 32}; // default
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, rect.x, rect.y, true);
    }
    return seui_selector_at(ctx, rect, value, min, max);
}

/// Colour picker

RGBA seui_colour_picker_at_hsv(SE_UI *ctx, Rect rect, i32 *h, i32 *s, i32 *v) {
    Rect h_rect = {
        rect.x, rect.y, rect.w, 16
    };
    Rect sv_rect = {
        rect.x, rect.y + h_rect.h, rect.w, rect.h - h_rect.h
    };

    u32 id = generate_ui_id(ctx);
    UI_STATES ui_state = get_ui_state(ctx, id, sv_rect, false);
    if (ui_state == UI_STATE_HOT) {
        Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
        i32 window_w, window_h;
        SDL_GetWindowSize(ctx->input->window, &window_w, &window_h);

        // mouse_pos.y = window_h - mouse_pos.y;
        // mouse_pos.x = remapf(mouse_pos.x, window_w, sv_rect.w + sv_rect.x) - sv_rect.x;
        // mouse_pos.y = remapf(mouse_pos.y, window_h, sv_rect.h + sv_rect.y) - sv_rect.y;

        // *s = remapf(mouse_pos.x, sv_rect.w, 100);
        // *v = remapf(mouse_pos.y, sv_rect.h, 100);
        mouse_pos.y = window_h - mouse_pos.y;
        mouse_pos.x -= sv_rect.x;
        mouse_pos.y -= sv_rect.y;
        mouse_pos.x /= sv_rect.w;
        mouse_pos.y /= sv_rect.h;

        (*s) = (i32)(mouse_pos.x * 100);
        (*v) = (i32)(mouse_pos.y * 100);
    }

    // draw the saturation value rect
    seui_render_colour_box(&ctx->renderer, sv_rect, *h);
    // draw the cursor on sv rect
    Vec2 cursor;
    cursor.x = remapf(*s, 100, sv_rect.w) + sv_rect.x;
    cursor.y = remapf(*v, 100, sv_rect.h) + sv_rect.y;
    seui_render_texture(&ctx->renderer, (Rect) {cursor.x - 4, cursor.y - 4, 8, 8}, UI_ICON_INDEX_CIRCLE_EMPTY, RGBA_WHITE);
    // draw the hue slider
    static f32 slider_value = 0.5f;
    seui_slider_at(ctx, v2f(h_rect.x, h_rect.y + h_rect.h / 2), v2f(h_rect.x + h_rect.w, h_rect.y + h_rect.h / 2), &slider_value);
    *h = remapf(slider_value, 1, 360);

    RGBA result;
    hsv_to_rgba(*h, *s, *v, &result);
    return result;
}

RGBA seui_colour_picker_hsv(SE_UI *ctx, i32 *h, i32 *s, i32 *v) {
    Rect rect = {0, 0, 128, 128}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, rect.w, rect.h, true);
    }
    return seui_colour_picker_at_hsv(ctx, rect, h, s, v);
}