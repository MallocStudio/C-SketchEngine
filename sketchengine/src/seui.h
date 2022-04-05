#if 0
#ifndef SEUI_H
#define SEUI_H

#define SE_OPENGL // compatibility for other graphics APIs
#ifdef SE_OPENGL

#include "sedefines.h"
#include "semath_defines.h"
#include "serenderer_opengl.h"
#include "seinput.h"

#endif // SE_OPENGL

///
/// RENDERER
///

typedef struct UI_Vertex {
    Vec2 pos;
    RGBA colour;
} UI_Vertex;

#define UI_SHAPE_VERTEX_MAX_SIZE 64
typedef struct UI_Shape {
    u32 vertex_count;
    UI_Vertex vertices[UI_SHAPE_VERTEX_MAX_SIZE];
} UI_Shape;

#define UI_RENDERER_SHAPE_MAX_SIZE 1024
typedef struct UI_Renderer {
    u32 shape_count;
    u32 vertex_count; // calculated when data is uploaded to the GPU
    UI_Shape shapes[UI_RENDERER_SHAPE_MAX_SIZE];

    u32 vao; // vertex array object
    u32 vbo; // vertex buffer object
    u32 ibo; // index  buffer object

    SE_Shader shader; // the shader used to render the UI
    bool initialised; // is the renderer initialised

    Mat4 view_projection;
} UI_Renderer;

void seui_renderer_init(UI_Renderer *renderer, const char *vsd, const char *fsd, u32 window_w, u32 window_h);
void seui_renderer_deinit(UI_Renderer *renderer);
/// Upload the renderer's data to the GPU so we can draw it
void seui_renderer_upload(UI_Renderer *renderer);
/// Sets shape_count to zero
void seui_renderer_clear(UI_Renderer *renderer);
/// The draw call. (Remember to call seui_renderer_upload() before this procedure)
void seui_renderer_draw(UI_Renderer *renderer);
void seui_render_rect(UI_Renderer *renderer, Rect rect, RGBA colour);
// void seui_render_rect_outline(UI_Renderer *renderer, Rect rect, RGBA colour);
// void seui_render_circle(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour);
// void seui_render_circle_outline(UI_Renderer *renderer, Vec2 center, f32 radius, RGBA colour);

void seui_render_text(UI_Renderer *renderer, const char *text, Vec2 pos);
void seui_render_text_rect(UI_Renderer *renderer, const char *text, Rect rect);
void seui_render_text_colour(UI_Renderer *renderer, const char *text, Vec2 pos, RGBA colour);
void seui_render_text_ex(UI_Renderer *renderer, const char *text, Rect rect, RGBA colour);

///
/// THEME
///

typedef struct UI_Theme {
    RGBA colour_primary;
    RGBA colour_secondary;
    RGBA colour_bg;
    RGBA colour_fg;

    RGBA colour_hot;
    RGBA colour_active;
} UI_Theme;

void seui_theme_init(UI_Theme *theme);

typedef u32 UI_ID;
#define UI_ID_NULL 0

///
/// PANEL
///

/// each context is made out of multiple panels. It is the panels that hold and arrange widgets
typedef struct SEUI_Panel {
    Rect rect;
    UI_Theme *theme; // ! NOT OWNED

    Rect min_rect;
    Rect prev_item_rect;
    f32 at_x;
    f32 at_y;
    f32 at_w;
    f32 at_h;
} SEUI_Panel;

///
/// CTX CONTEXT
///

typedef struct SEUI_Context {
    SEUI_Panel *current_panel; // the panel we are adding widgets to right now
    u32 panel_count; // number of panels
    SEUI_Panel panels[10]; // a list of all panels inside of this context

    UI_Renderer renderer;
    // SE_Text_Renderer txt_renderer;
    UI_Theme theme;

    // -- input
    SE_Input *input; // ! not owned

    // -- to be compare against id (internally)
    UI_ID hot;            // the item is about to be interacted with (eg mouse over)
    UI_ID active;         // the currently active item
    UI_ID current_max_id; // UI_ID_NULL means none. At start time, this value should be UI_ID_NULL

    Rect view_rect;
} SEUI_Context;

void seui_init(SEUI_Context *ctx, Rect viewport, const char *vsd, const char *fsd, SE_Input *input);
void seui_deinit(SEUI_Context *ctx);

void seui_begin(SEUI_Context *ctx);
void seui_render(SEUI_Context *ctx);

/// begin a panel
void seui_panel_begin(SEUI_Context *ctx, Rect initial_size);
void seui_button_grab(SEUI_Context *ctx, Vec2 pos);

#endif // SEUI_H
#endif