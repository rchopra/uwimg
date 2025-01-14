#include "image.h"
#include "matrix.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SUPPRESSED_VALUE -999999

// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n) {
  int i;
  for (i = 0; i < n; ++i) {
    free(d[i].data);
  }
  free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i) {
  int w = 5;
  descriptor d;
  d.p.x = i % im.w;
  d.p.y = i / im.w;
  d.data = calloc(w * w * im.c, sizeof(float));
  d.n = w * w * im.c;
  int c, dx, dy;
  int count = 0;
  // If you want you can experiment with other descriptors
  // This subtracts the central value from neighbors
  // to compensate some for exposure/lighting changes.
  for (c = 0; c < im.c; ++c) {
    float cval = im.data[c * im.w * im.h + i];
    for (dx = -w / 2; dx < (w + 1) / 2; ++dx) {
      for (dy = -w / 2; dy < (w + 1) / 2; ++dy) {
        float val = get_pixel(im, i % im.w + dx, i / im.w + dy, c);
        d.data[count++] = cval - val;
      }
    }
  }
  return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p) {
  int x = p.x;
  int y = p.y;
  int i;
  for (i = -9; i < 10; ++i) {
    set_pixel(im, x + i, y, 0, 1);
    set_pixel(im, x, y + i, 0, 1);
    set_pixel(im, x + i, y, 1, 0);
    set_pixel(im, x, y + i, 1, 0);
    set_pixel(im, x + i, y, 2, 1);
    set_pixel(im, x, y + i, 2, 1);
  }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n) {
  int i;
  for (i = 0; i < n; ++i) {
    mark_spot(im, d[i].p);
  }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row image of the filter.
image make_1d_gaussian(float sigma) {
  int size = ceil(6 * sigma);
  size = size % 2 == 0 ? size + 1 : size;
  float normalizer = 1 / (TWOPI * sigma * sigma);

  image im = make_image(size, 1, 1);
  for (int i = -size / 2; i <= size / 2; i++) {
    float val = normalizer * exp(-(i * i) / (2 * sigma * sigma));
    set_pixel(im, i + size / 2, 0, 0, val);
  }

  l1_normalize(im);
  return im;
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma) {
  image row = make_1d_gaussian(sigma);

  // Transpose n x 1 Gaussian to 1 x n
  image col = make_image(1, row.w, 1);
  for (int j = 0; j < im.h; j++) {
    set_pixel(col, 0, j, 0, get_pixel(row, j, 0, 0));
  }

  // Apply the two filters one after the other
  image s = convolve_image(convolve_image(im, row, 1), col, 1);

  // Clean up and return
  free_image(row);
  free_image(col);
  return s;
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma) {
  image S = make_image(im.w, im.h, 3);

  // Calculcate image gradients
  image gx = convolve_image(im, make_gx_filter(), 0);
  image gy = convolve_image(im, make_gy_filter(), 0);

  // Calculate each measure value
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float ix = get_pixel(gx, i, j, 0);
      float iy = get_pixel(gy, i, j, 0);
      set_pixel(S, i, j, 0, ix * ix);
      set_pixel(S, i, j, 1, iy * iy);
      set_pixel(S, i, j, 2, ix * iy);
    }
  }

  // Return the weighted sum via Gaussian blur
  return smooth_image(S, sigma);
}

// Estimate the cornerness of each pixel given a structure matrix S.
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S) {
  image R = make_image(S.w, S.h, 1);
  // We'll use formulation det(S) - alpha * trace(S)^2, alpha = .06.
  float alpha = 0.06;
  float sx, sy, sxy, det, trace;

  for (int i = 0; i < R.w; i++) {
    for (int j = 0; j < R.h; j++) {
      sx = get_pixel(S, i, j, 0);
      sy = get_pixel(S, i, j, 1);
      sxy = get_pixel(S, i, j, 2);
      det = sx * sy - sxy * sxy;
      trace = sx + sy;
      set_pixel(R, i, j, 0, det - alpha * trace * trace);
    }
  }

  return R;
}

void suppress_pixel(image im, image r, int w, int x, int y) {
  float currPixel = get_pixel(im, x, y, 0);
  for (int i = -w; i <= w; i++) {
    for (int j = -w; j <= w; j++) {
      if (currPixel < get_pixel(im, x + i, y + j, 0)) {
        set_pixel(r, x, y, 0, SUPPRESSED_VALUE);
        return;
      }
    }
  }
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w) {
  image r = copy_image(im);
  for (int i = 0; i < r.w; i++) {
    for (int j = 0; j < r.h; j++) {
      suppress_pixel(im, r, w, i, j);
    }
  }

  return r;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms,
                                   int *n) {
  // Calculate structure matrix
  image S = structure_matrix(im, sigma);

  // Estimate cornerness
  image R = cornerness_response(S);

  // Run NMS on the responses
  image Rnms = nms_image(R, nms);

  // Count number of responses over threshold
  int count = 0;
  for (int i = 0; i < Rnms.w; i++) {
    for (int j = 0; j < Rnms.h; j++) {
      if (get_pixel(Rnms, i, j, 0) > thresh) {
        count++;
      }
    }
  }

  *n = count; // <- set *n equal to number of corners in image.
  descriptor *d = calloc(count, sizeof(descriptor));
  // Fill in array *d with descriptors of corners
  int idx = 0;
  for (int i = 0; i < Rnms.w * Rnms.h; i++) {
    if (get_pixel(Rnms, i % im.w, i / im.w, 0) > thresh) {
      d[idx++] = describe_index(im, i);
    }
  }

  free_image(S);
  free_image(R);
  free_image(Rnms);
  return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms) {
  int n = 0;
  descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
  mark_corners(im, d, n);
}
