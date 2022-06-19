#pragma once

#include "sketchengine.h"
#include "entity.hpp"

struct Game {
public:
    SE_Input input;
    SE_Camera3D camera;
    SE_Renderer3D renderer;
    SDL_Window *window;
    bool should_quit;

    Entities entities;

    Game(SDL_Window *window);
    ~Game();

    void update(f32 delta_time);
    void render();
};
