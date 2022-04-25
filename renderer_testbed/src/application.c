#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"

#include "temp_shadows.h" // @remove this after implementing shadows and abstracting it from simple procedures

/* ui */
SE_UI *ctx;
SEUI_Panel panel;

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
        app->entities[player2].transform = mat4_translation((Vec3) {0, -1.2f, 0}); // mat4_translation(vec3_create(10, 0, 5));
    }

    { // -- init UI
        ctx = new (SE_UI);
        seui_init(ctx, &app->input, window_w, window_h);
        panel.initial_rect = (Rect) {250, 300, 300, 400};
        panel.minimised = false;
    }

    { // -- load mesh
        app->entities[player].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");
        // app->entities[player2].mesh_index = serender3d_add_cube(&app->renderer);
        app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/models/plane/plane.fbx");
        app->entities[player2].transform = mat4_mul(mat4_euler_x(SEMATH_HALF_PI), app->entities[player2].transform);
    }

    { // -- shadow mapping
        shadow_init();
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    seui_deinit(ctx);
    seshader_deinit(&shadow_shader);
    seshader_deinit(&shadow_depth_map_shader);
}

f32 slider_value = 0.5f;
Vec2 slider2d_value = {0};
void app_update(Application *app) {
    // -- input
    u32 window_w, window_h;
    SDL_GetWindowSize(app->window, &window_w, &window_h);
    secamera3d_update_projection(&app->camera, window_w, window_h);
    seinput_update(&app->input, app->camera.projection, app->window);
    seui_resize(ctx, window_w, window_h);

    const u8 *keyboard = app->input.keyboard;
    if (keyboard[SDL_SCANCODE_ESCAPE]) app->should_quit = true;

    secamera3d_input(&app->camera, &app->input);

    { // -- ui
        seui_reset(ctx);

        if (seui_panel_at(ctx, "panel", 2, 100, &panel)) {
            seui_label(ctx, "light direction:");
            seui_slider2d(ctx, &slider2d_value);
        }
    }
}

void app_render(Application *app) {
    { // -- application level render
        i32 window_w, window_h;
        SDL_GetWindowSize(app->window, &window_w, &window_h);
        secamera3d_update_projection(&app->camera, window_w, window_h);

        // Vec3 cam_forward = mat4_forward(app->camera.view);
        // app->renderer.light_directional.direction = cam_forward;
        Vec3 light_direction = {
            -slider2d_value.x,
            -slider2d_value.y,
            0
        };
        vec3_normalise(&light_direction);
        app->renderer.light_directional.direction = light_direction; //vec3_right();

        // 1. render to depth map
        { // -- shadow mapping
            shadow_render(app, &app->renderer);
        }

        // 2. render normally with the shadow map
        RGB ambient = app->renderer.light_directional.ambient;
        rgb_normalise(&ambient);
        glClearColor(ambient.r, ambient.g, ambient.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_w, window_h);
        { // @cleanup
        glActiveTexture(GL_TEXTURE0 + 3); // shadow map
        glBindTexture(GL_TEXTURE_2D, shadow_depth_map);
        }

        for (u32 i = 0; i < app->entity_count; ++i) {
            entity_render(&app->entities[i], &app->renderer);
        }

        // // { // @temp an attempt to draw our shadow map to the screen
        //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //     seshader_use(&shadow_depth_map_shader);
        //     glBindBuffer(GL_ARRAY_BUFFER, shadow_vbo);
        //     glActiveTexture(GL_TEXTURE0);
        //     glBindTexture(GL_TEXTURE_2D, shadow_depth_map);
        //     glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        //     glBindBuffer(GL_ARRAY_BUFFER, 0);
        // // }
    }
    { // -- ui
        seui_render(ctx);
    }
}

u32 app_add_entity(Application *app) {
    u32 result = app->entity_count;
    app->entity_count++;
    return result;
}