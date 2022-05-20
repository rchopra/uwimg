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
  // TODO
  return make_image(1, 1, 1);
}

image add_image(image a, image b) {
  // TODO
  return make_image(1, 1, 1);
}

image sub_image(image a, image b) {
  // TODO
  return make_image(1, 1, 1);
}

image make_gx_filter() {
  // TODO
  return make_image(1, 1, 1);
}

image make_gy_filter() {
  // TODO
  return make_image(1, 1, 1);
}

void feature_normalize(image im) {
  // TODO
}

image *sobel_image(image im) {
  // TODO
  return calloc(2, sizeof(image));
}

image colorize_sobel(image im) {
  // TODO
  return make_image(1, 1, 1);
}
