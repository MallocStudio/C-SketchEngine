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
#define SEUI_VIEW_REGION_SIZE_X 0.2f
#define SEUI_VIEW_REGION_SIZE_Y 0.9f
#define SEUI_VIEW_REGION_COLLISION_SIZE_X 0.01f
#define SEUI_VIEW_REGION_COLLISION_SIZE_Y SEUI_VIEW_REGION_SIZE_Y
// #define SEUI_VIEW_REGION_CENTER (Rect) {SEUI_VIEW_REGION_PADDING, SEUI_VIEW_REGION_PADDING, 1 - SEUI_VIEW_REGION_SIZE, SEUI_VIEW_REGION_PADDING * 2}
#define SEUI_VIEW_REGION_RIGHT  (Rect) {1 - SEUI_VIEW_REGION_SIZE_X, (1 - SEUI_VIEW_REGION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_SIZE_X, SEUI_VIEW_REGION_SIZE_Y}
#define SEUI_VIEW_REGION_LEFT   (Rect) {0, (1 - SEUI_VIEW_REGION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_SIZE_X, SEUI_VIEW_REGION_SIZE_Y}

#define SEUI_VIEW_REGION_COLLISION_RIGHT  (Rect) {1 - SEUI_VIEW_REGION_COLLISION_SIZE_X, (1 - SEUI_VIEW_REGION_COLLISION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_COLLISION_SIZE_X, SEUI_VIEW_REGION_COLLISION_SIZE_Y}
#define SEUI_VIEW_REGION_COLLISION_LEFT   (Rect) {0, (1 - SEUI_VIEW_REGION_COLLISION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_COLLISION_SIZE_X, SEUI_VIEW_REGION_COLLISION_SIZE_Y}

typedef struct SEUI_Panel {
    /* settings */
    Rect rect;
    bool minimised;
    f32 min_item_height;

    /* auto calculated */
    Rect final_rect;   // the rect of the panel
    f32 item_height;
    i32 item_count;
    Vec2 min_size;
    Vec2 cursor; // the relative cursor used to position the placement of the items
    u32 columns; // number of columns -> calculate by panel_row(number_of_columns)
    bool is_embedded; // is inside of another panel

    u32 docked_dir; // 0 means not docked, 1 means left, 2 means right
} SEUI_Panel;

SEINLINE void seui_panel_configure(SEUI_Panel *panel, Rect initial_rect, bool minimised, f32 min_item_height) {
    panel->rect = initial_rect;
    panel->minimised = minimised;
    panel->columns = 1;
    panel->min_item_height = min_item_height;
    panel->is_embedded = false;
    panel->docked_dir = 0; // not docked
}

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
    SEUI_Panel *current_panel;         // the panel we put the widgets on

    /* text input */
    // if some widget is constantly active, we set active to their id
    // this is to allow certain widgets hold the attention of input.
    // For example this is used for text input. If we press on text input,
    // active will be set to that ID, and until the user clicks somewhere else
    // the active ID will remain the same. (This case is handled inside of seui_input_text_at())
    u32 active; // being used (used for text input)
    SE_String text_input_cache;   // used for all sorts of stuff. For example editing floating point values
    SE_String text_input; // store a copy of what's being typed into the active text input
    bool text_input_only_numerical;
} SE_UI;

/// call this at the beginning of every frame before creating other widgets
SEINLINE void seui_reset(SE_UI *ctx) {
    ctx->max_id = SEUI_ID_NULL;
    ctx->current_panel = NULL;
}

SEINLINE void seui_resize(SE_UI *ctx, u32 window_w, u32 window_h) {
    seui_renderer_resize(&ctx->renderer, window_w, window_h);
    setext_set_viewport(&ctx->txt_renderer, (Rect) {0, 0, window_w, window_h});
}

SEINLINE void seui_init(SE_UI *ctx, SE_Input *input, u32 window_w, u32 window_h) {
    ctx->warm = SEUI_ID_NULL;
    ctx->hot = SEUI_ID_NULL;
    ctx->active = SEUI_ID_NULL;
    seui_reset(ctx);
    ctx->input = input;
    seui_renderer_init(&ctx->renderer, "shaders/UI.vsd", "shaders/UI.fsd", window_w, window_h);
    setext_init (&ctx->txt_renderer, (Rect) {0, 0, window_w, window_h});
    seui_theme_default(&ctx->theme);

    sestring_init(&ctx->text_input_cache, "");
    sestring_init(&ctx->text_input, "");
}

SEINLINE void seui_deinit(SE_UI *ctx) {
    seui_renderer_deinit(&ctx->renderer);
    setext_deinit(&ctx->txt_renderer);
    sestring_deinit(&ctx->text_input_cache);
    sestring_deinit(&ctx->text_input);
}

SEINLINE void seui_render(SE_UI *ctx) {
    seui_renderer_upload(&ctx->renderer);
    glDisable(GL_CULL_FACE);
    seui_renderer_draw(&ctx->renderer);
    setext_render(&ctx->txt_renderer);
    glEnable(GL_CULL_FACE);
    seui_renderer_clear (&ctx->renderer);
}

/// Set text input configurations to default values. It's good practice to
/// do call this procedure after modifying the configuration for a usecase.
SEINLINE void seui_configure_text_input_reset(SE_UI *ctx) {
    ctx->text_input_only_numerical = false;
}

/// Start a panel at the given position. Aligns the items inside of the panel
/// based on the given number of columns.
/// Returns true if the panel is not minimised.
bool seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel_data);
bool seui_panel(SE_UI *ctx, const char *title, SEUI_Panel *panel_data);

void seui_panel_row(SEUI_Panel *panel, f32 num_of_columns);

void seui_label_at(SE_UI *ctx, const char *text, Rect rect);
void seui_label(SE_UI *ctx, const char *text);
void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, bool editable);

/// Draws a button but figures out the position and the rect based on the current
/// context and panel.
bool seui_button(SE_UI *ctx, const char *text);
/// Draws a button at the given rectangle.
bool seui_button_at(SE_UI *ctx, const char *text, Rect rect);

bool seui_button_textured(SE_UI *ctx, Vec2 texture_index);
bool seui_button_textured_at(SE_UI *ctx, Vec2 texture_index, Rect rect);

/// Draws a button that returns the drag if the mouse is trying to drag it.
/// That means is the mouse is hovering over the button and pressing down.
Vec2 seui_drag_button_at(SE_UI *ctx, Rect rect, UI_STATES *state);

/// Draws a button that returns the drag if the mouse is trying to drag it.
/// That means is the mouse is hovering over the button and pressing down.
/// If the texture_index is (Vec2) {0} it will draw a simple rectangle.
Vec2 seui_drag_button_textured_at(SE_UI *ctx, Rect rect, Vec2 texture_index, UI_STATES *state);

/// value is clamped between 0 - 1
void seui_slider_at(SE_UI *ctx, Vec2 pos1, Vec2 pos2, f32 *value);
void seui_slider(SE_UI *ctx, f32 *value);

void seui_colour_picker_at(SE_UI *ctx, Rect rect, RGBA *value);
void seui_colour_picker(SE_UI *ctx, RGBA *value);

/// a 2d slider that returns a normalised vec2
void seui_slider2d_at(SE_UI *ctx, Vec2 center, f32 radius, Vec2 *value);
void seui_slider2d(SE_UI *ctx, Vec2 *value);

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect);
void seui_input_text(SE_UI *ctx, SE_String *text);

/// A horizontal slider with buttons on its sides.
/// Within the range of [min, max]
/// If min AND max are zero, the limits will be ignored
bool seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max);
bool seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max);
#endif // SEUI_H