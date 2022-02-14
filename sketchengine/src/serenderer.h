#ifndef SKETCHENGINE_OPENGL
#define SKETCHENGINE_OPENGL
#include "GL/glew.h"
#include "semath.h"
#include "defines.h"

/// ---------
/// 2D camera
/// ---------
typedef struct SEGL_Camera {
    f32 height;
    Vec2 center;
    f32 aspect_ratio;
    f32 speed;
} SEGL_Camera;
void segl_camera_init(SEGL_Camera *cam);
Mat4 segl_get_camera_transform(SEGL_Camera *cam);
void segl_camera_zoom(SEGL_Camera *cam, f32 zoom_factor);

/// ---------------------
/// FINN'S SHADER PROGRAM
/// ---------------------
typedef struct SEGL_Shader_Program {
    // Remember, OpenGL manages its resources itself and gives you handles to them.
    // That's what all this 'GLuint' business is about.
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    bool loaded_successfully;
} SEGL_Shader_Program;
void segl_shader_program_init(SEGL_Shader_Program *shader_program);
/// creates GL resources and links the given shaders
void segl_shader_program_init_from (SEGL_Shader_Program *shader_program, const char *vertex_filename, const char *fragment_filename);
/// unloads GL resources
void segl_shader_program_deinit(SEGL_Shader_Program *sp);
void segl_shader_program_use_shader(SEGL_Shader_Program *sp); // @check do we need this?
GLuint segl_shader_program_get_uniform_loc  (SEGL_Shader_Program *sp, const char *var_name);
void   segl_shader_program_set_uniform_f32  (SEGL_Shader_Program *sp, const char *var_name, f32 value);
void   segl_shader_program_set_uniform_vec3 (SEGL_Shader_Program *sp, const char *var_name, Vec3 value);
void   segl_shader_program_set_uniform_mat4 (SEGL_Shader_Program *sp, const char *var_name, Mat4 value);
/// returns a pointer to a string on the heap.
//! Needs to be freed by the called
char* segl_load_file_as_string(const char *file_name);

/// -------------
/// LINE RENDERER
/// -------------
#define LINE_RENDERER_POSITIONS_MAX 1024
#define LINE_RENDERER_COLOURS_MAX 1024
typedef struct SEGL_Line_Renderer {
    u32 positions_current_index;
    u32 colours_current_index;
    Vec2 positions[LINE_RENDERER_POSITIONS_MAX];
    Vec3 colours[LINE_RENDERER_COLOURS_MAX];

    bool initialised;
    Vec3 current_colour;     // can be set directly
    Vec2 first_pos;
    Vec2 last_pos;
    Vec3 first_colour;
    Vec3 last_colour;
    bool line_active;
    GLuint position_buffer_id;
    GLuint colour_buffer_id;
} SEGL_Line_Renderer;
void segl_lines_init(SEGL_Line_Renderer *lines);
void segl_lines_deinit(SEGL_Line_Renderer *lines);
void segl_lines_draw_line_segment(SEGL_Line_Renderer *lines, Vec2 start, Vec2 end);
void segl_lines_draw_arrow(SEGL_Line_Renderer *lines, Vec2 start, Vec2 end);
void segl_lines_draw_circle(SEGL_Line_Renderer *lines, Vec2 center, f32 size, u32 segment_count);
void segl_lines_update_frame(SEGL_Line_Renderer *lines);
void segl_lines_clear(SEGL_Line_Renderer *lines);
void segl_lines_compile(SEGL_Line_Renderer *lines);
void segl_lines_draw(SEGL_Line_Renderer *lines);
void segl_lines_draw_cross(SEGL_Line_Renderer *lines, Vec2 center, f32 size);
/// -----------
/// 2D RENDERER
/// -----------
typedef struct SEGL_Renderer2D_Shape {
    i32 num_of_vertices;
} SEGL_Renderer2D_Shape;

#define SEGL_RENDERER_2D_VERTICES_MAX 1024
#define SEGL_RENDERER_2D_COLOURS_MAX 1024
#define SEGL_RENDERER_2D_SHAPES_MAX 300
typedef struct SEGL_Renderer2D {
    bool initialised;    // whether the renderer has been initialised and ready to roll
    Vec3 current_colour; // can be set directly
    Vec3 default_colour; // set during init, but when changed this will become the default
    
    GLuint vertices_buffer_id;
    GLuint colours_buffer_id;
    GLsizei vertices_current_index;
    GLsizei colours_current_index;
    // the way this works is that we store the vertices of all of our shapes into one array
    // same with the color of each vertex. we also have an array of shapes which tells us
    // how many of those vertices belong to one shape, so we can render them separately.
    Vec2 vertices[SEGL_RENDERER_2D_VERTICES_MAX]; // verticies to be rendered
    Vec3 colours[SEGL_RENDERER_2D_VERTICES_MAX];  // colour of each of those verticies
    i32 shapes_count; // number of shapes that we need to draw
    SEGL_Renderer2D_Shape shapes[SEGL_RENDERER_2D_SHAPES_MAX]; // each shape this renderer needs to draw
} SEGL_Renderer2D;
void segl_render_2d_init(SEGL_Renderer2D *renderer);
void segl_render_2d_deinit(SEGL_Renderer2D *renderer);
void segl_render_2d_rect(SEGL_Renderer2D *renderer, Rect rect);
// void segl_2D_render_rect_outline();
// void segl_2D_render_circle();

void segl_render_2d_compile(SEGL_Renderer2D *renderer);
void segl_render_2d_render(SEGL_Renderer2D *renderer);
void segl_render_2d_clear(SEGL_Renderer2D *renderer);
void segl_render_2d_update_frame(SEGL_Renderer2D *renderer); // compile, render, clear

#endif // SKETCHENGINE_OPENGL