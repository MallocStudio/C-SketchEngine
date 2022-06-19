#include "seui_ctx.h"

/// -----------------------------------------
///               UTILITIES
/// -----------------------------------------

static f32 get_depth_middleground(SE_UI *ctx) {
    if (ctx->current_panel != NULL) return ctx->current_panel->depth_mg;
    return ctx->max_depth_available-1;
}
static f32 get_depth_background(SE_UI *ctx) {
    if (ctx->current_panel != NULL) return ctx->current_panel->depth_bg;
    return ctx->max_depth_available-2;
}
static f32 get_depth_foreground(SE_UI *ctx) {
    if (ctx->current_panel != NULL) return ctx->current_panel->depth_fg;
    return ctx->max_depth_available;
}

static Rect apply_margin(Rect rect, Vec2 margin) {
    return (Rect) {
        rect.x + margin.x,
        rect.y + margin.y,
        rect.w - margin.x,
        rect.h - margin.y,
    };
}

static Rect expand_view_region(SE_UI *ctx, Rect normalised_rect) {
    Rect result = normalised_rect;
    result.x *= ctx->renderer.viewport.w;
    result.w *= ctx->renderer.viewport.w;
    result.y *= ctx->renderer.viewport.h;
    result.h *= ctx->renderer.viewport.h;
    return result;
}

void seui_theme_default(SE_Theme *theme) {
    // theme->colour_normal  = (RGBA) {65, 84, 105, 255};
    // theme->colour_hover   = (RGBA) {108, 145, 173, 255};
    // theme->colour_pressed = (RGBA) {43, 56, 71, 255};
    // theme->colour_bg      = (RGBA) {33, 39, 43, 200};
    // theme->colour_fg      = (RGBA) {56, 95, 161, 255};
    // theme->margin = v2f(8, 0);
    theme->colour_normal  = (RGBA) {227, 126, 39, 255};
    theme->colour_hover   = (RGBA) {242, 145, 85, 255};
    theme->colour_pressed = (RGBA) {156, 67, 12, 255};
    theme->colour_bg      = (RGBA) {59, 34, 32, 255};
    theme->colour_fg      = (RGBA) {56, 95, 161, 255};
    // theme->colour_bg_2    = (RGBA) {10, 10, 10, 230};
    theme->colour_bg_2      = (RGBA) {59, 34, 32, 255};
    theme->margin = v2f(16, 8);
}

/// Set text input configurations to default values. It's good practice to
/// do call this procedure after modifying the configuration for a usecase.
void seui_configure_text_input_reset(SE_UI *ctx) {
    ctx->text_input_only_numerical = false;
}

/// Reset the panel configurations to their default values.
/// These are the values that the user can manually set before each widget call
/// to customise their appearance.
void seui_configure_panel_reset(SEUI_Panel *panel) {
    panel->config_row_left_margin = 0;
    panel->config_row_right_margin = 0;
    panel->config_item_centered = false;
    panel->config_item_minimised = false;
}


/// call this at the beginning of every frame before creating other widgets
void seui_reset(SE_UI *ctx) {
    ctx->max_id = SEUI_ID_NULL;
    ctx->current_panel = NULL;
    ctx->panel_count = 0;
    ctx->panel_container_count = 0;
    ctx->current_max_depth = ctx->min_depth_available;
}

void seui_resize(SE_UI *ctx, u32 window_w, u32 window_h) {
    ctx->viewport = (Rect) {0, 0, window_w, window_h};
    serender2d_resize(&ctx->renderer, ctx->viewport, ctx->min_depth_available, ctx->max_depth_available);
    se_set_text_viewport(&ctx->txt_renderer, ctx->viewport, ctx->min_depth_available, ctx->max_depth_available);
}

void seui_init(SE_UI *ctx, SE_Input *input, Rect viewport, f32 min_depth, f32 max_depth) {
    ctx->warm = SEUI_ID_NULL;
    ctx->hot = SEUI_ID_NULL;
    ctx->active = SEUI_ID_NULL;
    seui_reset(ctx);
    ctx->input = input;

    ctx->min_depth_available = min_depth;
    ctx->max_depth_available = max_depth;

    ctx->viewport = viewport;
    serender2d_init(&ctx->renderer, ctx->viewport, ctx->min_depth_available, ctx->max_depth_available);
    se_init_text_default(&ctx->txt_renderer, ctx->viewport, ctx->min_depth_available, ctx->max_depth_available);

    seui_theme_default(&ctx->theme);

    // setexture_atlas_load(&ctx->icon_atlas, "assets/UI/icons/ui_icons_atlas.png", 4, 4);
    setexture_atlas_load(&ctx->icon_atlas, "core/textures/ui_icons_atlas.png", 4, 4);

    /* panels */
    ctx->current_panel = NULL;
    ctx->current_non_embedded_panel = NULL;
    ctx->current_dragging_panel = NULL;
    ctx->latest_activated_panel = NULL;
    ctx->current_max_depth = ctx->min_depth_available;

    ctx->panel_capacity = 100;
    ctx->panel_count = 0;
    ctx->panels = (SEUI_Panel*) malloc(sizeof(SEUI_Panel) * ctx->panel_capacity);
    memset(ctx->panels, 0, sizeof(SEUI_Panel) * ctx->panel_capacity);
    for (u32 i = 0; i < ctx->panel_capacity; ++i) {
        ctx->panels[i].index = -1;
    }

    ctx->panel_container_count = 0;
    memset(ctx->panel_containers, -1, sizeof(i32) * SEUI_PANEL_CONTAINER_CAPACITY);

    sestring_init(&ctx->text_input_cache, "");
    sestring_init(&ctx->text_input, "");
}

void seui_deinit(SE_UI *ctx) {
    serender2d_deinit(&ctx->renderer);
    se_deinit_text(&ctx->txt_renderer);
    sestring_deinit(&ctx->text_input_cache);
    sestring_deinit(&ctx->text_input);
    setexture_atlas_unload(&ctx->icon_atlas);
    free(ctx->panels);
    ctx->panel_count = 0;
}

void seui_close_panel(SE_UI *ctx, u32 panel_index) {
    if (panel_index < ctx->panel_count) ctx->panels[panel_index].is_closed = true;
}

void seui_render(SE_UI *ctx) {
    /* upload data */
    serender2d_upload_to_gpu(&ctx->renderer);

    /* configure */

    /* draw call */
    serender2d_render(&ctx->renderer);
    serender2d_clear_shapes(&ctx->renderer);

    /* text */
    se_render_text(&ctx->txt_renderer);
    se_clear_text_render_queue(&ctx->txt_renderer); // sense we're gonna recreate the queue next frame
}

SEUI_Panel* seui_ctx_get_panel(SE_UI *ctx) {
    if (ctx->panel_count >= ctx->panel_capacity) {
        ctx->panel_capacity += (ctx->panel_capacity+1) * 0.5f;
        ctx->panels = realloc(ctx->panels, sizeof(SEUI_Panel) * ctx->panel_capacity);
        memset(ctx->panels + ctx->panel_count, 0, sizeof(SEUI_Panel) * (ctx->panel_capacity - ctx->panel_count)); // @TODO test this
    }
    u32 panel = ctx->panel_count;
    if (ctx->panels[panel].index == ctx->panel_count) { // the order of panel draw calls has not changed
        // the indices match so we won't reset this panel, because we want to keep the data from the previous frame
        ctx->panel_count++;

        return &ctx->panels[panel];
    } else {
        // the indices don't match, so this is a different panel and we have to reset it
        Rect init_rect = {
            ctx->viewport.w / 2 - 128, // the hackiest way to get the viewport
            ctx->viewport.h / 2 - 128, // the hackiest way to get the viewport
            128 * 2,
            128 * 2
        };
        seui_panel_setup(&ctx->panels[panel], init_rect, false, 32, 0);
        ctx->panels[panel].index = panel;
        ctx->panels[panel].is_closed = false; // by default panels are closed
        ctx->panel_count++;
        return &ctx->panels[panel];
    }
}

SEUI_Panel* seui_ctx_get_panel_container(SE_UI *ctx) {
    i32 panel = ctx->panel_containers[ctx->panel_container_count];
    ctx->panel_container_count++;
    if (ctx->panels[panel].is_closed || panel < 0) return NULL;
    return &ctx->panels[panel];
}

/// -----------------------------------------
///                 PANEL
/// -----------------------------------------

/// Make a row
void seui_panel_row(SE_UI *ctx, f32 height, u32 columns) {
    SEUI_Panel *panel = ctx->current_panel;
    // panel->row_width = panel->row_width;     // keep the same width as the previous row
    // panel->row_height = panel->min_item_height; // reset row height
    panel->row_height = height; // reset row height
    panel->row_width  = panel->cached_rect.w;
    panel->row_columns = columns;

    panel->next_item_height = panel->row_height;

    /* reset for the new row */
    panel->cursor.x = 0;
    panel->cursor.y -= panel->row_height;

    panel->fit_cursor.x = 0;
    panel->fit_cursor.y -= panel->row_height; // sense row_height is shrunk to min height
    panel->fit_size.y += panel->row_height;

}

void seui_panel_setup(SEUI_Panel *panel, Rect initial_rect, b8 minimised, f32 min_item_height, i32 docked_dir /* = 0*/) {
    seui_configure_panel_reset(panel);
    panel->calc_rect = initial_rect;
    panel->cached_rect = initial_rect;
    panel->minimised = minimised;
    panel->min_item_height = min_item_height;
    panel->is_embedded = false;

    if (docked_dir != 1 && docked_dir != 2) panel->docked_dir = 0; else panel->docked_dir = docked_dir;

    panel->row_height = panel->min_item_height; // default row height
    panel->row_width  = panel->min_item_height; // default row width
}

/// Returns a rectangle that's suppose to be the rect
/// of the new item inside of the current panel.
Rect seui_panel_put(SE_UI *ctx, f32 min_width) {
    SEUI_Panel *panel = ctx->current_panel;
    if (panel == NULL) {
        printf("ERROR: seui_panel_put but panel was null\n");
        return (Rect) {0, 0, 32, 32};
    }

    // if (panel->next_item_height < min_height) {
    //     panel->next_item_height = min_height;
    // }
    // if (panel->row_height < min_height) {
    //     panel->row_height = min_height;
    // }

    Rect item;
    item.x = panel->cursor.x + panel->cached_rect.x;
    item.y = panel->cursor.y + panel->cached_rect.y;
    item.h = panel->next_item_height;

    // if (panel->config_item_minimised) {
    //     item.w = min_width;
    // } else {
    //     item.w = panel->row_width / (f32)panel->row_columns;
    // }
        item.w = panel->row_width / (f32)panel->row_columns;
    // if (item.w < min_width) item.w = min_width;

    /* advance the cursor based on row layout */
    // assuming default layout (to be changed)
    panel->cursor.x += item.w;
    if ((i32)panel->cursor.x > (i32)panel->row_width) {
        seui_panel_row(ctx, panel->row_height, panel->row_columns); // make a new row
    }

    /* update panel fit size */
    panel->fit_cursor.x += min_width;
    if (panel->fit_size.x < panel->fit_cursor.x) {
        panel->fit_size.x = panel->fit_cursor.x;
    }
    return item;
}

b8 seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {
    if (panel_data == NULL) return false;
    ctx->current_panel = panel_data; // record this panel as current panel before returning
    if (panel_data->is_closed) return false;

    b8 *minimised   = &panel_data->minimised;
    b8 is_minimised = *minimised;
    RGBA colour = ctx->theme.colour_bg;

    if (!panel_data->is_embedded) {
        ctx->current_non_embedded_panel = panel_data;
    }

        // depth
    if (panel_data->is_embedded && ctx->current_non_embedded_panel != NULL) {
        panel_data->depth_bg = ctx->current_non_embedded_panel->depth_bg;
        panel_data->depth_mg = ctx->current_non_embedded_panel->depth_mg;
        panel_data->depth_fg = ctx->current_non_embedded_panel->depth_fg;
    } else {
        if (ctx->latest_activated_panel == panel_data) {
            // panel_data->depth_bg = -ctx->current_max_depth+1;
            // panel_data->depth_mg = -ctx->current_max_depth+2;
            // panel_data->depth_fg = -ctx->current_max_depth+3;
            panel_data->depth_bg = ctx->max_depth_available-2 - 3; // reserve 3 spaces for widgets that are not bound to a panel (so we want them on most top)
            panel_data->depth_mg = ctx->max_depth_available-1 - 3; // reserve 3 spaces for widgets that are not bound to a panel (so we want them on most top)
            panel_data->depth_fg = ctx->max_depth_available-0 - 3; // reserve 3 spaces for widgets that are not bound to a panel (so we want them on most top)
        } else {
            panel_data->depth_bg = ctx->current_max_depth+1;
            panel_data->depth_mg = ctx->current_max_depth+2;
            panel_data->depth_fg = ctx->current_max_depth+3;
            ctx->current_max_depth += 3;
        }
    }

    /* move the panel inside of the viewport of any portion of it is outside */
        // right
    if (panel_data->calc_rect.x + panel_data->calc_rect.w > ctx->viewport.w) {
        panel_data->calc_rect.x = ctx->viewport.w - panel_data->calc_rect.w;
    }
        // left
    if (panel_data->calc_rect.x < 0) {
        panel_data->calc_rect.x = 0;
    }
        // top
    if (panel_data->calc_rect.y + panel_data->calc_rect.h > ctx->viewport.h) {
        panel_data->calc_rect.y = ctx->viewport.h - panel_data->calc_rect.h;
    }
        // bottom
    if (panel_data->calc_rect.y < 0) {
        panel_data->calc_rect.y = 0;
    }

    /* record previous frame's data */
    panel_data->cached_rect = panel_data->calc_rect; // record previous frame's rect
    Vec2 cached_fit_size = panel_data->fit_size;
    /* reset panel for calculation again */
    panel_data->cursor = (Vec2) {
        0, // start from the top left
        panel_data->calc_rect.h
    };
    panel_data->fit_size = v2f(0, 0);
    panel_data->fit_cursor = panel_data->cursor;
    panel_data->next_item_height = panel_data->min_item_height;

    // draw a rectangle that represents the panel's dimensions
    if (!is_minimised && !panel_data->is_embedded) serender2d_add_rect(&ctx->renderer, panel_data->cached_rect, panel_data->depth_bg, colour);

    { // panel widgets

        const f32 button_size = 32;
        seui_panel_row(ctx, button_size, 1); // make space for top bar

        Rect top_bar = seui_panel_put(ctx, 0);


        Vec2 cursor = {
            top_bar.x, top_bar.y
        };

        /* minimise button */
        Rect minimise_button_rect = (Rect) {cursor.x + panel_data->calc_rect.w - button_size * 2, cursor.y, button_size, button_size};
        Vec2 index = is_minimised ? UI_ICON_INDEX_UNCOLLAPSE : UI_ICON_INDEX_COLLAPSE;
        serender2d_add_rect_textured_atlas(&ctx->renderer, minimise_button_rect, panel_data->depth_fg, RGBA_WHITE, &ctx->icon_atlas, index);
        if (seui_button_at(ctx, "", minimise_button_rect)) {
            *minimised = !*minimised;
        }

        UI_STATES drag_state = UI_STATE_DISABLED;
        if (panel_data->is_embedded == false) {
            /* drag button */
            Rect drag_button_rect = (Rect) {cursor.x, cursor.y, panel_data->calc_rect.w - button_size * 2, button_size};
            Vec2 drag = seui_drag_button_at(ctx, drag_button_rect, &drag_state);
            se_add_text_rect(&ctx->txt_renderer, title, drag_button_rect, get_depth_foreground(ctx));
            panel_data->calc_rect.x += drag.x;
            panel_data->calc_rect.y += drag.y;

                // update the top panel / current_dragging_panel
            if (drag_state == UI_STATE_HOT && ctx->current_dragging_panel != panel_data) ctx->current_dragging_panel = panel_data;
            if (ctx->current_dragging_panel == panel_data && drag_state != UI_STATE_HOT) ctx->current_dragging_panel = NULL;
            if (drag_state == UI_STATE_HOT) ctx->latest_activated_panel = panel_data;

            /* close button */
            Rect close_button_rect = (Rect) {cursor.x + panel_data->calc_rect.w - button_size, cursor.y, button_size, button_size};
            serender2d_add_rect_textured_atlas(&ctx->renderer, close_button_rect, panel_data->depth_fg, RGBA_WHITE, &ctx->icon_atlas, UI_ICON_INDEX_CLOSE);
            if (seui_button_at(ctx, "", close_button_rect)) {
                seui_close_panel(ctx, panel_data->index);
            }
        }

        /* panel outline */
        serender2d_add_rect_outline(&ctx->renderer, panel_data->cached_rect, panel_data->depth_fg, RGBA_BLACK, 2);

        /* resizeing */
        // Vec2 min_size = panel_data->min_size;
        Vec2 min_size = cached_fit_size;
        min_size.y += minimise_button_rect.h;
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


        if (!panel_data->is_embedded) { // -- docking
            RGBA dock_colour = {150, 0, 0, 100};
            Rect normalised_rect = panel_data->calc_rect;
            normalised_rect.x /= ctx->renderer.viewport.w;
            normalised_rect.w /= ctx->renderer.viewport.w;
            normalised_rect.y /= ctx->renderer.viewport.h;
            normalised_rect.h /= ctx->renderer.viewport.h;

            Vec2 normalised_cursor = ctx->input->mouse_screen_pos;
            normalised_cursor.x /= ctx->renderer.viewport.w;
            normalised_cursor.y /= ctx->renderer.viewport.h;

            // @note UI_STATE_ACTIVE means that the button was just released
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_RIGHT, normalised_cursor)) { // right
                if (drag_state == UI_STATE_HOT) {
                    serender2d_add_rect(&ctx->renderer,  expand_view_region(ctx, SEUI_VIEW_REGION_RIGHT), panel_data->depth_fg, dock_colour);
                }
                if (drag_state == UI_STATE_ACTIVE && !ctx->input->is_mouse_left_down) { // mouse released so dock
                    panel_data->docked_dir = 2;
                }
            } else
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_LEFT, normalised_cursor)) { // left
                if (drag_state == UI_STATE_HOT) {
                    serender2d_add_rect(&ctx->renderer,  expand_view_region(ctx, SEUI_VIEW_REGION_LEFT), panel_data->depth_fg, dock_colour);
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

    return !panel_data->is_closed && !panel_data->minimised;
}

b8 seui_panel(SE_UI *ctx, const char *title) {
    SEUI_Panel *panel_data = seui_ctx_get_panel(ctx);
    return seui_panel_at(ctx, title, panel_data);
}

//// COMPONENTS ////

u32 generate_ui_id(SE_UI *ctx) {
    ctx->max_id++; // we start with zero. So we increase first.
    u32 id = ctx->max_id;
    return id;
}

/// note that stay_active_on_mouse_leave is used for dragging ui items
UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, b8 stay_active_on_mouse_leave /* = false */) {
    UI_STATES result = UI_STATE_IDLE;
    SE_Input *input = ctx->input;
    b8 mouse_down   = input->is_mouse_left_down;
    b8 mouse_up     = !mouse_down;
    b8 mouse_inside = rect_overlaps_point(rect, input->mouse_screen_pos);

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

b8 seui_button_at(SE_UI *ctx, const char *text, Rect rect) {
    SE_Input *input = ctx->input;
    SE_Renderer2D *renderer = &ctx->renderer;

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

    serender2d_add_rect(renderer, rect, get_depth_middleground(ctx), colour);
    se_add_text_rect(&ctx->txt_renderer, text, rect, get_depth_foreground(ctx));

    return ui_state == UI_STATE_ACTIVE;
}

b8 seui_button_textured_at(SE_UI *ctx, Vec2 texture_index, Rect rect) {
    SE_Input *input = ctx->input;
    SE_Renderer2D *renderer = &ctx->renderer;

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
        serender2d_add_rect(renderer, rect, get_depth_middleground(ctx), colour);
    } else {
        serender2d_add_rect_textured_atlas(renderer, rect, get_depth_middleground(ctx), colour, &ctx->icon_atlas, texture_index);
    }

    return ui_state == UI_STATE_ACTIVE;
}

Vec2 seui_drag_button_textured_at(SE_UI *ctx, Rect rect, Vec2 texture_index, UI_STATES *state) {
    SE_Input *input = ctx->input;
    SE_Renderer2D *renderer = &ctx->renderer;

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
        serender2d_add_rect(renderer, rect, get_depth_middleground(ctx), colour);
    } else {
        serender2d_add_rect_textured_atlas(renderer, rect, get_depth_middleground(ctx), colour, &ctx->icon_atlas, texture_index);
    }

    return drag;
}

Vec2 seui_drag_button_at(SE_UI *ctx, Rect rect, UI_STATES *state) {
    return seui_drag_button_textured_at(ctx, rect, UI_ICON_INDEX_NULL, state);
}

void seui_label_at(SE_UI *ctx, const char *text, Rect rect) {
    RGBA colour = RGBA_WHITE;
    // change config for this item
    b8 previous_setting = ctx->txt_renderer.config_centered;
    ctx->txt_renderer.config_centered = ctx->current_panel->config_item_centered;

    se_add_text_rect(&ctx->txt_renderer, text, rect, get_depth_foreground(ctx));

    // reset config
    ctx->txt_renderer.config_centered = previous_setting;
    serender2d_add_rect(&ctx->renderer, rect, get_depth_middleground(ctx), ctx->theme.colour_bg_2);
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
    serender2d_add_line(&ctx->renderer, pos1, pos2, get_depth_middleground(ctx), RGBA_BLACK, 3);

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
    serender2d_add_circle(&ctx->renderer, center, radius, get_depth_middleground(ctx), 10, (RGBA) {0, 0, 0, 50});

    /* draw the button */
    Rect button_rect = rect_create(button_pos, button_size);
    Vec2 drag = seui_drag_button_textured_at(ctx, button_rect, UI_ICON_INDEX_SLIDER, NULL);

    // clamp before concluding
    // *value += drag.x * 0.01f;
    *value = vec2_add(*value, vec2_mul_scalar(drag, 0.05f));
    vec2_normalise(value);
}

b8 seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max) {
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
    serender2d_add_rect(&ctx->renderer, rect, get_depth_background(ctx), colour_bg);
    /* left button */
    Rect button = {
        rect.x,
        rect.y,
        button_size,
        button_size
    };
    b8 decrease = seui_button_textured_at(ctx, UI_ICON_INDEX_ARROW_LEFT, button);
    /* right button */
    button = (Rect) {
        rect.x + rect.w - button_size,
        rect.y + rect.h - button_size,
        button_size,
        button_size
    };
    b8 increase = seui_button_textured_at(ctx, UI_ICON_INDEX_ARROW_RIGHT, button);

    /* value change */
    b8 changed = false;
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
    se_add_text_rect(&ctx->txt_renderer, buffer, label, get_depth_foreground(ctx));

    return changed;
}

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect) {
    SE_Input *input = ctx->input;
    SE_Renderer2D *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_bg = ctx->theme.colour_normal;
    RGBA colour_highlight = ctx->theme.colour_hover;
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
            if (ctx->text_input_only_numerical && ctx->active != id) { // if we are hovering but have not selected the text input to edit with keyboard
                f32 numerical_value = sestring_as_f32(text);
                if (ctx->input->mouse_wheel != 0) {
                    char value_as_string[SESTRING_MAX_NUM_OF_DIGITS];
                    numerical_value += ctx->input->mouse_wheel;
                    sprintf(value_as_string, "%i", (i32)numerical_value);
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

            // render the input cursor
        // Rect cursor;
        // Vec2 text_size = se_size_text(&ctx->txt_renderer, ctx->text_input.buffer);
        // cursor.x = text_size.x + rect.x;
        // cursor.y = text_size.y + rect.y;
        // cursor.w = se_size_text(&ctx->txt_renderer, "W").x;
        // cursor.h = text_size.y;
        // RGBA cursor_colour = RGBA_WHITE;
        // static f32 alpha = 0;
        // alpha += 0.167; // @incomplete dummy delta
        // cursor_colour.a = semath_sin(alpha);
        // if (alpha > 1000) alpha = 0;
        // serender2d_add_rect(renderer, cursor, get_depth_foreground(ctx), cursor_colour);

        /* accept */
        if ((seinput_is_mouse_left_pressed(ctx->input) || seinput_is_mouse_right_pressed(input)) && ui_state != UI_STATE_WARM // clicked outside
            || seinput_is_key_pressed(ctx->input, SDL_SCANCODE_RETURN)) {
            // copy the input text data over to text
            sestring_clear(text); // get rid of what was there and put in the new edited text
            sestring_duplicate(&ctx->text_input, text);
            ctx->active = SEUI_ID_NULL;
            seinput_text_input_deactivate(input);
        } else
        /* cancel */
        if (seinput_is_key_pressed(ctx->input, SDL_SCANCODE_ESCAPE)) {
            // DON'T copy the input text data over to text
            ctx->active = SEUI_ID_NULL;
            seinput_text_input_deactivate(input);
        }
    }

    serender2d_add_rect(renderer, rect, get_depth_middleground(ctx), colour);
    if (display_text->size > 0) {
        ctx->txt_renderer.config_colour = colour_text;
        se_add_text_rect(&ctx->txt_renderer, display_text->buffer, rect, get_depth_foreground(ctx));
    } else if (!ctx->text_input_only_numerical) {
        ctx->txt_renderer.config_colour = colour_text_hint;
        se_add_text_rect(&ctx->txt_renderer, "press to type", rect, get_depth_foreground(ctx));
    } else {
        ctx->txt_renderer.config_colour = colour_text_hint;
        se_add_text_rect(&ctx->txt_renderer, "...", rect, get_depth_foreground(ctx));
    }
    se_text_reset_config(&ctx->txt_renderer);
}

b8 seui_button_textured(SE_UI *ctx, Vec2 texture_index) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, rect.w);
    }
    return seui_button_textured_at(ctx, texture_index, rect);
}

b8 seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max) {
    Rect rect = {0, 0, 100, 32}; // default
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, rect.w);
    }
    return seui_selector_at(ctx, rect, value, min, max);
}

void seui_hsv_picker(SE_UI *ctx, HSV *hsv) {
    if (hsv->h < 0) hsv->h = 0;
    if (hsv->h >= 360) hsv->h = 0;

    if (seui_panel(ctx, "hsv picker")) {
        { /* colour picker wheel and triangle and functionality */
            {
                seui_panel_row(ctx, 16, 1);
                Rect rect = seui_panel_put(ctx, 0);
                serender2d_add_rect(&ctx->renderer, rect, get_depth_background(ctx), ctx->theme.colour_bg_2);
            }
            seui_panel_row(ctx, 240, 1);
            Rect rect = seui_panel_put(ctx, 240);
            Vec2 center = v2f(rect.x + rect.w / 2, rect.y + rect.h / 2);
            f32 outer_radius = rect.h / 2;
            f32 thickness = 16;
            /* background */
            serender2d_add_rect(&ctx->renderer, rect, get_depth_background(ctx), ctx->theme.colour_bg_2);

            /* colour wheel */
            serender2d_add_hsv_wheel(&ctx->renderer, center, outer_radius - thickness, thickness, get_depth_middleground(ctx));

            f32 angle = hsv->h * SEMATH_DEG2RAD_MULTIPLIER;
            f32 radius = outer_radius - thickness;
#if 0   // colour triangle
            { /* colour triangle */
                serender2d_add_hsv_triangle(&ctx->renderer, center, radius, get_depth_middleground(ctx), angle);
                if (seinput_is_key_down(ctx->input, SDL_SCANCODE_H)) {
                    hsv->h += 5;
                }
            }
            { /* the cursor on the triangle */
                Vec2 colour_tip = {
                    semath_cos(angle) * radius + center.x,
                    semath_sin(angle) * radius + center.y,
                };
                Vec2 white_tip = {
                    semath_cos(angle + 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.x,
                    semath_sin(angle + 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.y,
                };
                Vec2 black_tip = {
                    semath_cos(angle - 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.x,
                    semath_sin(angle - 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.y,
                };
                Vec2 hsv_point = colour_tip;

                hsv_point.x = lerp(hsv_point.x, white_tip.x, 1 - hsv->s);
                hsv_point.y = lerp(hsv_point.y, white_tip.y, 1 - hsv->s);

                hsv_point.x = lerp(hsv_point.x, black_tip.x, 1 - hsv->v);
                hsv_point.y = lerp(hsv_point.y, black_tip.y, 1 - hsv->v);

                serender2d_add_circle_outline(&ctx->renderer, hsv_point, 4, get_depth_foreground(ctx), 8, RGBA_WHITE, 1);

                /* changing the saturation and value based on mouse input */
                Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
                mouse_pos.y = ctx->viewport.h - mouse_pos.y;

                if (ctx->input->is_mouse_left_down) {
                    if (point_overlaps_circle(mouse_pos, center, radius)) {
                        Vec3 result = cartesian_to_barycentric_coordinates(mouse_pos, colour_tip, white_tip, black_tip);
                        if (result.x >= 0 && result.x <= 1 && result.y >= 0 && result.y <= 1 && result.z >= 0 && result.z <= 1) {
                            // hsv->s = 1 - result.y;
                            // hsv->v = 1 - result.z;
                            hsv->s = result.x;
                            hsv->v = 1-result.y;
                        }
                    } else
                    if (point_overlaps_circle(mouse_pos, center, outer_radius)) {
                        f32 cursor_angle = vec2_angle(vec2_sub(mouse_pos, center));
                        hsv->h = SEMATH_RAD2DEG(cursor_angle);
                        hsv_clamp(hsv);
                    }
                }
            }
#else   // colour rectangle
            { // colour rect
                f32 extends = outer_radius - thickness;
                extends *= 0.6f;
                Rect hsv_rect = {center.x - extends, center.y - extends, extends * 2, extends * 2};
                serender2d_add_hsv_rect(&ctx->renderer, hsv_rect, get_depth_middleground(ctx), hsv->h);

                { // the cursor on the rect
                    Vec2 cursor = v2f(hsv_rect.x, hsv_rect.y);
                    cursor.x += hsv->s * hsv_rect.w;
                    cursor.y += hsv->v * hsv_rect.h;
                    serender2d_add_circle_outline(&ctx->renderer, cursor, 4, get_depth_foreground(ctx), 8, RGBA_WHITE, 2);

                        //-- changing saturation and value
                    Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
                    mouse_pos.y = ctx->viewport.h - mouse_pos.y;

                    if (ctx->input->is_mouse_left_down) {
                        if (rect_overlaps_point(hsv_rect, mouse_pos)) {
                            Vec2 result = mouse_pos;
                            result.x -= hsv_rect.x;
                            result.y -= hsv_rect.y;
                            hsv->s = result.x / hsv_rect.w;
                            hsv->v = result.y / hsv_rect.h;
                        }

                        if (!point_overlaps_circle(mouse_pos, center, radius) && point_overlaps_circle(mouse_pos, center, outer_radius)) {
                            f32 cursor_angle = vec2_angle(vec2_sub(mouse_pos, center));
                            hsv->h = SEMATH_RAD2DEG(cursor_angle);
                            hsv_clamp(hsv);
                        }
                    }
                }
            }
#endif
        }
        { /* buttons and sliders */
            seui_label_hsv(ctx, "hsv", hsv, true);
        }
        { /* colour preview */
            seui_panel_row(ctx, 32, 1);
            Rect preview_rect = seui_panel_put(ctx, 16);
            RGBA c = RGBA_WHITE;
            hsv_to_rgba(hsv->h, hsv->s, hsv->v, &c);
            serender2d_add_rect(&ctx->renderer, preview_rect, get_depth_foreground(ctx), c);
        }
    }
}

void seui_texture_viewer(SE_UI *ctx, u32 texture_id) {
    if (seui_panel(ctx, "texture viewer")) {
        Rect rect = ctx->current_panel->cached_rect;
        rect.h -= 48;
        rect.y += 16;
        ctx->current_panel->fit_size = v2f(128, 128);

        if (rect.h > rect.w) {
            rect.h = rect.w;
        }
        if (rect.w > rect.h) {
            rect.w = rect.h;
        }

        serender2d_add_rect_textured(&ctx->renderer, rect, get_depth_middleground(ctx), RGBA_WHITE, texture_id);
    }
}

void seui_panel_container(SE_UI *ctx) {
    SEUI_Panel *panel = seui_ctx_get_panel_container(ctx);
    if (panel == NULL) {
        seui_panel_row(ctx, 240, 1);
        Rect rect = seui_panel_put(ctx, 0);
        if (seui_button_at(ctx, "drag a panel", rect)) {
                printf("released\n");
            if (ctx->current_dragging_panel != NULL) {
                printf("released panel\n");
            }
        }
    } else {
        seui_panel_row(ctx, 240, 1);
        Rect rect = seui_panel_put(ctx, 0);
        // draw the panel
        panel->is_embedded = true;
        panel->calc_rect.x = rect.x;
        panel->calc_rect.y = rect.y;
        panel->calc_rect.w = rect.w;
        panel->calc_rect.h = rect.h;
    }
}

//// ADVANCED WIDGETS ////

/// -----------------------------------------
///                WIDGETS
/// -----------------------------------------

b8 seui_button(SE_UI *ctx, const char *text) {
    Vec2 text_size = se_size_text(&ctx->txt_renderer, text);
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, text_size.x);
    }
    return seui_button_at(ctx, text, rect);
}

void seui_label(SE_UI *ctx, const char *text) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        Vec2 text_size = se_size_text(&ctx->txt_renderer, text);
        rect = seui_panel_put(ctx, text_size.x + ctx->theme.margin.x);
    }
    seui_label_at(ctx, text, rect);
}

void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, b8 editable) {
    char label_buffer[255];
    seui_panel_row(ctx, 32, 1);

    b8 previous_setting = ctx->current_panel->config_item_centered;
    ctx->current_panel->config_item_centered = true;
    seui_label(ctx, title);
        // reset setting
    ctx->current_panel->config_item_centered = previous_setting;

    if (!editable) {
        seui_panel_row(ctx, 32, 3);
        sprintf(label_buffer, "x: %.2f", value->x);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "y: %.2f", value->y);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "z: %.2f", value->z);
        seui_label(ctx, label_buffer);
    } else {
        ctx->text_input_only_numerical = true;
        b8 previous_item_minimised_config = ctx->current_panel->config_item_minimised;
        seui_panel_row(ctx, 32, 6);

            //- X
        ctx->current_panel->config_item_minimised = true;
        sprintf(label_buffer, "x:");
        seui_label(ctx, label_buffer);
        ctx->current_panel->config_item_minimised = previous_item_minimised_config;

        sprintf(label_buffer, "%.2f", value->x);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->x = sestring_as_f32(&ctx->text_input_cache);

            //- Y
        ctx->current_panel->config_item_minimised = true;
        sprintf(label_buffer, "y:");
        seui_label(ctx, label_buffer);
        ctx->current_panel->config_item_minimised = previous_item_minimised_config;

        sprintf(label_buffer, "%.2f", value->y);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->y = sestring_as_f32(&ctx->text_input_cache);

            //- Z
        ctx->current_panel->config_item_minimised = true;
        sprintf(label_buffer, "z:");
        seui_label(ctx, label_buffer);
        ctx->current_panel->config_item_minimised = previous_item_minimised_config;

        sprintf(label_buffer, "%.2f", value->z);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->z = sestring_as_f32(&ctx->text_input_cache);

        seui_configure_text_input_reset(ctx); // reset configurations
    }
}

void seui_label_hsv(SE_UI *ctx, const char *title, HSV *value, b8 editable) {
    char label_buffer[255];
    seui_panel_row(ctx, 32, 1);

    b8 previous_setting = ctx->current_panel->config_item_centered;
    ctx->current_panel->config_item_centered = true;
    seui_label(ctx, title);
        // reset setting
    ctx->current_panel->config_item_centered = previous_setting;

    if (!editable) {
        seui_panel_row(ctx, 32, 3);
        sprintf(label_buffer, "h: %i", value->h);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "s: %i", (i32)(value->s * 100));
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "v: %i", (i32)(value->v * 100));
        seui_label(ctx, label_buffer);
    } else {
        ctx->text_input_only_numerical = true;
        seui_panel_row(ctx, 32, 6);

        sprintf(label_buffer, "h:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%i", value->h);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->h = (i32)sestring_as_f32(&ctx->text_input_cache);
        if (value->h < 0) value->h = 359;
        if (value->h > 359) value->h = 0;

        sprintf(label_buffer, "s:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%i", (i32)(value->s * 100));
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->s = sestring_as_f32(&ctx->text_input_cache) / 100.0f;
        if (value->s < 0) value->s = 0;
        if (value->s > 1) value->s = 1;

        sprintf(label_buffer, "v:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%i", (i32)(value->v * 100));
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->v = sestring_as_f32(&ctx->text_input_cache) / 100.0f;
        if (value->v < 0) value->v = 0;
        if (value->v > 1) value->v = 1;

        seui_configure_text_input_reset(ctx); // reset configurations
    }
}

void seui_slider(SE_UI *ctx, f32 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, 24);
    }
    Vec2 pos1 = {rect.x, (rect.y + rect.y + rect.h) * 0.5f};
    Vec2 pos2 = {rect.x + rect.w, (rect.y + rect.y + rect.h) * 0.5f};
    seui_slider_at(ctx, pos1, pos2, value);
}

void seui_slider2d(SE_UI *ctx, Vec2 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, 32);
    }
    Vec2 center = {
        (rect.x + rect.x + rect.w) * 0.5f,
        (rect.y + rect.y + rect.h) * 0.5f
    };
    f32 radius = rect.h * 0.5f;
    seui_slider2d_at(ctx, center, radius, value);
}

void seui_input_text(SE_UI *ctx, SE_String *text) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, 100);
    }
    seui_input_text_at(ctx, text, rect);
}