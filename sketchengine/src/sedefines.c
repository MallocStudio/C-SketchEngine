#include "sedefines.h"
#include "semath.h"
#include <stdio.h> // ! required for printf

void print_sdl_error() {
    const char *error = SDL_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}

void rgb_to_hsv(RGB rgb, i32 *hue, i32 *saturation, i32 *value) {
    f32 r = ((f32)rgb.r) / (f32)255;
    f32 g = ((f32)rgb.g) / (f32)255;
    f32 b = ((f32)rgb.b) / (f32)255;

    f32 cmax = semath_max(r, g);
    cmax = semath_max(cmax, b);
    f32 cmin = semath_min(r, g);
    cmin = semath_min(cmin, b);
    f32 delta = cmax - cmin;

    i32 h;
    if (delta == 0) {
        h = 0;
    } else
    if (cmax == r) {
        h = 60.0f * ((i32)((g - b) / delta) % 6);
    } else
    if (cmax == g) {
        h = 60.0f * (((b - r) / delta) + 2);
    } else
    if (cmax == b) {
        h = 60.0f * (((r - g) / delta) + 4);
    }

    if (hue != NULL) {
        *hue = h;
    }

    i32 s;
    if (cmax == 0) {
        s = 0;
    } else {
        s = (delta / cmax) * 100;
    }

    if (saturation != NULL) {
        *saturation = s;
    }

    if (value != NULL) {
        *value = cmax * 100;
    }
}

void hsv_to_rgb(i32 hue, i32 saturation, i32 value, RGB *rgb) {
    if (rgb == NULL) return;
    ubyte rr, r, gg, g, bb, b;
    f32 c = value * saturation;
    f32 x = c * (1 - semath_abs((hue / 60) % 2 - 1));
    f32 m = value - c;
    if ( 0 <= hue && hue < 60 ) {
        rr = c;
        gg = x;
        bb = 0;
    } else
    if ( 60 <= hue && hue < 120 ) {
        rr = x;
        gg = c;
        bb = 0;
    } else
    if ( 120 <= hue && hue < 180 ) {
        rr = 0;
        gg = c;
        bb = x;
    } else
    if ( 180 <= hue && hue < 240 ) {
        rr = 0;
        gg = x;
        bb = c;
    } else
    if ( 240 <= hue && hue < 300 ) {
        rr = x;
        gg = 0;
        bb = c;
    } else
    if ( 300 <= hue && hue < 360 ) {
        rr = c;
        gg = 0;
        bb = x;
    }

    r = (rr + m) * 255;
    g = (gg + m) * 255;
    b = (bb + m) * 255;

    rgb->r = r;
    rgb->g = g;
    rgb->b = b;
}

void hsv_to_rgba(i32 hue, i32 saturation, i32 value, RGBA *rgba) {
    RGB rgb;
    hsv_to_rgb(hue, saturation, value, &rgb);
    rgba->r = rgb.r;
    rgba->g = rgb.g;
    rgba->b = rgb.b;
}