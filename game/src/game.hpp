#pragma once

#include "sketchengine.h"
#include "level.hpp"

enum class GAME_MODES {
    GAME,
    ENGINE
};

#define MAX_NUM_CAMERA 3
#define MAX_NUM_ASSETS 100
struct App {
public:
    SE_Input m_input;
    u32 m_camera_count;
    SE_Camera3D m_cameras[MAX_NUM_CAMERA];
    SE_Renderer3D m_renderer;
    SE_Gizmo_Renderer m_gizmo_renderer;
    SDL_Window *m_window;
    bool should_quit;
    f32 fps;

    GAME_MODES m_mode;

    Level m_level;
    i32 m_selected_entity; // when == -1 it means no entity is selected

    App(SDL_Window *window);
    ~App();

    void update(f32 delta_time);
    void render();
    void end_of_frame();

private:
        /// This is called at the beginning of init_engine and init_game
        /// to clear out entity data so they can be loaded from a level file
    void clear();
    void init_engine();
    void init_game();
        /// Save level data, assets data
    void save();
        /// Load level data, assets data
    void load_assets_and_level();

    u32 add_camera();
        /// Raycasts into the scene to see if the user is trying to select an entiy.
        /// It will return the index of the entity, -1 if nothing was being selected.
    i32 raycast_to_select_entity();

    u32 m_mesh_assets_count;
    SE_String m_mesh_assets[MAX_NUM_ASSETS]; // @TODO change this to a dynamic array
    bool m_has_queued_for_change_of_mode;
    GAME_MODES m_queued_mode;

    SE_Render_Target m_render_target_scene; // the geometry in the scene (entities and particles)
    SE_Render_Target m_render_target_downsample;
    SE_Render_Target m_render_target_upsample;
    SE_Render_Target m_render_target_blur;  // blurred scene
    SE_Render_Target m_render_target_bloom;  // blurred scene
    SE_Render_Target m_render_target_gaussian_blur_h;  // horizontally blurred
    SE_Render_Target m_render_target_gaussian_blur_v;  // vertically blurred

    ///
    ///     UTILITY FUNCTIONALITIES
    ///
    void util_load_meshes_from_disk();
    void util_create_default_scene();
    void util_create_scene_from_image(const char *filepath);

    void util_update_game_mode(f32 delta_time);
    void util_update_engine_mode(f32 delta_time);

    void util_render_game_mode();
    void util_render_engine_mode();

    void util_switch_mode(GAME_MODES mode);

    /// Displays an ImGui window with entity's data.
    /// It has the ability to scroll through all entities and
    /// so it returns the entity_index selected through the UI
    i32 util_show_entity_data(i32 entity_index);
    void util_show_light_data();
    void util_show_assets();
    void util_show_material(u32 material_index);
    void util_show_menubar();
};
