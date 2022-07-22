#ifndef SE_MESH_H
#define SE_MESH_H

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

#include "seshader.h"
#include "serender_target.h"
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
void skeleton_deep_copy(SE_Skeleton *dest, const SE_Skeleton *src);

//// MATERIAL ////
typedef enum SE_MATERIAL_TYPES {
    SE_MATERIAL_TYPE_LIT,
    SE_MATERIAL_TYPE_TRANSPARENT,
    SE_MATERIAL_TYPE_CUSTOM,
} SE_MATERIAL_TYPES;
    // The default material is stored at the zero'th element during init() of renderer3D.
    // It must be zero because by default all meshes point to that index
#define SE_DEFAULT_MATERIAL_INDEX 0
typedef struct SE_Material {
    SE_MATERIAL_TYPES type; // the type of material this is (the default uniforms to be set during rendering)
    u32 shader_index;       // the index into user shaders
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
    b8 should_cast_shadow; // whether or not this mesh should cast shadows
        //- Material
    SE_MATERIAL_TYPES material_type;
    u32 material_shader_index;
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
    b8 should_cast_shadow; // whether or not this mesh should cast shadows

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

void ai_scene_to_mesh_save_data(const struct aiScene *scene, SE_Save_Data_Meshes *save_data, const char *filepath);

void sedefault_mesh(SE_Mesh *mesh);
void se_mesh_generate_skinned(SE_Mesh *mesh, u32 vert_count, const SE_Skinned_Vertex *vertices, u32 index_count, u32 *indices);
/// calculate the bounding box of a collection of vertices
AABB3D se_mesh_calc_aabb(const SE_Vertex3D *verts, u32 verts_count);
AABB3D se_mesh_calc_aabb_skinned(const SE_Skinned_Vertex *verts, u32 verts_count);
/// calculate the bounding box of a collection of bounding boxes
AABB3D aabb3d_calc(const AABB3D *aabb, u32 aabb_count);
AABB3D aabb3d_from_points(Vec3 point1, Vec3 point2, Mat4 transform);

#endif // SE_MESH_H