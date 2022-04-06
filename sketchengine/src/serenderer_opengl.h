#ifndef SERENDERER_OPENGL
#define SERENDERER_OPENGL

/// After implementing physics testbed, and learning more about rendering and graphics,
/// here's a second attempt at building a graphics library in c

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"
#include "seinput.h" // for camera
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
    Vec3 position;
    Vec3 normal; // @TODO change normal and position to Vec3
    Vec3 tangent;
    Vec3 bitangent;
    Vec2 texture_coord;
    RGBA rgba;
} SE_Vertex3D;

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
void seshader_set_uniform_vec2 (SE_Shader *shader, const char *uniform_name, Vec2 value);
/// Set a shader uniform
void seshader_set_uniform_rgb (SE_Shader *shader, const char *uniform_name, RGB value);
/// Set a shader uniform
void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value);
/// returns a pointer to a string on the heap.
/// ! Needs to be freed by the caller
char* se_load_file_as_string(const char *filename);

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
/// MATERIAL
/// (think of material as a bunch of parameters)

// #define SEMATERIAL_NAME_SIZE 256
typedef struct SE_Material {
    /* Material name */
    // char name[SEMATERIAL_NAME_SIZE];

    /* Texture maps */
    SE_Texture texture_diffuse;
    SE_Texture texture_specular;
    SE_Texture texture_normal;
/*    SE_Texture map_Ka;
    SE_Texture map_Ks;
    SE_Texture map_Ke;
    SE_Texture map_Kt;
    SE_Texture map_Ns;
    SE_Texture map_Ni;
    SE_Texture map_d;
    SE_Texture map_bump;
*/
} SE_Material;

/// Deallocates memory and uninitialises the textures
SEINLINE void sematerial_deinit(SE_Material *material) {
    if (material->texture_diffuse.loaded)  setexture_unload(&material->texture_diffuse);
    if (material->texture_specular.loaded) setexture_unload(&material->texture_specular);
    if (material->texture_normal.loaded)   setexture_unload(&material->texture_normal);
}

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

    Mat4 transform;
    u32 material_index;
} SE_Mesh;

/// delete vao, vbo, ibo
void semesh_deinit(SE_Mesh *mesh);
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
// void semesh_generate_unindexed(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices);

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
    Mat4 projection; // projection transform
    Mat4 view;       // view transform
    Vec3 position;
    // Vec2 oriantation; // x horizontal rotation, y is vertical rotation
    // Vec3 target;
    f32 yaw;
    f32 pitch;
    Vec3 up;
} SE_Camera3D;

SEINLINE void secamera3d_init(SE_Camera3D *cam) {
    cam->position = vec3_zero();
    // cam->target = target;
    cam->yaw = 0;
    cam->pitch = 0;
    cam->up = vec3_up();
}

SEINLINE Vec3 secamera3d_get_front(const SE_Camera3D *cam) {
    f32 yaw = cam->yaw;
    f32 pitch = cam->pitch;
    Vec3 camera_front = {0};
    camera_front.x = semath_cos(yaw * SEMATH_DEG2RAD_MULTIPLIER) * semath_cos(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    camera_front.y = semath_sin(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    camera_front.z = semath_sin(yaw * SEMATH_DEG2RAD_MULTIPLIER) * semath_cos(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    vec3_normalise(&camera_front);
    return camera_front;
}

SEINLINE Mat4 secamera3d_get_view(const SE_Camera3D *cam) {
    // Mat4 rotation = mat4_euler_xyz(cam->oriantation.y, cam->oriantation.x, 0);

    // Quat rotation_q_x = quat_from_axis_angle(vec3_up(), cam->oriantation.x, true);
    // Quat rotation_q_y = quat_from_axis_angle(vec3_right(), cam->oriantation.y, true);
    // Quat rotation_q = quat_mul(rotation_q_x, rotation_q_y);

    // Mat4 rotation = quat_to_mat4(rotation_q);
    // Vec3 forward = mat4_forward(rotation);
    // return mat4_lookat(cam->position, vec3_add(cam->position, forward), vec3_up());


    // return mat4_lookat(cam->position, cam->target, cam->up);

    Vec3 camera_front = secamera3d_get_front(cam);
    return mat4_lookat(cam->position, vec3_add(cam->position, camera_front), cam->up);
}

/// updates the given camera's view and projection
SEINLINE void secamera3d_update_projection(SE_Camera3D *cam, i32 window_w, i32 window_h) {
    cam->view = secamera3d_get_view(cam);
    cam->projection = mat4_perspective(SEMATH_PI * 0.25f,
                                        window_w / (f32) window_h,
                                        0.1f, 1000.0f);
}

// @TODO turn this into a non-inline procedure
SEINLINE void secamera3d_input(SE_Camera3D *camera, SE_Input *seinput) {
    { // movement
        const u8 *keyboard = seinput->keyboard;
        i32 r = keyboard[SDL_SCANCODE_D] == true       ? 1 : 0;
        i32 l = keyboard[SDL_SCANCODE_A] == true       ? 1 : 0;
        i32 d = keyboard[SDL_SCANCODE_S] == true       ? 1 : 0;
        i32 u = keyboard[SDL_SCANCODE_W] == true       ? 1 : 0;
        i32 elevate = keyboard[SDL_SCANCODE_E] == true ? 1 : 0;
        i32 dive = keyboard[SDL_SCANCODE_Q] == true    ? 1 : 0;

        Vec3 input = vec3_create(r - l, d - u, elevate - dive);

        f32 camera_speed = 0.05f; // adjust accordingly

        Vec3 movement = {
            -input.x * camera_speed,
            -input.y * camera_speed,
            input.z * camera_speed,
        };

        Vec3 camera_front = secamera3d_get_front(camera);
        Vec3 camera_right = vec3_normalised(vec3_cross(camera->up, camera_front));
        Vec3 camera_up = vec3_normalised(vec3_cross(camera_front, camera_right));

        if (movement.x != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_right, movement.x));
        }
        if (movement.y != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_front, movement.y));
        }
        if (movement.z != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_up, movement.z));
        }

        // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //     cameraPos += cameraSpeed * cameraFront;
        // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //     cameraPos -= cameraSpeed * cameraFront;
        // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //     cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //     cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    { // -- rotate camera
        u8 mouse_state = SDL_GetMouseState(NULL, NULL);
        if (mouse_state & SDL_BUTTON_RMASK) {
            f32 sensitivity = 0.1f;
            f32 xoffset = seinput->mouse_screen_pos_delta.x * sensitivity;
            f32 yoffset = seinput->mouse_screen_pos_delta.y * sensitivity * -1;

            if (semath_abs(xoffset) > sensitivity) camera->yaw += xoffset;
            if (semath_abs(yoffset) > sensitivity) camera->pitch += yoffset;
            if(camera->pitch > +89.0f) camera->pitch = +89.0f;
            if(camera->pitch < -89.0f) camera->pitch = -89.0f;
        }
    }
}

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

    u32 materials_count;
    SE_Material *materials[SERENDERER3D_MAX_MATERIALS];

    SE_Camera3D *current_camera;
    SE_Light light_directional;
} SE_Renderer3D;

SEINLINE void serender3d_add_shader(SE_Renderer3D *renderer, const char *vsd, const char *fsd) {
    // add a default shader
    renderer->shaders[renderer->shaders_count] = new (SE_Shader);
    seshader_init_from(renderer->shaders[renderer->shaders_count], vsd, fsd);
    renderer->shaders_count++;
}

SEINLINE void serender3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera, const char *vsd, const char *fsd) {
    memset(renderer, 0, sizeof(SE_Renderer3D));
    renderer->current_camera = current_camera;

    serender3d_add_shader(renderer, vsd, fsd);
}

SEINLINE void serender3d_deinit(SE_Renderer3D *renderer) {
    for (u32 i = 0; i < renderer->meshes_count; ++i) {
        semesh_deinit(renderer->meshes[i]);
    }
    renderer->meshes_count = 0;

    for (u32 i = 0; i < renderer->shaders_count; ++i) {
        seshader_deinit(renderer->shaders[i]);
    }
    renderer->shaders_count = 0;

    for (u32 i = 0; i < renderer->materials_count; ++i) {
        sematerial_deinit(renderer->materials[i]);
    }
    renderer->materials_count = 0;
}

/// Load a mesh and add it to the renderer
void serender3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath);
/// Render all of the meshes the renderer contains
void serender3d_render(SE_Renderer3D *renderer);

#endif // SERENDERER_OPENGL