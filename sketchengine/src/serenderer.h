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
#include "secamera.h"
#include "khash.h"

//// VERTEX ////

/// Vertex info of a mesh
typedef struct SE_Vertex3D {
    Vec3 position;
    Vec3 normal;
    Vec3 tangent;
    Vec3 bitangent;
    Vec2 texture_coord;
    RGBA colour; // used for colouring lines and points
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
    Mat4 local_transform; // local space (in t-pose)
    Mat4 inverse_neutral_transform; // the inverse t-pose model space transform of the bone
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

        // the pose sent to GPU, call se_skeleton_calculate_pose to update this pose
    Mat4 final_pose[SE_SKELETON_BONES_CAPACITY];
} SE_Skeleton;

    /// We do not have an animator class, instead use this procedure.
    /// Based on the given skeleton, skeleton->current_animation, and animation_time, we update the pose of the skeleton.
    /// The result is stored in final_bone_transforms and final_bone_transforms_count.
void se_skeleton_calculate_pose(SE_Skeleton *skeleton, f32 animation_time);
void se_skeleton_deinit(SE_Skeleton *skeleton);

//// MATERIAL ////
    // The default material is stored at the zero'th element during init() of renderer3D.
    // It must be zero because by default all meshes point to that index
#define SE_DEFAULT_MATERIAL_INDEX 0
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
void se_material_deinit(SE_Material *material);

//// MESH ////

typedef enum SE_MESH_TYPES {
    SE_MESH_TYPE_NORMAL, // normal mesh
    SE_MESH_TYPE_LINE,   // line mesh
    SE_MESH_TYPE_POINT,  // mesh made out of points
    SE_MESH_TYPE_SPRITE, // meant for sprite (a quad)
    SE_MESH_TYPE_SKINNED,// meant for skeletal mesh animation

    SE_MESH_TYPES_COUNT
} SE_MESH_TYPES;


/// A temporary struct that holds the vertices, indices, and all the raw
/// data associated with what makes up a mesh.
/// This is used for saving and loading meshes to and from disk
typedef struct SE_Mesh_Raw_Data {
        //- Headers
    SE_MESH_TYPES type;
        //- Verts
    // note that depending on the "type" one of
    // SE_Vertex3D or SE_Skinned_Vertex will be used and not NULL
    u32 vert_count;
    SE_Vertex3D *verts; // array of verts
    SE_Skinned_Vertex *skinned_verts; // array of skinned verts
    u32 index_count;
    u32 *indices;       // array of indices
        //- Shape
    f32 line_width;
    f32 point_radius;
    b8 is_indexed;
    AABB3D aabb;
        //- Material
    Vec4 base_diffuse;   // in range of [0 - 1]
    SE_String texture_diffuse_filepath;
    SE_String texture_specular_filepath;
    SE_String texture_normal_filepath;
        //- Skeleton
    SE_Skeleton *skeleton_data;
} SE_Mesh_Raw_Data;

typedef struct SE_Save_Data_Meshes {
    u32 meshes_count;     // number of meshes in this file that are linked together
    SE_Mesh_Raw_Data *meshes; // array of raw data
} SE_Save_Data_Meshes;

    /// Free the memory resources used by the "raw_data"
void se_save_data_mesh_deinit(SE_Save_Data_Meshes *save_data);
    /// Load SE_Mesh_Raw_Data from "save_file" and load a SE_Mesh from that.
    /// Returns the index of the loaded mesh.
    //! The user must manage memory. Call "se_save_data_mesh_deinit" to
    //! properly manage the data's memory
void se_save_data_read_mesh(SE_Save_Data_Meshes *save_data, const char *save_file);
    /// Saves the given SE_Mesh_Raw_Data to disk.
void se_save_data_write_mesh(const SE_Save_Data_Meshes *save_data, const char *save_file);

#define SE_MESH_VERTICES_MAX 10000
typedef struct SE_Mesh {
    i32 next_mesh_index; // a link to the next mesh (a mesh can consist of multiple meshes) if set to -1, then there is no other mesh
    u32 element_count;
    u32 vao;      // vertex array object
    u32 vbo;      // vertex buffer object
    u32 ibo;      // index buffer object
    b8 indexed; // whether we're using index buffers
    AABB3D aabb;  // bounding box, calculated on load
    u32 material_index;

    // note that based on the material type, different shaders will be used
    SE_MESH_TYPES type;

    /* line */
    f32 line_width;
    /* point */
    f32 point_radius;

    /* skinned */
    SE_Skeleton *skeleton; // ! not owned. // @TODO change to a u32 index into SE_Renderer3D user_skeletons array
} SE_Mesh;

/// delete vao, vbo, ibo
void se_mesh_deinit(SE_Mesh *mesh);
/// generate a quad. The mesh better be uninitialised because this function assumes there are no previous data stored on the mesh
void se_mesh_generate_quad(SE_Mesh *mesh, Vec2 scale); // 2D plane
void se_mesh_generate_sprite(SE_Mesh *mesh, Vec2 scale);
void se_mesh_generate_cube(SE_Mesh *mesh, Vec3 scale);
void se_mesh_generate_line(SE_Mesh *mesh, Vec3 pos1, Vec3 pos2, f32 width, RGBA colour);
void se_mesh_generate_line_fan(SE_Mesh *mesh, Vec3 origin, Vec3 *positions, u32 positions_count, f32 line_width);
void se_mesh_generate_gizmos_aabb(SE_Mesh *mesh, Vec3 min, Vec3 max, f32 line_width);
void se_mesh_generate_gizmos_coordinates(SE_Mesh *mesh, f32 width);
void se_mesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices);
    /// If line is true, we will render the bones as lines not as points
    /// If with_animation is true, the animation REFERENCE is copied over to this mesh.
void se_mesh_generate_skinned_skeleton(SE_Mesh *mesh, SE_Skeleton *skeleton, b8 line, b8 with_animation);
    /// Generates a mesh of type lines that shows the bone node heirachy of the given skeleton
void se_mesh_generate_static_skeleton(SE_Mesh *mesh, const SE_Skeleton *skeleton);

//// Light ////

typedef struct SE_Light {
    Vec3 direction;
    Vec3 calculated_position; // calculated during the rendering of the shadow map
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

//// RENDERER ////

#define SERENDERER3D_MAX_MESHES 10000
#define SERENDERER3D_MAX_SKELETONS SERENDERER3D_MAX_MESHES
#define SERENDERER3D_MAX_SHADERS 100
#define SERENDERER3D_MAX_MATERIALS 10000
#define SERENDERER3D_MAX_POINT_LIGHTS 4

typedef struct SE_Renderer3D {
    // ! NOTE THAT EVERYTHING IS SET TO ZERO AT THE BEGINNING OF INIT()
    // ! LOOK AT se_render3d_init TO SEE THE DEFAULT VALUES

        //- USER MESHES
// ! IMPORTANT NOTE AND WARNING. THE RENDERER SHOULD NOT LOAD ANY DEFAULT MESH.
// ! Because the user would like to save and load from the disk.
// ! Adding meshes on init() will cause exponential duplicates.
// ! Because they will get saved and loaded from the disk.
    u32 user_meshes_count;
    SE_Mesh *user_meshes[SERENDERER3D_MAX_MESHES];

        //- USER MATERIALS
// ! IMPORTANT NOTE AND WARNING. THE RENDERER SHOULD NOT LOAD ANY DEFAULT MATERIAL.
// ! Because the user would like to save and
// ! load from the disk. Adding materials on init() will cause exponential duplicates.
// ! Because they will get saved and loaded from the disk.
    u32 user_materials_count;
    SE_Material *user_materials[SERENDERER3D_MAX_MATERIALS];

        //- USER SKELETONS
// ! IMPORTANT NOTE AND WARNING. THE RENDERER SHOULD NOT LOAD ANY DEFAULT SKELETONS.
// ! Because the user would like to save and load from the disk.
// ! Adding meshes on init() will cause exponential duplicates.
// ! Because they will get saved and loaded from the disk.
    u32 user_skeletons_count;
    SE_Skeleton *user_skeletons[SERENDERER3D_MAX_SKELETONS];

        //- SHADERS
    SE_Shader shader_lit;                     // handles static meshes affected by light and the material system
    SE_Shader shader_skinned_mesh;            // handles skinned meshes (uses a special vertex shader but the same fragment shader as shader_lit)
    SE_Shader shader_shadow_calc;             // handles directional light shadow calulation
    SE_Shader shader_shadow_calc_skinned_mesh;// handles directional light shadow calulation
    SE_Shader shader_shadow_omnidir_calc;     // handls point light shadow calculation
    SE_Shader shader_lines;                   // handles rendering lines
    SE_Shader shader_outline;                 // handles rendering outlines of static meshes
    SE_Shader shader_sprite;                  // handles rendering sprites
    SE_Shader shader_skinned_mesh_skeleton;   // handles rendering the skeleton (lines) of a given mesh with skeleton and animation

        //- Camera and Light
    SE_Camera3D *current_camera;
    SE_Light light_directional;

    u32 point_lights_count;
    SE_Light_Point point_lights[SERENDERER3D_MAX_POINT_LIGHTS];

    /* shadow mapping */
    // u32 shadow_depth_map_fbo;
    // u32 shadow_depth_map;
    SE_Render_Target shadow_render_target;
    Mat4 light_space_matrix;
} SE_Renderer3D;

void se_render3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera);
void se_render3d_deinit(SE_Renderer3D *renderer);
u32 se_render3d_add_cube(SE_Renderer3D *renderer);
u32 se_render3d_add_plane(SE_Renderer3D *renderer, Vec3 scale);
u32 se_render3d_add_sprite_mesh(SE_Renderer3D *renderer, Vec2 scale);
u32 se_render3d_add_line(SE_Renderer3D *renderer, Vec3 pos1, Vec3 pos2, f32 width, RGBA colour);

    //- SAVING AND LOADING MESHES
    /// Load a mesh and add it to the renderer. Returns the index of that loaded mesh.
u32 se_render3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath, b8 with_animation);
    /// Generates "SE_Mesh" and adds it to the renderer based on the given save file.
    /// Returns the index of the generated mesh.
u32 se_save_data_mesh_to_mesh(SE_Renderer3D *renderer, const SE_Save_Data_Meshes *save_data);

    /// Create one of those 3D coordinate gizmos that show the directions
u32 se_render3d_add_gizmos_coordniates(SE_Renderer3D *renderer);
u32 se_render3d_add_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width);
    /// Returns an empty mesh, this mesh can be generated by calling one of the semesh_generate_X procedures.
u32 se_render3d_add_mesh_empty(SE_Renderer3D *renderer);
void se_render3d_update_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width, u32 mesh_index);
    /// Load a shader program and att it to the renderer. Returns the index of that shader.
// u32 se_render3d_add_shader(SE_Renderer3D *renderer, const char *vsd, const char *fsd);
    /// Add an empty material to the renderer
u32 se_render3d_add_material(SE_Renderer3D *renderer);
    /// Add an uninitialised skeleton to the renderer
u32 se_render3d_add_skeleton(SE_Renderer3D *renderer);
    /// Add a point light to the renderer
u32 se_render3d_add_point_light(SE_Renderer3D *renderer);
    /// Setup renderer for rendering (set the configurations to their default values)
void se_render3d_reset_render_config();
void se_render_mesh_index(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);
void se_render_mesh(SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform);
    /// Render the given mesh with the given shader. The called must be passing the uniforms to the shader and make sure
    /// that everything lines up
void se_render_mesh_with_shader(SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform, SE_Shader *shader);
void se_render3d_render_mesh_outline(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform);
    /// Render a directional shadow map to the renderer.
    /// "transforms_count" must be equal to or less than the number of meshes in the renderer.
    /// This procedure will render each mesh based on the given array of transforms.
void se_render_directional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 transforms_count, AABB3D world_aabb);
void se_render_omnidirectional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 count);

//// UTILITIES ////

/// calculate the bounding box of a collection of vertices
AABB3D se_mesh_calc_aabb(const SE_Vertex3D *verts, u32 verts_count);
AABB3D se_mesh_calc_aabb_skinned(const SE_Skinned_Vertex *verts, u32 verts_count);
/// calculate the bounding box of a collection of bounding boxes
AABB3D aabb3d_calc(const AABB3D *aabb, u32 aabb_count);
AABB3D aabb3d_from_points(Vec3 point1, Vec3 point2, Mat4 transform);
#endif // SERENDERER_OPENGL