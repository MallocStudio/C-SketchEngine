#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"

#if 0
SEUI_Context *ctx;
#endif

SE_UI *ctx;

void app_init(Application *app, SDL_Window *window) {
    u32 player = -1; // @remove
    u32 player2 = -1;
    memset(app, 0, sizeof(Application));

    app->window = window;
    app->should_quit = false;
    u32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    // -- input input
    seinput_init(&app->input);

    { // -- init camera
        secamera3d_init(&app->camera);
    }

    { // -- init renderer
        serender3d_init(&app->renderer, &app->camera, "shaders/Simple.vsd", "shaders/Simple.fsd");
        app->renderer.light_directional.direction = (Vec3) {0, -1, 0};
        app->renderer.light_directional.ambient   = (RGB)  {50, 50, 50};
        app->renderer.light_directional.diffuse   = (RGB)  {255, 255, 255};
    }

    { // -- init entities
        player  = app_add_entity(app);
        player2 = app_add_entity(app);
        app->entities[player].transform = mat4_translation(vec3_zero());
        app->entities[player2].transform = mat4_translation(vec3_create(10, 0, 5));
    }

    { // -- init UI
        // ctx = new(SEUI_Context);
        // seui_init(ctx, (Rect) {0, 0, window_w, window_h}, "UI.vsd", "UI.fsd", &app->input);
        ctx = new (SE_UI);
        seui_init(ctx, &app->input, window_w, window_h);
    }

    { // -- load mesh
        // serender3d_load_mesh(&app->renderer, "assets/skull/12140_Skull_v3_L2.obj");
        app->entities[player].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");
        app->entities[player2].mesh_index = serender3d_add_cube(&app->renderer);
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    seui_deinit(ctx);
}

void app_update(Application *app) {
    // -- input
    u32 window_w, window_h;
    SDL_GetWindowSize(app->window, &window_w, &window_h);
    secamera3d_update_projection(&app->camera, window_w, window_h);
    seinput_update(&app->input, app->camera.projection, app->window);

    const u8 *keyboard = app->input.keyboard;
    if (keyboard[SDL_SCANCODE_ESCAPE]) app->should_quit = true;

    secamera3d_input(&app->camera, &app->input);

    #if 0
    { // -- ui
        // seui_render_rect(&ctx->renderer, (Rect) {0, 0, 200, 100}, RGBA_RED);
        // seui_render_rect(&ctx->renderer, (Rect) {100, 200, 200, 100}, RGBA_RED);
        // seui_renderer_upload(&ctx->renderer);
        seui_begin(ctx);
        seui_panel_begin(ctx, (Rect) {0, 0, 200, 100});
    }
    #endif
}

void app_render(Application *app) {
    {
        RGB ambient = app->renderer.light_directional.ambient;
        rgb_normalise(&ambient);
        glClearColor(ambient.r, ambient.g, ambient.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    { // -- application level render
        i32 window_w, window_h;
        SDL_GetWindowSize(app->window, &window_w, &window_h);
        secamera3d_update_projection(&app->camera, window_w, window_h);

        // Vec3 cam_forward = mat4_forward(app->camera.view);
        // app->renderer.light_directional.direction = cam_forward;
        app->renderer.light_directional.direction = vec3_right();

        for (u32 i = 0; i < app->entity_count; ++i) {
            entity_render(&app->entities[i], &app->renderer);
        }
    }
    { // -- ui
        // seui_renderer_draw(&ctx->renderer);
        // seui_render(ctx);
        seui_reset(ctx);
        if (seui_button(ctx, "test", (Rect) {10, 10, 200, 100})) {
            printf("pressed\n");
        }
        if (seui_button(ctx, "test 2", (Rect) {10, 400, 200, 100})) {
            printf("pressed 2\n");
        }
        seui_render(ctx);
    }
}

u32 app_add_entity(Application *app) {
    u32 result = app->entity_count;
    app->entity_count++;
    return result;
}