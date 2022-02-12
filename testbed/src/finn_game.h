#ifndef FINN_GAME_H
#define FINN_GAME_H

#include "sketchengine.h"

typedef struct Finn_Game {
    SEGL_Shader_Program *shader_program;
    SEGL_Camera *camera;
    SDL_Window *window;
    Vec2 mouse_pos;
    Uint8 *keyboard;
    SEGL_Line_Renderer grid;
    SEGL_Line_Renderer lines;
    SEGL_Renderer2D renderer2D;
} Finn_Game;
void finn_game_init(Finn_Game *game, SDL_Window *window);
void finn_game_deinit(Finn_Game *game);
void finn_game_update(Finn_Game *game, f32 delta_time);
void finn_game_render(Finn_Game *game);

#endif // FINN_GAME_H
