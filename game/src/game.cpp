#include "game.hpp"

SE_UI *ctx;

    //@temp a temporary way of loading required meshes once at init_application() time
u32 mesh_soulspear = -1;
u32 mesh_plane = -1;
u32 mesh_guy = -1;
u32 mesh_skeleton = -1;
u32 mesh_gizmos_translate = -1;

u32 main_camera = -1;

#define SAVE_FILE_NAME "test_save_level.level"

App::App(SDL_Window *window) {
    this->init_application(window);
        //- Start with Engine Mode
    this->init_engine();
}

App::~App() {
    free(ctx);
    serender3d_deinit(&m_renderer);
}

void App::update(f32 delta_time) {
        //- Update Window and Input
    i32 window_w, window_h;
    SDL_GetWindowSize(m_window, &window_w, &window_h);
    secamera3d_update_projection(&m_cameras[main_camera], window_w, window_h);
    seinput_update(&m_input, m_cameras[main_camera].projection, m_window);
    seui_resize(ctx, window_w, window_h);

        //- 3D Movement
    secamera3d_input(&m_cameras[main_camera], &m_input);

        //- Entities
    m_level.entities.update_transforms();

        // select entities
    this->raycast_to_select_entity();
    if (seinput_is_mouse_left_released(&m_input) && seinput_is_key_down(&m_input, SDL_SCANCODE_LCTRL)) {
        printf("checking\n"); // @debug
        m_selected_entity = this->raycast_to_select_entity();
        m_widget_entity.entity = m_selected_entity;

        if (m_selected_entity >= 0) {   // @debug
            printf("hit %i\n", m_selected_entity);
        }
    }

        //- UI
    seui_reset(ctx);
    m_widget_entity.construct_panel(ctx);
    m_selected_entity = m_widget_entity.entity;

        // make entity widget pop up
    if (seinput_is_key_pressed(&m_input, SDL_SCANCODE_SPACE)) {
        m_widget_entity.toggle_visibility(ctx);
    }

        // save
    if (seui_button_at(ctx, "save", {0, 0, 128, 32})) {
        m_level.save(SAVE_FILE_NAME);
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
    secamera3d_update_projection(&m_cameras[main_camera], window_w, window_h);

        //- Shadows
    se_render_directional_shadow_map(&m_renderer,     m_level.entities.transform, m_level.entities.count);
    se_render_omnidirectional_shadow_map(&m_renderer, m_level.entities.transform, m_level.entities.count);

        //- Clear Previous Frame
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_w, window_h);

        //- Render Entities
    for (u32 i = 0; i < m_level.entities.count; ++i) {
        if (m_level.entities.has_mesh[i] && m_level.entities.should_render_mesh[i]) {
            serender_mesh_index(&m_renderer, m_level.entities.mesh_index[i], m_level.entities.transform[i]);
        }
    }

        //- Gizmos
    glClear(GL_DEPTH_BUFFER_BIT);

        // selected entity
    if (m_selected_entity >= 0) {
        se_assert(m_selected_entity < m_level.entities.count);
        serender_mesh_index(&m_renderer, mesh_gizmos_translate, m_level.entities.transform[m_selected_entity]);
    }

        //- UI
    glClear(GL_DEPTH_BUFFER_BIT);
    seui_render(ctx);
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
    seinput_init(&m_input);

        //- Renderer
    m_camera_count = 0;
    main_camera = this->add_camera();
    serender3d_init(&m_renderer, &m_cameras[main_camera]);
    m_renderer.light_directional.direction = {0, -1, 0};
    m_renderer.light_directional.ambient   = {50, 50, 50};
    m_renderer.light_directional.diffuse   = {255, 255, 255};

        //- Load meshes
    mesh_soulspear = serender3d_load_mesh(&m_renderer, "game/meshes/soulspear/soulspear.obj", false);
    mesh_plane = serender3d_add_plane(&m_renderer, v3f(10, 10, 10));
    mesh_gizmos_translate = serender3d_add_gizmos_coordniates(&m_renderer);
}

void App::init_engine() {
    this->clear();
    m_mode = GAME_MODES::ENGINE;

#if 0 // manually create entities
    // @temp add entities
    u32 soulspear = level.add_entity();
    level.entities.mesh_index[soulspear] = mesh_soulspear;
    level.entities.has_mesh[soulspear] = true;
    level.entities.should_render_mesh[soulspear] = true;
    level.entities.has_name[soulspear] = true;
    sestring_init(&level.entities.name[soulspear], "soulspear_entity");
    level.entities.position[soulspear] = v3f(3, 1, 0);

    u32 plane = level.add_entity();
    level.entities.mesh_index[plane] = mesh_plane;
    level.entities.has_mesh[plane] = true;
    level.entities.should_render_mesh[plane] = true;
    level.entities.has_name[plane] = true;
    sestring_init(&level.entities.name[plane], "plane_entity");

    level.save(SAVE_FILE_NAME);
#else // load from file
    m_level.load(SAVE_FILE_NAME);
#endif
}

void App::init_game() {
    this->clear();
    m_mode = GAME_MODES::GAME;
}

void App::clear() {
        // free memory if required
    m_level.entities.clear();
        // set entity data to their default value
    m_level.entities.set_to_default();
}

i32 App::raycast_to_select_entity() {
    Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
    i32 window_w, window_h;
    SDL_GetWindowSize(m_window, &window_w, &window_h);

    // mouse_pos.y = window_h - mouse_pos.y;
    mouse_pos.x -= window_w / 2; // anchor mouse pos from center of the window not a corner
    mouse_pos.y -= window_h / 2; // anchor mouse pos from center of the window not a corner
    mouse_pos.x /= window_w;
    mouse_pos.y /= window_h;

    Vec3 camera_pos = m_cameras[main_camera].position;
    Vec3 raycast_dir = secamera3d_get_front(&m_cameras[main_camera]);

    Vec3 raycast_dir_tangent;
    Vec3 raycast_dir_bitangent;
    vec3_calculate_tangent_bitangent(raycast_dir, &raycast_dir_tangent, &raycast_dir_bitangent);

    Vec3 mouse_pos_in_camera_space_offset = camera_pos;
    mouse_pos_in_camera_space_offset.x += mouse_pos.x * raycast_dir_tangent.x;
    mouse_pos_in_camera_space_offset.y += mouse_pos.y * raycast_dir_bitangent.y;

    i32 result = -1;

    f32 closest_hit = -SEMATH_INFINITY;
    for (u32 i = 0; i < m_level.entities.count; ++i) {
        f32 hit;
        if (ray_overlaps_sphere(mouse_pos_in_camera_space_offset, raycast_dir, 1000, m_level.entities.position[i], 3, &hit)) {
            if (hit > closest_hit) {
                closest_hit = hit;
                result = i;
            }
        }
    }

    return result;
}

u32 App::add_camera() {
    se_assert(m_camera_count < MAX_NUM_CAMERA && "too many cameras");
    u32 result = m_camera_count;
    m_camera_count++;
    secamera3d_init(&m_cameras[result]);
    return result;
}