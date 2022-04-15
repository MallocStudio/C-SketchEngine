#ifndef SEUI_H
#define SEUI_H

#include "seui_renderer.h"
#include "seinput.h"
#include "setext.h"

typedef struct SE_Theme {
    RGBA colour_normal;
    RGBA colour_hover;
    RGBA colour_pressed;
    RGBA colour_bg; // background
    RGBA colour_fg; // foreground
} SE_Theme;
SEINLINE void seui_theme_default(SE_Theme *theme) {
    theme->colour_normal  = (RGBA) {65, 84, 105, 255};
    theme->colour_hover   = (RGBA) {108, 145, 173, 255};
    theme->colour_pressed = (RGBA) {43, 56, 71, 255};
    theme->colour_bg      = (RGBA) {33, 39, 43, 200};
    theme->colour_fg      = (RGBA) {56, 95, 161, 255};
}

typedef enum UI_STATES {
    UI_STATE_DISABLED, // this UI element is unusable but is rendererd
    UI_STATE_IDLE,     // this UI element is usable and is rendered
    UI_STATE_WARM,     // about to be selected (hover, selection)
    UI_STATE_HOT,      // this UI element is being used (pressed, a text editor that might be active)
    UI_STATE_ACTIVE,   // this UI element must execute (mouse released on a button...)
} UI_STATES;

#define SEUI_ID_NULL 0

// #define SEUI_VIEW_REGION_PADDING 0.1f
#define SEUI_VIEW_REGION_SIZE_X 0.25f
#define SEUI_VIEW_REGION_SIZE_Y 0.75f
// #define SEUI_VIEW_REGION_CENTER (Rect) {SEUI_VIEW_REGION_PADDING, SEUI_VIEW_REGION_PADDING, 1 - SEUI_VIEW_REGION_SIZE, SEUI_VIEW_REGION_PADDING * 2}
#define SEUI_VIEW_REGION_RIGHT  (Rect) {1 - SEUI_VIEW_REGION_SIZE_X, (1 - SEUI_VIEW_REGION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_SIZE_X, SEUI_VIEW_REGION_SIZE_Y}
#define SEUI_VIEW_REGION_LEFT   (Rect) {0, (1 - SEUI_VIEW_REGION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_SIZE_X, SEUI_VIEW_REGION_SIZE_Y}

typedef struct SE_UI {
    /* UI Widgets */
    u32 warm; // hover / selection
    u32 hot;  // pressed / active
    u32 max_id; // the maximum generated id

    /* Renderes and Inputs */
    struct UI_Renderer renderer;
    SE_Text_Renderer txt_renderer;
    struct SE_Input *input; // ! not owned
    SE_Theme theme;

    /* Panels */
    u32 current_panel;         // the panel id
    Rect current_panel_rect;   // the rect of the panel
    u32 current_panel_columns; // number of columns
    Vec2 current_panel_cursor; // the relative cursor used to position the placement of the items
    f32 current_panel_item_height;
} SE_UI;

/// call this at the beginning of every frame before creating other widgets
SEINLINE void seui_reset(SE_UI *ctx) {
    ctx->max_id = SEUI_ID_NULL;
    ctx->current_panel = SEUI_ID_NULL;
    ctx->current_panel_rect = (Rect) {0};
    ctx->current_panel_columns = 0;
    ctx->current_panel_cursor = (Vec2) {0};
    ctx->current_panel_item_height = 0;
}

SEINLINE void seui_init(SE_UI *ctx, SE_Input *input, u32 window_w, u32 window_h) {
    ctx->warm = SEUI_ID_NULL;
    ctx->hot = SEUI_ID_NULL;
    seui_reset(ctx);
    ctx->input = input;
    seui_renderer_init(&ctx->renderer, "shaders/UI.vsd", "shaders/UI.fsd", window_w, window_h);
    setext_init (&ctx->txt_renderer, (Rect) {0, 0, window_w, window_h});
    seui_theme_default(&ctx->theme);
}

SEINLINE void seui_deinit(SE_UI *ctx) {
    seui_renderer_deinit(&ctx->renderer);
    setext_deinit(&ctx->txt_renderer);
}

SEINLINE void seui_render(SE_UI *ctx) {
    seui_renderer_upload(&ctx->renderer);
    seui_renderer_draw  (&ctx->renderer);
    seui_renderer_clear (&ctx->renderer);
    setext_render(&ctx->txt_renderer);
}

/// Start a panel at the given position. Aligns the items inside of the panel
/// based on the given number of columns.
/// Returns true if the panel is not minimised.
bool seui_panel_at(SE_UI *ctx, const char *title, u32 columns, f32 item_height, Rect *initial_rect, bool *minimised);

/// Draws a button but figures out the position and the rect based on the current
/// context and panel.
bool seui_button(SE_UI *ctx, const char *text);
void seui_label(SE_UI *ctx, const char *text);

/// Draws a button at the given rectangle.
bool seui_button_at(SE_UI *ctx, const char *text, Rect rect);

/// Draws a button that returns the drag if the mouse is trying to drag it.
/// That means is the mouse is hovering over the button and pressing down.
Vec2 seui_drag_button_at(SE_UI *ctx, Rect rect, UI_STATES *state);

/// Draws a button that returns the drag if the mouse is trying to drag it.
/// That means is the mouse is hovering over the button and pressing down.
/// If the texture_index is (Vec2) {0} it will draw a simple rectangle.
Vec2 seui_drag_button_textured_at(SE_UI *ctx, Rect rect, Vec2 texture_index, UI_STATES *state);

void seui_label_at(SE_UI *ctx, const char *text, Rect rect);

/// value is clamped between 0 - 1
void seui_slider_at(SE_UI *ctx, Vec2 pos1, Vec2 pos2, f32 *value);
void seui_slider(SE_UI *ctx, f32 *value);

/// a 2d slider that returns a normalised vec2
void seui_slider2d_at(SE_UI *ctx, Vec2 center, f32 radius, Vec2 *value);
void seui_slider2d(SE_UI *ctx, Vec2 *value);
#endif // SEUI_H