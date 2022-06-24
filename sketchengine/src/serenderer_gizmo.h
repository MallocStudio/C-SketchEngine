#ifndef SE_RENDERER_GIZMOS
#define SE_RENDERER_GIZMOS

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

#include "seshader.h"
#include "Serender_target.h"
#include "sesprite.h"
#include "sestring.h"
#include "secamera.h"

    /// Vertex used for gizmos
typedef struct SE_Gizmo_Vertex {
    Vec3 position;
    RGBA colour;
} SE_Gizmo_Vertex;

    /// Types of gizmos shapes. Used for different rendering primitive types
typedef enum SE_GIZMO_TYPES {
    SE_GIZMO_TYPE_MESH,
    SE_GIZMO_TYPE_LINE,
    SE_GIZMO_TYPE_POINT,
    SE_GIZMO_TYPE_SPRITE
} SE_GIZMO_TYPES;

    /// The generated shape
typedef struct SE_Gizmo_Shape {
    u32 vert_count;
    u32 vao;
    u32 vbo;
    u32 ibo;
    b8 indexed;
    AABB3D aabb;
    SE_GIZMO_TYPES type;

    f32 line_width;
    f32 point_size;
} SE_Gizmo_Shape;

#define _SEGIZMO_RENDERER_MAX_SHAPES 100

    /// The renderer
typedef struct SE_Gizmo_Renderer {
        //- Shapes
    u32 shapes_count;
    SE_Gizmo_Shape shapes[_SEGIZMO_RENDERER_MAX_SHAPES];

        //- Shaders
    SE_Shader shader_mesh;  // type: MESH
    SE_Shader shader_sprite; // type: SPRITE

    SE_Camera3D *current_camera;  // ! not owned
} SE_Gizmo_Renderer;

void se_gizmo_renderer_init(SE_Gizmo_Renderer *renderer, SE_Camera3D *current_camera);
void se_gizmo_renderer_deinit(SE_Gizmo_Renderer *renderer);
void se_gizmo_render_index(SE_Gizmo_Renderer *renderer, u32 shape_index, Mat4 transform);
void se_gizmo_render(SE_Gizmo_Renderer *renderer, SE_Gizmo_Shape *shape, Mat4 transform);

//// GIZMO SHAPES WE CAN MAKE

u32 se_gizmo_add_coordniates(SE_Gizmo_Renderer *renderer);
u32 se_gizmo_add_aabb(SE_Gizmo_Renderer *renderer, Vec3 min, Vec3 max, f32 line_width);

#endif // SE_RENDERER_GIZMOS