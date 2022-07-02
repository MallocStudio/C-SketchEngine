#include "seui2.h"

void ui_init(UI_CTX *ctx, SE_Input *input, Rect viewport) {
    memset(ctx, 0, sizeof(UI_CTX));

    ctx->input = input;
    ctx->viewport = viewport;
    f32 min_depth = -1000;
    f32 max_depth = 1000;

    serender2d_init(&ctx->renderer, viewport, min_depth, max_depth);
    se_init_text_default(&ctx->txt_renderer, ctx->viewport, min_depth, max_depth);
    setexture_atlas_load(&ctx->icon_atlas, "core/textures/ui_icons_atlas.png", 4, 4);
    ctx->theme = ui_default_theme();
    ctx->root = NULL;
}

void ui_deinit(UI_CTX *ctx) {
    serender2d_deinit(&ctx->renderer);
    se_deinit_text(&ctx->txt_renderer);
}

void ui_resize(UI_CTX *ctx, Rect viewport) {
    ctx->viewport = viewport;
    f32 min_depth = -1000;
    f32 max_depth = 1000;
    serender2d_resize(&ctx->renderer, viewport, min_depth, max_depth);
    se_set_text_viewport(&ctx->txt_renderer, viewport, min_depth, max_depth);
}

static void render_widget(UI_CTX *ctx, UI_Widget *widget) {
    f32 bg_depth = widget->depth + 0; // background
    f32 mg_depth = widget->depth + 1; // middle ground
    f32 fg_depth = widget->depth + 2; // foreground

    RGBA colour_bg = ctx->theme.colour_bg;
    RGBA colour_outline = ctx->theme.colour_outline;

        //- Decide on colours
    // interactable button colour
    if (widget->flags & UI_WIDGET_FLAG_CLICKABLE) {
        // normal
        colour_bg = ctx->theme.colour_button_normal;

        // hover highlight
        if (widget->recorded_interaction.hovering) {
            colour_bg = ctx->theme.colour_button_highlight;
        }

        // mouse down dim
        if (widget->recorded_interaction.pressed) {
            colour_bg = ctx->theme.colour_button_dim;
        }
    }

    {   //- RENDERING
        Rect rect = widget->rect;
            // Background
        if (widget->flags & UI_WIDGET_FLAG_DRAW_BACKGROUND) {
            serender2d_add_rect(&ctx->renderer, rect, bg_depth, colour_bg);
        }

            // Foreground
        if (widget->flags & UI_WIDGET_FLAG_DRAW_BORDER) {
            serender2d_add_rect_outline(&ctx->renderer, rect, fg_depth, colour_outline, 1);
        }

            // Text
        if (widget->flags & UI_WIDGET_FLAG_DRAW_TEXT && widget->text.buffer != NULL) {
            ctx->txt_renderer.config_colour = ctx->theme.colour_text;
            ctx->txt_renderer.config_centered = ctx->theme.alignment_centered;
            se_add_text_rect(&ctx->txt_renderer, widget->text.buffer, rect, mg_depth);
        }
    }


        //- Draw Child
    for (u32 i = 0; i < widget->child_count; ++i) {
        render_widget(ctx, widget->children[i]);
    }
}

static void calculate_widget_rect_static(UI_CTX *ctx, UI_Widget *widget) {
    UI_Layout layout = widget->layout;
    if (widget->semantic_size[0].type == UI_SIZE_TYPE_NULL) {
        // widget's rect remains the same as before
    }

    if (widget->semantic_size[0].type == UI_SIZE_TYPE_PIXELS ||
        widget->semantic_size[0].type == UI_SIZE_TYPE_TEXT) {
        f32 x;
        f32 w = widget->semantic_size[0].value;

        if (widget->parent) {
            widget->computed_rel_position.x = widget->parent->rect.x + widget->parent->layout.cursor.x;
        } else {
            widget->computed_rel_position.x = 0;
        }

        x = widget->computed_rel_position.x;

        widget->rect.x = x;
        widget->rect.w = w;
    }

    if (widget->semantic_size[1].type == UI_SIZE_TYPE_PIXELS ||
        widget->semantic_size[1].type == UI_SIZE_TYPE_TEXT) {
        f32 y;
        f32 h = widget->semantic_size[1].value;

        if (widget->parent) {
            widget->computed_rel_position.y = widget->parent->rect.y + widget->parent->layout.cursor.y;
        } else {
            widget->computed_rel_position.y = 0;
        }

        y = widget->computed_rel_position.y;

        widget->rect.y = y;
        widget->rect.h = h;
    }

        //- Advance layout cursor
    if (widget->parent) {
        widget->parent->layout.cursor.x += widget->rect.x * layout.advance_x;
        widget->parent->layout.cursor.y += widget->rect.y * layout.advance_y;
    }


}

static void calculate_widget_rect_percentage_of_parent(UI_CTX *ctx, UI_Widget *widget) {
    UI_Layout layout = widget->layout;

    if (widget->parent) { // we must have a parent
        if (widget->semantic_size[0].type == UI_SIZE_TYPE_PRECENTAGE_OF_PARENT &&
            widget->parent->semantic_size[0].type != UI_SIZE_TYPE_CHILDREN_SUM) {

        }

        if (widget->semantic_size[1].type == UI_SIZE_TYPE_PRECENTAGE_OF_PARENT &&
            widget->parent->semantic_size[1].type != UI_SIZE_TYPE_CHILDREN_SUM) {

        }
    }

    for (u32 i = 0; i < widget->child_count; ++i) {
        calculate_widget_rect_percentage_of_parent(ctx, widget->children[i]);
    }
}

static void calculate_widget_rect_sum_of_children(UI_CTX *ctx, UI_Widget *widget) {
    UI_Layout layout = widget->layout;
        //- CHILDREN SUM
    if (widget->semantic_size[0].type == UI_SIZE_TYPE_CHILDREN_SUM) {
    }
    if (widget->semantic_size[1].type == UI_SIZE_TYPE_CHILDREN_SUM) {
    }
}


void ui_render(UI_CTX *ctx) {
    if (ctx->root) {
        calculate_widget_rect_static(ctx, ctx->root);
        calculate_widget_rect_percentage_of_parent(ctx, ctx->root);
        calculate_widget_rect_sum_of_children(ctx, ctx->root);
        render_widget(ctx, ctx->root);
    }

        //- Shapes
    serender2d_upload_to_gpu(&ctx->renderer);
    serender2d_render(&ctx->renderer);
    serender2d_clear_shapes(&ctx->renderer);

        //- Text
    se_render_text(&ctx->txt_renderer);
    se_clear_text_render_queue(&ctx->txt_renderer); // sense we're gonna recreate the queue next frame
}

static void reset_widget(UI_Widget *widget) {
    for (u32 i = 0; i < widget->child_count; ++i) {
        reset_widget(widget->children[i]);
    }
    widget->child_count = 0;
}

void ui_reset(UI_CTX *ctx) {
    ctx->widgets_count = 0;
    ctx->layout_stack_count = 0;
    ctx->parent_stack_count = 0;
    if (ctx->root) {
        reset_widget(ctx->root);
    }
}

///
/// Layout
///


void ui_push_layout(UI_CTX *ctx, UI_Layout layout) {
    se_assert(ctx->layout_stack_count < UI_MAX_LAYOUTS);
    ctx->layout_stack[ctx->layout_stack_count] = layout;
    ctx->layout_stack_count++;
}

void ui_pop_layout(UI_CTX *ctx) {
    if (ctx->layout_stack_count > 0) ctx->layout_stack_count--;
}

UI_Layout get_layout_horizontal() {
    UI_Layout layout;
    layout.advance_x = true;
    layout.advance_y = false;
    return layout;
}

void ui_layout_horizontal(UI_CTX *ctx, Vec2 anchor) {
    ui_push_layout(ctx, get_layout_horizontal(anchor));
}

UI_Theme ui_default_theme() {
    return (UI_Theme) {
        .colour_bg                  = (RGBA) {40, 48, 61, 255},
        .colour_button_normal       = (RGBA) {200, 100, 19, 255},
        .colour_button_dim          = (RGBA) {156, 67, 12, 255},
        .colour_button_highlight    = (RGBA) {242, 145, 85, 255},
        .colour_outline             = (RGBA) {0, 0, 0, 255},
        .colour_text                = v3f(255, 255, 255),
        .alignment_centered         = true,
    };
}

///
/// Widgets
///

static UI_Interaction get_interaction(SE_Input *input, UI_Widget *widget) {
    UI_Interaction result = {0};
    if (rect_overlaps_point(widget->rect, input->mouse_screen_pos)) {
        result.hovering = true;
    }

    if (result.hovering && input->is_left_mouse_clicked) {
        result.clicked = true;
    }

    if (result.hovering && input->is_left_mouse_down) {
        result.pressed = true;
    }

    widget->recorded_interaction = result;
    return result;
}

static UI_Layout get_layout(UI_CTX *ctx) {
    if (ctx->layout_stack_count == 0) {
        ui_push_layout(ctx, get_layout_horizontal(v2f(0, 0)));
    }
    return ctx->layout_stack[ctx->layout_stack_count - 1];
}

static void push_parent(UI_CTX *ctx, UI_Widget *parent) {
    se_assert(ctx->parent_stack_count < UI_MAX_WIDGETS);
    ctx->parent_stack[ctx->parent_stack_count] = parent;
    ctx->parent_stack_count++;
}

static void pop_parent(UI_CTX *ctx) {
    if (ctx->parent_stack_count > 0) ctx->parent_stack_count--;
}

static UI_Widget* get_parent(UI_CTX *ctx) {
    if (ctx->parent_stack_count > 0) {
        return ctx->parent_stack[ctx->parent_stack_count - 1];
    }
    else return NULL;
}

    /// Set the "child" as a child of the "parent". Reparent child to "parent"
static void parent_push_child(UI_Widget *parent, UI_Widget *child) {
    se_assert(parent->child_count < UI_WIDGET_MAX_CHILDREN);
    child->parent = parent;
    parent->children[parent->child_count] = child;
    parent->child_count++;
}

static UI_Widget* add_widget(UI_CTX *ctx, UI_WIDGET_FLAGS flags) {
    se_assert(ctx->widgets_count < UI_MAX_WIDGETS);
    UI_Widget *widget = &ctx->widgets[ctx->widgets_count];

        //- Update CTX heirarchy
    if (ctx->widgets_count == 0) {
        ctx->root = widget;
        ctx->last = widget;
        push_parent(ctx, widget);
    } else {
        ctx->last = widget;
    }

        //- Widget Settings
    widget->flags = flags;
    if (ctx->widgets_count == 0) {
        widget->parent = NULL;
    } else {
        parent_push_child(get_parent(ctx), widget);
    }

        //- Layout
    widget->layout = get_layout(ctx);

    ctx->widgets_count++;
    return widget;
}

UI_Interaction ui_button(UI_CTX *ctx, const char *string) {
    UI_Widget *widget = add_widget(ctx, UI_WIDGET_FLAG_CLICKABLE |
                                        UI_WIDGET_FLAG_DRAW_BACKGROUND |
                                        UI_WIDGET_FLAG_DRAW_TEXT |
                                        UI_WIDGET_FLAG_DRAW_BORDER |
                                        UI_WIDGET_FLAG_HOT_ANIMATION |
                                        UI_WIDGET_FLAG_ACTIVE_ANIMATION);

    sestring_init(&widget->text, string);

        //- Decide on semantic size
    widget->semantic_size[0].type = UI_SIZE_TYPE_TEXT;
    widget->semantic_size[0].strictness = 1;
    widget->semantic_size[0].value = se_size_text(&ctx->txt_renderer, string).x;
    widget->semantic_size[1].type = UI_SIZE_TYPE_TEXT;
    widget->semantic_size[1].strictness = 1;
    widget->semantic_size[1].value = se_size_text(&ctx->txt_renderer, string).y;

        //- User Interaction
    return get_interaction(ctx->input, widget);
}

void ui_label(UI_CTX *ctx, const char *string) {
    UI_Widget *widget = add_widget(ctx, UI_WIDGET_FLAG_DRAW_BACKGROUND |
                                        UI_WIDGET_FLAG_DRAW_TEXT);

    sestring_init(&widget->text, string);

        //- Decide on semantic size
    widget->semantic_size[0].type = UI_SIZE_TYPE_TEXT;
    widget->semantic_size[0].strictness = 1;
    widget->semantic_size[0].value = se_size_text(&ctx->txt_renderer, string).x + 16;
    widget->semantic_size[1].type = UI_SIZE_TYPE_TEXT;
    widget->semantic_size[1].strictness = 1;
    widget->semantic_size[1].value = se_size_text(&ctx->txt_renderer, string).y + 8;
}

void ui_panel(UI_CTX *ctx) {
    UI_Widget *widget = add_widget(ctx, UI_WIDGET_FLAG_DRAW_BACKGROUND |
                                        UI_WIDGET_FLAG_DRAW_BORDER     |
                                        UI_WIDGET_FLAG_CLICKABLE       |
                                        UI_WIDGET_FLAG_VIEW_SCROLL);
        //- Decide on semantic size
    widget->semantic_size[0].type = UI_SIZE_TYPE_PIXELS;
    widget->semantic_size[0].strictness = 1;
    widget->semantic_size[0].value = 200;
    widget->semantic_size[1].type = UI_SIZE_TYPE_PIXELS;
    widget->semantic_size[1].strictness = 1;
    widget->semantic_size[1].value = 400;

        //- Panel Child Nodes
    ui_push_layout(ctx, get_layout_horizontal());
    UI_Interaction drag_button = ui_button(ctx, "");
    if (drag_button.dragging) {

    }
    ui_pop_layout(ctx);
}

void ui_empty(UI_CTX *ctx, Rect rect) {
    UI_Widget *widget = add_widget(ctx, UI_WIDGET_FLAG_NULL);
    widget->rect = rect;
}