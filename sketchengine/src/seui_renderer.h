#ifndef SEUI_RENDERER_H
#define SEUI_RENDERER_H

/* meant to be used in seui.c only */

#include "sedefines.h"
#include "semath_defines.h"
#include "serenderer.h"

///
/// RENDERER
///

typedef struct UI_Vertex {
    Vec2 pos;        // vertex position
    RGBA colour;     // vertex colour
    Vec2 texture_uv; // the uv of the texture atlas
} UI_Vertex;

#define UI_SHAPE_VERTEX_MAX_SIZE 64
#define UI_SHAPE_INDEX_MAX_SIZE 66
typedef struct UI_Shape {
    u32 vertex_count;
    UI_Vertex vertices[UI_SHAPE_VERTEX_MAX_SIZE];
    u32 index_count;
    u32 indices[UI_SHAPE_INDEX_MAX_SIZE];
} UI_Shape;

typedef struct UI_Shape_Textured{
    Rect rect;
    u32 opengl_texture;
} UI_Shape_Textured;

#define UI_RENDERER_SHAPE_MAX_SIZE 1024
#define UI_RENDERER_SHAPE_TEXTURED_MAX_SIZE 10
#define UI_ICON_INDEX_NULL       (Vec2) {0, 0}
#define UI_ICON_INDEX_COLLAPSE   (Vec2) {1, 0}
#define UI_ICON_INDEX_UNCOLLAPSE (Vec2) {2, 0}
#define UI_ICON_INDEX_CLOSE      (Vec2) {0, 1}
#define UI_ICON_INDEX_SLIDER     (Vec2) {3, 0}
#define UI_ICON_INDEX_CIRCLE_FILLED (Vec2) {1, 1}
#define UI_ICON_INDEX_CIRCLE_EMPTY (Vec2) {1, 2}
#define UI_ICON_INDEX_CHECKBOX_FILLED (Vec2) {3, 2}
#define UI_ICON_INDEX_CHECKBOX_EMPTY (Vec2) {2, 2}
#define UI_ICON_INDEX_ARROW_RIGHT (Vec2) {2, 1}
#define UI_ICON_INDEX_ARROW_LEFT (Vec2) {3, 1}

typedef struct UI_Renderer {
    u32 vertex_count; // calculated when data is uploaded to the GPU
    u32 index_count; // calculated when data is uploaded to the GPU
    u32 vertex_count_lines; // calculated when data is uploaded to the GPU
    u32 index_count_lines; // calculated when data is uploaded to the GPU
    u32 shape_count;
    UI_Shape shapes[UI_RENDERER_SHAPE_MAX_SIZE];
    u32 shape_textured_count;
    UI_Shape_Textured shapes_textured[UI_RENDERER_SHAPE_TEXTURED_MAX_SIZE];

    u32 vao; // vertex array object
    u32 vbo; // vertex buffer object
    u32 ibo; // index  buffer object

    u32 vao_dynamic; // vertex array object
    u32 vbo_dynamic; // vertex buffer object

    u32 vao_lines;
    u32 vbo_lines;
    u32 ibo_lines;

    SE_Shader shader; // the shader used to render the UI
    SE_Shader shader_texture; // the shader used to render textured ui elements
    bool initialised; // is the renderer initialised

    SE_Texture_Atlas icons;

    Mat4 view_projection;
    f32 view_width;
    f32 view_height;
} UI_Renderer;

void seui_renderer_init(UI_Renderer *renderer, u32 window_w, u32 window_h);
void seui_renderer_resize(UI_Renderer *renderer, u32 window_w, u32 window_h);
void seui_renderer_deinit(UI_Renderer *renderer);
/// Upload the renderer's data to the GPU so we can draw it
void seui_renderer_upload(UI_Renderer *renderer);
/// Sets shape_count to zero
void seui_renderer_clear(UI_Renderer *renderer);
/// The draw call. (Remember to call seui_renderer_upload() before this procedure)
void seui_renderer_draw(UI_Renderer *renderer);

///
/// shapes:
///
void seui_render_shape_colour_wheel(UI_Renderer *renderer, Vec2 center, f32 outer_radius, f32 width);
void seui_render_shape_colour_triangle(UI_Renderer *renderer, Vec2 center, f32 radius, f32 angle);
/// render a filled rectangle
void seui_render_rect(UI_Renderer *renderer, Rect rect, RGBA colour);
/// indexes into the texture atlas and creates vertices with proper UVs
void seui_render_texture(UI_Renderer *renderer, Rect rect, Vec2 index, RGBA tint);
void seui_render_texture_raw(UI_Renderer *renderer, Rect rect, u32 opengl_texture);
/// render a line
void seui_render_line(UI_Renderer *renderer, Vec2 pos1, Vec2 pos2, f32 width, RGBA colour);
void seui_render_circle(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour);
void seui_render_circle_outline(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour);
void seui_render_circle_outline_ext(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour, u32 num_of_edges);
void seui_render_rect_outline(UI_Renderer *renderer, Rect rect, f32 width, RGBA colour);
/// colour is the most saturated and bright colour the box will display
void seui_render_colour_box(UI_Renderer *renderer, Rect rect, i32 hue); // rename to seui_render_hsv_rect

// void seui_render_circle_outline(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour);

// void seui_render_text(UI_Renderer *renderer, const char *text, Vec2 pos);
// void seui_render_text_rect(UI_Renderer *renderer, const char *text, Rect rect);
// void seui_render_text_colour(UI_Renderer *renderer, const char *text, Vec2 pos, RGBA colour);
// void seui_render_text_ex(UI_Renderer *renderer, const char *text, Rect rect, RGBA colour);

#endif // SEUI_RENDERER_H