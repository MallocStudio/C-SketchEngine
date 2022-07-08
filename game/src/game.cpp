#include "game.hpp"
#include <iostream>         // used for writing save files
#include <fstream>          // used for writing save files
#include "game_util.hpp"    // utility functions
#include "assets.hpp"       // saving and loading assets

#define SAVE_FILE_NAME "test_save_level.level"
#define SAVE_FILE_ASSETS_NAME "test_save_assets.assets"
#define ASSETS_SAVE_DATA_VERSION 1

App::App(SDL_Window *window) {
    this->init_application(window);
        //- Start with Engine Mode
    this->init_engine();
    debug_raycast_visual = se_render3d_add_mesh_empty(&m_renderer);
    world_aabb_mesh = se_render3d_add_mesh_empty(&m_renderer);
}

App::~App() {
    free(ctx);
    se_render3d_deinit(&m_renderer);
    se_gizmo_renderer_deinit(&m_gizmo_renderer);
}

    /// Init the application. ust be called once, and before init_engine or init_game
void App::init_application(SDL_Window *window) {
    //- window and viewport
    m_window = window;
    i32 window_w;
    i32 window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    Rect viewport = {0, 0, (f32)window_w, (f32)window_h};
    should_quit = false;

        //- UI
    ctx = NEW(SE_UI);
    seui_init(ctx, &m_input, viewport, -1000, 1000);

        // entity widget
    // m_selected_entity = -1; // no entity has been selected
    m_widget_entity.entities = &m_level.entities;
    m_selected_entity = 0; // select first entity
    m_widget_entity.entity = m_selected_entity;

        //- Input
    se_input_init(&m_input);

        //- Renderer
    m_camera_count = 0;
    main_camera = this->add_camera();
    se_render3d_init(&m_renderer, &m_cameras[main_camera]);
    m_renderer.light_directional.direction = {-1, -1, -1};
    vec3_normalise(&m_renderer.light_directional.direction);
    m_renderer.light_directional.ambient   = {50, 50, 50};
    m_renderer.light_directional.diffuse   = {255, 255, 255};

        //- Gizmo Renderer
    se_gizmo_renderer_init(&m_gizmo_renderer, &m_cameras[main_camera]);

        //- Load meshes
    m_mesh_assets_count = 0;
    memset(m_mesh_assets, 0, sizeof(m_mesh_assets));
    // TODO Add the loader of all meshes used in the game here.
}

void App::init_engine() {
    this->clear();
    m_mode = GAME_MODES::ENGINE;
    util_load_meshes_from_disk();

#if 1 /// manually create entities
    util_create_default_scene();
    this->save();
#else /// load from file
    this->load();
#endif
}

void App::init_game() {
    this->clear();
    m_mode = GAME_MODES::GAME;
}

void App::clear() {
    m_level.clear();
}

void App::update(f32 delta_time) {
        //- Update Window and Input
    i32 window_w, window_h;
    SDL_GetWindowSize(m_window, &window_w, &window_h);
    se_camera3d_update_projection(&m_cameras[main_camera], window_w, window_h);
    se_input_update(&m_input, m_cameras[main_camera].projection, m_window);
    seui_resize(ctx, window_w, window_h);

        //- 3D Movement
    se_camera3d_input(&m_cameras[main_camera], &m_input);

        //- Entities
    m_level.entities.update(&m_renderer, delta_time);
    se_animation_update(&animation, delta_time);
    se_skeleton_calculate_pose(m_renderer.meshes[mesh_guy]->skeleton, animation.current_frame);

        // select entities
    if (se_input_is_mouse_left_released(&m_input) && se_input_is_key_down(&m_input, SDL_SCANCODE_LCTRL)) {
        printf("checking\n"); // @debug
        m_selected_entity = this->raycast_to_select_entity();
        m_widget_entity.entity = m_selected_entity;

        if (m_selected_entity >= 0) {   // @debug
            printf("hit %i\n", m_selected_entity);
        }
    }

    if (m_selected_entity >= 0) {   // @temp
        Vec3 dir;
        Vec3 origin;
        se_camera3d_get_raycast(&m_cameras[main_camera], m_window, &dir, &origin);
        if (ray_overlaps_sphere(origin, dir, 100, m_level.entities.position[m_selected_entity], 0.5f, NULL)) {
            m_gizmo_renderer.shapes[mesh_gizmos_translate].base_colour = lit;
        } else {
            m_gizmo_renderer.shapes[mesh_gizmos_translate].base_colour = dim;
        }
    }
        //- UI
    seui_reset(ctx);
    m_widget_entity.construct_panel(ctx, &m_renderer);
    m_selected_entity = m_widget_entity.entity;

        // make entity widget pop up
    if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_SPACE)) {
        m_widget_entity.toggle_visibility(ctx);
    }

        // save
    if (seui_button_at(ctx, "save", {0, 0, 128, 32})) {
        Assets::save_level(&m_level, SAVE_FILE_NAME);
    }

    seui_texture_viewer(ctx, m_renderer.shadow_render_target.texture);
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
        se_mesh_generate_gizmos_aabb(m_renderer.meshes[world_aabb_mesh], world_aabb.min, world_aabb.max, 2);
        se_render_directional_shadow_map(&m_renderer, m_level.entities.mesh_index, m_level.entities.transform, m_level.entities.count, world_aabb);
    }
    se_render_omnidirectional_shadow_map(&m_renderer, m_level.entities.transform, m_level.entities.count);

        //- Clear Previous Frame
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_w, window_h);

        //- Render Entities
    m_level.entities.render(&m_renderer);

        // aabb of selected entity
    if (m_selected_entity >= 0) {
        se_mesh_generate_gizmos_aabb(m_renderer.meshes[current_obj_aabb],
            m_level.entities.aabb[m_selected_entity].min,
            m_level.entities.aabb[m_selected_entity].max,
            2);

        se_render_mesh_index(&m_renderer, current_obj_aabb, m_level.entities.transform[m_selected_entity]);
    }
        // skeleton mesh
    se_render_mesh_index(&m_renderer, mesh_skeleton, m_level.entities.transform[mesh_guy]);
    se_render_mesh_index(&m_renderer, world_aabb_mesh, mat4_identity());

        //- Gizmos
    glClear(GL_DEPTH_BUFFER_BIT);

        // selected entity
    if (m_selected_entity >= 0) {
        se_assert(m_selected_entity < m_level.entities.count);
            // calculate transfrom without scale
        Vec3 pos   = m_level.entities.position[m_selected_entity];
        Vec3 rot   = m_level.entities.oriantation[m_selected_entity];

        Mat4 transform = mat4_identity();
        transform = mat4_mul(transform, mat4_euler_xyz(rot.x, rot.y, rot.z));
        transform = mat4_mul(transform, mat4_translation(pos));

        se_gizmo_render_index(&m_gizmo_renderer, mesh_gizmos_translate, transform);
    }
        // @debug directional light pos
    {
        Vec3 pos = m_renderer.light_directional.calculated_position;
        Mat4 transform = mat4_identity();
        transform = mat4_mul(transform, mat4_translation(pos));
        se_gizmo_render_index(&m_gizmo_renderer, mesh_light_pos_gizmos, transform);
    }

    se_render_mesh_index(&m_renderer, debug_raycast_visual, mat4_identity());

        //- UI
    glClear(GL_DEPTH_BUFFER_BIT);
    seui_render(ctx);

    glClear(GL_DEPTH_BUFFER_BIT);
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

    se_mesh_generate_line(m_renderer.meshes[debug_raycast_visual], raycast_origin,
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
    Assets::save_level(&m_level, SAVE_FILE_NAME);
}

void App::load() {
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
        //! this is important because we sometimes generate our own custom meshes. So We'll wait until
        //! we have that feature done.
    // {   //- Load the actual asset data
    //     for (u32 i = 0; i < m_mesh_assets_count; ++i) {
    //         se_render3d_load_mesh(&m_renderer, m_mesh_assets[i].buffer, false);
    //     }
    // }

        //- Level
    Assets::load_level(&m_level, SAVE_FILE_NAME);
}