#include "input.h"

Vec2i get_mouse_pos(bool *lpressed, bool *rpressed) {
    int x, y;
    u32 state = SDL_GetMouseState(&x, &y);
    if (lpressed != NULL) {
        *lpressed = false;
        if (state & SDL_BUTTON_LEFT) *lpressed = true;
    }
    if (rpressed != NULL) {
        *rpressed = false;
        if (state & SDL_BUTTON_RIGHT) *rpressed = true;
    }
    return (Vec2i){x, y};
}

bool input_is_key_pressed(App *app, i32 sdl_scancode) {
    if (app->keyboard_pressed && app->keyboard[sdl_scancode]) return true;
    return false;
}