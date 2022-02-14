#include "setext.h"

/// ----------------------
/// HASHMAP AND CHARACTERS : https://github.com/tidwall/hashmap.c
/// ----------------------

i32 character_compare(const void *a, const void *b, void *udata) {
    const SE_Text_Character *ca = a;
    const SE_Text_Character *cb = b;
    return ca->texture_id - cb->texture_id; // @check
}

bool character_iter(const void *item, void *udata) {
    const SE_Text_Character *citem = item;
    printf("---\ntexture_id: %i; width: %i; height: %i; advance: %i\n---\n",
        citem->texture_id, citem->width, citem->height, citem->advance);
    return true;
}

u64 character_hash(const void *item, u64 seed0, u64 seed1) {
    const SE_Text_Character *citem = item;
    return hashmap_sip(citem->texture_id, sizeof(citem->texture_id), seed0, seed1);
}

/// -------
/// SE_Text
/// -------

SE_Text* setext_init() {
    SE_Text *txt = new(SE_Text);
    if (FT_Init_FreeType(&txt->library)) {
        printf("ERROR:FREETYPE: Could not init freetype library\n");
        return NULL;
    }
    // create the characters hashmap
    txt->characters = hashmap_new(sizeof(SE_Text_Character), NUM_OF_GLYPHS, 0, 0, character_hash, character_compare, NULL, NULL);
    setext_load_font(txt, DEFAULT_FONT_PATH, 0, 48);

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
    return txt;
}

void setext_deinit(SE_Text *txt) {
    if (txt->initialised) {
        glDeleteBuffers(1, txt->VBO);
        glDeleteVertexArrays(1, txt->VAO);
        
        FT_Done_Face(txt->face); // use this to free faces after using them
        FT_Done_FreeType(txt->library);
        hashmap_free(txt->characters);

        txt->initialised = false;
    }
}

i32 setext_load_font(SE_Text *txt, const char *font_path, i32 width, i32 height) {
    // -- load font
    if (FT_New_Face(txt->library, font_path, 0, &txt->face)) {
        printf("ERROR:FREETYPE: Failed to load font %s\n", font_path);
        return SETEXT_ERROR;
    }
    FT_Set_Pixel_Sizes(txt->face, width, height);

    // -- update txt->characters
    hashmap_clear(txt->characters, NUM_OF_GLYPHS);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    for (unsigned char c = 0; c < NUM_OF_GLYPHS; ++c) {
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
            texture,
            txt->face->glyph->bitmap.width,
            txt->face->glyph->bitmap.rows,
            txt->face->glyph->bitmap_left,
            txt->face->glyph->bitmap_top,
            txt->face->glyph->advance.x
        };
        hashmap_set(txt->characters, &character); // a copy of the data is stored in the hashmap, so it getting freed at the end of this scope is not an issue
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // set the alignment back to default (4)
    return SETEXT_SUCCESS;
}

i32 setext_render_text(SE_Text *txt, const char *string, i32 x, i32 y) {
    glEnable(GL_BLEND); // @check // @question // @incomplete maybe move this out to the main loop? ask finn if it's a good idea to have this here or not
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // ------------------------------------------------------------------ > left off here
    glDisable(GL_BLEND);
    return SETEXT_SUCCESS;
}

i32 setext_print_loaded_characters(SE_Text *txt) {
    hashmap_scan(txt->characters, character_iter, NULL);
    return SETEXT_SUCCESS;
}