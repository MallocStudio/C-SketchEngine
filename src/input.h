#ifndef INPUT_H
#define INPUT_H

#include "core.h"

/// get the mouse position. Optionally pass bools to get mouse state
Vec2i get_mouse_pos(bool *lpressed, bool *rpressed);

#endif // INPUT_H