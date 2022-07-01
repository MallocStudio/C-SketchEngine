#ifndef SE_UI2_H
#define SE_UI2_H

/// Inspired by https://ryanfleury.substack.com/p/ui-part-1-the-interaction-medium UI series

#include "serenderer2D.h"
#include "seinput.h"
#include "setext.h"

#define UI_ICON_INDEX_NULL       (Vec2) {0, 0}
#define UI_ICON_INDEX_COLLAPSE   (Vec2) {1, 0}
#define UI_ICON_INDEX_UNCOLLAPSE (Vec2) {2, 0}
#define UI_ICON_INDEX_CLOSE      (Vec2) {0, 1}
#define UI_ICON_INDEX_SLIDER     (Vec2) {3, 0}
#define UI_ICON_INDEX_CIRCLE_FILLED (Vec2) {1, 1}
#define UI_ICON_INDEX_CIRCLE_EMPTY (Vec2) {1, 2}
#define UI_ICON_INDEX_CHECKBOX_FILLED (Vec2) {3, 2}
#define UI_ICON_INDEX_CHECKBOX_EMPTY (Vec2) {2, 2}
#define UI_ICON_INDEX_ARROW_RIGHT (Vec2) {2, 1}
#define UI_ICON_INDEX_ARROW_LEFT (Vec2) {3, 1}

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

typedef enum UI_SIZE_TYPES {
    UI_SIZE_TYPE_NULL,
    UI_SIZE_TYPE_PIXELS,                // Allows us to encode a direct size in pixels.
    UI_SIZE_TYPE_TEXT,                  // Allows us to encode that we’d like the size to be determined by the required
                                        // dimensions to display whatever string is attached to the widget in question.
    UI_SIZE_TYPE_PRECENTAGE_OF_PARENT,  // Allows us to encode that we’d like a certain percentage value of the parent
                                        // widget’s size on the same axis.
    UI_SIZE_TYPE_CHILDREN_SUM           // Allows us to encode that the size on a given axis should be computed by summing
                                        // the sizes of children widgets when they are laid out in order.
} UI_SIZE_TYPES;

typedef struct UI_Size {
    UI_SIZE_TYPES type;
    f32 value;
    f32 strictness;     // How much we accept value. [0-1] range
} UI_Size;

typedef enum UI_AXIS2 {
  UI_AXIS2_X,
  UI_AXIS2_Y,
  UI_AXIS2_COUNT
} UI_AXIS2;

typedef struct UI_Layout {
    b8 advance_x;
    b8 advance_y;
} UI_Layout;

typedef enum UI_WIDGET_FLAGS {
    UI_WIDGET_FLAG_NULL             = (1 << 0),
    UI_WIDGET_FLAG_CLICKABLE        = (1 << 1),
    UI_WIDGET_FLAG_VIEW_SCROLL      = (1 << 2),
    UI_WIDGET_FLAG_DRAW_TEXT        = (1 << 3),
    UI_WIDGET_FLAG_DRAW_BORDER      = (1 << 4),
    UI_WIDGET_FLAG_DRAW_BACKGROUND  = (1 << 5),
    UI_WIDGET_FLAG_DRAW_DROP_SHADOW = (1 << 6),
    UI_WIDGET_FLAG_HOT_ANIMATION    = (1 << 7),
    UI_WIDGET_FLAG_ACTIVE_ANIMATION = (1 << 8),
} UI_WIDGET_FLAGS;

typedef struct UI_Theme {
    RGBA colour_bg;
    RGBA colour_button_normal;
    RGBA colour_button_highlight;
    RGBA colour_button_dim;
    RGBA colour_outline;
    Vec3 colour_text;
    b8   alignment_centered;
} UI_Theme;

typedef struct UI_Interaction {
    struct UI_Widget *widget; // not owned
    Vec2 mouse;
    Vec2 drag_delta;
    b8 dragging;
    b8 clicked;     // mouse button just went up
    b8 pressed;     // mouse button is down
    b8 hovering;
} UI_Interaction;

#define UI_WIDGET_MAX_CHILDREN 1000
typedef struct UI_Widget {
        //- Per-frame info provided by builders
    UI_WIDGET_FLAGS flags;
    UI_Size semantic_size[UI_AXIS2_COUNT];
    SE_String text;

        //- Recomputed every frame
    f32 computed_rel_position[UI_AXIS2_COUNT];  // The computed position relative to the parent position.
    f32 computed_size[UI_AXIS2_COUNT];          // The computed size in pixels
    Rect rect;                                  // The final on-screen rectangular coordinates
    f32 depth;

        //- Persistent data
    f32 hot_transition;
    f32 active_transition;

        //- Appearance
    UI_Layout layout;
    UI_Interaction recorded_interaction;

        //- Heirarchy
    struct UI_Widget *parent;
    u32 child_count;
    struct UI_Widget *children[UI_WIDGET_MAX_CHILDREN];
} UI_Widget;
se_array_struct(UI_Widget);

#define UI_MAX_WIDGETS 1000
#define UI_MAX_LAYOUTS 1000
typedef struct UI_CTX {
        //- Needs for functionality
    SE_Renderer2D renderer;
    SE_Text txt_renderer;
    SE_Input *input; // not owned
    Rect viewport;
    SE_Texture_Atlas icon_atlas;

        //- Widget Information
    UI_Widget *root;
    UI_Widget *last;

    u32 parent_stack_count;
    UI_Widget *parent_stack[UI_MAX_WIDGETS];

    u32 widgets_count;
    UI_Widget widgets[UI_MAX_WIDGETS];

    u32 layout_stack_count;
    UI_Layout layout_stack[UI_MAX_LAYOUTS];

    UI_Theme theme;
} UI_CTX;

void ui_init(UI_CTX *ctx, SE_Input *input, Rect viewport);
void ui_deinit(UI_CTX *ctx);
void ui_render(UI_CTX *ctx);
void ui_reset(UI_CTX *ctx);
void ui_resize(UI_CTX *ctx, Rect viewport);

///
/// Layout
///

void ui_push_layout(UI_CTX *ctx, UI_Layout layout);
void ui_pop_layout(UI_CTX *ctx);
void ui_layout_horizontal(UI_CTX *ctx, Vec2 anchor);

UI_Theme ui_default_theme();

///
/// Widgets
///

UI_Interaction ui_button(UI_CTX *ctx, const char *string);
UI_Interaction ui_button_at(UI_CTX *ctx, const char *string, Rect rect);

UI_Interaction ui_button_icon(UI_CTX *ctx, const char *string, Vec2 icon);
UI_Interaction ui_button_icon_at(UI_CTX *ctx, const char *string, Vec2 icon, Rect rect);

void ui_checkbox(UI_CTX *ctx, b8 *value);
void ui_checkbox_at(UI_CTX *ctx, b8 *value, Rect rect);

void ui_slider(UI_CTX *ctx, f32 min, f32 max, f32 *value);
void ui_slider_at(UI_CTX *ctx, f32 min, f32 max, f32 *value, Rect rect);

void ui_label(UI_CTX *ctx, const char *string);
void ui_label_at(UI_CTX *ctx, const char *string, Rect rect);

void ui_text_input(UI_CTX *ctx, SE_String *text);
void ui_text_input_at(UI_CTX *ctx, SE_String *text, Rect rect);

void ui_panel(UI_CTX *ctx);
void ui_empty(UI_CTX *ctx, Rect rect);

void ui_panel_begin(UI_CTX *ctx);
void ui_panel_end(UI_CTX *ctx);

#endif // SE_UI2_H