#include "application.h"
#include "semesh_loader.h"

void app_init(Application *app, SDL_Window *window) {
    app->window = window;
    app->should_quit = false;

    // -- input input
    seinput_init(&app->input);

    { // -- init camera
        app->camera.position = vec3_create(10, 10, 10);
        app->camera.rotation = quat_identity();
    }

    { // -- init mesh
        seshader_init_from(&app->mesh.material.shader, "Simple.vsd", "Simple.fsd"); // @TODO move to semesh_load_shader()

        { // -- cube
            // semesh_generate_cube(&app->mesh, (Vec3){1, 1, 1});
            // semesh_generate_quad(&app->mesh, (Vec2) {1, 1});
        }

        { // -- obj file
            // semesh_load(&app->mesh, "assets/spaceship/Intergalactic_Spaceship-(Wavefront).obj");
            semesh_load(&app->mesh, "assets/spaceship2/Intergalactic_Spaceship-(FBX 7.4 binary).fbx");
            // semesh_load(&app->mesh, "assets/skull/12140_Skull_v3_L2.obj");

            // semesh_load_obj(&app->mesh, "assets/soulspear/soulspear/soulspear.obj");

            // semesh_load_obj(&app->mesh, "assets/assassins-creed-altair-obj/assassins-creed-altair.obj");

            // semesh_load_obj(&app->mesh, "assets/cube/cube3.obj");
            // semesh_load_obj(&app->mesh, "assets/skull/12140_Skull_v3_L2.obj");
        }

        app->mesh.transform = (Mat4) {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0,  1,
        };
    }
}

void app_deinit(Application *app) {
    semesh_deinit(&app->mesh);
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
        Mat4 movement_transform = mat4_translation(vec3_mul_scalar(movement, 0.016 * 10));
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
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
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

        // app->quad_transform = mat4_mul(app->quad_transform, mat4_euler_y(0.01f));

        { // -- normals

        }

        { // -- lighting
            // Vec3 from_light = {0, -1, 0}; // light towards down
            // vec3_normalise(&from_light);

            // seshader_set_uniform_vec3(&app->shader, "fromt_light", from_light);
        }

        semesh_draw(&app->mesh, &app->camera);
    }
}