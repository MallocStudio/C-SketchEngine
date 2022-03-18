#include "application.h"

void app_init(Application *app, SDL_Window *window) {
    app->window = window;
    app->should_quit = false;

    // -- input input
    seinput_init(&app->input);

    { // -- init camera
        app->camera.position = vec3_create(10, 10, 10);
        app->camera.rotation = quat_identity();
    }

    { // -- init shader
        seshader_init_from(&app->shader, "Simple.vsd", "Simple.fsd");
    }

    { // -- init mesh
        SE_Vertex3D verts[5];
        verts[0].position = (Vec4) {-0.5f, 0, +0.5f, 1};
        verts[1].position = (Vec4) {+0.5f, 0, +0.5f, 1};
        verts[2].position = (Vec4) {-0.5f, 0, -0.5f, 1};
        verts[3].position = (Vec4) {+0.5f, 0, -0.5f, 1};
        verts[4].position = (Vec4) {+1.0f, -0.5f, -0.5f, 1};

        verts[0].rgba = RGBA_RED;
        verts[1].rgba = RGBA_RED;
        verts[2].rgba = RGBA_RED;
        verts[3].rgba = RGBA_BLUE;
        verts[4].rgba = RGBA_GREEN;

        u32 indices[9] = {
            0, 1, 2,
            2, 1, 3,
            3, 1, 4,
        };

        semesh_generate(&app->mesh, 5, verts, 9, indices);

        app->quad_transform = (Mat4) {
            30, 0, 0, 0,
            0, 30, 0, 0,
            0, 0, 30, 0,
            0, 0, 0,  1,
        };
    }
}

void app_deinit(Application *app) {
    semesh_deinit(&app->mesh);
    seshader_deinit(&app->shader);
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

        movement = quat_mul_vec3(app->camera.rotation, movement);

        Mat4 cam_transform = mat4_translation(app->camera.position);
        Mat4 movement_transform = mat4_translation(movement);
        Mat4 final_transform = mat4_mul(cam_transform, movement_transform);

        app->camera.position = mat4_get_translation(final_transform);

        { // AIE tutorial way
            static Vec2 mouse_pos_pre = {0};
            u8 mouse_state = SDL_GetMouseState(NULL, NULL);
            if (mouse_state & SDL_BUTTON_RMASK) {
                app->input.should_mouse_warp = true;
                f32 turn_speed = -0.1f * SEMATH_DEG2RAD_MULTIPLIER;

                Vec3 rot_input = {
                    .y = (f32)(app->input.mouse_screen_pos.x - mouse_pos_pre.x),
                    .x = (f32)(app->input.mouse_screen_pos.y - mouse_pos_pre.y) * -1,
                    .z = 0.0f,
                };

                // app->camera.theta -= turn_speed * (f32)(mouse_pos.x - mouse_pos_pre.x);
                // app->camera.phi   += turn_speed * (f32)(mouse_pos.y - mouse_pos_pre.y);
                Quat rot = quat_from_axis_angle(rot_input, turn_speed, true);
                app->camera.rotation = quat_mul(app->camera.rotation, rot);
            } else {
                // SDL_SetRelativeMouseMode(SDL_FALSE);
                app->input.should_mouse_warp = false;
            }
            mouse_pos_pre = app->input.mouse_screen_pos;
        }
    }
}

void app_render(Application *app) {
    { // -- application level render
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        // secamera3d_update_view(&camera);
        // camera.view = mat4_lookat(camera.pos, vec3_zero(), vec3_up());

        i32 window_w, window_h;
        SDL_GetWindowSize(app->window, &window_w, &window_h);

        app->camera.view = secamera3d_get_view(&app->camera);
        app->camera.projection = mat4_perspective(SEMATH_PI * 0.25f,
                                        window_w / (f32) window_h,
                                        0.1f, 1000.0f);

        // quad_transform = mat4_mul(quad_transform, mat4_euler_y(0.01f));

        seshader_use(&app->shader);

        // take the quad (world space) and project it to view space
        Mat4 pvm = mat4_mul(app->quad_transform, app->camera.view);
        // then take that and project it to the clip space
        pvm = mat4_mul(pvm, app->camera.projection);
        // then pass that final projection matrix and give it to the shader
        seshader_set_uniform_mat4(&app->shader, "projection_view_model", pvm);

        semesh_draw(&app->mesh);
    }
}