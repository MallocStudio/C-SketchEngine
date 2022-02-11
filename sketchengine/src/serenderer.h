#ifndef SKETCHENGINE_OPENGL
#define SKETCHENGINE_OPENGL
#include "GL/glew.h"
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "semath.h"
#include "defines.h"

/// debugging for SDL2
void print_sdl_error();

#define new(type) ( type *) malloc (sizeof( type ))
#define ERROR_ON_NOTZERO_SDL(x, additional_message) if( x != 0) {printf("(%s)\n", additional_message); print_sdl_error();}
#define ERROR_ON_NULL_SDL(x, additional_message) if( x == NULL) {printf("(%s)\n", additional_message); print_sdl_error();}

typedef struct SEGL_Camera {
    f32 height;
    vec2 center;
    f32 aspect_ratio;
    f32 speed;
} SEGL_Camera;
void segl_camera_init(SEGL_Camera *cam);
mat4 segl_get_camera_transform(SEGL_Camera *cam);
void segl_camera_zoom(SEGL_Camera *cam, f32 zoom_factor);

/// A copy of what Finn did
typedef struct Shader_Program {
    // Remember, OpenGL manages its resources itself and gives you handles to them.
    // That's what all this 'GLuint' business is about.
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    bool loaded_successfully;
} Shader_Program;
void segl_shader_program_init(Shader_Program *shader_program);
/// creates GL resources and links the given shaders
void segl_shader_program_init_from (Shader_Program *shader_program, const char *vertex_filename, const char *fragment_filename);
/// unloads GL resources
void segl_shader_program_deinit(Shader_Program *sp);
void segl_shader_program_use_shader(Shader_Program *sp); // @check do we need this?
GLuint segl_shader_program_get_uniform_loc  (Shader_Program *sp, const char *var_name);
void   segl_shader_program_set_uniform_f32  (Shader_Program *sp, const char *var_name, f32 value);
void   segl_shader_program_set_uniform_vec3 (Shader_Program *sp, const char *var_name, vec3 value);
void   segl_shader_program_set_uniform_mat4 (Shader_Program *sp, const char *var_name, mat4 value);
/// returns a pointer to a string on the heap.
//! Needs to be freed by the called
const char* load_file_as_string(const char *file_name);

/// -------------
/// LINE RENDERER
/// -------------
#define LINE_RENDERER_POSITIONS_MAX 1024
#define LINE_RENDERER_COLOURS_MAX 1024
typedef struct Line_Renderer {
    u32 positions_current_index;
    u32 colours_current_index;
    vec2 positions[LINE_RENDERER_POSITIONS_MAX];
    vec3 colours[LINE_RENDERER_COLOURS_MAX];

    bool initialised;
    vec3 current_colour;     // can be set directly
    vec2 first_pos;
    vec2 last_pos;
    vec3 first_colour;
    vec3 last_colour;
    bool line_active;
    GLuint position_buffer_id;
    GLuint colour_buffer_id;
} Line_Renderer;
void segl_lines_init(Line_Renderer *lines);
void segl_lines_deinit(Line_Renderer *lines);
void segl_lines_draw_line_segment(Line_Renderer *lines, vec2 start, vec2 end);
void segl_lines_draw_arrow(Line_Renderer *lines, vec2 start, vec2 end);
void segl_lines_draw_circle(Line_Renderer *lines, vec2 center, f32 size, u32 segment_count);
void segl_lines_update_frame(Line_Renderer *lines);
void segl_lines_clear(Line_Renderer *lines);
void segl_lines_compile(Line_Renderer *lines);
void segl_lines_draw(Line_Renderer *lines);
void segl_lines_draw_cross(Line_Renderer *lines, vec2 center, f32 size);
/// -----------
/// 2D RENDERER
/// -----------
void segl_2D_render_rect();
// void segl_2D_render_rect_outline();
// void segl_2D_render_circle();

#endif // SKETCHENGINE_OPENGL