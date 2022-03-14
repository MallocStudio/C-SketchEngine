#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "defines.h"
#include "GL/glew.h"
#include "semath.h"

/// Vertex info of a mesh
typedef struct SE_Vertex2D {
    Vec2 position;
    RGBA rgba;
} SE_Vertex2D;

///
/// Shader program info
///

typedef struct SE_Shader {
    // Remember, OpenGL manages its resources itself and gives you handles to them.
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    bool loaded_successfully;
} SE_Shader;

/// Creates GL resources and compiles & links the given shaders
void seshader_init_from(SE_Shader *shader_program, const char *vertex_filename, const char *fragment_filename);
/// Unloads GL resources used by the shader program
seshader_deinit(SE_Shader *shader);
/// Binds the given shader for the GPU to use
seshader_use(SE_Shader *shader);
/// Get the address of a uniform
GLuint seshader_get_uniform_loc(SE_Shader *shader, const char *uniform_name);
/// Set a shader uniform
void seshader_set_uniform_f32  (SE_Shader *shader, const char *uniform_name, f32 value);
/// Set a shader uniform
void seshader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, f32 value);
/// Set a shader uniform
void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, f32 value);
/// returns a pointer to a string on the heap.
/// ! Needs to be freed by the caller
char* se_load_file_as_string(const char *filename);

///
/// RENDERER
///

#define SE_RENDERER2D_VERTICES_MAX 100000
typedef struct SE_Renderer2D {
    u32 vertices_count;
    SE_Vertex2D vertices[SE_RENDERER2D_VERTICES_MAX];
    GLuint vertices_buffer_id;

    bool initialised;
    SE_Shader shader_program;
} SE_Renderer2D;

#endif // SERENDERER_OPENGL