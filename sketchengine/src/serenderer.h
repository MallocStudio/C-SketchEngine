#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

#include "seshader.h"
#include "Serender_target.h"
#include "sesprite.h"

/// Vertex info of a mesh
typedef struct SE_Vertex3D {
    Vec3 position;
    Vec3 normal;
    Vec3 tangent;
    Vec3 bitangent;
    Vec2 texture_coord;
} SE_Vertex3D;

///
/// MATERIAL
/// (think of material as a bunch of parameters)

typedef struct SE_Material {
    /* lit, line, sprite */
    Vec4 base_diffuse; // ! [0, 1] range !
    /* lit */
    SE_Texture texture_diffuse;
    SE_Texture texture_specular;
    SE_Texture texture_normal;
    /* sprite */
    SE_Sprite sprite;
} SE_Material;

/// Deallocates memory and frees resources (textures ...)
void sematerial_deinit(SE_Material *material);

///
/// MESH
///

typedef enum SE_MESH_TYPES {
    SE_MESH_TYPE_NORMAL, // normal mesh
    SE_MESH_TYPE_LINE,   // line mesh
    SE_MESH_TYPE_SPRITE, // meant for sprite (a quad)

    SE_MESH_TYPES_COUNT
} SE_MESH_TYPES;

#define SE_MESH_VERTICES_MAX 10000
typedef struct SE_Mesh {
    u32 vert_count;
    u32 vao;      // vertex array object
    u32 vbo;      // vertex buffer object
    u32 ibo;      // index buffer object
    bool indexed; // whether we're using index buffers
    AABB3D aabb;  // bounding box, calculated on load
    u32 material_index;

    // note that based on the material type, different shaders will be used
    SE_MESH_TYPES type;

    /* line */
    f32 line_width;
} SE_Mesh;

/// delete vao, vbo, ibo
void semesh_deinit(SE_Mesh *mesh);
/// generate a quad. The mesh better be uninitialised because this function assumes there are no previous data stored on the mesh
void semesh_generate_quad(SE_Mesh *mesh, Vec2 scale); // 2D plane
void semesh_generate_sprite(SE_Mesh *mesh, Vec2 scale);
void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale);
void semesh_generate_line(SE_Mesh *mesh, Vec3 pos1, Vec3 pos2, f32 width);
void semesh_generate_line_fan(SE_Mesh *mesh, Vec3 origin, Vec3 *positions, u32 positions_count, f32 line_width);
void semesh_generate_gizmos_aabb(SE_Mesh *mesh, Vec3 min, Vec3 max, f32 line_width);
void semesh_generate_gizmos_coordinates(SE_Mesh *mesh, f32 scale, f32 width);
void semesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices);
///
/// Light
///

typedef struct SE_Light {
    Vec3 direction;
    RGB ambient;
    RGB diffuse;
    f32 intensity;
} SE_Light;

typedef struct SE_Light_Point {
    /* light properties */
    Vec3 position;
    RGB ambient;
    RGB diffuse;
    RGB specular;
    f32 constant;
    f32 linear;
    f32 quadratic;

    /* shadow render target */
    GLuint depth_cube_map; // a cube map
    GLuint depth_map_fbo;
} SE_Light_Point;

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
    u32 shader_shadow_omnidir_calc;
    u32 shader_lines;
    u32 shader_outline;
    u32 shader_sprite;

    u32 materials_count;
    SE_Material *materials[SERENDERER3D_MAX_MATERIALS];

    u32 material_lines;  // load this once not per obj
    SE_Texture texture_default_diffuse;
    SE_Texture texture_default_normal;
    SE_Texture texture_default_specular;

    SE_Camera3D *current_camera;
    SE_Light light_directional;

    u32 point_lights_count;
    SE_Light_Point point_lights[4];

    /* shadow mapping */
    // u32 shadow_depth_map_fbo;
    // u32 shadow_depth_map;
    SE_Render_Target shadow_render_target;
    Mat4 light_space_matrix;
} SE_Renderer3D;

void serender3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera);
void serender3d_deinit(SE_Renderer3D *renderer);
/// Load a mesh and add it to the renderer. Returns the index of that loaded mesh.
u32 serender3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath);
u32 serender3d_add_cube(SE_Renderer3D *renderer);
u32 serender3d_add_plane(SE_Renderer3D *renderer, Vec3 scale);
u32 serender3d_add_sprite_mesh(SE_Renderer3D *renderer, Vec2 scale);
u32 serender3d_add_line(SE_Renderer3D *renderer, Vec3 pos1, Vec3 pos2, f32 width);

/// Create one of those 3D coordinate gizmos that show the directions
u32 serender3d_add_gizmos_coordniates(SE_Renderer3D *renderer, f32 scale, f32 width);
u32 serender3d_add_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width);
/// Returns an empty mesh, this mesh can be generated by calling one of the semesh_generate_X procedures.
u32 serender3d_add_mesh_empty(SE_Renderer3D *renderer);
void serender3d_update_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width, u32 mesh_index);
/// Load a shader program and att it to the renderer. Returns the index of that shader.
u32 serender3d_add_shader(SE_Renderer3D *renderer, const char *vsd, const char *fsd);
/// Add an empty material to the renderer
u32 serender3d_add_material(SE_Renderer3D *renderer);
/// Render all of the meshes the renderer contains
void serender3d_render_mesh(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);
void serender3d_render_mesh_outline(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);


///
/// UTILITIES
///

/// calculate the bounding box of a collection of vertices
AABB3D semesh_calc_aabb(const SE_Vertex3D *verts, u32 verts_count);
/// calculate the bounding box of a collection of bounding boxes
AABB3D aabb3d_calc(const AABB3D *aabb, u32 aabb_count);
AABB3D aabb3d_from_points(Vec3 point1, Vec3 point2, Mat4 transform);
#endif // SERENDERER_OPENGL