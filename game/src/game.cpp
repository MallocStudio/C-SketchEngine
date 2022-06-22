#include "game.hpp"

SE_UI *ctx;

    //@temp a temporary way of loading required meshes once at init_application() time
u32 mesh_soulspear = -1;
u32 mesh_plane = -1;
u32 mesh_guy = -1;
u32 mesh_skeleton = -1;
u32 mesh_gizmos_translate = -1;

#define SAVE_FILE_NAME "test_save_level.level"

App::App(SDL_Window *window) {
    this->init_application(window);
        //- Start with Engine Mode
    this->init_engine();
}

App::~App() {
    free(ctx);
    serender3d_deinit(&this->renderer);
}

void App::update(f32 delta_time) {
        //- Update Window and Input
    i32 window_w, window_h;
    SDL_GetWindowSize(this->window, &window_w, &window_h);
    secamera3d_update_projection(&this->camera, window_w, window_h);
    seinput_update(&this->input, this->camera.projection, this->window);
    seui_resize(ctx, window_w, window_h);

        //- 3D Movement
    secamera3d_input(&this->camera, &this->input);

        //- Entities
    this->level.entities.update_transforms();

        // select entities
    if (seinput_is_mouse_left_released(&this->input) && seinput_is_key_down(&input, SDL_SCANCODE_LCTRL)) {
        printf("checking\n"); // @debug
        this->selected_entity = this->raycast_to_select_entity();
        this->widget_entity.entity = this->selected_entity;

        if (this->selected_entity >= 0) {   // @debug
            printf("hit %i\n", this->selected_entity);
        }
    }

        //- UI
    seui_reset(ctx);
    this->widget_entity.construct_panel(ctx);
    this->selected_entity = this->widget_entity.entity;

        // make entity widget pop up
    if (seinput_is_key_pressed(&input, SDL_SCANCODE_SPACE)) {
        this->widget_entity.toggle_visibility(ctx);
    }

        // save
    if (seui_button_at(ctx, "save", {0, 0, 128, 32})) {
        this->level.save(SAVE_FILE_NAME);
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
    SDL_GetWindowSize(this->window, &window_w, &window_h);
    secamera3d_update_projection(&this->camera, window_w, window_h);

        //- Shadows
    se_render_directional_shadow_map(&this->renderer,     this->level.entities.transform, this->level.entities.count);
    se_render_omnidirectional_shadow_map(&this->renderer, this->level.entities.transform, this->level.entities.count);

        //- Clear Previous Frame
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_w, window_h);

        //- Render Entities
    for (u32 i = 0; i < this->level.entities.count; ++i) {
        if (this->level.entities.has_mesh[i] && this->level.entities.should_render_mesh[i]) {
            serender_mesh_index(&this->renderer, this->level.entities.mesh_index[i], this->level.entities.transform[i]);
        }
    }

        //- Gizmos
    glClear(GL_DEPTH_BUFFER_BIT);

        // selected entity
    if (this->selected_entity >= 0) {
        se_assert(this->selected_entity < this->level.entities.count);
        serender_mesh_index(&this->renderer, mesh_gizmos_translate, this->level.entities.transform[this->selected_entity]);
    }

        //- UI
    glClear(GL_DEPTH_BUFFER_BIT);
    seui_render(ctx);
}

    /// Init the application. This must be called once, and before init_engine or init_game
void App::init_application(SDL_Window *window) {
    //- window and viewport
    this->window = window;
    i32 window_w;
    i32 window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    Rect viewport = {0, 0, (f32)window_w, (f32)window_h};
    this->should_quit = false;

        //- UI
    ctx = NEW(SE_UI);
    seui_init(ctx, &this->input, viewport, -1000, 1000);

        // entity widget
    // this->selected_entity = -1; // no entity has been selected
    this->widget_entity.entities = &this->level.entities;
    this->selected_entity = 0; // select first entity
    this->widget_entity.entity = this->selected_entity;

        //- Input
    seinput_init(&this->input);

        //- Renderer
    secamera3d_init(&this->camera);
    serender3d_init(&this->renderer, &this->camera);
    this->renderer.light_directional.direction = {0, -1, 0};
    this->renderer.light_directional.ambient   = {50, 50, 50};
    this->renderer.light_directional.diffuse   = {255, 255, 255};

        //- Load meshes
    mesh_soulspear = serender3d_load_mesh(&this->renderer, "game/meshes/soulspear/soulspear.obj", false);
    mesh_plane = serender3d_add_plane(&this->renderer, v3f(10, 10, 10));
    mesh_gizmos_translate = serender3d_add_gizmos_coordniates(&this->renderer);
}

void App::init_engine() {
    this->clear();
    this->mode = GAME_MODES::ENGINE;

#if 0 // manually create entities
    // @temp add entities
    u32 soulspear = this->level.add_entity();
    level.entities.mesh_index[soulspear] = mesh_soulspear;
    level.entities.has_mesh[soulspear] = true;
    level.entities.should_render_mesh[soulspear] = true;
    level.entities.has_name[soulspear] = true;
    sestring_init(&level.entities.name[soulspear], "soulspear_entity");
    level.entities.position[soulspear] = v3f(3, 1, 0);

    u32 plane = this->level.add_entity();
    level.entities.mesh_index[plane] = mesh_plane;
    level.entities.has_mesh[plane] = true;
    level.entities.should_render_mesh[plane] = true;
    level.entities.has_name[plane] = true;
    sestring_init(&level.entities.name[plane], "plane_entity");

    this->level.save(SAVE_FILE_NAME);
#else // load from file
    this->level.load(SAVE_FILE_NAME);
#endif
}

void App::init_game() {
    this->clear();
    this->mode = GAME_MODES::GAME;
}

void App::clear() {
        // free memory if required
    this->level.entities.clear();
        // set entity data to their default value
    this->level.entities.set_to_default();
}

i32 App::raycast_to_select_entity() {
    Vec2 mouse_pos = get_mouse_pos(NULL, NULL);

    Vec3 raycast_origin = this->camera.position;
    Vec3 raycast_dir = secamera3d_get_front(&this->camera);

    Vec3 raycast_dir_tangent;
    Vec3 raycast_dir_bitanget;
    {
        Vec3 c1 = vec3_cross(raycast_dir, vec3_up());
        Vec3 c2 = vec3_cross(raycast_dir, vec3_forward());

        if (vec3_magnitude_squared(c1) > vec3_magnitude_squared(c2)) {
            raycast_dir_tangent = c1;
        } else {
            raycast_dir_tangent = c2;
        }
        vec3_normalise(&raycast_dir_tangent);
        raycast_dir_bitanget = vec3_cross(raycast_dir, raycast_dir_tangent);
        vec3_normalise(&raycast_dir_bitanget);
    }

    Vec3 mouse_pos_in_camera_space_offset;
    mouse_pos_in_camera_space_offset.x = mouse_pos.x + raycast_dir_tangent.x;
    mouse_pos_in_camera_space_offset.y = mouse_pos.y + raycast_dir_tangent.y;
    raycast_origin = vec3_add(raycast_origin, mouse_pos_in_camera_space_offset);

    i32 result = -1;

    f32 closest_hit = -SEMATH_INFINITY;
    for (u32 i = 0; i < this->level.entities.count; ++i) {
        f32 hit;
        if (ray_overlaps_sphere(raycast_origin, raycast_dir, 1000, level.entities.position[i], 3, &hit)) {
            if (hit > closest_hit) {
                closest_hit = hit;
                result = i;
            }
        }
    }

    return result;
}