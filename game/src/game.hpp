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

    Entities entities; // @remove and add it to Level
    Level level;

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
};