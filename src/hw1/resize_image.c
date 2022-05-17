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

float bilinear_interpolate(image im, float x, float y, int c)
{
    // TODO
    return 0;
}

image bilinear_resize(image im, int w, int h)
{
    // TODO
    return make_image(1,1,1);
}

