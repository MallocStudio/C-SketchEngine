#ifndef UI_H
#define UI_H

#include "core.h"

typedef struct UI_Button {
    RGBA current_color; // current color of the button to be rendered
    RGBA target_color;  // for animation transition from current_color to target_color
    Rect rect;          // the position of the button
} UI_Button; void ui_init_button (UI_Button *button);

typedef enum UI_STATES {
    UI_STATES_NORMAL, UI_STATES_SELECTED, UI_STATES_PRESSED, UI_STATES_DISABLED,
} UI_STATES;

/// draw the given button. if it's pressed returns true
bool ui_draw_button(UI_Button *button);
/// return the state of a ui button
UI_STATES ui_get_button_state(UI_Button *button);

#endif // UI_H