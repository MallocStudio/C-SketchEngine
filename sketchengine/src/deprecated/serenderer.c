#include "serenderer.h"
#include "sephysics.h"
#include <stdio.h>

void segl_camera_init(SEGL_Camera *cam) {
    cam->aspect_ratio = 16.0f / 9.0f; // default
    cam->height = 10.0f;
    cam->speed = 0.8f;
}

Mat4 segl_get_camera_transform(SEGL_Camera *cam) {
    Mat4 result =  mat4_ortho(
        -cam->aspect_ratio * cam->height * 0.5f + cam->center.x,
        +cam->aspect_ratio * cam->height * 0.5f + cam->center.x,
        -cam->height * 0.5f + cam->center.y,
        +cam->height * 0.5f + cam->center.y,
        -1.0f, 1.0f);
    return result;
}

void segl_camera_zoom(SEGL_Camera *cam, f32 zoom_factor) {
    cam->height /= zoom_factor;
}

void segl_shader_program_init(SEGL_Shader_Program *shader_program) {
    shader_program->vertex_shader = -1;
    shader_program->fragment_shader = -1;
    shader_program->shader_program = -1;
    shader_program->loaded_successfully = false;
}

void segl_shader_program_init_from (SEGL_Shader_Program *sp, const char *vertex_filename, const char *fragment_filename) {
    sp->loaded_successfully = true;

    sp->vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    sp->shader_program = glCreateProgram();

    char *vertex_src = segl_load_file_as_string(vertex_filename);
    char *frag_src   = segl_load_file_as_string(fragment_filename);

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

void segl_shader_program_deinit(SEGL_Shader_Program *sp) {
    if (sp->loaded_successfully) {
        glDeleteShader(sp->vertex_shader);
        glDeleteShader(sp->fragment_shader);
        glDeleteProgram(sp->shader_program);
    }
}

GLuint segl_shader_program_get_uniform_loc (SEGL_Shader_Program *sp, const char *var_name) {
    return glGetUniformLocation(sp->shader_program, var_name);
}

void segl_shader_program_set_uniform_f32 (SEGL_Shader_Program *sp, const char *var_name, f32 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniform1f(var_loc, value);
}

void segl_shader_program_set_uniform_vec3 (SEGL_Shader_Program *sp, const char *var_name, Vec3 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniform3f(var_loc, value.x, value.y, value.z);
}

void segl_shader_program_set_uniform_mat4 (SEGL_Shader_Program *sp, const char *var_name, Mat4 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniformMatrix4fv(var_loc, 1, GL_FALSE, (const GLfloat*)&value); // @check (Finn did &value[0][0] with glm::mat4)
}

void segl_shader_program_use_shader(SEGL_Shader_Program *sp) {
    glUseProgram(sp->shader_program);
}

char* segl_load_file_as_string(const char *file_name) {
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

/// -------------
/// Line Renderer
/// -------------
void segl_lines_init(SEGL_Line_Renderer *lines) {
    lines->positions_current_index = 0;
    lines->colours_current_index   = 0;
    lines->current_colour = (Vec3) {1, 1, 1};
    lines->line_active = false;
    glGenBuffers(1, &lines->position_buffer_id);
    glGenBuffers(1, &lines->colour_buffer_id);
    lines->initialised = true;
}

void segl_lines_deinit(SEGL_Line_Renderer *lines) {
    if (lines->initialised) {
        glDeleteBuffers(1, &lines->position_buffer_id);
        glDeleteBuffers(1, &lines->colour_buffer_id);
    }
}

void lines_add_pos(SEGL_Line_Renderer *lines, Vec2 pos) {
    lines->positions[lines->positions_current_index] = pos;
    lines->positions_current_index++;
    SDL_assert(lines->positions_current_index < LINE_RENDERER_POSITIONS_MAX && "lines positions current index is over LINE_RENDERER_POSITIONS_MAX");
}

void lines_add_col(SEGL_Line_Renderer *lines, Vec3 colour) {
    lines->colours[lines->colours_current_index] = colour;
    lines->colours_current_index++;
    SDL_assert(lines->colours_current_index < LINE_RENDERER_COLOURS_MAX && "lines positions current index is over LINE_RENDERER_COLOURS_MAX");
}

void segl_lines_draw_line_segment(SEGL_Line_Renderer *lines, Vec2 start, Vec2 end) {
    lines_add_pos(lines, start);
    lines_add_pos(lines, end);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
}

void segl_lines_draw_arrow(SEGL_Line_Renderer *lines, Vec2 start, Vec2 end) {
    // line
    lines_add_pos(lines, start);
    lines_add_pos(lines, end);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    /// arrow // @check wtf is even happening
    f32 angle = SEMATH_DEG2RAD_MULTIPLIER * 45;
    f32 tip_size = 0.1f;
    Vec2 line = vec2_sub(start, end);
    Vec2 tip_1 = vec2_mul_scalar(line, tip_size);
    Vec2 tip_2 = tip_1;
    tip_1 = vec2_rotated(tip_1, angle);
    tip_2 = vec2_rotated(tip_2, -angle);
    tip_1 = vec2_add(tip_1, end);
    tip_2 = vec2_add(tip_2, end);

    lines_add_pos(lines, tip_1);
    lines_add_pos(lines, end);
    lines_add_pos(lines, tip_2);
    lines_add_pos(lines, end);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
}

void segl_lines_draw_cross(SEGL_Line_Renderer *lines, Vec2 center, f32 size) {
    lines_add_pos(lines, (Vec2) {center.x - size, center.y - size});
    lines_add_pos(lines, (Vec2) {center.x + size, center.y + size});
    lines_add_pos(lines, (Vec2) {center.x + size, center.y - size});
    lines_add_pos(lines, (Vec2) {center.x - size, center.y + size});
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
    lines_add_col(lines, lines->current_colour);
}

void segl_lines_draw_circle(SEGL_Line_Renderer *lines, Vec2 center, f32 size, u32 segment_count) {
    if (segment_count < 0) segment_count = 1;
    f32 cos_angle = cos(2 * SEMATH_PI / segment_count);
    f32 sin_angle = sin(2 * SEMATH_PI / segment_count);

    Mat2 rot_mat = mat2_create(cos_angle, -sin_angle, sin_angle, cos_angle);
    Vec2 plot_point = vec2_create(0, size);

    for (i32 i = 0; i <= segment_count; i++) {
        lines_add_pos(lines, vec2_add(center, plot_point));
        plot_point = mat2_mul_vec2(rot_mat, plot_point);
        lines_add_pos(lines, vec2_add(center, plot_point));
        lines_add_col(lines, lines->current_colour);
        lines_add_col(lines, lines->current_colour);
    }
}

void segl_lines_update_frame(SEGL_Line_Renderer *lines) {
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

void segl_lines_clear(SEGL_Line_Renderer *lines) {
    lines->line_active = false;
    lines->positions_current_index = 0;
    lines->colours_current_index = 0;
}

void segl_lines_compile(SEGL_Line_Renderer *lines) {
    glBindBuffer(GL_ARRAY_BUFFER, lines->position_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * lines->positions_current_index, lines->positions, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, lines->colour_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * lines->colours_current_index, lines->colours, GL_DYNAMIC_DRAW);
}

void segl_lines_draw(SEGL_Line_Renderer *lines) {
    glBindBuffer(GL_ARRAY_BUFFER, lines->position_buffer_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), 0);

    glBindBuffer(GL_ARRAY_BUFFER, lines->colour_buffer_id);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_LINES, 0, (GLsizei)lines->positions_current_index);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1); // @question Finn doesn't do it in lines, why is that?
}

/// ----------------
/// SEGL_Renderer2D
/// ----------------
void segl_render_2d_init(SEGL_Renderer2D *renderer, const char *vertex_shader_filepath, const char *fragment_shader_filepath) {
    renderer->shapes_count = 0;
    renderer->default_colour = (RGB) {1, 1, 1};
    renderer->current_colour = renderer->default_colour;

    renderer->colours_current_index = 0;
    renderer->vertices_current_index = 0;

    segl_shader_program_init_from(&renderer->shader_program, vertex_shader_filepath, fragment_shader_filepath);

    glGenBuffers(1, &renderer->vertices_buffer_id);
    glGenBuffers(1, &renderer->colours_buffer_id);
    renderer->initialised = true;
}

void segl_render_2d_deinit(SEGL_Renderer2D *renderer) {
    if (renderer->initialised) {
        renderer->shapes_count = 0;
        glGenBuffers(1, &renderer->vertices_buffer_id);
        glGenBuffers(1, &renderer->colours_buffer_id); // @question wtf?
        segl_shader_program_deinit(&renderer->shader_program);
        renderer->initialised = false;
    }
}

void segl_render_2d_compile(SEGL_Renderer2D *renderer) {
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vertices_buffer_id); // select vertex buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * renderer->vertices_current_index, renderer->vertices, GL_DYNAMIC_DRAW); // give it the data
    glBindBuffer(GL_ARRAY_BUFFER, renderer->colours_buffer_id); // select colour
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * renderer->colours_current_index, renderer->colours, GL_DYNAMIC_DRAW); // give it the data
}

void segl_render_2d_update_frame(SEGL_Renderer2D *renderer) {
    if (renderer->colours_current_index != renderer->vertices_current_index) {
        printf("ERROR: Somehow, vertices and colour buffer are different sizes in the 2d renderer. This should not have happened\n");
        return;
    }
    if (renderer->vertices_current_index > 0) {
        segl_render_2d_compile(renderer);
        segl_render_2d_render(renderer);
    }
    segl_render_2d_clear(renderer);
}

void segl_render_2d_clear(SEGL_Renderer2D *renderer) {
    renderer->vertices_current_index = 0;
    renderer->colours_current_index  = 0;
    renderer->shapes_count = 0;
}

void segl_render_2d_render(SEGL_Renderer2D *renderer) {
    segl_shader_program_use_shader(&renderer->shader_program);
    // Render OpenGL here
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vertices_buffer_id); // select the vertices buffer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), 0);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->colours_buffer_id); // select the colour buffer
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // set the selected buffer back to zero

    // render each rect separately
    for (i32 i = 0; i < renderer->shapes_count; ++i) {
        i32 current_shape = renderer->shapes[i].num_of_vertices;
        glDrawArrays(GL_TRIANGLE_FAN, i * current_shape, current_shape);
    }

    // render everything as one object (not a good idea)
    // glDrawArrays(GL_TRIANGLE_FAN, 0, renderer->vertices_current_index);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

/// File Scope ///

/// add vertex point to renderer
void segl_render_2d_add_pos(SEGL_Renderer2D *renderer, Vec2 pos) {
    renderer->vertices[renderer->vertices_current_index] = pos;
    renderer->vertices_current_index++;
    SDL_assert(renderer->vertices_current_index < SEGL_RENDERER_2D_VERTICES_MAX && "2D renderer vertices current index is over SEGL_RENDERER_2D_VERTICES_MAX");
}

void segl_render_2d_add_col(SEGL_Renderer2D *renderer, RGB col) {
    renderer->colours[renderer->colours_current_index] = col;
    renderer->colours_current_index++;
    SDL_assert(renderer->colours_current_index < SEGL_RENDERER_2D_COLOURS_MAX && "2D renderer colour current index is over SEGL_RENDERER_2D_VERTICES_MAX");
}

/// End of File Scope ///

void segl_render_2d_rect(SEGL_Renderer2D *renderer, Rect rect) {
    segl_render_2d_add_pos(renderer, vec2_create(rect.x, rect.y));
    segl_render_2d_add_pos(renderer, vec2_create(rect.x + rect.w, rect.y));
    segl_render_2d_add_pos(renderer, vec2_create(rect.x + rect.w, rect.y + rect.h));
    segl_render_2d_add_pos(renderer, vec2_create(rect.x, rect.y + rect.h));

    segl_render_2d_add_col(renderer, renderer->current_colour);
    segl_render_2d_add_col(renderer, renderer->current_colour);
    segl_render_2d_add_col(renderer, renderer->current_colour);
    segl_render_2d_add_col(renderer, renderer->current_colour);

    renderer->shapes[renderer->shapes_count].num_of_vertices = 4; // this shape has 4 vertices
    renderer->shapes_count++;
}

void segl_render_2d_set_color(SEGL_Renderer2D *renderer, RGB color) {
    renderer->current_colour = color;
}

void segl_render_2d_reset_color(SEGL_Renderer2D *renderer) {
    renderer->current_colour = renderer->default_colour;
}

void se_render_rect(SEGL_Line_Renderer *lines, const Rect *rect) {
    f32 xmin, ymin, xmax, ymax;
    xmin = rect->x;
    ymin = rect->y;
    xmax = rect->x + rect->w;
    ymax = rect->y + rect->h;
    se_render_rect_min_max(lines, xmin, ymin, xmax, ymax);
}

void se_render_rect_min_max(SEGL_Line_Renderer *lines, f32 xmin, f32 ymin, f32 xmax, f32 ymax) {
    Vec2 p1 = {xmin, ymin};
    Vec2 p2 = {xmax, ymin};
    Vec2 p3 = {xmax, ymax};
    Vec2 p4 = {xmin, ymax};
    segl_lines_draw_line_segment(lines, p1, p2);
    segl_lines_draw_line_segment(lines, p2, p3);
    segl_lines_draw_line_segment(lines, p3, p4);
    segl_lines_draw_line_segment(lines, p4, p1);
}