#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "defines.h"
#include "GL/glew.h"
#include "semath.h"

/// Vertex info of a mesh
typedef struct SE_Vertex3D {
    Vec4 position;
    Vec4 normal;
    Vec2 texture_coord;
    // RGBA rgba;
} SE_Vertex3D;

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
void seshader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, Vec3 value);
/// Set a shader uniform
void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value);
/// returns a pointer to a string on the heap.
/// ! Needs to be freed by the caller
char* se_load_file_as_string(const char *filename);

///
/// MESH // @temp after understanding OpenGL better, remove this struct and just have procedures that populate one giant vertex array and vertex index buffer with the appropriate data
///

#define SE_MESH_VERTICES_MAX 10000
typedef struct SE_Mesh {
    // u32 vert_count;
    // SE_Vertex3D verts[SE_MESH_VERTICES_MAX];
    u32 tri_count;
    u32 vao; // vertex array object
    u32 vbo; // vertex buffer object
    u32 ibo; // index buffer object
} SE_Mesh;
/// delete vao, vbo, ibo
void semesh_deinit(SE_Mesh *mesh);
/// generate a quad. The mesh better be uninitialised because this function assumes there are
/// no previous data stored on the mesh
void semesh_generate_quad(SE_Mesh *mesh);
/// draw the mesh
void semesh_draw(SE_Mesh *mesh);

///
/// RENDERER
///

#define SE_RENDERER2D_VERTICES_MAX 100000
typedef struct SE_Renderer3D {
    u32 vertices_count;
    SE_Vertex3D vertices[SE_RENDERER2D_VERTICES_MAX];
    GLuint vertices_buffer_id;

    bool initialised;
    SE_Shader shader_program;
} SE_Renderer3D;

/// initialise the 2D renderer
void serender3d_init(SE_Renderer3D *renderer, const char *vertex_filepath, const char *fragment_filepath);
/// deinitialise the 2D renderer
void serender3d_deinit(SE_Renderer3D *renderer);
///
void serender3d_update_frame(SE_Renderer3D *renderer);
///
void serender3d_clear(SE_Renderer3D *renderer);
///
void serender3d_render(SE_Renderer3D *renderer);

///
/// Camera
///

typedef struct SE_Camera3D {
    // f32 height; // @TODO change to FOV
    // Vec3 pos;
    // f32 aspect_ratio;
    // f32 speed;
    Mat4 projection; // projection transform
    Mat4 view;       // view transform
} SE_Camera3D;

#endif // SERENDERER_OPENGL