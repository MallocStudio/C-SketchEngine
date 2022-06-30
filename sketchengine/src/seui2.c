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

    ctx->root = NULL;
}

void ui_deinit(UI_CTX *ctx) {
    serender2d_deinit(&ctx->renderer);
    se_deinit_text(&ctx->txt_renderer);
}

static void render_widget(UI_CTX *ctx, UI_Widget *widget) {
    f32 bg_depth = widget->depth + 0; // background
    f32 mg_depth = widget->depth + 1; // middle ground
    f32 fg_depth = widget->depth + 2; // foreground

        //- Draw Background
    serender2d_add_rect(&ctx->renderer, widget->rect, bg_depth, widget->background);

        //- Draw Foreground
    serender2d_add_rect_outline(&ctx->renderer, widget->rect, fg_depth, widget->foreground, 1);

        //- Draw Text
    if (widget->text.buffer != NULL) {
        ctx->txt_renderer.config_colour = v3f(widget->text_colour.r, widget->text_colour.g, widget->text_colour.b);
        ctx->txt_renderer.config_centered = widget->centered;
        se_add_text_rect(&ctx->txt_renderer, widget->text.buffer, widget->rect, mg_depth);
    }

        //- Draw Child
    if (widget->next) {
        render_widget(ctx, widget->next);
    }
}

static void calculate_widget_rect(UI_CTX *ctx, UI_Widget *widget) {
    UI_Layout layout = widget->layout;
    f32 x, y;
    if (widget->parent) {
        layout = widget->parent->layout;
        x = widget->parent->child_count * layout.advance_x * layout.min_size.x + layout.anchor.x;
        y = widget->parent->child_count * layout.advance_y * layout.min_size.y + layout.anchor.y;
    } else {
        x = layout.advance_x * layout.min_size.x + layout.anchor.x;
        y = layout.advance_y * layout.min_size.y + layout.anchor.y;
    }

    f32 w = layout.min_size.x;
    f32 h = layout.min_size.y;

    widget->rect = (Rect) {x, y, w, h};

    widget->background = RGBA_RED;
    widget->foreground = RGBA_BLACK;
    widget->text_colour = RGBA_GREEN;
    widget->centered = true;

    if (widget->next) {
        calculate_widget_rect(ctx, widget->next);
    }
}

void ui_render(UI_CTX *ctx) {
    if (ctx->root) {
        calculate_widget_rect(ctx, ctx->root);
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

void ui_reset(UI_CTX *ctx) {
    ctx->widgets_count = 0;
    ctx->layout_stack_count = 0;
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

UI_Layout get_layout_horizontal(Vec2 anchor) {
    UI_Layout layout;
    layout.advance_x = false;
    layout.advance_y = true;
    layout.anchor = anchor;
    layout.min_size = v2f(64, 32);
    return layout;
}

void ui_layout_horizontal(UI_CTX *ctx, Vec2 anchor) {
    ui_push_layout(ctx, get_layout_horizontal(anchor));
}

///
/// Widgets
///

static UI_Layout get_layout(UI_CTX *ctx) {
    if (ctx->layout_stack_count > 0) {
        return ctx->layout_stack[ctx->layout_stack_count - 1];
    } else {
        return get_layout_horizontal(v2f(0, 0)); // default layout
    }
}

static UI_Widget* add_widget(UI_CTX *ctx) {
    se_assert(ctx->widgets_count < UI_MAX_WIDGETS);
    UI_Widget *result = &ctx->widgets[ctx->widgets_count];

    if (ctx->widgets_count == 0) ctx->root = result;
    ctx->last = result;

    ctx->widgets_count++;
    return result;
}

b8 ui_button(UI_CTX *ctx, const char *string) {
    UI_Widget *widget = add_widget(ctx);
    sestring_init(&widget->text, string);
    widget->layout = get_layout(ctx);
    return false;
}