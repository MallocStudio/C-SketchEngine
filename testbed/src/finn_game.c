#include "finn_game.h"
#include "string.h"
/// get the mouse position (relative to the window). Optionally pass bools to get mouse state
Vec2 get_mouse_pos(bool *lpressed, bool *rpressed) {
    i32 x, y;
    u32 state = SDL_GetMouseState(&x, &y);
    if (lpressed != NULL) {
        *lpressed = false;
        if (state & SDL_BUTTON_LEFT) *lpressed = true;
    }
    if (rpressed != NULL) {
        *rpressed = false;
        if (state & SDL_BUTTON_RIGHT) *rpressed = true;
    }
    return (Vec2){x, y};
}

/// -----------------
/// Finn Game example
/// -----------------
void finn_game_init(Finn_Game *game, SDL_Window *window) {
    game->window = window;
    
    i32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    // -- world camera
    game->camera = new(SEGL_Camera);
    segl_camera_init(game->camera);
    game->camera->aspect_ratio = window_w / window_h;

    game->shader_program = new(SEGL_Shader_Program);
    segl_shader_program_init_from(game->shader_program, "Simple.vsd", "Simple.fsd");

    segl_lines_init(&game->lines);

    // -- txt library
    game->txt_library = setext_init();

    // -- init ui
    ui_init_context(&game->ui_context);
    game->test_ui_init_rect = (Rect) {100, 100, 400, 600};

    // -- render grid once
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
    setext_deinit(game->txt_library);

    // -- camera
    free(game->camera);
}

void finn_game_update(Finn_Game *game, f32 delta_time) {
    // -- update input
    game->keyboard = SDL_GetKeyboardState(NULL);
    
    // -- update mouse pos
    i32 width, height;
    SDL_GetWindowSize(game->window, &width, &height);
    game->camera->aspect_ratio = width / (float)height;
    glViewport(0, 0, width, height);

    Mat4 deprojection_world  = mat4_transposed(mat4_inverse(segl_get_camera_transform(game->camera)));
    Mat4 deprojection_screen = mat4_transposed(mat4_inverse(mat4_ortho(0, width, height, 0, 1, -1)));

    { // -- world mouse pos
        Vec2 cursor_pos;
        cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / width ) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / height) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_world = mat4_mul_vec4(&deprojection_world, &mouse_pos_ndc);

        cursor_pos.x = mouse_pos_world.x;
        cursor_pos.y = mouse_pos_world.y;
        game->mouse_pos_world = cursor_pos;
    }
    { // -- screen mouse pos
        Vec2 cursor_pos;
        cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / width ) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / height) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_screen = mat4_mul_vec4(&deprojection_screen, &mouse_pos_ndc);

        cursor_pos.x = mouse_pos_screen.x;
        cursor_pos.y = mouse_pos_screen.y;
        game->mouse_pos_screen = cursor_pos;
    }

    // -- ui context
    bool left_pressed, right_pressed;
    get_mouse_pos(&left_pressed, &right_pressed);
    ui_update_context(&game->ui_context, (Vec2i) {game->mouse_pos_screen.x, game->mouse_pos_screen.y}, left_pressed, right_pressed);

    // -- move the camera around
    if (game->keyboard[SDL_SCANCODE_LEFT]) {
        game->camera->center.x -= game->camera->speed * delta_time * game->camera->height;
    }
    if (game->keyboard[SDL_SCANCODE_RIGHT]) {
        game->camera->center.x += game->camera->speed * delta_time * game->camera->height;
    }
    if (game->keyboard[SDL_SCANCODE_UP]) {
        game->camera->center.y += game->camera->speed * delta_time * game->camera->height;
    }
    if (game->keyboard[SDL_SCANCODE_DOWN]) {
        game->camera->center.y -= game->camera->speed * delta_time * game->camera->height;
    }
}

void finn_game_render(Finn_Game *game) {
    glClear(GL_COLOR_BUFFER_BIT);
    {// -- render the world
        segl_shader_program_use_shader(game->shader_program);

        // -- draw mouse cursor
        game->lines.current_colour = (Vec3) {0.8f, 0.2f, 0.2f};
        segl_lines_draw_cross(&game->lines, game->mouse_pos_world, 0.05f);
        game->lines.current_colour = (Vec3) {1, 1, 1};
        
        { // -- draw collision test bed circle plane
            SE_Circle c;
            init_circle(&c);
            c.pos = game->mouse_pos_world;
            c.radius = 1.2f;
            se_render_circle(&game->lines, &c);

            SE_Plane p;
            init_plane(&p);
            p.depth = 2;
            p.normal = vec2_normalised(vec2_create(1, -1));
            Vec2 plane_pos = vec2_mul_scalar(p.normal, p.depth);
            Vec2 plane_vec = vec2_create(p.normal.y, -p.normal.x);
            // -- the line segment
            segl_lines_draw_line_segment(&game->lines, 
                vec2_add(plane_pos, vec2_mul_scalar(plane_vec, -3.0f)), 
                vec2_add(plane_pos, vec2_mul_scalar(plane_vec, +3.0f)));
            // -- the normal arrow
            segl_lines_draw_line_segment(&game->lines,
                plane_pos,
                vec2_add(plane_pos, p.normal)
                );
            se_phys_check_circle_plane(&c, &p);
        }
        
        // -- render
        Mat4 ortho_mat = segl_get_camera_transform(game->camera);
        segl_shader_program_set_uniform_mat4(game->shader_program, "vpMatrix", ortho_mat);
        // sense grid lines don't change, we just draw them
        segl_lines_draw(&game->grid);
        // other lines potentially change every frame, so we have to compile/draw/clear them
        segl_lines_update_frame(&game->lines);
    }

    { // -- game->ui_context ui test
        // -- setup renderer matrix
        i32 window_w, window_h;
        SDL_GetWindowSize(game->window, &window_w, &window_h);
        Mat4 ortho_mat = mat4_ortho(0, window_w, window_h, 0, 1, -1);
        segl_shader_program_use_shader(&game->ui_context.renderer.shader_program);
        segl_shader_program_set_uniform_mat4(&game->ui_context.renderer.shader_program, "vpMatrix", ortho_mat);
        // -- update renderer vertices
        ui_begin(&game->ui_context, &game->test_ui_init_rect);
        ui_row(&game->ui_context, 3, 48, 100);
        if (ui_button(&game->ui_context, "button 1")) printf("button 1 pressed\n");
        if (ui_button(&game->ui_context, "button 2")) printf("button 2 pressed\n");
        if (ui_button(&game->ui_context, "button 3")) printf("button 3 pressed\n");
        ui_row(&game->ui_context, 1, 32, 0);
        if (ui_button(&game->ui_context, "button 4")) printf("button 4 pressed\n");
        ui_row(&game->ui_context, 1, 64, 0);
        if (ui_button(&game->ui_context, "button 5")) printf("button 5 pressed\n");
        ui_row(&game->ui_context, 1, 200, 0);
        ui_label(&game->ui_context, "ma danny long text lalbal blbll balaha labal increasing text to take even more space to test the wrapping functionality");

        // ui_row(&game->ui_context, 1, 48, 100);
        // if (ui_button(&game->ui_context, "button 1")) printf("button 1 pressed\n");
        
        // -- render
        ui_render(&game->ui_context);
    }

    // -- text
    Mat4 projection = mat4_ortho(0, 800, 0, 400, 1, -1);
    char *text = malloc(sizeof(char) * 100);
    sprintf(text, "mouse pos screen: {%f, %f}", game->mouse_pos_screen.x, game->mouse_pos_screen.y);
    // printf("mouse pos screen: {%f, %f}\n", game->mouse_pos_screen.x, game->mouse_pos_screen.y);
    setext_render_text(game->txt_library, text, 0, 0, 1.0f, (Vec3) {0.5f, 0.8f, 0.2f}, projection);
    free(text);
}


    /*{ // -- draw collision test bed : aabb circle
        game->lines.current_colour = (Vec3) {1.0f, 1.0f, 1.0f};
        SE_Circle circle;
        init_circle(&circle);
        circle.pos = game->mouse_pos;
        circle.radius = 1.2f;
        se_render_circle(&game->lines, &circle);

        SE_AABB aabb;
        init_aabb(&aabb);
        aabb.xmin = 1;
        aabb.xmax = 2;
        aabb.ymin = 1;
        aabb.ymax = 2;
        se_render_aabb(&game->lines, &aabb);
        se_phys_check_aabb_circle(&aabb, &circle);
    }
    { // -- draw collision test bed circle circle
        game->lines.current_colour = (Vec3) {1.0f, 1.0f, 1.0f};
        SE_Circle c1;
        init_circle(&c1);
        c1.pos = game->mouse_pos;
        c1.radius = 1.2f;
        se_render_circle(&game->lines, &c1);

        SE_Circle c2;
        init_circle(&c2);
        c2.pos = (Vec2) {0, 0};
        c2.radius = 3.2f;
        se_render_circle(&game->lines, &c2);

        se_phys_check_circle_circle(&c1, &c2);
        // se_phys_check_circle_circle(&c2, &c1);
    }*/