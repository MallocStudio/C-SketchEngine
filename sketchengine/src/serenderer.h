#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "semesh.h"

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

    u32 user_shaders_count;
    SE_Shader *user_shaders[SERENDERER3D_MAX_SHADERS];

        //- SHADERS
    u32 shader_lit;                      // handles static meshes affected by light and the material system
    u32 shader_skinned_mesh;             // handles skinned meshes
    u32 shader_shadow_calc;              // handles directional light shadow calulation
    u32 shader_shadow_calc_skinned_mesh; // handles directional light shadow calulation
    u32 shader_shadow_omnidir_calc;      // handls point light shadow calculation
    u32 shader_lines;                    // handles rendering lines
    u32 shader_outline;                  // handles rendering outlines of static meshes
    u32 shader_sprite;                   // handles rendering sprites
    u32 shader_skinned_mesh_skeleton;    // handles rendering the skeleton (lines) of a given mesh with skeleton and animation
    u32 shader_shadow_omnidir_calc_skinned_mesh; // handles point light shadow calculation for skinned meshes

        //- Post Process Shaders
    u32 shader_post_process_tonemap;      // applies tone mapping and gamma correction
    u32 shader_post_process_blur;         // renders the given texture with a blur
    u32 shader_post_process_downsample;
    u32 shader_post_process_upsample;
    u32 shader_post_process_bloom;        // combines 2 textures, the second one being the bloom effect
    u32 shader_post_process_gaussian_blur;// blurs the BrightColour channel of post process framebuffer

    // Generated on init. Used for rendering quads to the screen.
    // Use this by simpling binding the vao
    GLuint screen_quad_vao;
    GLuint screen_quad_vbo;

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
    f32 directional_shadow_map_size;
    f32 omnidirectional_shadow_map_size;

    Rect viewport;

    f32 gamma;
    f32 time; // the time passed since the beginning (passed into shaders)
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
u32 se_render3d_add_point_light_ext(SE_Renderer3D *renderer, f32 constant, f32 linear, f32 quadratic);
    /// Setup renderer for rendering (set the configurations to their default values)
void se_render_mesh_index(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform, b8 transparent_pass);
void se_render_mesh(SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform, b8 transparent_pass);


// @remove
    /// Renders the given mesh with the given shader.
    /// The uniforms must be setup by the user.
// void se_render_mesh_with_shader(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform, u32 user_shader_index);

    /// Render a directional shadow map to the renderer.
    /// "transforms_count" must be equal to or less than the number of meshes in the renderer.
    /// This procedure will render each mesh based on the given array of transforms.
void se_render_directional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 transforms_count, AABB3D world_aabb);
void se_render_omnidirectional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 count);

    /// Adds a custom shader to the renderer and returns its index
u32 se_render3d_add_shader(SE_Renderer3D *renderer,
                            const char **vsd_files,
                            u32 vsd_count,
                            const char **fsd_files,
                            u32 fsd_count,
                            const char **gsd_files,
                            u32 gsd_count);

typedef enum SE_RENDER_POSTPROCESS {
    SE_RENDER_POSTPROCESS_TONEMAP,
    SE_RENDER_POSTPROCESS_BLUR,
    SE_RENDER_POSTPROCESS_DOWNSAMPLE,
    SE_RENDER_POSTPROCESS_UPSAMPLE,
    SE_RENDER_POSTPROCESS_BLOOM,
} SE_RENDER_POSTPROCESS;

    /// Takes the given texture and renders it to the current selected framebuffer with the given post process shader
void se_render_post_process(SE_Renderer3D *renderer, SE_RENDER_POSTPROCESS post_process, const SE_Render_Target *previous_render_pass);
void se_render_post_process_gaussian_blur(SE_Renderer3D *renderer, const SE_Render_Target *previous_render_pass, b8 horizontal);
#endif // SERENDERER_OPENGL