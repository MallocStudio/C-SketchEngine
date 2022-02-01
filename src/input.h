#ifndef INPUT_H
#define INPUT_H

#include "core.h"

/// get the mouse position (relative to the window). Optionally pass bools to get mouse state
Vec2i get_mouse_pos(bool *lpressed, bool *rpressed);

/// returns true if the given sdl scancode has been pressed.
bool input_is_key_pressed(App *app, i32 sdl_scancode);
#endif // INPUT_H