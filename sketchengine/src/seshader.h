#ifndef SESHADER_H
#define SESHADER_H

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

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
void seshader_deinit(SE_Shader *shader);
/// Binds the given shader for the GPU to use
void seshader_use(const SE_Shader *shader);
/// Get the address of a uniform
GLuint seshader_get_uniform_loc(SE_Shader *shader, const char *uniform_name);
/// Set a shader uniform
void seshader_set_uniform_f32  (SE_Shader *shader, const char *uniform_name, f32 value);
/// Set a shader uniform
void seshader_set_uniform_i32  (SE_Shader *shader, const char *uniform_name, i32 value);
/// Set a shader uniform
void seshader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, Vec3 value);
///
void seshader_set_uniform_vec4 (SE_Shader *shader, const char *uniform_name, Vec4 value);
///
void seshader_set_uniform_vec2 (SE_Shader *shader, const char *uniform_name, Vec2 value);
/// Set a shader uniform
void seshader_set_uniform_rgb (SE_Shader *shader, const char *uniform_name, RGB value);
/// Set a shader uniform
void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value);
/// returns a pointer to a string on the heap.
/// ! Needs to be freed by the caller
char* se_load_file_as_string(const char *filename);

#endif // SESHADER_H