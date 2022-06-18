#include "setext.h"
#include "sesprite.h"

static const char *vertex_shader_src ="        \n\
#version 330 core                       \n\
layout (location = 0) in vec4 vertex;   \n\
layout (location = 1) in float depth;   \n\
out vec2 TexCoords;                     \n\
                                        \n\
uniform mat4 projection;                \n\
                                        \n\
void main() {                           \n\
    gl_Position = projection * vec4(vertex.xy, depth, 1.0); \n\
    TexCoords = vertex.zw;              \n\
}";

static const char *fragment_shader_src ="      \n\
#version 330 core                       \n\
in vec2 TexCoords;                      \n\
out vec4 color;                         \n\
                                        \n\
uniform sampler2D atlas;                \n\
uniform vec3 textColor;                 \n\
                                        \n\
void main() {                           \n\
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(atlas, TexCoords).r); \n\
    color = vec4(textColor, 1.0) * sampled; \n\
}";

/* default fonts */
// #define DEFAULT_FONT_PATH "assets/fonts/Ya'ahowu/Yaahowu.ttf"
// #define DEFAULT_FONT_PATH "assets/fonts/josefin-sans-font/JosefinSansRegular-x3LYV.ttf"
#define DEFAULT_FONT_PATH "assets/fonts/Nunito/static/Nunito-Medium.ttf"

/// create the vertex buffers
static void setup_text_opengl_data(SE_Text *text) {
    // we only have one quad (6 vertices and each vertex has 4 floats to represent pos and uv)
    glGenVertexArrays(1, &text->vao);
    glGenBuffers(1, &text->vbo);
    glBindVertexArray(text->vao);
    glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Text_Vertex) * 6, NULL, GL_DYNAMIC_DRAW);

        // vertex
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Text_Vertex), (void*)offsetof(SE_Text_Vertex, vertex));
        // depth
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SE_Text_Vertex), (void*)offsetof(SE_Text_Vertex, depth));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static b8 load_glyphs_to_atlas(SE_Text *text, const char *fontpath, u32 fontsize) {
    Vec2 texture_size = {1024, 1024}; // must be a multiple of 4
    SE_Image image;
    seimage_load_empty(&image, texture_size.x, texture_size.y, 1);

    glGenTextures(1, &text->glyph_atlas);
    glBindTexture(GL_TEXTURE_2D, text->glyph_atlas);

    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* load font */
    if (FT_New_Face(text->library, fontpath, 0, &text->face)) {
        printf("ERROR:FREETYPE: Failed to load font %s\n", fontpath);
        return false;
    }
    FT_Set_Pixel_Sizes(text->face, 0, fontsize);

    Vec2 cursor = {0, 0};
    for (i32 i = 0; i < SE_TEXT_NUM_OF_GLYPHS; ++i) {
        unsigned char c = i;
        // load character glyph
        if (FT_Load_Char(text->face, c, FT_LOAD_RENDER)) {
            printf("ERROR:FREETYPE: Failed to load Glyph %c\n", c);
            return false;
        }

        i32 buffer_between_glyphs = 4; // to avoid them bleeding into each other while filtering
        Vec2 bitmap_size = {
            text->face->glyph->bitmap.width,
            text->face->glyph->bitmap.rows
        };

        // slap the bitmap on top of the image
        seimage_blit_data(&image, text->face->glyph->bitmap.buffer, bitmap_size.x, bitmap_size.y, cursor.x, cursor.y);

        // now store glyph for later use
        SE_Text_Glyph *glyph = &text->glyphs[(i32)c];

        glyph->character  = c;
        glyph->width      = text->face->glyph->bitmap.width;
        glyph->height     = text->face->glyph->bitmap.rows;
        glyph->bearing_x  = text->face->glyph->bitmap_left;
        glyph->bearing_y  = text->face->glyph->bitmap_top;
        glyph->advance    = text->face->glyph->advance.x / 64; // @check why 64?

        glyph->uv_min.x   = cursor.x;
        glyph->uv_min.y   = cursor.y;
        glyph->uv_max.x   = cursor.x + bitmap_size.x;
        glyph->uv_max.y   = cursor.y + bitmap_size.y;

        glyph->uv_min = vec2_mul_scalar(glyph->uv_min, 1 / texture_size.x);
        glyph->uv_max = vec2_mul_scalar(glyph->uv_max, 1 / texture_size.x);

        cursor.x += bitmap_size.x + buffer_between_glyphs;
        if (cursor.x >= texture_size.x - bitmap_size.x) {
            cursor.x = 0;
            cursor.y += 32; // horizontal offset // @TODO chagne to a better value
        }
    }

    /* upload the atlas texture to gpu */
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            image.width, image.height,
            0, GL_RED, GL_UNSIGNED_BYTE, image.data);

    glBindTexture(GL_TEXTURE_2D, 0);
    seimage_unload(&image);
    return true;
}

void se_set_text_viewport(SE_Text *text, Rect viewport, f32 min_depth, f32 max_depth) {
    text->viewport = viewport;
    text->shader_projection_matrix = viewport_to_ortho_projection_matrix(viewport);
    text->shader_projection_matrix = viewport_to_ortho_projection_matrix_extra(viewport, min_depth, max_depth);
}

Vec2 se_size_text(SE_Text *text, const char *string) {
    Vec2 size = {0};
    for (i32 i = 0; i < strlen(string); ++i) {
        SE_Text_Glyph glyph = text->glyphs[(i32)string[i]];
        // increase x
        // printf("width: %i, advance: %i\n", glyph.width, glyph.advance);
        i32 glyph_size = glyph.advance;
        if (glyph.width > glyph_size) glyph_size = glyph.width;
        size.x += glyph_size;
        // increase y IF a letter has a larger height
        if (glyph.height > size.y) size.y = glyph.height;
    }
    return size;
}

b8 se_init_text(SE_Text *text, const char *fontpath, u32 fontsize, Rect viewport, f32 min_depth, f32 max_depth) {
    text->initialised = false;
    if (FT_Init_FreeType(&text->library)) {
        printf("ERROR:FREETYPE: Could not init freetype library\n");
        return false;
    }

    /* shader */
    seshader_init_from_string(&text->shader_program, vertex_shader_src, fragment_shader_src, "Text Vertex Shader", "Text Fragment Shader");

    /* load glyphs to atlas */
    load_glyphs_to_atlas(text, fontpath, fontsize);

    /* projection matrix */
    se_set_text_viewport(text, viewport, min_depth, max_depth);

    /* opengl */
    setup_text_opengl_data(text);

    /* default config */
    se_text_reset_config(text);

    text->initialised = true;
    return text->initialised;
}

b8 se_init_text_default(SE_Text *text, Rect viewport, f32 min_depth, f32 max_depth) {
    return se_init_text(text, DEFAULT_FONT_PATH, 20, viewport, min_depth, max_depth);
}

void se_deinit_text(SE_Text *text) {
    if (text->initialised) {
        /* shader */
        seshader_deinit(&text->shader_program);

        /* opengl */
        glDeleteBuffers(1, &text->vbo);
        glDeleteVertexArrays(1, &text->vao);

        /* ft library */
        FT_Done_Face(text->face); // use this to free faces after using them
        FT_Done_FreeType(text->library);
    }
}

/// Render to the screen
void se_render_text(SE_Text *text) {
        // gl config
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    /* shader */
    seshader_use(&text->shader_program);
    seshader_set_uniform_mat4(&text->shader_program, "projection", text->shader_projection_matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text->vao);

    glBindTexture(GL_TEXTURE_2D, text->glyph_atlas);
    seshader_set_uniform_i32(&text->shader_program, "atlas", 0);

    glEnable(GL_SCISSOR_TEST);

    for (u32 q = 0; q < text->render_queue_size; ++q) { // go through every queue item
        SE_Text_Render_Queue queue = text->render_queue[q];
        seshader_set_uniform_vec3(&text->shader_program, "textColor", queue.colour);
        const f32 scale = 1;
        f32 x = queue.rect.x;
        f32 y = queue.rect.y;
        f32 depth = queue.depth;

        if (queue.rect.w > 0 && queue.rect.h > 0) {
            glScissor(queue.rect.x, queue.rect.y, queue.rect.w, queue.rect.h);
        } else {
            glScissor(text->viewport.x, text->viewport.y, text->viewport.w, text->viewport.h);
        }

        if (queue.centered) {
            x += (queue.rect.w - queue.string_size.x) * 0.5f;
            y += (queue.rect.h - queue.string_size.y) * 0.5f;
        } else
        if (queue.rect.h > 0) {
            y += (queue.rect.h - queue.string_size.y) * 0.5f;
        }

        for (u32 i = 0; i < queue.glyph_count; ++i) { // go through every glyph of that queue item
            SE_Text_Glyph glyph = text->glyphs[queue.glyph_indices[i]];


            float xpos = x + glyph.bearing_x * scale;
            float ypos = y - (glyph.height - glyph.bearing_y) * scale;

            float w = glyph.width  * scale;
            float h = glyph.height * scale;
            Vec2 uv_min = glyph.uv_min;
            Vec2 uv_max = glyph.uv_max;

            // update VBO for each character
            SE_Text_Vertex vertices[6];
            vertices[0].vertex = (Vec4) { xpos,     ypos + h,   uv_min.x, uv_min.y };
            vertices[1].vertex = (Vec4) { xpos,     ypos,       uv_min.x, uv_max.y };
            vertices[2].vertex = (Vec4) { xpos + w, ypos,       uv_max.x, uv_max.y };

            vertices[3].vertex = (Vec4) { xpos,     ypos + h,   uv_min.x, uv_min.y };
            vertices[4].vertex = (Vec4) { xpos + w, ypos,       uv_max.x, uv_max.y };
            vertices[5].vertex = (Vec4) { xpos + w, ypos + h,   uv_max.x, uv_min.y };

            vertices[0].depth = depth;
            vertices[1].depth = depth;
            vertices[2].depth = depth;
            vertices[3].depth = depth;
            vertices[4].depth = depth;
            vertices[5].depth = depth;

            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += glyph.advance * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
    }
        // reset gl config
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void se_text_reset_config(SE_Text *text) {
    text->config_centered = true;
    text->config_colour = v3f(1, 1, 1);
}

void se_add_text(SE_Text *text, const char *string, Vec2 pos, f32 depth) {
    se_add_text_rect(text, string, (Rect) {pos.x, pos.y, 0, 0}, depth);
}

void se_add_text_rect(SE_Text *text, const char *string, Rect rect, f32 depth) {
    SE_Text_Render_Queue queue_item;
    queue_item.glyph_count = 0;
    queue_item.rect = rect;
    queue_item.depth = depth;
    queue_item.colour = text->config_colour;
    queue_item.centered = text->config_centered;
    queue_item.string_size = se_size_text(text, string);
    u32 string_size = SDL_strlen(string);
    for (u32 i = 0; i < string_size; ++i) {
        /* save glyph to be rendered later */
        queue_item.glyph_indices[queue_item.glyph_count] = (i32)string[i];
        queue_item.glyph_count++;
    }
    text->render_queue[text->render_queue_size] = queue_item;
    text->render_queue_size++;
}

void se_clear_text_render_queue(SE_Text *text) {
    text->render_queue_size = 0;
}