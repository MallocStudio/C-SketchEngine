#ifndef SEINPUT_H
#define SEINPUT_H

#include "sedefines.h"
#include "semath_defines.h"
#include "semath.h"
#include "sestring.h"

/// get the mouse position (relative to the window's top left position). Optionally pass bools to get mouse state
SEINLINE Vec2 get_mouse_pos(bool *lpressed, bool *rpressed) {
    i32 x, y;
    u32 state = SDL_GetMouseState(&x, &y);
    if (lpressed != NULL) {
        *lpressed = false;
        if (state & SDL_BUTTON_LMASK) *lpressed = true;
    }
    if (rpressed != NULL) {
        *rpressed = false;
        if (state & SDL_BUTTON_RMASK) *rpressed = true;
    }
    return (Vec2){x, y};
}

/// get the mouse position (relative to the window's top left position). Optionally pass bools to get mouse state
SEINLINE Vec2 get_mouse_pos_normalised(SDL_Window *window, bool *lpressed, bool *rpressed) {
    i32 x, y;
    u32 state = SDL_GetMouseState(&x, &y);
    if (lpressed != NULL) {
        *lpressed = false;
        if (state & SDL_BUTTON_LMASK) *lpressed = true;
    }
    if (rpressed != NULL) {
        *rpressed = false;
        if (state & SDL_BUTTON_RMASK) *rpressed = true;
    }
    i32 w = 1, h = 1;
    SDL_GetWindowSize(window, &w, &h);
    return (Vec2){x / w, y / h};
}

typedef struct SE_Input {
    // -- mouse

    /* the mouse position */
    Vec2 mouse_world_pos;
    Vec2 mouse_screen_pos;
    /* mouse position last time update was called */
    Vec2 previous_mouse_world_pos;
    Vec2 previous_mouse_screen_pos;
    /* delta */
    Vec2 mouse_screen_pos_delta;
    f32 mouse_wheel; // set through SDL_MOUSEWHEEL event system. Also note that it gets set to zero before polling events at the beginning of every frame
    // the position the mouse was initially pressed
    Vec2 mouse_world_pressed_pos;
    Vec2 mouse_screen_pressed_pos;
    // information from the current frame
    bool is_mouse_left_down;
    bool is_mouse_right_down;
    // information from the previous frame
    bool was_mouse_left_down;
    bool was_mouse_right_down;
    // whethered we want to let others know that we've handled the mouse input
    bool is_mouse_left_handled;
    bool is_mouse_right_handled;
    // warp mouse around window
    SDL_Window *window; // not owned
    bool is_mouse_fps_activated;

    /* text input */
    bool is_text_input_activated; // use seinput_text_input_activate()
    SE_String *text_input_stream; // not owned. When this is not null and text_input is activated, stream to this string
    bool is_text_input_only_numeric;
    // u32 text_input_selection_index;
    // u32 text_input_selection_length;
    // u32 text_input_cursor;

    // -- keyboard

    // use SDL_SCANCODE_... to get the state of a key
    // eg: keyboard[SDL_SCANCODE_LEFT] returns true if the left arrow
    // key is down

    #define SEINPUT_NUMKEYS_MAX SDL_NUM_SCANCODES
    Uint8 keyboard[SEINPUT_NUMKEYS_MAX];
    Uint8 keyboard_previous_frame[SEINPUT_NUMKEYS_MAX];

} SE_Input;

/// initialise input once! allocates memory
SEINLINE void seinput_init(SE_Input *input) {
    // set all values to zero to begin with
    memset(input, 0, sizeof(SE_Input));
}

/// note that mouse pos will be relative to top left position of window
SEINLINE void seinput_update(SE_Input *input, Mat4 otho_projection_world, SDL_Window *window) {
    Vec2i window_size;
    SDL_GetWindowSize(window, &window_size.x, &window_size.y);
    input->window = window;

    { // -- keyboard
        if (input->keyboard != NULL) {
            SDL_memcpy(input->keyboard_previous_frame, input->keyboard, sizeof(Uint8) * SEINPUT_NUMKEYS_MAX);
        }
        i32 numkeys;
        Uint8 *current_keyboard_state = (Uint8*)SDL_GetKeyboardState(&numkeys);
        SDL_memcpy(input->keyboard, current_keyboard_state, sizeof(Uint8) * SEINPUT_NUMKEYS_MAX);
        SDL_assert(numkeys == SEINPUT_NUMKEYS_MAX); // we assert this, because we need to allocate this much memory for input->keyboard_previous_frame
    }

    { // -- remember what happened the previous frame
        // pos
        input->previous_mouse_screen_pos = input->mouse_screen_pos;
        input->previous_mouse_world_pos = input->mouse_world_pos;
        // pressed
        input->was_mouse_left_down = input->is_mouse_left_down;
        input->was_mouse_right_down = input->is_mouse_right_down;
        // update for this frame
        get_mouse_pos(&input->is_mouse_left_down, &input->is_mouse_right_down);
    }

    { // reset
        if (!input->is_mouse_left_down) {
            input->is_mouse_left_handled = false;
        }

        if (!input->is_mouse_right_down) {
            input->is_mouse_right_handled = false;
        }
    }

    { // -- world pos
        Mat4 deprojection_world = mat4_inverse(otho_projection_world);
        Vec2 cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / window_size.x) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / window_size.y) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_world = mat4_mul_vec4(deprojection_world, mouse_pos_ndc);
        cursor_pos.x = mouse_pos_world.x;
        cursor_pos.y = mouse_pos_world.y;
        input->mouse_world_pos = cursor_pos;

        if (input->is_mouse_left_down && !input->was_mouse_left_down) {
            input->mouse_world_pressed_pos = input->mouse_world_pos;
        }
    }

    { // -- screen pos
        Mat4 deprojection_screen = mat4_inverse(viewport_to_ortho_projection_matrix(
            (Rect) {0, 0, window_size.x, window_size.y}
        ));

        Vec2 cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / window_size.x) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / window_size.y) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_screen = mat4_mul_vec4(deprojection_screen, mouse_pos_ndc);
        cursor_pos.x = mouse_pos_screen.x;
        cursor_pos.y = mouse_pos_screen.y;

        input->mouse_screen_pos = cursor_pos;

        if (input->is_mouse_left_down && !input->was_mouse_left_down) {
            input->mouse_screen_pressed_pos = input->mouse_screen_pos;
        }
    }

    { // -- delta
        Vec2i delta;
        SDL_GetRelativeMouseState(&delta.x, &delta.y);
        input->mouse_screen_pos_delta.x = delta.x;
        input->mouse_screen_pos_delta.y = -delta.y; // in sketchengine up is positive y
    }
}

SEINLINE bool seinput_is_mouse_left_pressed (const SE_Input *input) {
    return (input->is_mouse_left_handled == false
            && input->is_mouse_left_down
            && !input->was_mouse_left_down);
}

SEINLINE bool seinput_is_mouse_left_released (const SE_Input *input) {
    return (input->was_mouse_left_down && !input->is_mouse_left_down);
}

SEINLINE bool seinput_is_mouse_right_pressed (const SE_Input *input) {
    return (input->is_mouse_right_handled == false
            && input->is_mouse_right_down
            && !input->was_mouse_right_down);
}

SEINLINE bool seinput_is_mouse_right_released (const SE_Input *input) {
    return (input->was_mouse_right_down && !input->is_mouse_right_down);
}

SEINLINE bool seinput_is_key_pressed(const SE_Input *input, SDL_Scancode sdl_scancode) {
    return (input->keyboard[sdl_scancode] == 1 && input->keyboard_previous_frame[sdl_scancode] == 0);
}

SEINLINE bool seinput_is_key_down(const SE_Input *input, SDL_Scancode sdl_scancode) {
    return input->keyboard[sdl_scancode] == 1;
}

SEINLINE void seinput_text_input_activate(SE_Input *input, SE_String *stream_to, bool only_numeric) {
    if (stream_to != NULL) {
        input->is_text_input_activated = true;
        input->text_input_stream = stream_to;
        input->is_text_input_only_numeric = only_numeric;
        SDL_StartTextInput();
    }
}

SEINLINE void seinput_text_input_deactivate(SE_Input *input) {
    input->is_text_input_activated = false;
    input->text_input_stream = NULL;
    SDL_StopTextInput();
}

SEINLINE void seui_mouse_fps_activate(SE_Input *input) {
    if (input->is_mouse_fps_activated) return;

    input->is_mouse_fps_activated = true;
    // SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1"); // for some reason this causes jitters sometimes when used instead of below:
    SDL_SetRelativeMouseMode(true);
    // and by below I mean this:
    Vec2i window_size;
    SDL_GetWindowSize(input->window, &window_size.x, &window_size.y);
    SDL_WarpMouseInWindow(input->window, window_size.x * 0.5f, window_size.y * 0.5f);
}

SEINLINE void seui_mouse_fps_deactivate(SE_Input *input) {
    if (!input->is_mouse_fps_activated) return;
    input->is_mouse_fps_activated = false;
    SDL_SetRelativeMouseMode(false);
}
#endif // SEINPUT_H