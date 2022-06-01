#ifndef SEUI_RENDERER_2D
#define SEUI_RENDERER_2D

#include "sedefines.h"
#include "semath_defines.h"
#include "serenderer.h"

typedef struct SE_Vertex2D {
    Vec2 pos;
    f32 depth;
    RGBA colour;
    Vec2 uv;
} SE_Vertex2D;

typedef struct SE_Shape_Line {
    Vec2 pos1; // the start position of the line
    Vec2 pos2; // the end position of the line
    RGBA colour;
    f32 depth; // used for depth testing
    f32 width; // line width
} SE_Shape_Line;

typedef struct SE_Shape_Rect {
    Rect rect; // the position and size of the shape
    RGBA colour;
    f32 depth;
    u32 texture_id; // the opengl texture id of this rectangle
} SE_Shape_Rect;

#define SE_SHAPE_POLYGON_VERTEX_MAX_SIZE 64
typedef struct SE_Shape_Polygon {
    u32 vertex_count;
    SE_Vertex2D vertices[SE_SHAPE_POLYGON_VERTEX_MAX_SIZE];
} SE_Shape_Polygon;

#define SE_RENDERER2D_SHAPE_MAX_SIZE 1024
typedef struct SE_Renderer2D {
        /* misc */
    bool initialised;
        /* viewport */
    Mat4 view_projection; // calculated once viewport is set
    Rect viewport;        // the viewport used to render
        /* shapes */
    u32 shape_line_count;
    u32 shape_rect_count;
    u32 shape_polygon_count;
    SE_Shape_Line    shape_lines[SE_RENDERER2D_SHAPE_MAX_SIZE];
    SE_Shape_Rect    shape_rects[SE_RENDERER2D_SHAPE_MAX_SIZE];
    SE_Shape_Polygon shape_polygons[SE_RENDERER2D_SHAPE_MAX_SIZE];
        /* opengl */
    u32 vao_dynamic;
    u32 vbo_dynamic;
    SE_Shader shader;          // shader used to render untextured shapes
    SE_Shader shader_textured; // shader used to render textured shapes
} SE_Renderer2D;

///
///     RENDERER
///

void serender2d_init                   (SE_Renderer2D *renderer, Rect viewport);
void serender2d_deinit                 (SE_Renderer2D *renderer);
void serender2d_resize                 (SE_Renderer2D *renderer, Rect viewport);
void serender2d_upload_to_gpu          (SE_Renderer2D *renderer);
void serender2d_clear_shapes           (SE_Renderer2D *renderer);
void serender2d_render_uploaded_shapes (SE_Renderer2D *renderer);

void serender2d_add_rect                (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour);
void serender2d_add_line                (SE_Renderer2D *renderer, Vec2 pos1, Vec2 pos2, f32 depth, RGBA colour);
void serender2d_add_circle              (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour);
void serender2d_add_rect_textured_atlas (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA tint, const SE_Texture_Atlas *atlas, Vec2 atlas_index);

void serender2d_add_rect_outline        (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour);
void serender2d_add_line_outline        (SE_Renderer2D *renderer, Vec2 pos1, Vec2 pos2, f32 depth, RGBA colour);
void serender2d_add_circle_outline      (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour);

#endif // SEUI_RENDERER_2D