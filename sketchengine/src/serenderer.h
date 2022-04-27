#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

#include "seshader.h"
#include "Serender_target.h"

/// Vertex info of a mesh
typedef struct SE_Vertex3D {
    Vec3 position;
    Vec3 normal;
    Vec3 tangent;
    Vec3 bitangent;
    Vec2 texture_coord;
    RGBA rgba;
} SE_Vertex3D;

///
/// TEXTURE
///

typedef struct SE_Texture {
    GLuint id;
    i32 width;
    i32 height;
    i32 channel_count;
    bool loaded;
} SE_Texture;

void setexture_load(SE_Texture *texture, const char *filepath);
void setexture_load_data(SE_Texture *texture, ubyte *data);
void setexture_unload(SE_Texture *texture);
void setexture_bind(const SE_Texture *texture, u32 index);
void setexture_unbind();

///
/// TEXTURE ATLAS
///

typedef struct SE_Texture_Atlas {
    SE_Texture texture;
    u32 columns; // number of columns
    u32 rows;    // number of rows
} SE_Texture_Atlas;

void setexture_atlas_load(SE_Texture_Atlas *texture_atlas, const char *filepath, u32 columns, u32 rows);
void setexture_atlas_unload(SE_Texture_Atlas *texture_atlas);
void setexture_atlas_bind(SE_Texture_Atlas *texture_atlas);
void setexture_atlas_unbind();

///
/// MATERIAL
/// (think of material as a bunch of parameters)

typedef struct SE_Material {
    Vec4 base_diffuse;
    SE_Texture texture_diffuse;
    SE_Texture texture_specular;
    SE_Texture texture_normal;
} SE_Material;

/// Deallocates memory and uninitialises the textures
void sematerial_deinit(SE_Material *material);

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
    u32 material_index;
} SE_Mesh;

/// delete vao, vbo, ibo
void semesh_deinit(SE_Mesh *mesh);
/// generate a quad. The mesh better be uninitialised because this function assumes there are no previous data stored on the mesh
void semesh_generate_quad(SE_Mesh *mesh, Vec2 scale);
void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale);
void semesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices);

///
/// Light
///

typedef struct SE_Light {
    Vec3 direction;
    RGB ambient;
    RGB diffuse;
} SE_Light;

///
/// Camera
///

typedef struct SE_Camera3D {
    // @note that these two matrices are updated by secamera3d_update_projection()
    Mat4 projection; // projection transform
    Mat4 view;       // view transform

    Vec3 position;
    f32 yaw;
    f32 pitch;
    Vec3 up;
} SE_Camera3D;

void secamera3d_init(SE_Camera3D *cam);
Vec3 secamera3d_get_front(const SE_Camera3D *cam);
Mat4 secamera3d_get_view(const SE_Camera3D *cam);
/// updates the given camera's view and projection
void secamera3d_update_projection(SE_Camera3D *cam, i32 window_w, i32 window_h);
void secamera3d_input(SE_Camera3D *camera, struct SE_Input *seinput);

///
/// RENDERER
///

#define SERENDERER3D_MAX_MESHES 100
#define SERENDERER3D_MAX_SHADERS 100
#define SERENDERER3D_MAX_MATERIALS 100

typedef struct SE_Renderer3D {
    u32 meshes_count;
    SE_Mesh *meshes[SERENDERER3D_MAX_MESHES];

    u32 shaders_count;
    SE_Shader *shaders[SERENDERER3D_MAX_SHADERS];

    u32 shader_lit;
    u32 shader_shadow_calc;
    u32 shader_shadow_debug_render;

    u32 materials_count;
    SE_Material *materials[SERENDERER3D_MAX_MATERIALS];

    SE_Camera3D *current_camera;
    SE_Light light_directional;

    /* shadow mapping */
    // u32 shadow_depth_map_fbo;
    // u32 shadow_depth_map;
    SE_Render_Target shadow_render_target;
    Mat4 light_space_matrix;

} SE_Renderer3D;

u32 serender3d_add_shader(SE_Renderer3D *renderer, const char *vsd, const char *fsd);
void serender3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera);
void serender3d_deinit(SE_Renderer3D *renderer);
/// Load a mesh and add it to the renderer. Returns the index of that loaded mesh.
u32 serender3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath);
u32 serender3d_add_cube(SE_Renderer3D *renderer);
u32 serender3d_add_plane(SE_Renderer3D *renderer, Vec3 scale);
/// Render all of the meshes the renderer contains
void serender3d_render_mesh(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);
#endif // SERENDERER_OPENGL