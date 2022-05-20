#ifndef SE_TEXT_H
#define SE_TEXT_H

#include "sedefines.h"
#include "FreeType/freetype.h"
#include "GL/glew.h"
#include "seshader.h"

#define SE_TEXT_NUM_OF_GLYPHS 128
typedef struct SE_Text_Glyph {
    unsigned char character;
    i32 width;      // size of glyph
    i32 height;     // size of glyph
    i32 bearing_x;  // offset from baseline to the left of glyph
    i32 bearing_y;  // offset from baseline to the top of glyph
    u32 advance;    // offset to advance to next glyph
    Vec2 uv_min;
    Vec2 uv_max;
} SE_Text_Glyph;

#define SE_TEXT_RENDER_QUEUE_CAPACITY 1024
typedef struct SE_Text_Render_Queue {
    u32 glyph_index;
    Vec2 pos;
    Vec3 colour;
} SE_Text_Render_Queue;

typedef struct SE_Text {
    /* configs */
    bool config_centered;
    Vec3 config_colour;
    // f32  config_scale;

    /* font data */
    FT_Library library; // ft requires the library
    FT_Face face;       // then we can load fonts
    SE_Shader shader_program;
    bool initialised;

    /* rendering data */
    u32 vbo, vao;
    Mat4 shader_projection_matrix;
    u32 glyph_atlas;
    SE_Text_Glyph glyphs[SE_TEXT_NUM_OF_GLYPHS];

    u32 render_queue_size;
    SE_Text_Render_Queue render_queue[SE_TEXT_RENDER_QUEUE_CAPACITY];
} SE_Text;

/// initialise text with font and load the glyphs
bool se_init_text_default(SE_Text *text, Rect viewport);
bool se_init_text(SE_Text *text, const char *fontpath, u32 font_size, Rect viewport);
void se_deinit_text(SE_Text *text);

/// add strings to text's queue to render
void se_add_text(SE_Text *text, const char *string, Vec2 pos);
void se_add_text_rect(SE_Text *text, const char *string, Rect rect);

/// Reset the text rendering configuration (alignment) to their default values
void se_text_reset_config(SE_Text *text);

/// render the text glyphs to the screen
void se_render_text(SE_Text *text);
void se_clear_text_render_queue(SE_Text *text);
void se_set_text_viewport(SE_Text *text, Rect viewport);

/// size the given string based on the loaded font of text
Vec2 se_size_text(SE_Text *text, const char *string);

#endif // SE_TEXT_H