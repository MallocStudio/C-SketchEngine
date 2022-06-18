#ifndef SEINPUT_H
#define SEINPUT_H

#include "sedefines.h"
#include "semath_defines.h"
#include "semath.h"
#include "sestring.h"

/// get the mouse position (relative to the window's top left position). Optionally pass bools to get mouse state
 Vec2 get_mouse_pos(b8 *lpressed, b8 *rpressed);

/// get the mouse position (relative to the window's top left position). Optionally pass bools to get mouse state
 Vec2 get_mouse_pos_normalised(SDL_Window *window, b8 *lpressed, b8 *rpressed);

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
    b8 is_mouse_left_down;
    b8 is_mouse_right_down;
    // information from the previous frame
    b8 was_mouse_left_down;
    b8 was_mouse_right_down;
    // whethered we want to let others know that we've handled the mouse input
    b8 is_mouse_left_handled;
    b8 is_mouse_right_handled;
    // warp mouse around window
    SDL_Window *window; // not owned
    b8 is_mouse_fps_activated;

    /* text input */
    b8 is_text_input_activated; // use seinput_text_input_activate()
    SE_String *text_input_stream; // not owned. When this is not null and text_input is activated, stream to this string
    b8 is_text_input_only_numeric;
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
 void seinput_init(SE_Input *input);

/// note that mouse pos will be relative to top left position of window
 void seinput_update(SE_Input *input, Mat4 otho_projection_world, SDL_Window *window);

 b8 seinput_is_mouse_left_pressed (const SE_Input *input);

 b8 seinput_is_mouse_left_released (const SE_Input *input);

 b8 seinput_is_mouse_right_pressed (const SE_Input *input);

 b8 seinput_is_mouse_right_released (const SE_Input *input);

 b8 seinput_is_key_pressed(const SE_Input *input, SDL_Scancode sdl_scancode);

 b8 seinput_is_key_down(const SE_Input *input, SDL_Scancode sdl_scancode);

 void seinput_text_input_activate(SE_Input *input, SE_String *stream_to, b8 only_numeric);

 void seinput_text_input_deactivate(SE_Input *input);

 void seui_mouse_fps_activate(SE_Input *input);

 void seui_mouse_fps_deactivate(SE_Input *input);
#endif // SEINPUT_H