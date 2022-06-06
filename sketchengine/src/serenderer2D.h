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
} SE_Shape_Rect;

typedef struct SE_Shape_Textured_Rect {
    SE_Shape_Rect rect_shape;
    u32 texture_id;
    Vec2 uv_min;
    Vec2 uv_max;
} SE_Shape_Textured_Rect;

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
    u32                    shape_line_count;
    u32                    shape_rect_count;
    u32                    shape_textured_rect_count;
    u32                    shape_polygon_count;
    SE_Shape_Line          shape_lines[SE_RENDERER2D_SHAPE_MAX_SIZE];
    SE_Shape_Rect          shape_rects[SE_RENDERER2D_SHAPE_MAX_SIZE];
    SE_Shape_Textured_Rect shape_textured_rects[SE_RENDERER2D_SHAPE_MAX_SIZE];
    SE_Shape_Polygon       shape_polygons[SE_RENDERER2D_SHAPE_MAX_SIZE];
        /* opengl */
    u32 vao_dynamic;
    u32 vbo_dynamic;
    SE_Shader shader;          // shader used to render untextured shapes
    SE_Shader shader_textured; // shader used to render textured shapes
} SE_Renderer2D;

///
///     RENDERER
///

void serender2d_init                    (SE_Renderer2D *renderer, Rect viewport, f32 min_depth, f32 max_depth);
void serender2d_deinit                  (SE_Renderer2D *renderer);
void serender2d_resize                  (SE_Renderer2D *renderer, Rect viewport, f32 min_depth, f32 max_depth);
void serender2d_upload_to_gpu           (SE_Renderer2D *renderer);
void serender2d_clear_shapes            (SE_Renderer2D *renderer);
void serender2d_render                  (SE_Renderer2D *renderer);

void serender2d_add_rect                (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour);
void serender2d_add_circle              (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour);
void serender2d_add_rect_textured_atlas (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA tint, const SE_Texture_Atlas *atlas, Vec2 atlas_index);
void serender2d_add_rect_textured       (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA tint, i32 opengl_texture_id);

void serender2d_add_line                (SE_Renderer2D *renderer, Vec2 pos1, Vec2 pos2, f32 depth, RGBA colour, f32 width);
void serender2d_add_rect_outline        (SE_Renderer2D *renderer, Rect rect, f32 depth, RGBA colour, f32 width);
void serender2d_add_circle_outline      (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, u32 segment_count, RGBA colour, f32 width);

void serender2d_add_hsv_wheel           (SE_Renderer2D *renderer, Vec2 center, f32 inner_radius, f32 width, f32 depth);
void serender2d_add_hsv_triangle        (SE_Renderer2D *renderer, Vec2 center, f32 radius, f32 depth, f32 angle);

#endif // SEUI_RENDERER_2D