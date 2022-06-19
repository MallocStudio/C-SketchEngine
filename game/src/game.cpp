#include "game.hpp"

SE_UI *ctx;
SE_String input_text;

u32 player1 = -1;
u32 player2 = -1;
u32 man = -1;
u32 skeleton_mesh = -1;
SE_Animation animation;

u32 add_entity(Game *game) {
    u32 result = game->entities.count;
    game->entities.count++;
    return result;
}

Game::Game(SDL_Window *window) {
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

        //- Entities
    player1 = add_entity(this);
    this->entities.mesh_index[player1] = serender3d_load_mesh(&this->renderer, "game/meshes/soulspear/soulspear.obj", false);
    this->entities.has_mesh[player1]   = true;

    player2 = add_entity(this);
    this->entities.mesh_index[player2] = serender3d_add_plane(&this->renderer, v3f(10, 10, 10));
    this->entities.has_mesh[player2]   = true;

    man = add_entity(this);
    this->entities.mesh_index[man] = serender3d_load_mesh(&this->renderer, "game/meshes/Booty Hip Hop Dance.fbx", true);
    this->entities.has_mesh[man]   = true;
    this->entities.scale[man] = v3f(0.1f, 0.1f, 0.1f);

    skeleton_mesh = serender3d_add_mesh_empty(&this->renderer);
    this->renderer.meshes[skeleton_mesh]->material_index = this->renderer.material_lines;
    semesh_generate_static_skeleton(this->renderer.meshes[skeleton_mesh], this->renderer.meshes[this->entities.mesh_index[man]]->skeleton);
    se_assert(this->renderer.meshes[skeleton_mesh]->skeleton == NULL);

        //- Animation
    animation.duration = this->renderer.meshes[this->entities.mesh_index[man]]->skeleton->animations[0]->duration;
    animation.speed = this->renderer.meshes[this->entities.mesh_index[man]]->skeleton->animations[0]->ticks_per_second;
    animation.current_frame = 0;
}

Game::~Game() {
    free(ctx);
    sestring_deinit(&input_text);
    serender3d_deinit(&this->renderer);
}

void Game::update(f32 delta_time) {
        //- Update Window and Input
    i32 window_w, window_h;
    SDL_GetWindowSize(this->window, &window_w, &window_h);
    secamera3d_update_projection(&this->camera, window_w, window_h);
    seinput_update(&this->input, this->camera.projection, this->window);
    seui_resize(ctx, window_w, window_h);

        //- 3D Movement
    secamera3d_input(&this->camera, &this->input);

        //- Entities
    this->entities.update_transforms();

    {   //- Animation
        f32 current_frame = seanimation_update(&animation, delta_time);
        SE_Mesh *mesh = this->renderer.meshes[entities.mesh_index[man]];
        seskeleton_calculate_pose(mesh->skeleton, current_frame);
        seskeleton_calculate_pose(this->renderer.meshes[mesh->next_mesh_index]->skeleton, current_frame);
    }

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

void Game::render() {
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
    se_render_directional_shadow_map(&this->renderer, this->entities.transform, this->entities.count);
    se_render_omnidirectional_shadow_map(&this->renderer, this->entities.transform, this->entities.count);

        //- Clear Previous Frame
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_w, window_h);

        //- Render Entities
    for (u32 i = 0; i < this->entities.count; ++i) {
        if (this->entities.has_mesh[i] && this->entities.should_render_mesh[i]) {
            serender_mesh_index(&this->renderer, this->entities.mesh_index[i], this->entities.transform[i]);
        }
    }
        //- Render Special Meshes
    if (skeleton_mesh != -1) serender_mesh_index(&this->renderer, skeleton_mesh, this->entities.transform[man]);

        //- UI
    glClear(GL_DEPTH_BUFFER_BIT);
    seui_render(ctx);
}