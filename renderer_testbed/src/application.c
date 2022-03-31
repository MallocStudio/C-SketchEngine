#include "application.h"
#include "stdio.h" // @remove

void app_init(Application *app, SDL_Window *window) {
    app->window = window;
    app->should_quit = false;

    // -- input input
    seinput_init(&app->input);

    { // -- init camera
        app->camera.position = vec3_create(10, 10, 10);
        app->camera.rotation = quat_identity();
    }

    { // -- init renderer
        serender3d_init(&app->renderer, &app->camera, "Simple.vsd", "Simple.fsd");
    }

    { // -- load mesh

        { // -- cube
            // semesh_generate_cube(&app->mesh, (Vec3){1, 1, 1});
            // semesh_generate_quad(&app->mesh, (Vec2) {1, 1});
        }

        { // -- obj file
            // semesh_load(&app->mesh, "assets/spaceship/Intergalactic_Spaceship-(Wavefront).obj");
            // semesh_load(&app->mesh, "assets/spaceship2/Intergalactic_Spaceship-(FBX 7.4 binary).fbx");
            serender3d_load_mesh(&app->renderer, "assets/skull/12140_Skull_v3_L2.obj");

            // semesh_load_obj(&app->mesh, "assets/soulspear/soulspear/soulspear.obj");

            // semesh_load_obj(&app->mesh, "assets/assassins-creed-altair-obj/assassins-creed-altair.obj");

            // semesh_load_obj(&app->mesh, "assets/cube/cube3.obj");
            // semesh_load_obj(&app->mesh, "assets/skull/12140_Skull_v3_L2.obj");
        }
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
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

        { // @remove
        movement = quat_mul_vec3(app->camera.rotation, movement);

        Mat4 cam_transform = mat4_translation(app->camera.position);
        Mat4 movement_transform = mat4_translation(vec3_mul_scalar(movement, 0.016 * 10));
        Mat4 final_transform = mat4_mul(cam_transform, movement_transform);

        app->camera.position = mat4_get_translation(final_transform);
        }
        {

        }

        { // -- rotate camera
            u8 mouse_state = SDL_GetMouseState(NULL, NULL);
            if (mouse_state & SDL_BUTTON_RMASK) {
                f32 turn_speed = -0.1f * SEMATH_DEG2RAD_MULTIPLIER;

                { // method A
                    // Vec3 rot_input = {0};
                    // rot_input.x = app->input.mouse_screen_pos_delta.x;
                    // rot_input.y = app->input.mouse_screen_pos_delta.y - 1;
                    // printf("delta : {%f, %f}\n", app->input.mouse_screen_pos_delta.x, app->input.mouse_screen_pos_delta.y);

                    // Quat rot = quat_from_axis_angle(rot_input, turn_speed, true);
                    // app->camera.rotation = quat_mul(app->camera.rotation, rot);
                }
                { // method B
                    // Vec3 vertical   = {0};
                    // Vec3 horizontal = {0};
                    // vertical.x   += app->input.mouse_screen_pos_delta.y;
                    // horizontal.y += app->input.mouse_screen_pos_delta.x;
                    // if (horizontal.y != 0 || vertical.x != 0) {
                    //     Quat pitch = quat_from_axis_angle(horizontal, turn_speed, true); // rot around x
                    //     Quat yawn = quat_from_axis_angle(vertical, turn_speed, true);     // rot around y
                    //     Quat rot_delta = quat_mul(pitch, yawn);
                    //     app->camera.rotation = quat_mul(app->camera.rotation, rot_delta);
                    // }
                }
                { // Method C
                    Vec3 rotation = {0};
                    rotation.x = app->input.mouse_screen_pos_delta.x * turn_speed;
                    rotation.y = app->input.mouse_screen_pos_delta.y * turn_speed * -1;
                    Quat x_quat = quat_from_axis_angle(vec3_up(), rotation.x, true);
                    Quat y_quat = quat_from_axis_angle(vec3_left(), rotation.y, true);
                    Quat rot_quat = quat_mul(x_quat, y_quat);
                    app->camera.rotation = quat_mul(app->camera.rotation, rot_quat);
                }
            }
        }
    }
}

void app_render(Application *app) {
    { // -- application level render
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        i32 window_w, window_h;
        SDL_GetWindowSize(app->window, &window_w, &window_h);
        secamera3d_update_projection(&app->camera, window_w, window_h);

        serender3d_render(&app->renderer);
    }
}