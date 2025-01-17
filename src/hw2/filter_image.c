#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im) {
  float sum = 0;

  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        sum += get_pixel(im, i, j, k);
      }
    }
  }

  if (sum == 0) {
    return;
  }

  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        float v = get_pixel(im, i, j, k);
        set_pixel(im, i, j, k, v / sum);
      }
    }
  }
}

image make_box_filter(int w) {
  image im = make_image(w, w, 1);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      set_pixel(im, i, j, 0, 1);
    }
  }
  l1_normalize(im);
  return im;
}

float convolve_pixel(image im, image filter, int x, int y, int im_c, int fl_c) {
  float vi, vf, sum = 0.0;
  int x_delta = (filter.w) / 2;
  int y_delta = (filter.h) / 2;

  for (int i = -x_delta; i <= x_delta; i++) {
    for (int j = -y_delta; j <= y_delta; j++) {
      vi = get_pixel(im, x + i, y + j, im_c);
      vf = get_pixel(filter, x_delta + i, y_delta + j, fl_c);
      sum += vi * vf;
    }
  }

  return sum;
}

image convolve_image(image im, image filter, int preserve) {
  assert(filter.c == im.c || filter.c == 1);

  image convolved = make_image(im.w, im.h, preserve == 1 ? im.c : 1);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float channel_total = 0.0;
      for (int k = 0; k < im.c; k++) {
        int filter_chan = filter.c == 1 ? 0 : k;
        float val = convolve_pixel(im, filter, i, j, k, filter_chan);
        if (preserve == 1) {
          set_pixel(convolved, i, j, k, val);
        } else {
          channel_total += val;
          set_pixel(convolved, i, j, 0, channel_total);
        }
      }
    }
  }
  return convolved;
}

image make_highpass_filter() {
  image im = make_image(3, 3, 1);
  set_pixel(im, 0, 0, 0, 0);
  set_pixel(im, 1, 0, 0, -1);
  set_pixel(im, 2, 0, 0, 0);
  set_pixel(im, 0, 1, 0, -1);
  set_pixel(im, 1, 1, 0, 4);
  set_pixel(im, 2, 1, 0, -1);
  set_pixel(im, 0, 2, 0, 0);
  set_pixel(im, 1, 2, 0, -1);
  set_pixel(im, 2, 2, 0, 0);
  return im;
}

image make_sharpen_filter() {
  image im = make_image(3, 3, 1);
  set_pixel(im, 0, 0, 0, 0);
  set_pixel(im, 1, 0, 0, -1);
  set_pixel(im, 2, 0, 0, 0);
  set_pixel(im, 0, 1, 0, -1);
  set_pixel(im, 1, 1, 0, 5);
  set_pixel(im, 2, 1, 0, -1);
  set_pixel(im, 0, 2, 0, 0);
  set_pixel(im, 1, 2, 0, -1);
  set_pixel(im, 2, 2, 0, 0);
  return im;
}

image make_emboss_filter() {
  image im = make_image(3, 3, 1);
  set_pixel(im, 0, 0, 0, -2);
  set_pixel(im, 1, 0, 0, -1);
  set_pixel(im, 2, 0, 0, 0);
  set_pixel(im, 0, 1, 0, -1);
  set_pixel(im, 1, 1, 0, 1);
  set_pixel(im, 2, 1, 0, 1);
  set_pixel(im, 0, 2, 0, 0);
  set_pixel(im, 1, 2, 0, 1);
  set_pixel(im, 2, 2, 0, 2);
  return im;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our
// convolution and which ones should we not? Why?

// Answer
// Highpass: No Preserve.
// Emboss, Sharpen: Preserve.
// The highpass filter tends to 0 in areas of similarity and to 1 where values
// are changing quickly. By preserving each channel, we're getting that 0 or 1
// effect, but only on that channel's values. So unless the transition is from
// blackish to whiteish, these will not be large in all channels, leading to an
// overly dark image (except in the case of black-white transition).
//
// The others we wish to preserve the colors and apply the filter to each
// channel individually.

// Question 2.2.2: Do we have to do any post-processing for the above filters?
// Which ones and why?

// Answer
// Yes. For all filters we need to clamp values as they may have overflowed.

image make_gaussian_filter(float sigma) {
  int size = ceil(6 * sigma);
  size = size % 2 == 0 ? size + 1 : size;
  float normalizer = 1 / (TWOPI * sigma * sigma);

  image im = make_image(size, size, 1);
  for (int i = -size / 2; i <= size / 2; i++) {
    for (int j = -size / 2; j <= size / 2; j++) {
      float val = normalizer * exp(-(i * i + j * j) / (2 * sigma * sigma));
      set_pixel(im, i + size / 2, j + size / 2, 0, val);
    }
  }

  l1_normalize(im);
  return im;
}

image add_image(image a, image b) {
  assert(a.w == b.w && a.h == b.h && a.c == b.c);

  image im = make_image(a.w, a.h, a.c);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        float av = get_pixel(a, i, j, k);
        float bv = get_pixel(b, i, j, k);
        set_pixel(im, i, j, k, av + bv);
      }
    }
  }

  return im;
}

image sub_image(image a, image b) {
  assert(a.w == b.w && a.h == b.h && a.c == b.c);

  image im = make_image(a.w, a.h, a.c);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        float av = get_pixel(a, i, j, k);
        float bv = get_pixel(b, i, j, k);
        set_pixel(im, i, j, k, av - bv);
      }
    }
  }

  return im;
}

image make_gx_filter() {
  image im = make_image(3, 3, 1);
  set_pixel(im, 0, 0, 0, -1);
  set_pixel(im, 1, 0, 0, 0);
  set_pixel(im, 2, 0, 0, 1);
  set_pixel(im, 0, 1, 0, -2);
  set_pixel(im, 1, 1, 0, 0);
  set_pixel(im, 2, 1, 0, 2);
  set_pixel(im, 0, 2, 0, -1);
  set_pixel(im, 1, 2, 0, 0);
  set_pixel(im, 2, 2, 0, 1);
  return im;
}

image make_gy_filter() {
  image im = make_image(3, 3, 1);
  set_pixel(im, 0, 0, 0, -1);
  set_pixel(im, 1, 0, 0, -2);
  set_pixel(im, 2, 0, 0, -1);
  set_pixel(im, 0, 1, 0, 0);
  set_pixel(im, 1, 1, 0, 0);
  set_pixel(im, 2, 1, 0, 0);
  set_pixel(im, 0, 2, 0, 1);
  set_pixel(im, 1, 2, 0, 2);
  set_pixel(im, 2, 2, 0, 1);
  return im;
}

void feature_normalize(image im) {
  float min = get_pixel(im, 0, 0, 0);
  float max = get_pixel(im, 0, 0, 0);

  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        float val = get_pixel(im, i, j, k);
        if (val < min) {
          min = val;
        } else if (val > max) {
          max = val;
        }
      }
    }
  }

  float range = max - min;
  if (range != 0) {
    range = 1 / range;
  }

  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        float val = get_pixel(im, i, j, k);
        set_pixel(im, i, j, k, range * (val - min));
      }
    }
  }
}

image *sobel_image(image im) {
  image gx = convolve_image(im, make_gx_filter(), 0);
  image gy = convolve_image(im, make_gy_filter(), 0);

  image mag = make_image(im.w, im.h, 1);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float x = get_pixel(gx, i, j, 0);
      float y = get_pixel(gy, i, j, 0);
      set_pixel(mag, i, j, 0, sqrt(x * x + y * y));
    }
  }

  image theta = make_image(im.w, im.h, 1);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float x = get_pixel(gx, i, j, 0);
      float y = get_pixel(gy, i, j, 0);
      set_pixel(theta, i, j, 0, atan2(y, x));
    }
  }

  image *images = calloc(2, sizeof(image));
  images[0] = mag;
  images[1] = theta;
  return images;
}

image colorize_sobel(image im) {
  image *res = sobel_image(convolve_image(im, make_gaussian_filter(2), 1));
  image mag = res[0];
  image theta = res[1];
  feature_normalize(mag);
  feature_normalize(theta);

  image colorized = make_image(im.w, im.h, 3);
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float h = get_pixel(theta, i, j, 0);
      float sv = get_pixel(mag, i, j, 0);
      set_pixel(colorized, i, j, 0, h);
      set_pixel(colorized, i, j, 1, sv);
      set_pixel(colorized, i, j, 2, sv);
    }
  }

  hsv_to_rgb(colorized);
  return colorized;
}
