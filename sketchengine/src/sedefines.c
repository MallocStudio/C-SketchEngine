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

void rgb_to_hsv(RGB rgb, i32 *hue, f32 *saturation, f32 *value) {
    f32 r = ((f32)rgb.r) / (f32)255; // [0 - 1]
    f32 g = ((f32)rgb.g) / (f32)255;
    f32 b = ((f32)rgb.b) / (f32)255;

    f32 cmax = se_math_max(r, g);
    cmax = se_math_max(cmax, b);
    f32 cmin = se_math_min(r, g);
    cmin = se_math_min(cmin, b);
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
        s = (delta / cmax);// * 100;
    }

    if (saturation != NULL) {
        *saturation = s;
    }

    if (value != NULL) {
        *value = cmax;// * 100;
    }
}

void hsv_to_rgb(i32 hue, f32 saturation, f32 value, RGB *rgb) {
    if (rgb == NULL) return;

    f32 rr, gg, bb;
    f32 c = value * saturation;
    f32 h_prime = (hue / (f32)60.0f);
    // f32 x = c * (1 - se_math_abs(se_math_remainder(h_prime,2) - 1));
    f32 x = c * (1 - se_math_abs((i32)h_prime % 2 - 1));
    // f32 max = value;
    // f32 min = max - c;
    // f32 x = min + h_prime * c;
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
    } else {
        rr = 0; // this should never happen, if it does it means we're not clamping hsv's h value somewhere that's calling this procedure
        gg = 0; // this should never happen, if it does it means we're not clamping hsv's h value somewhere that's calling this procedure
        bb = 0; // this should never happen, if it does it means we're not clamping hsv's h value somewhere that's calling this procedure
    }

    f32 m = value - c;
    rgb->r = (rr + m) * 255;
    rgb->g = (gg + m) * 255;
    rgb->b = (bb + m) * 255;

    // f32 max = value;
    // f32 c = saturation * value;
    // f32 min = max - c;
    // f32 h_prime;
    // i32 r, g, b;
    // if (hue >= 300) {
    //     h_prime = (hue - 360) / 60;
    // } else {
    //     h_prime = hue / 60;
    // }

    // if (h_prime >= -1 && h_prime < 1) {
    //     if (h_prime < 0) {
    //         r = max;
    //         g = min;
    //         b = min - h_prime * c;
    //     } else {
    //         r = max;
    //         g = min + h_prime * c;
    //         b = min;
    //     }
    // } else
    // if (h_prime >= 1 && h_prime < 3) {
    //     if (h_prime - 2 < 0) {
    //         r = min - (h_prime - 2) * c;
    //         g = max;
    //         b = min;
    //     } else {
    //         r = min;
    //         g = max;
    //         b = min + (h_prime - 2) * c;
    //     }
    // } else
    // if (h_prime >= 3 && h_prime < 5) {
    //     if (h_prime - 4 < 0) {
    //         r = min;
    //         g = min - (h_prime - 4) * c;
    //         b = max;
    //     } else {
    //         r = min + (h_prime - 4) * c;
    //         g = min;
    //         b = max;
    //     }
    // }

    // rgb->r = r * 255;
    // rgb->g = g * 255;
    // rgb->b = b * 255;
}

RGBA se_rgba_brighten(RGBA rgba) {
    RGBA result = rgba;
    result.r += 10;
    result.g += 10;
    result.b += 10;
    if (result.r < rgba.r) result.r = 255;
    if (result.g < rgba.g) result.g = 255;
    if (result.b < rgba.b) result.b = 255;
    return result;
}

void hsv_to_rgba(i32 hue, f32 saturation, f32 value, RGBA *rgba) {
    RGB rgb;
    hsv_to_rgb(hue, saturation, value, &rgb);
    rgba->r = rgb.r;
    rgba->g = rgb.g;
    rgba->b = rgb.b;
}

void se_grid_init(SE_Grid *grid, u32 width, u32 height) {
    grid->w = width;
    grid->h = height;
    grid->value = malloc(sizeof(u32) * grid->w * grid->h);
    memset(grid->value, 0, sizeof(u32) * grid->w * grid->h);
}

void se_grid_deinit(SE_Grid *grid) {
    grid->w = 0;
    grid->h = 0;
    free(grid->value);
}

void se_grid_set(SE_Grid *grid, u32 x, u32 y, u32 value) {
    u32 index = y * grid->w + x;
    if (value < 0) value = 0;
    if (value > SE_GRID_MAX_VALUE) value = SE_GRID_MAX_VALUE;
    grid->value[index] = value;
}

u32 se_grid_get(const SE_Grid *grid, u32 x, u32 y) {
    u32 index = y * grid->w + x;
    if (index >= grid->h * grid->w) return 0;
    return grid->value[index];
}