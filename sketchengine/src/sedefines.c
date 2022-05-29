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

// void rgb_to_hsv(RGB rgb, i32 *hue, f32 *saturation, f32 *value) {
//     f32 r = ((f32)rgb.r) / (f32)255; // [0 - 1]
//     f32 g = ((f32)rgb.g) / (f32)255;
//     f32 b = ((f32)rgb.b) / (f32)255;

//     f32 cmax = semath_max(r, g);
//     cmax = semath_max(cmax, b);
//     f32 cmin = semath_min(r, g);
//     cmin = semath_min(cmin, b);
//     f32 delta = cmax - cmin;

//     i32 h;
//     if (delta == 0) {
//         h = 0;
//     } else
//     if (cmax == r) {
//         h = 60.0f * ((i32)((g - b) / delta) % 6);
//     } else
//     if (cmax == g) {
//         h = 60.0f * (((b - r) / delta) + 2);
//     } else
//     if (cmax == b) {
//         h = 60.0f * (((r - g) / delta) + 4);
//     }

//     if (hue != NULL) {
//         *hue = h;
//     }

//     i32 s;
//     if (cmax == 0) {
//         s = 0;
//     } else {
//         s = (delta / cmax);// * 100;
//     }

//     if (saturation != NULL) {
//         *saturation = s;
//     }

//     if (value != NULL) {
//         *value = cmax;// * 100;
//     }
// }

void hsv_to_rgb(i32 hue, f32 saturation, f32 value, RGB *rgb) {
    if (rgb == NULL) return;

    f32 rr, gg, bb;
    f32 c = value * saturation;
    f32 h_prime = (hue / 60);
    f32 x = c * (1 - semath_abs(semath_remainder(h_prime,2) - 1));
    // f32 max = value;
    // f32 min = max - c;
    // f32 x = min + h_prime * c;
    if ( 0 <= h_prime && h_prime < 1 ) {
        rr = c;
        gg = x;
        bb = 0;
    } else
    if ( 1 <= h_prime && h_prime < 2 ) {
        rr = x;
        gg = c;
        bb = 0;
    } else
    if ( 2 <= h_prime && h_prime < 3 ) {
        rr = 0;
        gg = c;
        bb = x;
    } else
    if ( 3 <= h_prime && h_prime < 4 ) {
        rr = 0;
        gg = x;
        bb = c;
    } else
    if ( 4 <= h_prime && h_prime < 5 ) {
        rr = x;
        gg = 0;
        bb = c;
    } else
    if ( 5 <= h_prime && h_prime < 6 ) {
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

void hsv_to_rgba(i32 hue, f32 saturation, f32 value, RGBA *rgba) {
    RGB rgb;
    hsv_to_rgb(hue, saturation, value, &rgb);
    rgba->r = rgb.r;
    rgba->g = rgb.g;
    rgba->b = rgb.b;
}