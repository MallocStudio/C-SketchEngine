#include "setext.h"

/// -------
/// SE_Text_Renderer
/// -------

i32 setext_init (SE_Text_Renderer *txt, Rect viewport) {
    return setext_init_from(txt, viewport, DEFAULT_FONT_PATH, 0, 16);
}

i32 setext_init_from (SE_Text_Renderer *txt, Rect viewport, const char *font_path, i32 width, i32 height) {
    if (FT_Init_FreeType(&txt->library)) {
        printf("ERROR:FREETYPE: Could not init freetype library\n");
        return SETEXT_ERROR;
    }

    // strings initialisation
    txt->strings_count = 0;

    // -- shader program
    txt->shader_program = new(SEGL_Shader_Program);
    segl_shader_program_init_from(txt->shader_program, "Text.vsd", "Text.fsd");

    setext_load_font(txt, font_path, width, height);

    // -- projection matrix
    setext_set_viewport(txt, viewport);

    // -- setup openGL part of the rendering
    glGenVertexArrays(1, &txt->VAO);
    glGenBuffers(1, &txt->VBO);

    glBindVertexArray(txt->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, txt->VBO);

    // * the 2D quad requires 5 vertices of 4 floats each
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    txt->initialised = true;
    return SETEXT_SUCCESS;
}

void setext_deinit(SE_Text_Renderer *txt) {
    if (txt->initialised) { // @debug when we try to exit, we throw a "access violation reading location exception" we probabily set txt->initialised to true when we shouldn't in someplace.
        txt->strings_count = 0;

        glDeleteBuffers(1, &txt->VBO);
        glDeleteVertexArrays(1, &txt->VAO);
        
        FT_Done_Face(txt->face); // use this to free faces after using them
        FT_Done_FreeType(txt->library);

        txt->initialised = false;

        segl_shader_program_deinit(txt->shader_program);
        free(txt->shader_program);
    }
}

void setext_set_viewport(SE_Text_Renderer *txt, Rect viewport) {
    txt->shader_projection_matrix = viewport_to_ortho_projection_matrix(viewport);
}

i32 setext_load_font(SE_Text_Renderer *txt, const char *font_path, i32 width, i32 height) {
    // -- load font
    if (FT_New_Face(txt->library, font_path, 0, &txt->face)) {
        printf("ERROR:FREETYPE: Failed to load font %s\n", font_path);
        return SETEXT_ERROR;
    }
    FT_Set_Pixel_Sizes(txt->face, width, height);

    // -- update txt->characters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (i32 i = 0; i < SE_TEXT_NUM_OF_GLYPHS; ++i) {
        unsigned char c = i;
        // load character glyph
        if (FT_Load_Char(txt->face, c, FT_LOAD_RENDER)) {
            printf("ERROR:FREETYPE: Failed to load Glyph %c\n", c);
            return SETEXT_ERROR;
            // continue;
        }
        // generate texture
        u32 texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture); // select the texture
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            txt->face->glyph->bitmap.width, txt->face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE, txt->face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        SE_Text_Character character = {
            c,
            texture,
            txt->face->glyph->bitmap.width,
            txt->face->glyph->bitmap.rows,
            txt->face->glyph->bitmap_left,
            txt->face->glyph->bitmap_top,
            txt->face->glyph->advance.x / 64, // @check why 64?
        };
        txt->characters[(i32)c] = character;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // set the alignment back to default (4)
    return SETEXT_SUCCESS;
}

i32 setext_render_text(SE_Text_Renderer *txt, const char *string, f32 x, f32 y, f32 scale, Vec3 color) {
    // -- loop through all the characters and generate the glyphs
    for (i32 i = 0; i < strlen(string); ++i) {
        SE_Text_Character character = txt->characters[(i32)string[i]];
        f32 xpos = x + character.bearing_x * scale;
        f32 ypos = y - (character.height - character.bearing_y) * scale;
        f32 w = character.width * scale;
        f32 h = character.height * scale;

        // -- generate the vertices and
        // -- store the data
        // txt->strings[txt->strings_count].vertices = {
        //     { xpos,     ypos + h,   0.0f, 0.0f },    // 0
        //     { xpos,     ypos,       0.0f, 1.0f },    // 1
        //     { xpos + w, ypos,       1.0f, 1.0f },    // 2

        //     { xpos,     ypos + h,   0.0f, 0.0f },    // 3
        //     { xpos + w, ypos,       1.0f, 1.0f },    // 4
        //     { xpos + w, ypos + h,   1.0f, 0.0f }     // 5
        // };
        SE_Text_Stored_Letter *txt_string = &txt->generated_letters[txt->strings_count];
        txt_string->vertices[0][0] = xpos;
        txt_string->vertices[0][1] = ypos + h;
        txt_string->vertices[0][2] = 0.0f;
        txt_string->vertices[0][3] = 0.0f;

        txt_string->vertices[1][0] = xpos;
        txt_string->vertices[1][1] = ypos;
        txt_string->vertices[1][2] = 0.0f;
        txt_string->vertices[1][3] = 1.0f;

        txt_string->vertices[2][0] = xpos + w;
        txt_string->vertices[2][1] = ypos;
        txt_string->vertices[2][2] = 1.0f;
        txt_string->vertices[2][3] = 1.0f;

        txt_string->vertices[3][0] = xpos;
        txt_string->vertices[3][1] = ypos + h;
        txt_string->vertices[3][2] = 0.0f;
        txt_string->vertices[3][3] = 0.0f;

        txt_string->vertices[4][0] = xpos + w;
        txt_string->vertices[4][1] = ypos;
        txt_string->vertices[4][2] = 1.0f;
        txt_string->vertices[4][3] = 1.0f;

        txt_string->vertices[5][0] = xpos + w;
        txt_string->vertices[5][1] = ypos + h;
        txt_string->vertices[5][2] = 1.0f;
        txt_string->vertices[5][3] = 0.0f;

        txt_string->x = x;
        txt_string->y = y;
        txt_string->scale = scale;
        txt_string->color = color;

        txt_string->texture_ids = character.texture_id;

        // advance cursor for next glyph
        x += character.advance;

        // -- advance letter
        txt->strings_count++;
    }
    return SETEXT_SUCCESS;
}

i32 setext_render(SE_Text_Renderer *txt) {
    glEnable(GL_BLEND); // @check // @question // @incomplete maybe move this out to the main loop? ask finn if it's a good idea to have this here or not
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    segl_shader_program_set_uniform_mat4(txt->shader_program, "projection", txt->shader_projection_matrix);

    // -- loop through all the strings
    for (i32 i = 0; i < txt->strings_count; ++i) {
        f32 vertices[6][4];
        vertices[0][0] = txt->generated_letters[i].vertices[0][0];
        vertices[0][1] = txt->generated_letters[i].vertices[0][1];
        vertices[0][2] = txt->generated_letters[i].vertices[0][2];
        vertices[0][3] = txt->generated_letters[i].vertices[0][3];

        vertices[1][0] = txt->generated_letters[i].vertices[1][0];
        vertices[1][1] = txt->generated_letters[i].vertices[1][1];
        vertices[1][2] = txt->generated_letters[i].vertices[1][2];
        vertices[1][3] = txt->generated_letters[i].vertices[1][3];
        
        vertices[2][0] = txt->generated_letters[i].vertices[2][0];
        vertices[2][1] = txt->generated_letters[i].vertices[2][1];
        vertices[2][2] = txt->generated_letters[i].vertices[2][2];
        vertices[2][3] = txt->generated_letters[i].vertices[2][3];
        
        vertices[3][0] = txt->generated_letters[i].vertices[3][0];
        vertices[3][1] = txt->generated_letters[i].vertices[3][1];
        vertices[3][2] = txt->generated_letters[i].vertices[3][2];
        vertices[3][3] = txt->generated_letters[i].vertices[3][3];
        
        vertices[4][0] = txt->generated_letters[i].vertices[4][0];
        vertices[4][1] = txt->generated_letters[i].vertices[4][1];
        vertices[4][2] = txt->generated_letters[i].vertices[4][2];
        vertices[4][3] = txt->generated_letters[i].vertices[4][3];
        
        vertices[5][0] = txt->generated_letters[i].vertices[5][0];
        vertices[5][1] = txt->generated_letters[i].vertices[5][1];
        vertices[5][2] = txt->generated_letters[i].vertices[5][2];
        vertices[5][3] = txt->generated_letters[i].vertices[5][3];

        Vec3 color = txt->generated_letters[i].color;

        segl_shader_program_use_shader(txt->shader_program);
        segl_shader_program_set_uniform_vec3(txt->shader_program, "textColor", color);

        
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(txt->VAO);
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, txt->generated_letters[i].texture_ids);
        // update content of VBO mem
        glBindBuffer(GL_ARRAY_BUFFER, txt->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glDisable(GL_BLEND);

    // -- "clear" the strings array
    txt->strings_count = 0;

    return SETEXT_SUCCESS;
}

i32 setext_print_loaded_characters(SE_Text_Renderer *txt) {
    for (i32 i = 0; i < SE_TEXT_NUM_OF_GLYPHS; ++i) {
        SE_Text_Character character = txt->characters[i];
        printf("character %c : texture id %i : width %i : height %i : advance %i\n", character.character, character.texture_id, character.width, character.height, character.advance);
    }
    return SETEXT_SUCCESS;
}

Vec2 setext_size_string(SE_Text_Renderer *txt, const char *string) {
    Vec2 size = {0};
    for (i32 i = 0; i < strlen(string); ++i) {
        SE_Text_Character character = txt->characters[(i32)string[i]];
        // increase x
        size.x += character.advance;
        // increase y IF a letter has a larger height
        if (character.height > size.y) size.y = character.height;
    }
    return size;
}