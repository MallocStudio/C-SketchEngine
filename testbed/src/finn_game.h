#ifndef FINN_GAME_H
#define FINN_GAME_H

#include "sketchengine.h"

#define FINN_GAME_MAX_NUM_OF_OBJECTS 40

typedef struct Finn_Game {
    // -- renderer related
    SEGL_Shader_Program *shader_program; // line renderer shader programs // @TODO move this to the SEGL_Line_Renderer class
    SEGL_Camera *camera;
    SDL_Window *window;
    SEGL_Line_Renderer grid;
    SEGL_Line_Renderer lines;
    SE_Text_Renderer txt_library;
    UI_Context ui_context;
    Rect test_ui_init_rect;

    // -- input
    SE_Input input;

    // -- physcis testbed related
    i32 objects_count;
    SE_Shape *objects[FINN_GAME_MAX_NUM_OF_OBJECTS];
    SE_SHAPES current_selected_shape_mode;

    bool is_paused;
    bool is_physics_update_queued;

    f32 elasticity;

} Finn_Game;
void finn_game_init(Finn_Game *game, SDL_Window *window);
void finn_game_deinit(Finn_Game *game);
void finn_game_update(Finn_Game *game, f32 delta_time);
void finn_game_render(Finn_Game *game);
void finn_game_physics_update(Finn_Game *game, f32 delta_time);
#endif // FINN_GAME_H
