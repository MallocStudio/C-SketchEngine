#include "SketchEngine_OpenGL.h"
#include <stdio.h>

void print_sdl_error() {
    const char *error = SDL_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}

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

void segl_camera_init(SEGL_Camera *cam) {
    // cam->aspect_ratio = 16.0f / 9.0f; // ! this is now determined in main, where we declare window size
    cam->height = 10.0f;
    cam->speed = 0.8f;
}

mat4 segl_get_camera_transform(SEGL_Camera *cam) {
    mat4 result =  MatrixOrtho(
        -cam->aspect_ratio * cam->height * 0.5f + cam->center.x,
        +cam->aspect_ratio * cam->height * 0.5f + cam->center.x,
        -cam->height * 0.5f + cam->center.y,
        +cam->height * 0.5f + cam->center.y,
        -1.0f, 1.0f);
    return MatrixTranspose(result); // ! for some reason, MatrixOrtho here needs to get transposed?
    // ! this fixes the rendering and mapping mouse pos
}

void segl_camera_zoom(SEGL_Camera *cam, f32 zoom_factor) {
    cam->height /= zoom_factor;
}

void segl_shader_program_init(Shader_Program *shader_program) {
    shader_program->vertex_shader = -1;
    shader_program->fragment_shader = -1;
    shader_program->shader_program = -1;
    shader_program->loaded_successfully = false;
}

void segl_shader_program_init_from (Shader_Program *sp, const char *vertex_filename, const char *fragment_filename) {
    sp->loaded_successfully = true;
    
    sp->vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    sp->shader_program = glCreateProgram();

    const char *vertex_src = load_file_as_string(vertex_filename);
    const char *frag_src   = load_file_as_string(fragment_filename);

    glShaderSource(sp->vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(sp->vertex_shader);

    GLchar error_log[512];
    GLint success = 0;

    glGetShaderiv(sp->vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // something failed with the vertex shader compilation
        printf ("vertex shader %s failed with error:\n", vertex_filename);
        glGetShaderInfoLog(sp->vertex_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", vertex_filename);
    }

    glShaderSource(sp->fragment_shader, 1, &frag_src, NULL);
    glCompileShader(sp->fragment_shader);

    glGetShaderiv(sp->fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        printf ("fragment shader %s failed with error:\n", fragment_filename);
        glGetShaderInfoLog(sp->fragment_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", fragment_filename);
    }

    glAttachShader(sp->shader_program, sp->vertex_shader);
    glAttachShader(sp->shader_program, sp->fragment_shader);
    glLinkProgram(sp->shader_program);
    glGetProgramiv(sp->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        printf ("Error linking shaders \\%s\\ and \\%s\\\n", vertex_filename, fragment_filename);
        glGetProgramInfoLog(sp->shader_program, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    }

    if (sp->loaded_successfully) {
        printf ("Shaders compiled and linked successfully.\n");
    } else {
        // if there was a problem, tell OpenGL that we don't need those resources after all
        glDeleteShader(sp->vertex_shader);
        glDeleteShader(sp->fragment_shader);
        glDeleteProgram(sp->shader_program);
    }

    free(vertex_src);
    free(frag_src);
}

void segl_shader_program_deinit(Shader_Program *sp) {
    if (sp->loaded_successfully) {
        glDeleteShader(sp->vertex_shader);
        glDeleteShader(sp->fragment_shader);
        glDeleteProgram(sp->shader_program);
    }
}

GLuint segl_shader_program_get_uniform_loc (Shader_Program *sp, const char *var_name) {
    return glGetUniformLocation(sp->shader_program, var_name);
}

void segl_shader_program_set_uniform_f32 (Shader_Program *sp, const char *var_name, f32 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniform1f(var_loc, value);
}

void segl_shader_program_set_uniform_vec3 (Shader_Program *sp, const char *var_name, vec3 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniform3f(var_loc, value.x, value.y, value.z);
}

void segl_shader_program_set_uniform_mat4 (Shader_Program *sp, const char *var_name, mat4 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniformMatrix4fv(var_loc, 1, GL_FALSE, &value); // @check (Finn did &value[0][0] with glm::mat4)
}

void segl_shader_program_use_shader(Shader_Program *sp) {
    glUseProgram(sp->shader_program);
}

const char* load_file_as_string(const char *file_name) {
    // https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
    char *source = NULL;
    FILE *fp = fopen(file_name, "r");
    if (fp != NULL) {
        // go to the end of the file
        if (fseek(fp, 0L, SEEK_END) == 0) {
            // get the size of the file
            long bufsize = ftell(fp);
            if (bufsize == -1) { // error
                printf("file reading error at %s: %i\n", __FILE__,__LINE__);
                return NULL;
            }
            // allocate our buffer to that size
            source = malloc(sizeof(char) * (bufsize + 1));

            // Go back to the start of the file
            if (fseek(fp, 0L, SEEK_SET) != 0) { // error
                printf("file reading error at %s: %i\n", __FILE__,__LINE__);
                return NULL;
            }

            // read the entire file into memory
            size_t new_len = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                fputs("Errpr reading file", stderr);
            } else {
                source[new_len++] = '\0'; // just to be safe
            }
        }
        fclose(fp);
    } else {
        printf("file reading error at %s: %i\n", __FILE__,__LINE__);
    }
    return source;
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

    mat4 deprojection = MatrixInvert(segl_get_camera_transform(game->camera));
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
    game->lines.current_colour = (vec3) {0.8f, 0.2f, 0.2f};
    segl_lines_draw_cross(&game->lines, game->mouse_pos, 0.2f);
    printf("mouse pos: { %f, %f }\n", game->mouse_pos.x, game->mouse_pos.y);

    glClear(GL_COLOR_BUFFER_BIT);
    mat4 ortho_mat = segl_get_camera_transform(game->camera);
    segl_shader_program_set_uniform_mat4(game->shader_program, "vpMatrix", ortho_mat);
    // sense grid lines don't change, we just draw them
    segl_lines_draw(&game->grid);
    // other lines potentially change every frame, so we have to compile/draw/clear them
    segl_lines_update_frame(&game->lines);
}

/// -------------
/// Line Renderer
/// -------------
void segl_lines_init(Line_Renderer *lines) {
    lines->positions_current_index = 0;
    lines->colours_current_index   = 0;
    lines->current_colour = (vec3) {1, 1, 1};
    lines->line_active = false;
    glGenBuffers(1, &lines->position_buffer_id);
    glGenBuffers(1, &lines->colour_buffer_id);
    lines->initialised = true;
}

void segl_lines_deinit(Line_Renderer *lines) {
    if (lines->initialised) {
        glDeleteBuffers(1, &lines->position_buffer_id);
        glDeleteBuffers(1, &lines->colour_buffer_id);
    }
}

void lines_add_pos(Line_Renderer *lines, vec2 pos) {
    lines->positions[lines->positions_current_index] = pos;
    lines->positions_current_index++;
    SDL_assert(lines->positions_current_index < LINE_RENDERER_POSITIONS_MAX && "lines positions current index is over LINE_RENDERER_POSITIONS_MAX");
}
void lines_add_col(Line_Renderer *lines, vec3 colour) {
    lines->colours[lines->colours_current_index] = colour;
    lines->colours_current_index++;
    SDL_assert(lines->colours_current_index < LINE_RENDERER_COLOURS_MAX && "lines positions current index is over LINE_RENDERER_COLOURS_MAX");

}

void segl_lines_draw_line_segment(Line_Renderer *lines, vec2 start, vec2 end) {
    lines_add_pos(lines, start);
    lines_add_pos(lines, end);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
}

void segl_lines_draw_cross(Line_Renderer *lines, vec2 center, f32 size) {
    lines_add_pos(lines, (vec2) {center.x - size, center.y - size});
    lines_add_pos(lines, (vec2) {center.x + size, center.y + size});
    lines_add_pos(lines, (vec2) {center.x + size, center.y - size});
    lines_add_pos(lines, (vec2) {center.x - size, center.y + size});
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
}

void segl_lines_update_frame(Line_Renderer *lines) {
    if (lines->colours_current_index != lines->positions_current_index) {
        printf("ERROR: Somehow, position and colour buffer are different sizes in the line renderer. This should not have happened\n");
        return;
    }
    if (lines->positions_current_index > 0) {
        segl_lines_compile(lines);
        segl_lines_draw(lines);
    }
    segl_lines_clear(lines);
}

void segl_lines_clear(Line_Renderer *lines) {
    lines->line_active = false;
    lines->positions_current_index = 0;
    lines->colours_current_index = 0;
}

void segl_lines_compile(Line_Renderer *lines) {
    glBindBuffer(GL_ARRAY_BUFFER, lines->position_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * lines->positions_current_index, lines->positions, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, lines->colour_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * lines->colours_current_index, lines->colours, GL_DYNAMIC_DRAW);
}

void segl_lines_draw(Line_Renderer *lines) {
    glBindBuffer(GL_ARRAY_BUFFER, lines->position_buffer_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

    glBindBuffer(GL_ARRAY_BUFFER, lines->colour_buffer_id);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_LINES, 0, (GLsizei)lines->positions_current_index);
}