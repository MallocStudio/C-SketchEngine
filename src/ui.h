#ifndef UI_H
#define UI_H

#include "core.h"

typedef enum UI_STATES {
    UI_STATES_NORMAL, UI_STATES_SELECTED, UI_STATES_PRESSED, UI_STATES_DISABLED,
} UI_STATES;

typedef enum UI_LAYOUTS {
    UI_LAYOUT_HORIZONTAL, UI_LAYOUT_VERTICAL,
} UI_LAYOUTS;

// typedef struct UI_ID {
//     i32 id;
// } UI_ID;
typedef i32 UI_ID;
// #define UI_LAYOUT_FLAG_HORIZONTAL (1 << 0)
// #define UI_LAYOUT_FLAG_EXPAND     (1 << 2)
typedef struct UI_Context {
    UI_ID hot;    // the item is about to be interacted with (eg mouse over)
    UI_ID active; // the currently active item
    Renderer *renderer;     // the renderer to use for rendering
    Vec2i mouse_pos;        // the mouse position
    bool  is_mouse_left_pressed;
    bool  is_mouse_right_pressed;
    UI_Theme *theme;

    // -- related to rendering other widgets (positioning)
    Rect window_rect;
    Rect view_rect;
    Vec2i used_area;
    i32 at_y;
    i32 at_x;
    i32 at_w;
    i32 at_h;
} UI_Context;
void ui_update_context(UI_Context *context);
/// Advance the UI_Context.current_new_item_rect attributes based on the latest widget rect
// void ui_context_increase_advance_by(UI_Context *ctx);

/// render a panel for other widgets. returns true if it's been successfully drawn. if the number_of_items is greater than zero, the items will be expanded
bool ui_begin(UI_Context *ctx, Rect rect);

/// start a new row
void ui_row(UI_Context *ctx, i32 number_of_items, i32 height);

/// render a button using context. returns true if the button is pressed
bool ui_button(UI_Context *ctx, UI_ID id, const char *string);

// /// render a panel
// void ui_panel(UI_Context *ctx, i32 number_of_items, UI_LAYOUTS layout);

// /// render a label using context.
// void ui_label(UI_Context *ctx, const char *title);

// /// render a rearrangable rect
// void ui_render_floating_rect(UI_Context *context, Rect *rect);

#endif // UI_H