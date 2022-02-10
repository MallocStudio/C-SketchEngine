#include "finn_game.h"

/// get the mouse position (relative to the window). Optionally pass bools to get mouse state
vec2 get_mouse_pos(bool *lpressed, bool *rpressed) {
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
    return (vec2){x, y};
}

/// -----------------
/// Finn Game example
/// -----------------
void finn_game_init(Finn_Game *game, SDL_Window *window) {
    game->window = window;
    
    i32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    game->camera = new(SEGL_Camera);
    segl_camera_init(game->camera);
    game->camera->aspect_ratio = window_w / window_h;

    game->shader_program = new(Shader_Program);
    segl_shader_program_init_from(game->shader_program, "Simple.vsd", "Simple.fsd");
    segl_shader_program_use_shader(game->shader_program);

    segl_lines_init(&game->lines);

    glClearColor(0, 0, 0, 1);

    segl_lines_init(&game->grid);
    f32 grid_limits = 10.0f;
    for (f32 i = -grid_limits; i <= grid_limits; i++) {
        game->grid.current_colour = (i == 0) ? (vec3) {0.8f, 0.8f, 0.8f} : (vec3) {0.3f, 0.3f, 0.3f};
        segl_lines_draw_line_segment(&game->grid, (vec2) {i, -grid_limits}, (vec2) {i, grid_limits});
        segl_lines_draw_line_segment(&game->grid, (vec2) {-grid_limits, i}, (vec2) {grid_limits, i});
    }
    game->grid.current_colour = (vec3) {1, 0, 0};
    segl_lines_draw_line_segment(&game->grid, (vec2) {0,0}, (vec2) {1, 0});
    game->grid.current_colour = (vec3) {0, 1, 0};
    segl_lines_draw_line_segment(&game->grid, (vec2) {0,0}, (vec2) {0, 1});
    segl_lines_compile(&game->grid);
}

void finn_game_deinit(Finn_Game *game) {
    free(game->camera);
    segl_lines_deinit(&game->grid);
    segl_lines_deinit(&game->lines);
    segl_shader_program_deinit(game->shader_program);
    free(game->shader_program);
}

void finn_game_update(Finn_Game *game, f32 delta_time) {
    // -- update input
    game->keyboard = SDL_GetKeyboardState(NULL);
    
    // -- update mouse pos
    i32 width, height;
    SDL_GetWindowSize(game->window, &width, &height);
    game->camera->aspect_ratio = width / (float)height;
    glViewport(0, 0, width, height);

    mat4 deprojection = mat4_transposed(mat4_inverse(segl_get_camera_transform(game->camera)));
    vec2 cursor_pos;
    cursor_pos = get_mouse_pos(NULL, NULL);
    cursor_pos.x = (cursor_pos.x / width ) * 2.0f - 1.0f;
    cursor_pos.y = (cursor_pos.y / height) * 2.0f - 1.0f;

    vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
    // vec4 mouse_pos_world = {cursor_pos.x, cursor_pos.y, 0, 0};
    vec4 mouse_pos_world = mat4_mul_vec4(&deprojection, &mouse_pos_ndc);

    cursor_pos.x = mouse_pos_world.x;
    cursor_pos.y = mouse_pos_world.y;
    game->mouse_pos = cursor_pos;

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
    // -- draw mouse cursor
    game->lines.current_colour = (vec3) {0.8f, 0.2f, 0.2f};
    segl_lines_draw_cross(&game->lines, game->mouse_pos, 0.2f);
    
    // -- draw collision test bed
    game->lines.current_colour = (vec3) {1.0f, 1.0f, 1.0f};
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

    // -- render
    glClear(GL_COLOR_BUFFER_BIT);
    mat4 ortho_mat = segl_get_camera_transform(game->camera);
    segl_shader_program_set_uniform_mat4(game->shader_program, "vpMatrix", ortho_mat);
    // sense grid lines don't change, we just draw them
    segl_lines_draw(&game->grid);
    // other lines potentially change every frame, so we have to compile/draw/clear them
    segl_lines_update_frame(&game->lines);
}