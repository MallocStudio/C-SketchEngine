#ifndef UI_H
#define UI_H

#include "core.h"

typedef enum UI_STATES {
    UI_STATES_NORMAL, UI_STATES_SELECTED, UI_STATES_PRESSED, UI_STATES_DISABLED,
} UI_STATES;

typedef enum UI_LAYOUTS {
    UI_LAYOUT_HORIZONTAL, UI_LAYOUT_VERTICAL,
} UI_LAYOUTS;

typedef struct UI_Context {
    Renderer *renderer;     // the renderer to use for rendering
    Vec2i mouse_pos;        // the mouse position
    bool  is_mouse_busy;    // is the user interacting with another ui?
    bool  is_mouse_left_pressed;
    bool  is_mouse_right_pressed;
    UI_Theme *theme;
    UI_LAYOUTS layout;

    // -- related to rendering other widgets (positioning)
    Rect current_rect; // use to figure out where to position a widget. must manually advance these parameters after drawing the widget
} UI_Context;
void ui_update_context(UI_Context *context);
/// Advance the UI_Context.current_rect attributes based on the latest widget rect
void ui_advance_context(UI_Context *ctx, Rect current_widget_rect);

/// render a panel for other widgets. returns true if it's been successfully drawn 
bool ui_begin_horizontal(UI_Context *ctx, const char *title, Rect rect); 
bool ui_begin_vertical(UI_Context *ctx, const char *title, Rect rect);
// bool ui_begin_horizontal_expanded(UI_Context *ctx, const char *title, Rect rect, i32 number_of_items); 
// bool ui_begin_vertical_expanded(UI_Context *ctx, const char *title,   Rect rect, i32 number_of_items);

/// render a label using context.
void ui_label(UI_Context *ctx, const char *title);

/// render a button using context. returns true if the button is pressed
bool ui_button(UI_Context *ctx, const char *string);

/// draw a simple button. returns true if the button is pressed
bool ui_draw_quick_button(UI_Context *ui_context, Rect rect, const char *text, UI_Theme *theme);

/// render a rearrangable rect
void ui_render_floating_rect(UI_Context *context, Rect *rect);

#endif // UI_H