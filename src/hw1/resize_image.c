#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c) {
    return get_pixel(im, round(x), round(y), c);
}

image nn_resize(image im, int w, int h) {
    float aw = (float)im.w / w;
    float ah = (float)im.h / h;
    float bw = 0.5 * (aw - 1);
    float bh = 0.5 * (ah - 1);

    image img = make_image(w, h, im.c);
    for (int i = 0; i < w; i++) {
      for (int j = 0; j < h; j++) {
        for (int k = 0; k < im.c; k++) {
          float val = nn_interpolate(im, aw * i + bw, ah * j + bh, k);
          set_pixel(img, i, j, k, val);
        }
      }
    }
    return img;
}

float bilinear_interpolate(image im, float x, float y, int c) {
    float x_floor = floor(x);
    float y_floor = floor(y);
    float x_ceil = ceil(x);
    float y_ceil = ceil(y);

    float d1 = x - x_floor;
    float d2 = x_ceil - x;
    float d3 = y - y_floor;
    float d4 = y_ceil - y;

    float v1 = get_pixel(im, x_floor, y_floor, c);
    float v2 = get_pixel(im, x_ceil, y_floor, c);
    float v3 = get_pixel(im, x_floor, y_ceil, c);
    float v4 = get_pixel(im, x_ceil, y_ceil, c);

    float q1 = v1 * d2 + v2 * d1;
    float q2 = v3 * d2 + v4 * d1;
    float q = q1 * d4 + q2 * d3;

    return q;
}

image bilinear_resize(image im, int w, int h) {
    float aw = (float)im.w / w;
    float ah = (float)im.h / h;
    float bw = 0.5 * (aw - 1);
    float bh = 0.5 * (ah - 1);

    image img = make_image(w, h, im.c);
    for (int i = 0; i < w; i++) {
      for (int j = 0; j < h; j++) {
        for (int k = 0; k < im.c; k++) {
          float val = bilinear_interpolate(im, aw * i + bw, ah * j + bh, k);
          set_pixel(img, i, j, k, val);
        }
      }
    }
    return img;
}

