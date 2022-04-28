#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"

/* ui */
SE_UI *ctx;
SEUI_Panel panel;

f32 light_intensity = 0.5f;

/* to render any texture on the screen */
// u32 cheat_vbo;
// Mat4 cheat_transform;

u32 player       = -1;
u32 player2      = -1;
u32 player3      = -1;

AABB3D world_aabb;
void app_init(Application *app, SDL_Window *window) {
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
        serender3d_init(&app->renderer, &app->camera);
        app->renderer.light_directional.direction = (Vec3) {0, -1, 0};
        app->renderer.light_directional.ambient   = (RGB)  {50, 50, 50};
        app->renderer.light_directional.diffuse   = (RGB)  {255, 255, 255};
        // light_pos = (Vec3) {0.5f, 1, 0.5f};
        // light_pos_normalised = vec3_normalised(light_pos);
    }

    { // -- init entities
        player  = app_add_entity(app);
        player2 = app_add_entity(app);
        player3 = app_add_entity(app);
        app->entities[player].transform = mat4_translation(vec3_zero());
        app->entities[player2].transform = mat4_translation((Vec3) {0, -1.2f, 0}); // mat4_translation(vec3_create(10, 0, 5));
        app->entities[player3].transform = mat4_translation((Vec3) {-2.0f, -2.2f, -1.0f}); // mat4_translation(vec3_create(10, 0, 5));
    }

    { // -- init UI
        ctx = new (SE_UI);
        seui_init(ctx, &app->input, window_w, window_h);
        panel.initial_rect = (Rect) {250, 300, 300, 400};
        panel.minimised = false;
    }

    { // -- load mesh
        app->entities[player].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");

        // app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/models/plane/plane.fbx");
        // app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/models/cube/cube3.obj");
        app->entities[player2].mesh_index = serender3d_add_plane(&app->renderer, (Vec3) {20.0f, 20.0f, 20.0f});

        app->entities[player3].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");
        app->entities[player3].transform = mat4_mul(mat4_euler_x(SEMATH_HALF_PI), app->entities[player3].transform);
    }

    { // -- calculate world aabb
        AABB3D aabb[3];
        for (u32 i = 0; i < 3; ++i) {
            aabb[i] = app->renderer.meshes[app->entities[player3].mesh_index]->aabb;
            aabb[i].min = vec3_add(aabb[i].min, mat4_get_translation(app->entities[i].transform));
            aabb[i].max = vec3_add(aabb[i].max, mat4_get_translation(app->entities[i].transform));
        }
        world_aabb = aabb3d_calc(&aabb[0], 3);
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    seui_deinit(ctx);
}

f32 slider_value = 0.5f;
Vec2 slider2d_value = {0, -1};
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
            seui_label(ctx, "light intensity:");
            seui_slider(ctx, &light_intensity);

            // char light_x_label[100];
            // sprintf(light_x_label, "x: %f", light_pos.x);
            // char light_y_label[100];
            // sprintf(light_y_label, "y: %f", light_pos.y);
            // char light_z_label[100];
            // sprintf(light_z_label, "z: %f", light_pos.z);

            // seui_label(ctx, light_x_label);
            // seui_slider(ctx, &light_pos_normalised.x);
            // seui_label(ctx, light_y_label);
            // seui_slider(ctx, &light_pos_normalised.y);
            // seui_label(ctx, light_z_label);
            // seui_slider(ctx, &light_pos_normalised.z);

            // light_pos.x = light_pos_normalised.x * 20 - 10;
            // light_pos.y = light_pos_normalised.y * 20 - 10;
            // light_pos.z = light_pos_normalised.z * 20 - 10;
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
            slider2d_value.x,
            slider2d_value.y,
            0
        };
        vec3_normalise(&light_direction);
        app->renderer.light_directional.direction = light_direction; //vec3_right();

        { // -- shadow mapping
            /* calculate the matrices */
            f32 near_plane = 0.1f, far_plane = 7.5f, border_size = 10.0f;
            // what is visible to the light
            // Mat4 light_proj = mat4_ortho(-border_size, border_size, -border_size, border_size, near_plane, far_plane);
            f32 left   = world_aabb.min.x;
            f32 right  = world_aabb.max.x;
            f32 bottom = world_aabb.min.y;
            f32 top    = world_aabb.max.y;
            f32 near = 0.1f;
            f32 far = vec3_distance(world_aabb.min, world_aabb.max);
            Mat4 light_proj = mat4_ortho(left, right, bottom, top, near, far);

            Vec3 light_pos = world_aabb.max;
            Vec3 light_target = vec3_add(app->renderer.light_directional.direction, light_pos);
            Mat4 light_view = mat4_lookat(light_pos, light_target, vec3_up());
            // Mat4 light_view = mat4_lookat(light_pos, vec3_zero(), vec3_up());

            Mat4 light_space_mat = mat4_mul(light_view, light_proj);

            { /* render the scene from the light's point of view */
                glCullFace(GL_FRONT);
                /* configure shadow shader */
                serender_target_use(&app->renderer.shadow_render_target);
                glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
                seshader_use(app->renderer.shaders[app->renderer.shader_shadow_calc]);

                seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_calc], "light_space_matrix", light_space_mat);

                for (u32 i = 0; i < app->entity_count; ++i) {
                    Entity *entity = &app->entities[i];
                    SE_Mesh *mesh = app->renderer.meshes[entity->mesh_index];
                    Mat4 model_mat = entity->transform;

                    seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_calc], "model", model_mat);

                    glBindVertexArray(mesh->vao);
                    if (mesh->indexed) {
                        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
                    } else {
                        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
                    }
                }
                glBindVertexArray(0);

                glCullFace(GL_BACK);
            }

            serender_target_use(NULL);

            app->renderer.light_space_matrix = light_space_mat;
        }

        // 2. render normally with the shadow map
        app->renderer.light_directional.intensity = light_intensity;
        RGB ambient = app->renderer.light_directional.ambient;
        rgb_normalise(&ambient);
        glClearColor(ambient.r, ambient.g, ambient.b, 1.0f);
        // glClearColor(1, 1, 1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_w, window_h);

        for (u32 i = 0; i < app->entity_count; ++i) {
            entity_render(&app->entities[i], &app->renderer);
        }
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