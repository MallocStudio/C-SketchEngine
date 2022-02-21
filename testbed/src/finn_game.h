#ifndef FINN_GAME_H
#define FINN_GAME_H

#include "sketchengine.h"

typedef struct Finn_Game {
    SEGL_Shader_Program *shader_program; // line renderer shader programs // @TODO move this to the SEGL_Line_Renderer class
    SEGL_Camera *camera;
    SDL_Window *window;
    Vec2 mouse_pos_world;
    Vec2 mouse_pos_screen;
    Uint8 *keyboard;
    SEGL_Line_Renderer grid;
    SEGL_Line_Renderer lines;
    SE_Text_Renderer txt_library;
    UI_Context ui_context;
    Rect test_ui_init_rect;
} Finn_Game;
void finn_game_init(Finn_Game *game, SDL_Window *window);
void finn_game_deinit(Finn_Game *game);
void finn_game_update(Finn_Game *game, f32 delta_time);
void finn_game_render(Finn_Game *game);

#endif // FINN_GAME_H
