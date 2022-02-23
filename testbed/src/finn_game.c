#include "finn_game.h"
#include "string.h"
#include "seinput.h"

/// add a shape to game
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
        case SE_SHAPES_AABB: {
            shape = (SE_Shape*)new(SE_AABB);
            init_aabb(shape);
        } break;
        case SE_SHAPES_PLANE: {
            shape = (SE_Shape*)new(SE_Plane);
            init_plane(shape);
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
    }
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

    { // @remove 
        // Mat4 deprojection_world  = mat4_transposed(mat4_inverse(segl_get_camera_transform(game->camera)));

        // Mat4 deprojection_screen = mat4_transposed(mat4_inverse(mat4_ortho(0, width, height, 0, 1, -1)));
        // ! moving all the deprojections to seinput_update. NOTE that viewport_to_ortho_projection_matrix is different than how we get the mat4_ortho in above line of code
        // Mat4 deprojection_screen = mat4_transposed(mat4_inverse(viewport_to_ortho_projection_matrix(screen_viewport)));
    }

    // -- update input
    seinput_update(&game->input, segl_get_camera_transform(game->camera), (Vec2i){width, height});

    { // @remove
        // { // -- world mouse pos
        //     Vec2 cursor_pos;
        //     cursor_pos = get_mouse_pos(NULL, NULL);
        //     cursor_pos.x = (cursor_pos.x / width ) * 2.0f - 1.0f;
        //     cursor_pos.y = (cursor_pos.y / height) * 2.0f - 1.0f;

        //     Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        //     Vec4 mouse_pos_world = mat4_mul_vec4(&deprojection_world, &mouse_pos_ndc);

        //     cursor_pos.x = mouse_pos_world.x;
        //     cursor_pos.y = mouse_pos_world.y;
        //     game->mouse_pos_world = cursor_pos;
        // }
        // { // -- screen mouse pos
        //     Vec2 cursor_pos;
        //     cursor_pos = get_mouse_pos(NULL, NULL);
        //     cursor_pos.x = (cursor_pos.x / width ) * 2.0f - 1.0f;
        //     cursor_pos.y = (cursor_pos.y / height) * 2.0f - 1.0f;

        //     Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        //     Vec4 mouse_pos_screen = mat4_mul_vec4(&deprojection_screen, &mouse_pos_ndc);

        //     cursor_pos.x = mouse_pos_screen.x;
        //     cursor_pos.y = mouse_pos_screen.y;
        //     game->mouse_pos_screen = cursor_pos;
        // }
    }

    { // -- ui context
        // -- call update
        ui_update_context(&game->ui_context, screen_viewport);
        
        // -- handle ui
        ui_begin(&game->ui_context, &game->test_ui_init_rect);
        
        ui_row(&game->ui_context, 1, 48, 120);
        if (ui_button(&game->ui_context, "aabb")) game->current_selected_shape_mode = SE_SHAPES_AABB;
        
        ui_row(&game->ui_context, 1, 48, 120);
        if (ui_button(&game->ui_context, "circle")) game->current_selected_shape_mode = SE_SHAPES_CIRCLE;

        ui_row(&game->ui_context, 1, 48, 120);
        if (ui_button(&game->ui_context, "plane")) game->current_selected_shape_mode = SE_SHAPES_PLANE;

        // give the resizable item enough space // @incomplete move this to ui_begin or ui_render        
        ui_row(&game->ui_context, 1, 18, 0);
    }

    // -- move the camera around
    if (game->input.keyboard[SDL_SCANCODE_LEFT]) {
        game->camera->center.x -= game->camera->speed * delta_time * game->camera->height;
    }
    if (game->input.keyboard[SDL_SCANCODE_RIGHT]) {
        game->camera->center.x += game->camera->speed * delta_time * game->camera->height;
    }
    if (game->input.keyboard[SDL_SCANCODE_UP]) {
        game->camera->center.y += game->camera->speed * delta_time * game->camera->height;
    }
    if (game->input.keyboard[SDL_SCANCODE_DOWN]) {
        game->camera->center.y -= game->camera->speed * delta_time * game->camera->height;
    }

    { // -- object instantiation
        if (game->input.is_mouse_left_handled == false && game->input.is_mouse_left_down && game->current_selected_shape_mode != SE_SHAPES_NONE) {
            add_shape_to_game(game, game->current_selected_shape_mode, game->input.mouse_world_pos, vec2_zero());
            printf("pressing left\n");
        }
        if (game->input.is_mouse_left_handled == false && game->input.is_mouse_right_down) {
            clear_shapes(game);
            printf("pressing right\n");
        }
    }

    { // -- physics collision check step
        for (i32 i = 0; i < game->objects_count; ++i) { 
            for (i32 j = i + 1; j < game->objects_count; ++j) {
                SE_Shape *shape_a = game->objects[i];
                SE_Shape *shape_b = game->objects[j];
                // ... // @incomplete
            }
        }
    }

    { // -- physics resolution step
    }
}

void finn_game_render(Finn_Game *game) {
    glClear(GL_COLOR_BUFFER_BIT);
    {// -- render the world
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
                case SE_SHAPES_AABB: {
                    SE_AABB shape;
                    init_aabb(&shape);
                    shape.xmin += game->input.mouse_world_pos.x;
                    shape.ymin += game->input.mouse_world_pos.y;
                    shape.xmax += game->input.mouse_world_pos.x;
                    shape.ymax += game->input.mouse_world_pos.y;
                    se_render_rect_min_max(&game->lines, shape.xmin, shape.ymin, shape.xmax, shape.ymax);
                } break;
                case SE_SHAPES_PLANE: {
                    SE_Plane shape;
                    init_plane(&shape);
                    shape.depth = vec2_distance(vec2_zero(), game->input.mouse_world_pos);
                    shape.normal = vec2_normalised(vec2_sub(game->input.mouse_world_pos, vec2_zero()));
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
                } break;
            }
        }

        { // -- render objects
            for (i32 i = 0; i < game->objects_count; ++i) {
                SE_Shape *shape = game->objects[i];
                switch (shape->type) {
                    case SE_SHAPES_CIRCLE: {
                        SE_Circle *circle = (SE_Circle*) shape;
                        segl_lines_draw_circle(&game->lines, circle->pos, circle->radius, 32);
                    } break;
                    case SE_SHAPES_AABB: {
                    } break;
                    case SE_SHAPES_PLANE: {
                    } break;
                }
            }
        }

        // { // -- draw collision test bed : aabb circle
        //     game->lines.current_colour = (Vec3) {1.0f, 1.0f, 1.0f};
        //     SE_Circle circle;
        //     init_circle(&circle);
        //     circle.pos = game->mouse_pos_world;
        //     circle.radius = 1.2f;
        //     segl_lines_draw_circle(&game->lines, circle.pos, circle.radius, 32);

        //     SE_AABB aabb;
        //     init_aabb(&aabb);
        //     aabb.xmin = 1;
        //     aabb.xmax = 2;
        //     aabb.ymin = 1;
        //     aabb.ymax = 2;
        //     se_render_rect_min_max(&game->lines, aabb.xmin, aabb.ymin, aabb.xmax, aabb.ymax);
        //     se_phys_check_aabb_circle(&aabb, &circle);
        // }
        // { // -- draw collision test bed circle circle
        //     game->lines.current_colour = (Vec3) {1.0f, 1.0f, 1.0f};
        //     SE_Circle c1;
        //     init_circle(&c1);
        //     c1.pos = game->mouse_pos_world;
        //     c1.radius = 1.2f;
        //     se_render_circle(&game->lines, &c1);

        //     SE_Circle c2;
        //     init_circle(&c2);
        //     c2.pos = (Vec2) {0, 0};
        //     c2.radius = 3.2f;
        //     se_render_circle(&game->lines, &c2);

        //     se_phys_check_circle_circle(&c1, &c2);
        //     // se_phys_check_circle_circle(&c2, &c1);
        // }      
        // { // -- draw collision test bed circle plane
        //     SE_Circle c;
        //     init_circle(&c);
        //     c.pos = game->mouse_pos_world;
        //     c.radius = 1.2f;
        //     segl_lines_draw_circle(&game->lines, c.pos, c.radius, 32);

        //     SE_Plane p;
        //     init_plane(&p);
        //     p.depth = 2;
        //     p.normal = vec2_normalised(vec2_create(1, -1));
        //     Vec2 plane_pos = vec2_mul_scalar(p.normal, p.depth);
        //     Vec2 plane_vec = vec2_create(p.normal.y, -p.normal.x);
        //     // -- the line segment
        //     segl_lines_draw_line_segment(&game->lines, 
        //         vec2_add(plane_pos, vec2_mul_scalar(plane_vec, -3.0f)), 
        //         vec2_add(plane_pos, vec2_mul_scalar(plane_vec, +3.0f)));
        //     // -- the normal arrow
        //     segl_lines_draw_line_segment(&game->lines,
        //         plane_pos,
        //         vec2_add(plane_pos, p.normal)
        //         );
        //     se_phys_check_circle_plane(&c, &p);
        // }
        
        // -- render
        Mat4 ortho_mat = segl_get_camera_transform(game->camera);
        segl_shader_program_set_uniform_mat4(game->shader_program, "vpMatrix", ortho_mat);
        // sense grid lines don't change, we just draw them
        segl_lines_draw(&game->grid);
        // other lines potentially change every frame, so we have to compile/draw/clear them
        segl_lines_update_frame(&game->lines);
        se_physics_global_render();
    }

    { // -- game ui
        ui_render(&game->ui_context);
    }


    // // -- text
    // char *text = malloc(sizeof(char) * 100);
    // sprintf(text, "mouse pos screen: {%f, %f}", game->mouse_pos_screen.x, game->mouse_pos_screen.y);
    // setext_render_text(&game->txt_library, text, 0, 0, 1.0f, (Vec3) {0.5f, 0.8f, 0.2f});
    // setext_render(&game->txt_library);
    // free(text);
}

    // { // -- game->ui_context ui test
    //     ui_begin(&game->ui_context, &game->test_ui_init_rect);

    //     ui_row(&game->ui_context, 3, 48, 100);
    //     if (ui_button(&game->ui_context, "button 1")) printf("button 1 pressed\n");
    //     if (ui_button(&game->ui_context, "button 2")) printf("button 2 pressed\n");
    //     if (ui_button(&game->ui_context, "button 3")) printf("button 3 pressed\n");

    //     ui_row(&game->ui_context, 1, 32, 0);
    //     if (ui_button(&game->ui_context, "button 4")) printf("button 4 pressed\n");

    //     ui_row(&game->ui_context, 1, 64, 0);
    //     if (ui_button(&game->ui_context, "button 5")) printf("button 5 pressed\n");

    //     ui_row(&game->ui_context, 1, 18, 0);

    //     // ui_label(&game->ui_context, "ma danny long text lalbal blbll balaha labal increasing text to take even more space to test the wrapping functionality");

    //     // ui_row(&game->ui_context, 1, 48, 100);
    //     // if (ui_button(&game->ui_context, "button 1")) printf("button 1 pressed\n");
        
    //     // -- render
    //     ui_render(&game->ui_context);
    // }