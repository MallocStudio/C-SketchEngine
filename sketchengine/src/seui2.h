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

typedef struct UI_Widget {
    UI_Size semantic_size[UI_AXIS2_COUNT];

        //- Recomputed every frame
    f32 computed_rel_position[UI_AXIS2_COUNT];  // The computed position relative to the parent position.
    f32 computed_size[UI_AXIS2_COUNT];          // The computed size in pixels
    Rect rect;                                  // The final on-screen rectangular coordinates

        //- Heirarchy
    UI_Widget *first;
    UI_Widget *last;
    UI_Widget *next;
    UI_Widget *prev;
    UI_Widget *parent;
} UI_Widget;
se_array_struct(UI_Widget);

typedef struct UI_CTX {
        //- Needs for functionality
    SE_Renderer2D renderer;
    SE_Text txt_renderer;
    SE_Input *input; // not owned
    Rect viewport;

        //- Widget Information
    Array(UI_Widget) widgets;
} UI_CTX;

void ui_init(UI_CTX *ctx, SE_Input *input, Rect viewport);
void ui_deinit(UI_CTX *ctx);
void ui_render(UI_CTX *ctx);

///
/// Widgets
///

b8   ui_button(UI_CTX *ctx, const char *string);
b8   ui_button_at(UI_CTX *ctx, const char *string, Rect rect);

b8   ui_button_icon(UI_CTX *ctx, const char *string, Vec2 icon);
b8   ui_button_icon_at(UI_CTX *ctx, const char *string, Vec2 icon, Rect rect);

void ui_checkbox(UI_CTX *ctx, b8 *value);
void ui_checkbox_at(UI_CTX *ctx, b8 *value, Rect rect);

void ui_slider(UI_CTX *ctx, f32 min, f32 max, f32 *value);
void ui_slider_at(UI_CTX *ctx, f32 min, f32 max, f32 *value, Rect rect);

void ui_label(UI_CTX *ctx, const char *string);
void ui_label_at(UI_CTX *ctx, const char *string, Rect rect);

void ui_text_input(UI_CTX *ctx, SE_String *text);
void ui_text_input_at(UI_CTX *ctx, SE_String *text, Rect rect);

void ui_panel_begin(UI_CTX *ctx);
void ui_panel_end(UI_CTX *ctx);

#endif // SE_UI2_H