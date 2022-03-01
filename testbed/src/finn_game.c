#include "finn_game.h"
#include "string.h"
#include "seinput.h"

/// add a shape to game
/// in case of planes, pos.x is the depth
void add_shape_to_game(Finn_Game *game, SE_SHAPES shape_type, Vec2 pos, Vec2 normal) {
    if (shape_type == SE_SHAPES_NONE || shape_type == SE_SHAPES_COUNT) return;
    if (game->objects_count >= FINN_GAME_MAX_NUM_OF_OBJECTS) {
        printf("WARNING: reached max object count. So we don't instantiate a new object\n");
        return;
    }
    SE_Shape *shape = NULL;
    switch (shape_type) {
        case SE_SHAPES_CIRCLE: {
            SE_Circle *circle = new(SE_Circle);
            init_circle(circle);
            circle->pos = pos;
            circle->radius = 1.0f;
            shape = (SE_Shape*) circle;
        } break;
        case SE_SHAPES_BOX: {
            SE_Box *box = new(SE_Box);
            init_box(box);
            box->x = pos.x;
            box->y = pos.y;
            shape = (SE_Shape*) box;
        } break;
        case SE_SHAPES_PLANE: {
            SE_Plane *plane = new(SE_Plane);
            init_plane(plane);
            plane->depth = pos.x;
            // plane->depth = vec2_magnitude(pos);
            plane->normal = normal;
            shape = (SE_Shape*) plane;
        } break;
    }

    SDL_assert(shape != NULL && "add_shape_to_game: shape could not be resolved based on its type");
    game->objects[game->objects_count] = shape;
    game->objects_count++;
}

void clear_shapes(Finn_Game *game) {
    for (i32 i = 0; i < game->objects_count; ++i) {
        free(game->objects[i]);
        game->objects[i] = NULL;
    }
    game->objects_count = 0;
}

void create_boundaries(Finn_Game *game) {
    f32 depth = -10.0f;
    Vec2 normals[4] = {
        {-1, 0}, {0, -1}, {1, 0}, {0, 1}
    };
    for (i32 i = 0; i < 4; ++i) {
        add_shape_to_game(game, SE_SHAPES_PLANE, (Vec2){depth, 0}, normals[i]);
    }
}

/// -----------------
/// Finn Game example
/// -----------------
void finn_game_init(Finn_Game *game, SDL_Window *window) {
    game->window = window;
    
    i32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    { // -- world camera
        game->camera = new(SEGL_Camera);
        segl_camera_init(game->camera);
        game->camera->aspect_ratio = window_w / window_h;

        game->shader_program = new(SEGL_Shader_Program);
        segl_shader_program_init_from(game->shader_program, "Simple.vsd", "Simple.fsd");

        segl_lines_init(&game->lines);
    }

    // -- txt library
    setext_init(&game->txt_library, (Rect) {0, 0, window_w, window_h});

    // -- input
    seinput_reset(&game->input);

    { // -- init ui
        ui_init_context(&game->ui_context, &game->input);
        game->test_ui_init_rect = (Rect) {0, 0, 0, 0};
    }

    { // -- render grid once
        glClearColor(0, 0, 0, 1);

        segl_lines_init(&game->grid);
        f32 grid_limits = 10.0f;
        for (f32 i = -grid_limits; i <= grid_limits; i++) {
            game->grid.current_colour = (i == 0) ? (Vec3) {0.8f, 0.8f, 0.8f} : (Vec3) {0.3f, 0.3f, 0.3f};
            segl_lines_draw_line_segment(&game->grid, (Vec2) {i, -grid_limits}, (Vec2) {i, grid_limits});
            segl_lines_draw_line_segment(&game->grid, (Vec2) {-grid_limits, i}, (Vec2) {grid_limits, i});
        }
        game->grid.current_colour = (Vec3) {1, 0, 0};
        segl_lines_draw_line_segment(&game->grid, (Vec2) {0,0}, (Vec2) {1, 0});
        game->grid.current_colour = (Vec3) {0, 1, 0};
        segl_lines_draw_line_segment(&game->grid, (Vec2) {0,0}, (Vec2) {0, 1});
        segl_lines_compile(&game->grid);
    }

    { // -- populate game objects
        game->current_selected_shape_mode = SE_SHAPES_NONE;
        game->objects_count = 0;
        memset(game->objects, 0, FINN_GAME_MAX_NUM_OF_OBJECTS);

        create_boundaries(game);
    }

    // -- physics
    game->is_paused = false;
    game->is_physics_update_queued = false;
    game->elasticity = 0;
}

void finn_game_deinit(Finn_Game *game) {
    // -- ui
    ui_deinit_context(&game->ui_context);

    // -- line renderer
    segl_lines_deinit(&game->grid);
    segl_lines_deinit(&game->lines);

    // -- shaders
    segl_shader_program_deinit(game->shader_program);
    free(game->shader_program);

    // -- text library
    setext_deinit(&game->txt_library);

    // -- camera
    free(game->camera);

    // -- objects
    clear_shapes(game);
}


void finn_game_update(Finn_Game *game, f32 delta_time) {
    // -- window and viewport size and resize
    i32 width, height;
    SDL_GetWindowSize(game->window, &width, &height);
    game->camera->aspect_ratio = width / (float)height;
    Rect screen_viewport = {0, 0, width, height};
    glViewport(screen_viewport.x, screen_viewport.y, screen_viewport.w, screen_viewport.h);

    // -- update input
    seinput_update(&game->input, segl_get_camera_transform(game->camera), (Vec2i){width, height});

    { // -- ui context
        // -- call update
        ui_update_context(&game->ui_context, screen_viewport);
        
        // -- handle ui
        ui_begin(&game->ui_context, &game->test_ui_init_rect);
        
        ui_row(&game->ui_context, 1, 48, 120);
        if (ui_button(&game->ui_context, "box")) game->current_selected_shape_mode = SE_SHAPES_BOX;
        
        ui_row(&game->ui_context, 1, 48, 120);
        if (ui_button(&game->ui_context, "circle")) game->current_selected_shape_mode = SE_SHAPES_CIRCLE;

        // ui_row(&game->ui_context, 1, 48, 120);
        // if (ui_button(&game->ui_context, "plane")) game->current_selected_shape_mode = SE_SHAPES_PLANE;
        
        ui_row(&game->ui_context, 2, 48, 120);
        if (ui_button(&game->ui_context, "elasticity 0")) game->elasticity = 0;
        if (ui_button(&game->ui_context, "elasticity 1")) game->elasticity = 1;

        ui_row(&game->ui_context, 3, 48, 240);
        if (game->is_paused) {
            if (ui_button(&game->ui_context, "unpause")) game->is_paused = false;
            if (ui_button(&game->ui_context, ">")) game->is_physics_update_queued = true;
        } else {
            if (ui_button(&game->ui_context, "pause")) game->is_paused = true;
        }

        // give the resizable item enough space // @incomplete move this to ui_begin or ui_render
        ui_row(&game->ui_context, 1, 18, 0);
    }

    // -- move the camera around
    if (game->input.keyboard[SDL_SCANCODE_LEFT] || game->input.keyboard[SDL_SCANCODE_A]) {
        game->camera->center.x -= game->camera->speed * delta_time * game->camera->height;
    }
    if (game->input.keyboard[SDL_SCANCODE_RIGHT] || game->input.keyboard[SDL_SCANCODE_D]) {
        game->camera->center.x += game->camera->speed * delta_time * game->camera->height;
    }
    if (game->input.keyboard[SDL_SCANCODE_UP] || game->input.keyboard[SDL_SCANCODE_W]) {
        game->camera->center.y += game->camera->speed * delta_time * game->camera->height;
    }
    if (game->input.keyboard[SDL_SCANCODE_DOWN] || game->input.keyboard[SDL_SCANCODE_S]) {
        game->camera->center.y -= game->camera->speed * delta_time * game->camera->height;
    }

    { // -- object instantiation
        if (game->input.is_mouse_left_handled == false && game->input.is_mouse_left_down && !game->input.was_mouse_left_down && game->current_selected_shape_mode != SE_SHAPES_NONE) {
            Vec2 normal = vec2_sub(game->input.mouse_world_pos, game->input.mouse_world_pressed_pos);
            vec2_normalise(&normal);
            add_shape_to_game(game, game->current_selected_shape_mode, game->input.mouse_world_pos, normal);
        }
        if (game->input.is_mouse_left_handled == false && game->input.is_mouse_right_down && !game->input.was_mouse_right_down) {
            clear_shapes(game);
            
            create_boundaries(game);
        }
    }

    { // -- pause unpause
        if (game->input.keyboard[SDL_SCANCODE_SPACE]) {
            game->is_paused = !game->is_paused;
        }
    }
}


void finn_game_physics_update(Finn_Game *game, f32 delta_time) {
    { // -- move physics objects
        for (i32 i = 0; i < game->objects_count; ++i) {
            SE_Shape *shape = game->objects[i];

            // if (shape->type == SE_SHAPES_PLANE) continue; // ! hhhhhhmmmm
            if (shape->inverse_mass == 0) continue;

            // -- apply a constant force
            Vec2 gravity = {0, -0.2f * delta_time};
            // Vec2 gravity = {0.2f * delta_time, -0.2f * delta_time};
            shape->velocity = vec2_add(shape->velocity, gravity);

            // -- move
            Vec2 shape_pos, shape_normal;
            se_phys_get_shape_transform(shape, &shape_pos, &shape_normal);
            shape_pos = vec2_add(shape_pos, shape->velocity);
            se_phys_set_shape_transform(shape, shape_pos, shape_normal);
        }
    }
    { // -- physics collision check through function pointer table
        for (i32 i = 0; i < game->objects_count; ++i) { 
            for (i32 j = i + 1; j < game->objects_count; ++j) {
                SE_Shape *shape_a = game->objects[i];
                SE_Shape *shape_b = game->objects[j];

                if (shape_a->inverse_mass == 0 && shape_b->inverse_mass == 0) continue;

                if (shape_a->type >= 0 && shape_b->type >= 0) {
                    i32 function_index = (shape_a->type * SE_SHAPES_COUNT) + shape_b->type;
                    se_phys_check collision_function_pointer = collision_function_array[function_index];
                    SDL_assert(collision_function_pointer != NULL && "collision_function_pointer resolved to null");
                    
                    SE_Collision_Data collision_data = collision_function_pointer(shape_a, shape_b);

                    // -- collision resolution
                    if (collision_data.is_collided == true) {
                        // just being paranoid
                        SDL_assert(collision_data.shape_a == shape_a);
                        SDL_assert(collision_data.shape_b == shape_b);

                        // e = elasticity
                        // n = collision normal
                        //
                        //              -(1+e)rel_v . n
                        // j =  -----------------------------------
                        //     n . n(inverse_massA + in0verse_massB)
                        //
                        // j = impulse magnitude

                        Vec2 rel_v = vec2_sub(shape_a->velocity, shape_b->velocity); // rel_v
                        f32 numerator = vec2_dot(rel_v, collision_data.normal);      // rel_v . n
                        numerator = numerator * -(1 + game->elasticity);             // -(1+e)rel_v . n

                        f32 denominator = shape_a->inverse_mass + shape_b->inverse_mass;
                        // ! note that the denominator cannot be zero becasue we should not run this block of code
                        // ! if shape_a->inverse_mass AND shape_b->inverse_mass == 0
                        f32 impulse_magnitude = numerator / denominator;
                        
                        // -- depenetrate
                        // figure out how much each shape should move based on their mass
                        f32 depth = collision_data.depth;
                        Vec2 normal = collision_data.normal;
                        
                        f32 shape_a_depentration_amount = 0;
                        f32 shape_b_depentration_amount = 0;

                        if (shape_a->inverse_mass != 0 ) { 
                            shape_a_depentration_amount = shape_a->inverse_mass / (shape_a->inverse_mass + shape_b->inverse_mass);
                        }

                        if (shape_b->inverse_mass != 0 ) { 
                            shape_b_depentration_amount = shape_b->inverse_mass / (shape_b->inverse_mass + shape_a->inverse_mass);
                        }

                        // move shape a
                        Vec2 shape_pos, shape_normal;
                        Vec2 depenetration = {
                            normal.x * -depth * shape_a_depentration_amount, 
                            normal.y * -depth * shape_a_depentration_amount
                        };
                        se_phys_get_shape_transform(shape_a, &shape_pos, &shape_normal);
                        shape_pos = vec2_add(shape_pos, depenetration);
                        se_phys_set_shape_transform(shape_a, shape_pos, shape_normal);

                        // move shape b
                        depenetration = (Vec2) {
                            normal.x * +depth * shape_b_depentration_amount, 
                            normal.y * +depth * shape_b_depentration_amount
                        };
                        se_phys_get_shape_transform(shape_b, &shape_pos, &shape_normal);
                        shape_pos = vec2_add(shape_pos, depenetration);
                        se_phys_set_shape_transform(shape_b, shape_pos, shape_normal);

                        // -- apply impulse
                        Vec2 va = vec2_mul_scalar(normal, impulse_magnitude * shape_a->inverse_mass);
                        va = vec2_add(shape_a->velocity, va);

                        Vec2 vb = vec2_mul_scalar(normal, -1 * impulse_magnitude * shape_b->inverse_mass);
                        vb = vec2_add(shape_b->velocity, vb);
                        
                        shape_a->velocity = va;
                        shape_b->velocity = vb;
                    }
                }
            }
        }
    }
}

void finn_game_render(Finn_Game *game) {
    glClear(GL_COLOR_BUFFER_BIT);
    { // -- render the world
        segl_shader_program_use_shader(game->shader_program);

        // -- draw mouse cursor
        game->lines.current_colour = (Vec3) {0.8f, 0.2f, 0.2f};
        segl_lines_draw_cross(&game->lines, game->input.mouse_world_pos, 0.05f);
        game->lines.current_colour = (Vec3) {1, 1, 1};
        
        { // -- world mouse cursor
            switch (game->current_selected_shape_mode) {
                case SE_SHAPES_CIRCLE: {
                    SE_Circle shape;
                    init_circle(&shape);
                    shape.pos = game->input.mouse_world_pos;
                    segl_lines_draw_circle(&game->lines, shape.pos, shape.radius, 30);
                } break;
                case SE_SHAPES_BOX: {
                    SE_Box shape;
                    init_box(&shape);
                    shape.x = game->input.mouse_world_pos.x;
                    shape.y = game->input.mouse_world_pos.y;

                    se_render_rect(&game->lines, &(Rect){shape.x, shape.y, shape.w, shape.h});
                } break;
                case SE_SHAPES_PLANE: {
                    
                    if (game->input.is_mouse_left_down) {
                        SE_Plane shape;
                        init_plane(&shape);

                        Vec2 normal = vec2_sub(game->input.mouse_world_pos, game->input.mouse_world_pressed_pos);
                        vec2_normalise(&normal);

                        shape.normal = normal;
                        shape.depth = vec2_magnitude(vec2_sub(game->input.mouse_world_pos, vec2_zero()));

                        Vec2 plane_pos = vec2_mul_scalar(shape.normal, shape.depth);
                        Vec2 plane_vec = vec2_create(shape.normal.y, -shape.normal.x);
                        // -- the line segment
                        segl_lines_draw_line_segment(&game->lines, 
                            vec2_add(plane_pos, vec2_mul_scalar(plane_vec, -3.0f)), 
                            vec2_add(plane_pos, vec2_mul_scalar(plane_vec, +3.0f)));
                        // -- the normal arrow
                        segl_lines_draw_line_segment(&game->lines,
                            plane_pos,
                            vec2_add(plane_pos, shape.normal)
                            );
                    }
                } break;
            }
        }

        { // -- render objects
            for (i32 i = 0; i < game->objects_count; ++i) {
                SE_Shape *shape = game->objects[i];
                se_phys_render_shape(&global_physics_debug->lines, shape);
            }
        }

        // -- render
        Mat4 ortho_mat = segl_get_camera_transform(game->camera);
        segl_shader_program_set_uniform_mat4(game->shader_program, "vpMatrix", ortho_mat);
        // sense grid lines don't change, we just draw them
        segl_lines_draw(&game->grid);
        // other lines potentially change every frame, so we have to compile/draw/clear them
        segl_lines_update_frame(&game->lines);

        { // -- se_physics_global_render();
                se_physics_global_render();
            // if (!game->is_paused) {
            //     se_physics_global_render();
            // } else {
            //     if (global_physics_debug->lines.positions_current_index > 0) {
            //         segl_lines_compile(&global_physics_debug->lines);
            //         segl_lines_draw(&global_physics_debug->lines);
            //     }
            //     if (game->is_physics_update_queued) {
            //         segl_lines_clear(&global_physics_debug->lines);
            //     }
            // }
        }
    }

    { // -- game ui
        ui_render(&game->ui_context);
    }
}