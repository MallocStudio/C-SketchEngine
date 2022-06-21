#pragma once

#include "sketchengine.h"
#include "level.hpp"

enum class GAME_MODES {
    GAME,
    ENGINE
};

struct App {
public:
    SE_Input input;
    SE_Camera3D camera;
    SE_Renderer3D renderer;
    SDL_Window *window;
    bool should_quit;

    GAME_MODES mode;

    Level level;
    i32 selected_entity; // when == -1 it means no entity is selected

    App(SDL_Window *window);
    ~App();

    void update(f32 delta_time);
    void render();

private:
    void init_application(SDL_Window *window);
        /// This is called at the beginning of init_engine and init_game
        /// to clear out entity data so they can be loaded from a level file
    void clear();
    void init_engine();
    void init_game();
        /// Raycasts into the scene to see if the user is trying to select an entiy.
        /// It will return the index of the entity, -1 if nothing was being selected.
    i32 raycast_to_select_entity();
};
