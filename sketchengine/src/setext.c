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
    printf("---\ntexture_id: %i; size_x: %i; size_y: %i; advance: %i\n---\n",
        citem->texture_id, citem->size_x, citem->size_y, citem->advance);
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
    txt->characters = hashmap_new(sizeof(SE_Text_Character), 0, 0, 0, character_hash, character_compare, NULL, NULL);
    return txt;
}

void setext_deinit(SE_Text *txt) {
    // * FT_Done_Face() use this to free faces after using them
    FT_Done_FreeType(txt->library);
    hashmap_free(txt->characters);
}

i32 setext_load_font(SE_Text *txt, i32 width, i32 height) {
    if (FT_New_Face(txt->library, DEFAULT_FONT_PATH, 0, &txt->face)) {
        printf("ERROR:FREETYPE: Failed to load font %s\n", DEFAULT_FONT_PATH);
        return SETEXT_ERROR;
    }
    FT_Set_Pixel_Sizes(txt->face, width, height);
    return SETEXT_SUCCESS;
}

i32 setext_print_loaded_characters(SE_Text *txt) {
    hashmap_scan(txt->characters, character_iter, NULL);
    return SETEXT_SUCCESS;
}