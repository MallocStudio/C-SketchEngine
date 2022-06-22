#pragma once

#include "sketchengine.h"
#include "level.hpp"
#include "widgets.hpp"

enum class GAME_MODES {
    GAME,
    ENGINE
};

#define MAX_NUM_CAMERA 3

struct App {
public:
    SE_Input m_input;
    u32 m_camera_count;
    SE_Camera3D m_cameras[MAX_NUM_CAMERA];
    SE_Renderer3D m_renderer;
    SDL_Window *m_window;
    bool should_quit;

    GAME_MODES m_mode;

    Level m_level;
    i32 m_selected_entity; // when == -1 it means no entity is selected

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
    u32 add_camera();
        /// Raycasts into the scene to see if the user is trying to select an entiy.
        /// It will return the index of the entity, -1 if nothing was being selected.
    i32 raycast_to_select_entity();

    Widget_Entity m_widget_entity;
};
