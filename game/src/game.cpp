#include "game.hpp"
#include <iostream>         // used for writing save files
#include <fstream>          // used for writing save files
#include "assets.hpp"       // saving and loading assets
#include "game_util.hpp"    // utility functions

#define SAVE_FILE_NAME "test_save_level.level"
#define SAVE_FILE_ASSETS_NAME "test_save_assets.assets"
#define ASSETS_SAVE_DATA_VERSION 1

App::App(SDL_Window *window) {
    se_grid_init(&grid, 10, 10);
    value_mappings[0] = {0, 0, 0, 0};
    value_mappings[1] = {255, 255, 255, 255};
    value_mappings[2] = {0  , 0  , 0  , 255};
    value_mappings[3] = {255, 0  , 0  , 255};
    value_mappings[4] = {0  , 255, 0  , 255};
    value_mappings[5] = {0  , 0  , 255, 255};
    se_grid_set(&grid, 0, 0, 1);
    se_grid_set(&grid, 3, 0, 3);
    se_grid_set(&grid, 2, 1, 4);

        //- window and viewport
    m_window = window;
    i32 window_w;
    i32 window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    Rect viewport = {0, 0, (f32)window_w, (f32)window_h};
    should_quit = false;

        //- Render Targets
    Vec2 render_target_size = {(f32)window_w, (f32)window_h};
    se_render_target_init_hdr(&m_render_target_scene, render_target_size, 2, true);
    se_render_target_init_hdr(&m_render_target_blur, render_target_size, 2, false);
    se_render_target_init_hdr(&m_render_target_bloom, render_target_size, 2, false);
    se_render_target_init_hdr(&m_render_target_gaussian_blur_h, render_target_size, 2, false);
    se_render_target_init_hdr(&m_render_target_gaussian_blur_v, render_target_size, 2, false);
    // se_render_target_init_hdr(&m_render_target_downsample, render_target_size, 2, true);
    // se_render_target_init_hdr(&m_render_target_upsample, render_target_size, 2, true);

        // entity widget
    // m_selected_entity = -1; // no entity has been selected
    m_selected_entity = 0; // select first entity

        //- Input
    se_input_init(&m_input);

        //- Renderer
    m_camera_count = 0;
    m_has_queued_for_change_of_mode = false;
    main_camera = this->add_camera();
    se_render3d_init(&m_renderer, &m_cameras[main_camera]);
    m_renderer.light_directional.direction = {-1, -1, -1};
    vec3_normalise(&m_renderer.light_directional.direction);
    m_renderer.light_directional.ambient   = {10, 10, 10};
    m_renderer.light_directional.diffuse   = {255, 255, 255};

        //- Gizmo Renderer
    se_gizmo_renderer_init(&m_gizmo_renderer, &m_cameras[main_camera]);

        //- Load user_meshes
    m_mesh_assets_count = 0;
    memset(m_mesh_assets, 0, sizeof(m_mesh_assets));
    // TODO Add the loader of all user_meshes used in the game here.
    util_load_meshes_from_disk(); // @temp

#if 1 /// load from save
    // util_create_default_scene();
    this->load_assets_and_level();
    this->save();
#else /// load from image
    util_create_scene_from_image("game/levels/test_level.png");
    this->save();
#endif

        //- Start with Engine Mode
    this->init_engine();
    debug_raycast_visual = se_render3d_add_mesh_empty(&m_renderer);
    world_aabb_mesh = se_render3d_add_mesh_empty(&m_renderer);

    // se_texture_load(&debug_screen_quad_texture, "game/textures/Rgnar-Death.png", SE_TEXTURE_LOAD_CONFIG_NULL);
}

App::~App() {
    se_render3d_deinit(&m_renderer);
    se_gizmo_renderer_deinit(&m_gizmo_renderer);
    // se_texture_unload(&debug_screen_quad_texture);
    serender_target_deinit(&m_render_target_blur);
    serender_target_deinit(&m_render_target_bloom);
    serender_target_deinit(&m_render_target_scene);
    serender_target_deinit(&m_render_target_gaussian_blur_h);
    serender_target_deinit(&m_render_target_gaussian_blur_v);
    // serender_target_deinit(&m_render_target_downsample);
    // serender_target_deinit(&m_render_target_upsample);
}

void App::init_engine() {
    m_mode = GAME_MODES::ENGINE;
}

void App::init_game() {
    m_mode = GAME_MODES::GAME;
}

void App::clear() {
    m_level.clear();
}

void App::update(f32 delta_time) {
    // @TODO: delete this line, then add the custom shader index to materials, and a type field to materials to determine what uniforms should be set by default
        //- Update Window and Input
    i32 window_w, window_h;
    SDL_GetWindowSize(m_window, &window_w, &window_h);
    se_camera3d_update_projection(&m_cameras[main_camera], window_w, window_h);
    se_input_update(&m_input, m_cameras[main_camera].projection, m_window);

    m_renderer.time += delta_time;

    if (m_mode == GAME_MODES::GAME) {
            //- GAME INPUT
        util_update_game_mode(delta_time);
    } else
    if (m_mode == GAME_MODES::ENGINE) {
            //- ENGINE INPUT
        util_update_engine_mode(delta_time);
    }

        //- Switch between game and engine mode
    if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_SPACE)) {
        GAME_MODES mode = GAME_MODES::ENGINE;
        if (m_mode == GAME_MODES::ENGINE) mode = GAME_MODES::GAME;
        util_switch_mode(mode);
    }
}

void App::render() {
        //- Default GL Mode
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glLineWidth(1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend mode

        //- 3D Renderer
    i32 window_w, window_h;
    SDL_GetWindowSize(m_window, &window_w, &window_h);
    se_camera3d_update_projection(&m_cameras[main_camera], window_w, window_h);

        //- Shadows
    {
        AABB3D world_aabb = aabb3d_calculate_from_array(m_level.entities.aabb_transformed, m_level.entities.count);
        se_mesh_generate_gizmos_aabb(m_renderer.user_meshes[world_aabb_mesh], world_aabb.min, world_aabb.max, 2);
        se_render_directional_shadow_map(&m_renderer, m_level.entities.mesh_index, m_level.entities.transform, m_level.entities.count, world_aabb);
    }
    se_render_omnidirectional_shadow_map(&m_renderer, m_level.entities.mesh_index, m_level.entities.transform, m_level.entities.count);

        //- Clear Previous Frame
    glClearColor(m_renderer.light_directional.ambient.r / 255.0f,
                 m_renderer.light_directional.ambient.g / 255.0f,
                 m_renderer.light_directional.ambient.b / 255.0f,
                 0.0f);
    // glClearColor(0.3,
    //              0.4,
    //              0.5,
    //              1.0f);
    glClearDepth(1);
    // glClearColor(0.2, 0.1, 0.1, 1);
    // glClearColor(0, 0, 0, 1.0f);

        //- Render Scene
    serender_target_use(&m_render_target_scene);
        glViewport(0, 0, m_render_target_scene.texture_size.x, m_render_target_scene.texture_size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_level.entities.render(&m_renderer);
    serender_target_use(NULL);

        //- Use Gaussian Blur to blur BrightColour channel of scene
    serender_target_use(&m_render_target_gaussian_blur_h);
        glViewport(0, 0, m_render_target_gaussian_blur_h.texture_size.x, m_render_target_gaussian_blur_h.texture_size.y);
        glClear(GL_COLOR_BUFFER_BIT);
        se_render_post_process_gaussian_blur(&m_renderer, &m_render_target_scene, true);
    serender_target_use(0);
    serender_target_use(&m_render_target_gaussian_blur_v);
        glViewport(0, 0, m_render_target_gaussian_blur_v.texture_size.x, m_render_target_gaussian_blur_v.texture_size.y);
        glClear(GL_COLOR_BUFFER_BIT);
        se_render_post_process_gaussian_blur(&m_renderer, &m_render_target_scene, false);
    serender_target_use(0);
        // then blur with the other gaussian framebuffer
    u32 blur_amount = 50;
    bool horizontal = true;
    for (u32 i = 0; i < blur_amount; ++i) {
        if (!horizontal) {
            serender_target_use(&m_render_target_gaussian_blur_v);
            se_render_post_process_gaussian_blur(&m_renderer, &m_render_target_gaussian_blur_h, true);
        } else {
            serender_target_use(&m_render_target_gaussian_blur_h);
            se_render_post_process_gaussian_blur(&m_renderer, &m_render_target_gaussian_blur_v, false);
        }
        horizontal = !horizontal;
    }

        //- Combine blurred bloom and scene
    serender_target_use(&m_render_target_bloom);
        glViewport(0, 0, m_render_target_bloom.texture_size.x, m_render_target_bloom.texture_size.y);
        glClear(GL_COLOR_BUFFER_BIT);
        se_render_post_process(&m_renderer, SE_RENDER_POSTPROCESS_BLOOM, &m_render_target_gaussian_blur_v);
    serender_target_use(NULL);

        //- Apply Tonemapping
    glViewport(0, 0, window_w, window_h);
    glClearColor(m_renderer.light_directional.ambient.r / 255.0f,
                m_renderer.light_directional.ambient.g / 255.0f,
                m_renderer.light_directional.ambient.b / 255.0f,
                1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    se_render_post_process(&m_renderer, SE_RENDER_POSTPROCESS_TONEMAP, &m_render_target_bloom);

    glClear(GL_DEPTH_BUFFER_BIT);
    if (m_mode == GAME_MODES::GAME) {
        //- GAME SPECIFIC
        util_render_game_mode();
    } else
    if (m_mode == GAME_MODES::ENGINE) {
        //- ENGINE SPECIFIC
        util_render_engine_mode();
    }

    // {   //- UI
    //     static float f = 0.0f;
    //     static int counter = 0;

    //     ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    //     ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

    //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //     ImGui::ColorEdit3("clear color", (float*)&test_colour); // Edit 3 floats representing a color

    //     if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    //         counter++;
    //     ImGui::SameLine();
    //     ImGui::Text("counter = %d", counter);

    //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //     ImGui::End();
    // }
}

void App::end_of_frame() {
    if (m_has_queued_for_change_of_mode) {
        m_has_queued_for_change_of_mode = false;

        switch (m_queued_mode) {
            this->save();
            case GAME_MODES::GAME: {
                //... setup
                m_mode = m_queued_mode;
                init_game();
            } break;
            case GAME_MODES::ENGINE: {
                //... setup
                m_mode = m_queued_mode;
                init_engine();
            } break;
        }
    }
}

i32 App::raycast_to_select_entity() {
    Vec3 raycast_dir;
    Vec3 raycast_origin;
    se_camera3d_get_raycast(&m_cameras[main_camera], m_window, &raycast_dir, &raycast_origin);

    i32 result = -1;

    f32 closest_hit = SEMATH_INFINITY;
    for (u32 i = 0; i < m_level.entities.count; ++i) {
        f32 hit;
        if (ray_overlaps_aabb3d(raycast_origin, raycast_dir, 100, m_level.entities.aabb_transformed[i], &hit)) {
            if (hit < closest_hit) {
                closest_hit = hit;
                result = i;
            }
        }
    }

    se_mesh_generate_line(m_renderer.user_meshes[debug_raycast_visual], raycast_origin,
        vec3_add(raycast_origin, vec3_mul_scalar(raycast_dir, 100)), 2, {255, 0,0, 255});

    return result;
}

u32 App::add_camera() {
    se_assert(m_camera_count < MAX_NUM_CAMERA && "too many cameras");
    u32 result = m_camera_count;
    m_camera_count++;
    se_camera3d_init(&m_cameras[result]);
    return result;
}

void App::save() {
    {   //- Assets
        std::ofstream file(SAVE_FILE_ASSETS_NAME);
        if (!file.is_open()) {
            SE_ERROR("could not open file to save:");
            SE_ERROR(SAVE_FILE_ASSETS_NAME);
            return;
        }

            // version
        file << ASSETS_SAVE_DATA_VERSION << std::endl;

            // mesh count
        file << m_mesh_assets_count << std::endl;

        for (u32 i = 0; i < m_mesh_assets_count; ++i) {
            file << m_mesh_assets[i].buffer << std::endl;
        }

        file.close();
    }

        //- Level
    m_level.main_camera_settings = m_cameras[main_camera];
    Assets::save_level(&m_level, SAVE_FILE_NAME);
}

void App::load_assets_and_level() {
    {   //- Assets From Save File
        std::ifstream file(SAVE_FILE_ASSETS_NAME);
        if (!file.is_open()) {
            SE_ERROR("could not open file to save:");
            SE_ERROR(SAVE_FILE_ASSETS_NAME);
        } else {
                // version
            u32 version;
            file >> version;

                // mesh count
            file >> m_mesh_assets_count;

            for (u32 i = 0; i < m_mesh_assets_count; ++i) {
                char buf[1024];
                file >> buf;
                se_string_init(&m_mesh_assets[i], buf);
            }

            file.close();
        }
    }

        //! the reason I can't do this right now is because we don't have an abstract mesh file type.
        //! this is important because we sometimes generate our own custom user_meshes. So We'll wait until
        //! we have that feature done.
    // {   //- Load the actual asset data
    //     for (u32 i = 0; i < m_mesh_assets_count; ++i) {
    //         se_render3d_load_mesh(&m_renderer, m_mesh_assets[i].buffer, false);
    //     }
    // }

        //- Level
    Assets::load_level(&m_level, SAVE_FILE_NAME);
    this->m_cameras[main_camera] = m_level.main_camera_settings;
}