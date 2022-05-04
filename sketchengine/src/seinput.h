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

typedef struct SE_Input {
    // -- mouse

    /* the mouse position */
    Vec2 mouse_world_pos;
    Vec2 mouse_screen_pos;
    /* mouse position last time update was called */
    Vec2 previous_mouse_world_pos;
    Vec2 previous_mouse_screen_pos;
    /* delta */
    // @unsupported // @incomplete
    // Vec2 mouse_world_pos_delta;
    Vec2 mouse_screen_pos_delta;
    // the position the mouse was initially pressed
    Vec2 mouse_world_pressed_pos;
    Vec2 mouse_screen_pressed_pos;
    // Vec2 mouse_grab_offset;
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
    bool should_mouse_warp; // @incomplete

    /* text input */
    bool is_text_input_activated; // use seinput_text_input_activate()
    SE_String *text_input_stream; // not owned. When this is not null and text_input is activated, stream to this string
    // u32 text_input_selection_index;
    // u32 text_input_selection_length;
    // u32 text_input_cursor;

    // -- keyboard

    // use SDL_SCANCODE_... to get the state of a key
    // eg: keyboard[SDL_SCANCODE_LEFT] returns true if the left arrow
    // key is down
    Uint8 *keyboard;

    #define SEINPUT_NUMKEYS_MAX SDL_NUM_SCANCODES
    Uint8 keyboard_previous_frame[SEINPUT_NUMKEYS_MAX];

} SE_Input;

/// initialise input once! allocates memory
SEINLINE void seinput_init(SE_Input *input) {
    // set all values to zero to begin with
    memset(input, 0, sizeof(SE_Input));

    // sestring_init(&input->text_input, ""); // @leak
}

/// note that mouse pos will be relative to top left position of window
SEINLINE void seinput_update(SE_Input *input, Mat4 otho_projection_world, SDL_Window *window) {
    Vec2i window_size;
    SDL_GetWindowSize(window, &window_size.x, &window_size.y);

    { // -- keyboard
        if (input->keyboard != NULL) {
            for (i32 i = 0; i < SEINPUT_NUMKEYS_MAX; ++i) {
                input->keyboard_previous_frame[i] = input->keyboard[i];
            }
        }
        i32 numkeys;
        input->keyboard = (Uint8*)SDL_GetKeyboardState(&numkeys);
        SDL_assert(numkeys == SEINPUT_NUMKEYS_MAX); // we assert this, because we need to allocate this much memory for input->keyboard_previous_frame
    }

    // ! don't try this right now, because we update mouse_grab_offset in seui.c "ui_update_mouse_grab_pos()"
    // { // -- update mouse grab pos
    //     if (input->is_mouse_left_down && !input->was_mouse_left_down) { // we just pressed
    //         input->mouse_grab_offset.x = input->mouse_pressed_pos.x;
    //         input->mouse_grab_offset.y = input->mouse_pressed_pos.y;
    //     }
    // }

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

    { // -- mouse warping // @incomplete
        if (input->should_mouse_warp) {
            // Vec2 cursor_pos = get_mouse_pos(NULL, NULL);

            // if (cursor_pos.x > window_size.x) cursor_pos.x = 0;
            // else if (cursor_pos.x < 0) cursor_pos.x = window_size.x;
            // if (cursor_pos.y > window_size.y) cursor_pos.y = 0;
            // else if (cursor_pos.y < 0) cursor_pos.y = window_size.y;

            // SDL_WarpMouseInWindow(window, cursor_pos.x, cursor_pos.y);
        }

        // SDL_SetWindowMouseGrab(window, input->should_mouse_warp);
    }

    { // -- delta
        input->mouse_screen_pos_delta.x = input->mouse_screen_pos.x - input->previous_mouse_screen_pos.x;
        input->mouse_screen_pos_delta.y = input->mouse_screen_pos.y - input->previous_mouse_screen_pos.y;
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
    return (input->keyboard[sdl_scancode] == true && input->keyboard_previous_frame[sdl_scancode] == false);
}

SEINLINE bool seinput_is_key_down(const SE_Input *input, SDL_Scancode sdl_scancode) {
    return input->keyboard[sdl_scancode];
}

SEINLINE void seinput_text_input_activate(SE_Input *input, SE_String *stream_to) {
    input->is_text_input_activated = true;
    input->text_input_stream = stream_to;
    SDL_StartTextInput();
}

SEINLINE void seinput_text_input_deactivate(SE_Input *input) {
    input->is_text_input_activated = false;
    input->text_input_stream = NULL;
    SDL_StopTextInput();
}
#endif // SEINPUT_H