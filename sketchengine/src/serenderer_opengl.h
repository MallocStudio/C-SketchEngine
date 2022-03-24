#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "defines.h"
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

/// Vertex info of a mesh
typedef struct SE_Vertex3D {
    Vec4 position;
    Vec4 normal;
    Vec2 texture_coord;
    RGBA rgba;
} SE_Vertex3D;

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
void seshader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, Vec3 value);
/// Set a shader uniform
void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value);
/// returns a pointer to a string on the heap.
/// ! Needs to be freed by the caller
char* se_load_file_as_string(const char *filename);

///
/// MESH
///

#define SE_MESH_VERTICES_MAX 10000
typedef struct SE_Mesh {
    u32 vert_count;
    u32 vao; // vertex array object
    u32 vbo; // vertex buffer object
    u32 ibo; // index buffer object
    bool indexed; // whether we're using index buffers
} SE_Mesh;
/// delete vao, vbo, ibo
void semesh_deinit(SE_Mesh *mesh);
/// draw the mesh
void semesh_draw(SE_Mesh *mesh);

/// generate a quad. The mesh better be uninitialised because this function assumes there are
/// no previous data stored on the mesh
void semesh_generate_quad(SE_Mesh *mesh, Vec2 scale);
///
void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale);
///
void semesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices);
///
// void semesh_generate_raw(SE_Mesh *mesh, u32 positions_count, const Vec3 *positions, u32 index_count, const u32 *indices);
///
void semesh_generate_unindexed(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices);

///
/// Camera
///

typedef struct SE_Camera3D {
    Mat4 projection; // projection transform
    Mat4 view;       // view transform
    Vec3 position;
    Quat rotation;
} SE_Camera3D;

SEINLINE Mat4 secamera3d_get_view(const SE_Camera3D *cam) {
    Mat4 rotation = quat_to_mat4(cam->rotation);
    Vec3 forward = mat4_forward(rotation);
    return mat4_lookat(cam->position, vec3_add(cam->position, forward), vec3_up());

    // Mat4 rotation = quat_to_mat4(cam->rotation);
    // Vec3 forward = mat4_forward(rotation);
    // Mat4 transform = mat4_identity();
    // mat4
}

#endif // SERENDERER_OPENGL