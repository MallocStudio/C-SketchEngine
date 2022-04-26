#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"

/* ui */
SE_UI *ctx;
SEUI_Panel panel;

Vec3 light_pos;
Vec3 light_pos_normalised;

/* to render any texture on the screen */
// u32 cheat_vbo;
// Mat4 cheat_transform;

u32 player       = -1;
u32 player2      = -1;
u32 player3      = -1;
u32 light_entity = -1;
u32 shadow_map_plane_entity = -1;

u32 shadow_map_shader = -1;
u32 lit_shader = -1;

// @temp
bool depth_mode_lequal = true;

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
        serender3d_init(&app->renderer, &app->camera, "shaders/Simple.vsd", "shaders/Simple.fsd");
        shadow_map_shader = serender3d_add_shader(&app->renderer, "shaders/_ShadowDepthMap.vsd", "shaders/_ShadowDepthMap.fsd");
        app->renderer.light_directional.direction = (Vec3) {0, -1, 0};
        app->renderer.light_directional.ambient   = (RGB)  {50, 50, 50};
        app->renderer.light_directional.diffuse   = (RGB)  {255, 255, 255};
        light_pos = (Vec3) {0, 1, 0};
        light_pos_normalised = vec3_normalised(light_pos);
    }

    { // -- init entities
        player  = app_add_entity(app);
        player2 = app_add_entity(app);
        player3 = app_add_entity(app);
        light_entity = app_add_entity(app);
        shadow_map_plane_entity = app_add_entity(app);
        app->entities[player].transform = mat4_translation(vec3_zero());
        app->entities[player2].transform = mat4_translation((Vec3) {0, -1.2f, 0}); // mat4_translation(vec3_create(10, 0, 5));
        app->entities[player3].transform = mat4_translation((Vec3) {-2.0f, -2.2f, -1.0f}); // mat4_translation(vec3_create(10, 0, 5));
        app->entities[light_entity].transform = mat4_translation(light_pos);

        app->entities[shadow_map_plane_entity].transform = mat4_translation((Vec3) {10, 0, 10});
        // app->entities[shadow_map_plane_entity].transform = mat4_mul(mat4_euler_z(SEMATH_HALF_PI), app->entities[shadow_map_plane_entity].transform);
        app->entities[shadow_map_plane_entity].transform = mat4_mul(mat4_scale((Vec3) {10, 10, 10}), app->entities[shadow_map_plane_entity].transform);
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
        // app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/models/plane/plane.fbx");

        app->entities[player2].mesh_index = serender3d_add_plane(&app->renderer);
        app->entities[player2].transform = mat4_mul(mat4_scale((Vec3) {20.0f, 20.0f, 20.0f}), app->entities[player2].transform);

        app->entities[light_entity].mesh_index = serender3d_add_cube(&app->renderer);

        app->entities[player3].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");
        app->entities[player3].transform = mat4_mul(mat4_euler_x(SEMATH_HALF_PI), app->entities[player3].transform);

        app->entities[shadow_map_plane_entity].mesh_index = serender3d_add_plane(&app->renderer);
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    seui_deinit(ctx);
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

            char light_x_label[100];
            sprintf(light_x_label, "x: %f", light_pos.x);
            char light_y_label[100];
            sprintf(light_y_label, "y: %f", light_pos.y);
            char light_z_label[100];
            sprintf(light_z_label, "z: %f", light_pos.z);

            seui_label(ctx, light_x_label);
            seui_slider(ctx, &light_pos_normalised.x);
            seui_label(ctx, light_y_label);
            seui_slider(ctx, &light_pos_normalised.y);
            seui_label(ctx, light_z_label);
            seui_slider(ctx, &light_pos_normalised.z);

            light_pos.x = light_pos_normalised.x * 10 - 5;
            light_pos.y = light_pos_normalised.y * 10 - 5;
            light_pos.z = light_pos_normalised.z * 10 - 5;

            app->entities[light_entity].transform = mat4_translation(light_pos);

            char depth_test_mode_label[100];
            if (depth_mode_lequal) {
                sprintf(depth_test_mode_label, "depth test: lequal");
            } else {
                sprintf(depth_test_mode_label, "depth test: gequal");
            }
            if (seui_button(ctx, depth_test_mode_label)) depth_mode_lequal = !depth_mode_lequal;
        }
    }
}

static void render_shadow_map(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];
    // take the quad (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    seshader_use(renderer->shaders[shadow_map_shader]); // use the default shader

    // the good old days when debugging:
    // material->texture_diffuse.width = 100;

    seshader_set_uniform_mat4(renderer->shaders[shadow_map_shader], "projection_view_model", pvm);
    seshader_set_uniform_mat4(renderer->shaders[shadow_map_shader], "model_matrix", transform);
    seshader_set_uniform_vec3(renderer->shaders[shadow_map_shader], "camera_pos", renderer->current_camera->position);

    /* material uniforms */
    seshader_set_uniform_i32(renderer->shaders[shadow_map_shader], "shadow_map", 3); // @temp

    // light uniforms
    glActiveTexture(GL_TEXTURE0 + 0); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.texture);

    glBindVertexArray(mesh->vao);

    if (mesh->indexed) {
        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
    }

    glBindVertexArray(0);
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

        { // -- shadow mapping
            Vec3 light_target = vec3_add(app->renderer.light_directional.direction, light_pos);
            Mat4 light_view = mat4_lookat(light_pos, light_target, vec3_up());
            // Mat4 light_view = mat4_lookat(light_pos, vec3_zero(), vec3_up());

            f32 near_plane = 1.0f, far_plane = 7.5f;
            Mat4 light_proj = mat4_ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); // what is visible to the light
            // Mat4 light_proj = mat4_perspective(SEMATH_PI * 0.25f, window_w / (f32) window_h, 0.1f, 1000.0f);

            Mat4 light_space_mat = mat4_mul(light_view, light_proj);
            // light_space_mat = mat4_mul(mat4_euler_x(SEMATH_HALF_PI), light_space_mat);

            /* configure shadow shader */
            serender_target_use(&app->renderer.shadow_render_target);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            seshader_use(&app->renderer.shadow_shader);
            seshader_set_uniform_mat4(&app->renderer.shadow_shader, "light_space_matrix", light_space_mat);

            { /* render the scene from the light's point of view */

                if (depth_mode_lequal) {
                    glDepthFunc(GL_LEQUAL);
                } else {
                    glDepthFunc(GL_GEQUAL);
                }

                for (u32 i = 0; i < app->entity_count; ++i) {
                    Entity *entity = &app->entities[i];
                    SE_Mesh *mesh = app->renderer.meshes[entity->mesh_index];
                    Mat4 model_mat = entity->transform;

                    seshader_set_uniform_mat4(&app->renderer.shadow_shader, "model", model_mat);

                    glBindVertexArray(mesh->vao);
                    if (mesh->indexed) {
                        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
                    } else {
                        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
                    }
                }
                glBindVertexArray(0);
                glDepthFunc(GL_LEQUAL);
            }

            serender_target_use(NULL);

            app->renderer.light_space_matrix = light_space_mat;
        }

        // 2. render normally with the shadow map
        RGB ambient = app->renderer.light_directional.ambient;
        rgb_normalise(&ambient);
        glClearColor(ambient.r, ambient.g, ambient.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_w, window_h);

        for (u32 i = 0; i < app->entity_count - 1; ++i) {
            entity_render(&app->entities[i], &app->renderer);
        }

        Entity *e = &app->entities[shadow_map_plane_entity];
        render_shadow_map(&app->renderer, e->mesh_index, e->transform);
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