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
    RGBA colour;
    Vec2 pos;
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

    SE_Shader shader; // the shader used to render the UI
    bool initialised; // is the renderer initialised
} UI_Renderer;

void seui_renderer_init(UI_Renderer *renderer, const char *vsd, const char *fsd);
void seui_renderer_deinit(UI_Renderer *renderer);
/// Upload the renderer's data to the GPU so we can draw it
void seui_renderer_upload(UI_Renderer *renderer);
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
} UI_Theme;

void seui_theme_init(UI_Theme *theme);

typedef enum SEUI_CTX_STATES {
    UI_CS_NORMAL, UI_CS_MAXIMISED, UI_CS_MINIMISED,
} SEUI_CTX_STATES;

typedef u32 UI_ID;
#define UI_ID_NULL 0

/// each context is made out of multiple panels. It is the panels that hold and arrange widgets
typedef struct SEUI_Panel {
    // -- to be compare against id (internally)
    UI_ID hot;            // the item is about to be interacted with (eg mouse over)
    UI_ID active;         // the currently active item
    UI_ID current_max_id; // UI_ID_NULL means none. At start time, this value should be UI_ID_NULL

    Vec2 mouse_grab_offset;
    UI_Theme *theme;

} SEUI_Panel;

typedef struct SEUI_Context {
    SEUI_Panel *current_panel; // the panel we are adding widgets to right now
    SEUI_Panel *panels; // a list of all panels inside of this context

    UI_Renderer renderer;
    // SE_Text_Renderer txt_renderer;

    // -- input
    SE_Input *input; // ! not owned
    SEUI_CTX_STATES ctx_state;

} SEUI_Context;

void seui_init(SEUI_Context *ctx);
void seui_deinit(SEUI_Context *ctx);

/// begin a panel
void seui_panel_begin(SEUI_Context *ctx, SEUI_Panel *panel);

#endif // SEUI_H