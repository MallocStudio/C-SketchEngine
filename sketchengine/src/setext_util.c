#include "setext_util.h"

Vec2 setext_get_string_size(const char *string, SE_Text_Renderer *txt) {
    Vec2 result = {0};
    u32 string_size = strlen(string);
    u32 max_height = 0;
    u32 cursor_w = 0;
    for (u32 i = 0; i < string_size; ++i) {
        u32 width = txt->characters[(u32)string[i]].advance;
        cursor_w += width;
        u32 height = txt->characters[(u32)string[i]].height;
        if (height > max_height) {
            max_height = height;
        }
    }
    result.x = cursor_w;
    result.y = max_height;
    return result;
}