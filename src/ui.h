#ifndef UI_H
#define UI_H

#include "core.h"

typedef struct UI_Theme {
    RGBA color_base;
    RGBA color_pressed;
    RGBA color_selected;
    RGBA color_disabled;
    f32  color_transition_amount;
} UI_Theme; void ui_init_theme (UI_Theme *theme);
UI_Theme *global_ui_theme;

typedef struct UI_Button {
    RGBA color_current; // current color of the ui item to be rendered
    RGBA color_target;  // for animation transition from current_color to target_color
    Rect rect;          // the position of the button
} UI_Button; void ui_init_button (UI_Button *button, UI_Theme *theme);

typedef enum UI_STATES {
    UI_STATES_NORMAL, UI_STATES_SELECTED, UI_STATES_PRESSED, UI_STATES_DISABLED,
} UI_STATES;

/// draw the given button. if it's pressed returns true
bool ui_draw_button(UI_Button *button, UI_Theme *theme);
/// return the state of a ui button
UI_STATES ui_get_button_state(UI_Button *button);

#endif // UI_H