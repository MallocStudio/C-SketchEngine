#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"

#if 0
SEUI_Context *ctx;
#endif

void app_init(Application *app, SDL_Window *window) {
    app->window = window;
    app->should_quit = false;
    u32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    // -- input input
    seinput_init(&app->input);

    { // -- init camera
        app->camera.position = vec3_create(10, 10, 10);
        app->camera.oriantation = vec2_zero();
    }

    { // -- init renderer
        serender3d_init(&app->renderer, &app->camera, "Simple.vsd", "Simple.fsd");
        app->renderer.light_directional.direction = vec3_create(0, -1, 0);
        app->renderer.light_directional.ambient   = (RGB) {50, 50, 50};
        app->renderer.light_directional.diffuse   = (RGB) {255, 255, 255};
    }

    #if 0
    { // -- init UI
        ctx = new(SEUI_Context);
        seui_init(ctx, (Rect) {0, 0, window_w, window_h}, "UI.vsd", "UI.fsd", &app->input);
    }
    #endif

    { // -- load mesh
        // serender3d_load_mesh(&app->renderer, "assets/skull/12140_Skull_v3_L2.obj");
        serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    #if 0
    seui_deinit(ctx);
    #endif
}

void app_update(Application *app) {
    // -- input
    seinput_update(&app->input, app->camera.projection, app->window);

    { // -- update camera
        const u8 *keyboard = app->input.keyboard;
        if (keyboard[SDL_SCANCODE_ESCAPE]) app->should_quit = true;

        i32 r = keyboard[SDL_SCANCODE_D] == true       ? 1 : 0;
        i32 l = keyboard[SDL_SCANCODE_A] == true       ? 1 : 0;
        i32 d = keyboard[SDL_SCANCODE_S] == true       ? 1 : 0;
        i32 u = keyboard[SDL_SCANCODE_W] == true       ? 1 : 0;
        i32 elevate = keyboard[SDL_SCANCODE_E] == true ? 1 : 0;
        i32 dive = keyboard[SDL_SCANCODE_Q] == true    ? 1 : 0;

        Vec3 input = vec3_create(r - l, d - u, elevate - dive);
        Vec3 movement = {
            input.x, input.z, input.y
        };

        { // @incomplete
            Mat4 cam_transform = mat4_translation(app->camera.position);

            Mat4 movement_transform = mat4_translation(vec3_mul_scalar(movement, 0.016 * 10));

            Quat rotation_q_x = quat_from_axis_angle(vec3_up(), app->camera.oriantation.x, true);
            Quat rotation_q_y = quat_from_axis_angle(vec3_right(), app->camera.oriantation.y, true);
            Quat rotation_q = quat_mul(rotation_q_x, rotation_q_y);

            Mat4 rotation = quat_to_rotation_matrix(rotation_q, app->camera.position);

            Mat4 final_transform = mat4_mul(cam_transform, movement_transform);
            final_transform = mat4_mul(rotation, final_transform); // @note doing this does not change the transform because we go mat4_get_translation below

            app->camera.position = mat4_get_translation(final_transform);
        }

        { // -- rotate camera
            u8 mouse_state = SDL_GetMouseState(NULL, NULL);
            if (mouse_state & SDL_BUTTON_RMASK) {
                f32 turn_speed = -0.1f * SEMATH_DEG2RAD_MULTIPLIER;
                app->camera.oriantation.x += app->input.mouse_screen_pos_delta.x * turn_speed;
                app->camera.oriantation.y += app->input.mouse_screen_pos_delta.y * turn_speed;
            }
        }
    }
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

        Vec3 cam_forward = mat4_forward(app->camera.view);
        app->renderer.light_directional.direction = cam_forward;

        serender3d_render(&app->renderer);
    }
    #if 0
    { // -- ui
        // seui_renderer_draw(&ctx->renderer);
        seui_render(ctx);
    }
    #endif
}