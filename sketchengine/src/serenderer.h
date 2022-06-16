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
#include "sestring.h"

#include "khash.h"

//// VERTEX ////

/// Vertex info of a mesh
typedef struct SE_Vertex3D {
    Vec3 position;
    Vec3 normal;
    Vec3 tangent;
    Vec3 bitangent;
    Vec2 texture_coord;
} SE_Vertex3D;

#define SE_MAX_BONE_WEIGHTS 4
typedef struct SE_Skinned_Vertex {
    SE_Vertex3D vert;
    /* the bones that manipulate this vertex */
    i32     bone_ids[SE_MAX_BONE_WEIGHTS];
    f32 bone_weights[SE_MAX_BONE_WEIGHTS];
} SE_Skinned_Vertex;

//// ANIMATION ////

#define SE_MAX_ANIMATION_BONE_KEYFRAMES 1000
typedef struct SE_Bone_Animations {
    // i32 bone_node_index;
    SE_String name; // name pf the bone
    u32 position_count;
    u32 rotation_count;
    u32 scale_count;
        // NOTE(Matin): these arrays must be of the same size (positions and position_time_stamps, ...)
        // the above count values describe how long each of these arrays are.
    Vec3 *positions;    // array of positions
    Quat *rotations;    // array of rotations
    Vec3 *scales;       // array of scales
    f32  *position_time_stamps; // array of position timestamps
    f32  *rotation_time_stamps; // array of rotation timestamps
    f32  *scale_time_stamps;    // array of scale timestamps
} SE_Bone_Animations;

typedef struct SE_Skeletal_Animation {
        // name of the animation
    SE_String name;
        // animation data
    u32 animated_bones_count;
    SE_Bone_Animations *animated_bones;
    f32 duration;
    f32 ticks_per_second;
} SE_Skeletal_Animation;

//// SKELETON AND BONES ////

typedef struct SE_Bone_Info {
        // id is index in 'bones' in the skinned_vertex.vsd
    i32 id;
        // offset matrix transforms vertex from model space to bone space
    Mat4 offset;
        // name of the bone, used for checking if nodes in the asset's scene is a bone node
    SE_String name;
} SE_Bone_Info;

#define MAX_BONE_CHILDREN 8
typedef struct SE_Bone_Node { // contains skeletal heirarchy information of a given bone
    SE_String name;
    i32 bones_info_index; // an index into the skeleton's bones_info array
    u32 children_count;
    i32 children[MAX_BONE_CHILDREN];
    i32 parent;
    Mat4 transform;
} SE_Bone_Node;

#define SE_SKELETON_BONES_CAPACITY 100 // ! needs to match with MAX_BONES in skinned_vertex.vsd
#define SE_SKELETON_MAX_ANIMATIONS 100
    /// contains skeletal heirarchy, bone info
typedef struct SE_Skeleton {
        // array of bone data (index into transformed bones in vertex shader, and offset matrix)
    u32 bone_count;
    SE_Bone_Info bones_info[SE_SKELETON_BONES_CAPACITY];
        // the heirarchy of bones (parent child relationship)
    u32 bone_node_count;
    SE_Bone_Node bone_nodes[SE_SKELETON_BONES_CAPACITY];

        // animations associated with this skeleton. The animations are loaded
        // when the skeleton is loaded from a file
    u32 animations_count;
    u32 current_animation;
    SE_Skeletal_Animation *animations[SE_SKELETON_MAX_ANIMATIONS];

        // the pose sent to GPU, call seskeleton_calculate_pose to update this pose
    Mat4 final_pose[SE_SKELETON_BONES_CAPACITY];
} SE_Skeleton;

    /// We do not have an animator class, instead use this procedure.
    /// Based on the given skeleton, skeleton->current_animation, and animation_time, we update the pose of the skeleton.
    /// The result is stored in final_bone_transforms and final_bone_transforms_count.
void seskeleton_calculate_pose(SE_Skeleton *skeleton, f32 animation_time);

//// MATERIAL ////

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

//// MESH ////

typedef enum SE_MESH_TYPES {
    SE_MESH_TYPE_NORMAL, // normal mesh
    SE_MESH_TYPE_LINE,   // line mesh
    SE_MESH_TYPE_POINT,  // mesh made out of points
    SE_MESH_TYPE_SPRITE, // meant for sprite (a quad)
    SE_MESH_TYPE_SKINNED,// meant for skeletal mesh animation

    SE_MESH_TYPES_COUNT
} SE_MESH_TYPES;

#define SE_MESH_VERTICES_MAX 10000
typedef struct SE_Mesh {
    i32 next_mesh_index; // a link to the next mesh (a mesh can consist of multiple meshes) if set to -1, then there is no other mesh
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
    /* point */
    f32 point_radius;

    /* skinned */
    // u32 bone_count;
    // kh_bone_info_t *bone_info;
    SE_Skeleton *skeleton; // if this mesh is skinned, this will not be nulled and must be freed
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
    /// If line is true, we will render the bones as lines not as points
    /// If with_animation is true, the animation REFERENCE is copied over to this mesh.
void semesh_generate_skinned_skeleton(SE_Mesh *mesh, SE_Skeleton *skeleton, bool line, bool with_animation);

//// Light ////

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

//// Camera ////

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

//// RENDERER ////

#define SERENDERER3D_MAX_MESHES 100
#define SERENDERER3D_MAX_SHADERS 100
#define SERENDERER3D_MAX_MATERIALS 100

typedef struct SE_Renderer3D {
        // -- meshes
    u32 meshes_count;
    SE_Mesh *meshes[SERENDERER3D_MAX_MESHES];

    u32 shaders_count;
    SE_Shader *shaders[SERENDERER3D_MAX_SHADERS];

    u32 shader_lit;                     // handles static meshes affected by light and the material system
    u32 shader_skinned_mesh;            // handles skinned meshes (uses a special vertex shader but the same fragment shader as shader_lit)
    u32 shader_shadow_calc;             // handles directional light shadow calulation
    u32 shader_shadow_omnidir_calc;     // handls point light shadow calculation
    u32 shader_lines;                   // handles rendering lines
    u32 shader_outline;                 // handles rendering outlines of static meshes
    u32 shader_sprite;                  // handles rendering sprites
    u32 shader_skinned_mesh_skeleton;   // handles rendering the skeleton (lines) of a given mesh with skeleton and animation

    u32 materials_count;
    SE_Material *materials[SERENDERER3D_MAX_MATERIALS];

    u32 material_lines;  // default line material (white lines)
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
u32 serender3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath, bool with_animation);
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
    /// Add an uninitialised skeleton to the renderer
u32 serender3d_add_skeletal_animation(SE_Renderer3D *renderer);
/// Setup renderer for rendering (set the configurations to their default values)
void serender3d_reset_render_config();
void serender_mesh_index(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);
void serender_mesh(const SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform);
void serender3d_render_mesh_outline(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);

//// UTILITIES ////

/// calculate the bounding box of a collection of vertices
AABB3D semesh_calc_aabb(const SE_Vertex3D *verts, u32 verts_count);
/// calculate the bounding box of a collection of bounding boxes
AABB3D aabb3d_calc(const AABB3D *aabb, u32 aabb_count);
AABB3D aabb3d_from_points(Vec3 point1, Vec3 point2, Mat4 transform);
#endif // SERENDERER_OPENGL