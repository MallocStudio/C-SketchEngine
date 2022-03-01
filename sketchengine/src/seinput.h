#ifndef SEINPUT_H
#define SEINPUT_H

#include "defines.h"
#include "semath_defines.h"

// /// get the mouse position (relative to the window). Optionally pass bools to get mouse state
// Vec2i get_mouse_pos(bool *lpressed, bool *rpressed);

// /// returns true if the given sdl scancode has been pressed.
// bool input_is_key_pressed(App *app, i32 sdl_scancode); // @incomplete not implemented yet


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

    // the mouse position
    Vec2 mouse_world_pos;
    Vec2 mouse_screen_pos;
    // the position the mouse was initially pressed
    Vec2 mouse_world_pressed_pos;
    Vec2 mouse_screen_pressed_pos;
    // Vec2 mouse_grab_offset;
    // information from the current frame
    bool  is_mouse_left_down;
    bool  is_mouse_right_down;
    // information from the previous frame
    bool  was_mouse_left_down;
    bool  was_mouse_right_down;
    // whethered we want to let others know that we've handled the mouse input
    bool is_mouse_left_handled;
    bool is_mouse_right_handled;

    // -- keyboard
    
    // use SDL_SCANCODE_... to get the state of a key
    // eg: keyboard[SDL_SCANCODE_LEFT] returns true if the left arrow
    // key is down
    Uint8 *keyboard;
    
} SE_Input;

SEINLINE void seinput_reset(SE_Input *input) {
    // input->mouse_pos = (Vec2i) {0};
    // input->mouse_pressed_pos = (Vec2i) {0};
    // // input->mouse_grab_offset = (Vec2) {0};
    // input->is_mouse_left_pressed = false;
    *input = (SE_Input) {0};
}

/// note that mouse pos will be relative to top left position of window
SEINLINE void seinput_update(SE_Input *input, Mat4 otho_projection_world, Vec2i window_size) {
    { // -- keyboard
        input->keyboard = (Uint8*)SDL_GetKeyboardState(NULL);
    }

    // ! don't try this right now, because we update mouse_grab_offset in seui.c "ui_update_mouse_grab_pos()"
    // { // -- update mouse grab pos
    //     if (input->is_mouse_left_down && !input->was_mouse_left_down) { // we just pressed
    //         input->mouse_grab_offset.x = input->mouse_pressed_pos.x;
    //         input->mouse_grab_offset.y = input->mouse_pressed_pos.y;
    //     }
    // }
    // -- remember what happened the previous frame
    input->was_mouse_left_down = input->is_mouse_left_down;
    input->was_mouse_right_down = input->is_mouse_right_down;
    get_mouse_pos(&input->is_mouse_left_down, &input->is_mouse_right_down);

    { // reset
        if (!input->is_mouse_left_down) {
            input->is_mouse_left_handled = false;
        }
        
        if (!input->is_mouse_right_down) {
            input->is_mouse_right_handled = false;
        }
    }

    { // -- world pos
        Mat4 deprojection_world = mat4_transposed(mat4_inverse(otho_projection_world));
        Vec2 cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / window_size.x) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / window_size.y) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_world = mat4_mul_vec4(&deprojection_world, &mouse_pos_ndc);
        cursor_pos.x = mouse_pos_world.x;
        cursor_pos.y = mouse_pos_world.y;
        input->mouse_world_pos = cursor_pos;

        if (input->is_mouse_left_down && !input->was_mouse_left_down) {
            input->mouse_world_pressed_pos = input->mouse_world_pos;
        }
    }

    { // -- screen pos
        Mat4 deprojection_screen = mat4_transposed(mat4_inverse(viewport_to_ortho_projection_matrix(
            (Rect) {0, 0, window_size.x, window_size.y}
        )));

        Vec2 cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / window_size.x) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / window_size.y) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_screen = mat4_mul_vec4(&deprojection_screen, &mouse_pos_ndc);
        cursor_pos.x = mouse_pos_screen.x;
        cursor_pos.y = mouse_pos_screen.y;

        input->mouse_screen_pos = cursor_pos;

        if (input->is_mouse_left_down && !input->was_mouse_left_down) {
            input->mouse_screen_pressed_pos = input->mouse_screen_pos;
        }
    }
}
#endif // SEINPUT_H