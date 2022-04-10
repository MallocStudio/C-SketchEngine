#ifndef SETEXT_UTIL_H
#define SETEXT_UTIL_H

#include "setext.h"
#include "freetype/freetype.h"

Rect setext_get_string_size(const char *string, FT_Face *font);

#endif // SETEXT_UTIL_H