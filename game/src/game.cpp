#include "game.hpp"

SE_UI *ctx;
SE_String input_text;

App::App(SDL_Window *window) {
    this->init_application(window);
        //- Start with Engine Mode
    this->init_engine();
}

App::~App() {
    free(ctx);
    sestring_deinit(&input_text);
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

        //- UI
    seui_reset(ctx);
    if (seui_panel(ctx, "test")) {
        seui_panel_row(ctx, 32, 3);

        seui_label(ctx, "text");

        if (seui_button(ctx, "button")) {
            printf("pressed button\n");
        }

        seui_input_text(ctx, &input_text);
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

    ctx = NEW(SE_UI);
    seui_init(ctx, &this->input, viewport, -1000, 1000);
    sestring_init(&input_text, "");

        //- Input
    seinput_init(&this->input);

        //- Renderer
    secamera3d_init(&this->camera);
    serender3d_init(&this->renderer, &this->camera);
    this->renderer.light_directional.direction = {0, -1, 0};
    this->renderer.light_directional.ambient   = {50, 50, 50};
    this->renderer.light_directional.diffuse   = {255, 255, 255};
}

void App::init_engine() {
    this->clear();
    this->mode = GAME_MODES::ENGINE;

#if 0 // manually create entities
    // @temp add entities
    u32 soulspear = this->level.add_entity();
    level.entities.mesh_index[soulspear] = serender3d_load_mesh(&this->renderer, "game/meshes/soulspear/soulspear.obj", false);
    level.entities.has_mesh[soulspear] = true;
    level.entities.should_render_mesh[soulspear] = true;
    level.entities.has_name[soulspear] = true;
    sestring_init(&level.entities.name[soulspear], "soulspear entity");

    u32 plane = this->level.add_entity();
    level.entities.mesh_index[plane] = serender3d_add_plane(&this->renderer, v3f(10, 10, 10));
    level.entities.has_mesh[plane] = true;
    level.entities.should_render_mesh[plane] = true;
    level.entities.has_name[plane] = true;
    sestring_init(&level.entities.name[plane], "plane entity");

    this->level.save("test_save_level.level");
#else // load from file
    serender3d_load_mesh(&this->renderer, "game/meshes/soulspear/soulspear.obj", false);
    serender3d_add_plane(&this->renderer, v3f(10, 10, 10));
    this->level.load("test_save_level.level");
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
    this->level.entities.init();
}