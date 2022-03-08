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
        deinit_shape(game->objects[i]);
        free(game->objects[i]);
        game->objects[i] = NULL;
    }
    game->objects_count = 0;
    game->selected_shape = NULL;
}

void create_boundaries(Finn_Game *game) {
    f32 depth = -10.0f;
    Vec2 normals[4] = {
        {-1, 0}, {0, -1}, {1, 0}, {0, 1}
    };
    for (i32 i = 0; i < 4; ++i) {
        add_shape_to_game(game, SE_SHAPES_PLANE, (Vec2){depth, 0}, normals[i]);
    }
    Vec2 normal = (Vec2) {-1, 1};
    vec2_normalise(&normal);
    add_shape_to_game(game, SE_SHAPES_PLANE, (Vec2){depth * 0.5f, 0}, normal);
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
        // general UI panel
        ui_init_context(&game->ui_context, &game->input);
        game->test_ui_init_rect = (Rect) {0, 0, 0, 0};
        // collision data debug panel
        ui_init_context(&game->ui_context_physics_debug, &game->input);
        game->ui_context_physics_debug_init_rect = (Rect) {0, 360, 0, 0};
    }

    { // -- render grid once
        glClearColor(0, 0, 0, 1);

        segl_lines_init(&game->grid);
        f32 grid_limits = 10.0f;
        for (f32 i = -grid_limits; i <= grid_limits; i++) {
            // if we're rendering one of the axis, make it bolder
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
        // creates 4 planes that bound the grid
        create_boundaries(game);
    }

    // -- physics
    game->is_paused = false;
    game->is_physics_update_queued = false;
    game->elasticity = 0;
    game->selected_shape = NULL;
}

void finn_game_deinit(Finn_Game *game) {
    // -- ui
    ui_deinit_context(&game->ui_context);
    ui_deinit_context(&game->ui_context_physics_debug);

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

    { // -- general ui context
        // -- call update
        ui_update_context_viewport(&game->ui_context, screen_viewport);
        
        // -- handle ui
        ui_begin(&game->ui_context, &game->test_ui_init_rect);

        ui_row(&game->ui_context, 3, 48, 300);
        if (ui_button(&game->ui_context, "box")) game->current_selected_shape_mode = SE_SHAPES_BOX;
        if (ui_button(&game->ui_context, "circle")) game->current_selected_shape_mode = SE_SHAPES_CIRCLE;
        if (ui_button(&game->ui_context, "NONE")) game->current_selected_shape_mode = SE_SHAPES_NONE;

        ui_row(&game->ui_context, 1, 36, 300);
        ui_label(&game->ui_context, "--shapes--");
        
        ui_row(&game->ui_context, 2, 48, 120);
        if (ui_button(&game->ui_context, "elasticity 0")) game->elasticity = 0;
        if (ui_button(&game->ui_context, "elasticity 1")) game->elasticity = 1;

        ui_row(&game->ui_context, 1, 48, 240);
        if (global_physics_debug->active) {
            if (ui_button(&game->ui_context, "deactivate debug rendering")) global_physics_debug->active = false;
        } else {
            if (ui_button(&game->ui_context, "activate debug rendering")) global_physics_debug->active = true;
        }
        ui_row(&game->ui_context, 2, 48, 240);
        if (game->is_paused) {
            if (ui_button(&game->ui_context, "unpause")) game->is_paused = false;
            if (ui_button(&game->ui_context, ">")) game->is_physics_update_queued = true;
        } else {
            if (ui_button(&game->ui_context, "pause")) game->is_paused = true;
        }

        ui_row(&game->ui_context, 1, 36, 300);
        ui_label(&game->ui_context, "--phys config--");

        // give the resizable item enough space // @incomplete move this to ui_begin or ui_render
        ui_row(&game->ui_context, 1, 18, 0);
    }

    /*{ // -- shape debug ui
        if (game->selected_shape != NULL) {
            UI_Context *ctx = &game->ui_context_physics_debug;
            SE_Collision_Data *collision_data = game->selected_shape->collision_data;

            // -- call update
            ui_update_context_viewport(ctx, screen_viewport);
            
            // -- handle ui
            ui_begin(ctx, &game->ui_context_physics_debug_init_rect);
            char debug_text[256];
            i32 min_width = 300;
            
            ui_row(ctx, 1, 48, min_width);
            if (ui_button(ctx, "deselect")) game->selected_shape = NULL;

            if (collision_data->is_collided) {
                ui_row(ctx, 1, 48, min_width);
                sprintf(debug_text, "world_pos: {%f, %f}", collision_data->world_pos.x, collision_data->world_pos.y);
                ui_button(ctx, debug_text);

                ui_row(ctx, 1, 48, min_width);
                sprintf(debug_text, "normal: {%f, %f}", collision_data->normal.x, collision_data->normal.y);
                ui_button(ctx, debug_text);

                ui_row(ctx, 1, 48, min_width);
                sprintf(debug_text, "depth: %f", collision_data->depth);
                ui_button(ctx, debug_text);
            } else {
                ui_row(ctx, 2, 48, min_width);
                sprintf(debug_text, "shape type: %i", (i32) game->selected_shape->type);
                ui_button(ctx, debug_text);

                ui_row(ctx, 1, 48, min_width);
                sprintf(debug_text, "not colliding");
                ui_button(ctx, debug_text);

                // @remove
                if (game->input.keyboard[SDL_SCANCODE_Z]) {
                    i32 x = 0;
                }
            }

            // give the resizable item enough space // @incomplete move this to ui_begin or ui_render
            ui_row(ctx, 1, 18, 0);
        }
    }*/

    { // -- move the camera around
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
    }

    { // -- object instantiation
        // if we're presseing the left mouse button AND shape mode is not NONE
        // instantiate the selected shape in the simulation
        if (seinput_is_mouse_left_pressed(&game->input) 
        && game->current_selected_shape_mode != SE_SHAPES_NONE) {
            Vec2 normal = vec2_sub(game->input.mouse_world_pos, game->input.mouse_world_pressed_pos);
            vec2_normalise(&normal);
            add_shape_to_game(game, game->current_selected_shape_mode, game->input.mouse_world_pos, normal);
        }

        // if we're pressing the right mouse button, clear all the shapes and recreate the boundaries
        if (seinput_is_mouse_right_pressed(&game->input)) {
            clear_shapes(game);
            create_boundaries(game);
        }
    }
    
    { // -- select shapes to render specific shape debug info in UI
        if (seinput_is_mouse_left_pressed(&game->input)
            && game->current_selected_shape_mode == SE_SHAPES_NONE) {
            for (i32 i = 0; i < game->objects_count; ++i) {
                SE_Shape *shape = game->objects[i];
                Vec2 shape_pos;
                se_phys_get_shape_transform(shape, &shape_pos, NULL);
                if (vec2_magnitude_squared(vec2_sub(game->input.mouse_world_pos, shape_pos)) < 0.1f) {
                    game->selected_shape = shape;
                }
            }
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

            if (shape->type == SE_SHAPES_PLANE) continue; // ! hhhhhhmmmm
            if (shape->inverse_mass == 0) continue;

            // -- apply a constant force
            Vec2 gravity = {0, -0.2f * delta_time};
            if (game->input.keyboard[SDL_SCANCODE_X]) {
                gravity = (Vec2) {-0.2f * delta_time, 0};
            }
            shape->velocity = vec2_add(shape->velocity, gravity);

            // -- move
            Vec2 shape_pos, shape_normal;
            se_phys_get_shape_transform(shape, &shape_pos, &shape_normal);
            shape_pos = vec2_add(shape_pos, shape->velocity);
            se_phys_set_shape_transform(shape, shape_pos, shape_normal);
        }
    }

    { // -- physics collision check through function pointer table
        game->collision_datas_count = 0; // reset collision datas for this frame

        for (i32 i = 0; i < game->objects_count; ++i) { 
            for (i32 j = i + 1; j < game->objects_count; ++j) {
                SE_Shape *shape_a = game->objects[i];
                SE_Shape *shape_b = game->objects[j];

                if (shape_a->inverse_mass == 0 && shape_b->inverse_mass == 0) continue;

                if (shape_a->type >= 0 && shape_b->type >= 0) {
                    i32 function_index = (shape_a->type * SE_SHAPES_COUNT) + shape_b->type;
                    se_phys_check collision_function_pointer = collision_function_array[function_index];
                    SDL_assert(collision_function_pointer != NULL && "collision_function_pointer resolved to null");

                    // SE_Collision_Data collision_data = collision_function_pointer(shape_a, shape_b);
                    game->collision_datas[game->collision_datas_count] = 
                        collision_function_pointer(shape_a, shape_b);
                    
                    // only consider the collision if we're actually colliding
                    if (game->collision_datas[game->collision_datas_count].is_collided) {
                        game->collision_datas_count++;
                    }
                }
            }
        }
    }

    { // -- collision resolution
        for (i32 i = 0; i < game->collision_datas_count; ++i) {
            const SE_Collision_Data collision_data = game->collision_datas[i];
            SE_Shape *shape_a = collision_data.shape_a;
            SE_Shape *shape_b = collision_data.shape_b;

            if (collision_data.is_collided == true) {

                // e = elasticity
                // n = collision normal
                //
                //              -(1+e)rel_v . n
                // j =  -----------------------------------
                //     n . n(inverse_massA + in0verse_massB)
                //
                // j = impulse magnitude

                // rel_v
                Vec2 rel_v = vec2_sub(shape_a->velocity, shape_b->velocity);
                // rel_v . n
                f32 numerator = vec2_dot(rel_v, collision_data.normal);
                // -(1+e)rel_v . n
                numerator = numerator * -(1 + game->elasticity);

                f32 denominator = shape_a->inverse_mass + shape_b->inverse_mass;
                // ! note that the denominator cannot be zero becasue we should not run this block of code
                // ! if shape_a->inverse_mass AND shape_b->inverse_mass == 0
                f32 impulse_magnitude = numerator / denominator;
                
                // -- depenetrate
                // figure out how much each shape should move based on their mass
                f32 depth = collision_data.depth;
                Vec2 normal = collision_data.normal;
                
                f32 shape_a_depenetration_amount = 0;
                f32 shape_b_depenetration_amount = 0;

                if (shape_a->inverse_mass != 0 ) { 
                    shape_a_depenetration_amount = shape_a->inverse_mass / (shape_a->inverse_mass + shape_b->inverse_mass);
                }

                if (shape_b->inverse_mass != 0 ) { 
                    shape_b_depenetration_amount = shape_b->inverse_mass / (shape_b->inverse_mass + shape_a->inverse_mass);
                }

                // move shape a
                Vec2 shape_pos, shape_normal;
                Vec2 depenetration = {
                    normal.x * -depth * shape_a_depenetration_amount, 
                    normal.y * -depth * shape_a_depenetration_amount
                };
                se_phys_get_shape_transform(shape_a, &shape_pos, &shape_normal);
                shape_pos = vec2_add(shape_pos, depenetration);
                se_phys_set_shape_transform(shape_a, shape_pos, shape_normal);

                // move shape b
                depenetration = (Vec2) {
                    normal.x * +depth * shape_b_depenetration_amount, 
                    normal.y * +depth * shape_b_depenetration_amount
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

                if (game->selected_shape == shape) {
                    game->lines.current_colour = (Vec3) {1, 0, 0};
                    printf("rendering selected shape\n");
                }

                se_phys_render_shape(&game->lines, shape);
                game->lines.current_colour = (Vec3) {1, 1, 1};
            }
        }

        { // -- render physics

            // collision data
            // if (game->selected_shape == NULL) { // render collision data for all shapes
            //     for (i32 i = 0; i < game->objects_count; ++i) {
            //         render_collision_data(game->objects[i]->collision_data);
            //     }
            // } else { // render collision data for the selected shape
            //     render_collision_data(game->selected_shape->collision_data);
            // }
            for (i32 i = 0; i < game->collision_datas_count; ++i) {
                render_collision_data(&game->collision_datas[i]);
            }

            se_physics_global_render();
            // segl_lines_clear(&global_physics_debug->lines);
        }

        // -- render
        Mat4 ortho_mat = segl_get_camera_transform(game->camera);
        segl_shader_program_set_uniform_mat4(game->shader_program, "vpMatrix", ortho_mat);
        // sense grid lines don't change, we just draw them
        segl_lines_draw(&game->grid);
        // other lines potentially change every frame, so we have to compile/draw/clear them
        segl_lines_update_frame(&game->lines);
    }

    { // -- game ui
        ui_render(&game->ui_context);
        ui_render(&game->ui_context_physics_debug);
    }
}