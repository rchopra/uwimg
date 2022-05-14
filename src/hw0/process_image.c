#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c) {
  if (x < 0) {
    x = 0;
  } else if (x >= im.w) {
    x = im.w - 1;
  }

  if (y < 0) {
    y = 0;
  } else if (y >= im.h) {
    y = im.h - 1;
  }

  int pixels = im.w * im.h;
  int index = (c * pixels) + (y * im.w) + x;
  return im.data[index];
}

void set_pixel(image im, int x, int y, int c, float v) {
  if (x < 0 || x >= im.w || y < 0 || y >= im.h) {
    return;
  }

  int pixels = im.w * im.h;
  int index = (c * pixels) + (y * im.w) + x;
  im.data[index] = v;
}

image copy_image(image im) {
    image copy = make_image(im.w, im.h, im.c);
    memcpy(copy.data, im.data, sizeof(float) * (im.w * im.h * im.c));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    float r, g, b;
    image gray = make_image(im.w, im.h, 1);
    for (int i = 0; i < im.w; i++) {
      for (int j = 0; j < im.h; j++) {
        r = get_pixel(im, i, j, 0);
        g = get_pixel(im, i, j, 1);
        b = get_pixel(im, i, j, 2);
        set_pixel(gray, i, j, 0, 0.299 * r + 0.587 * g + 0.114 * b);
      }
    }
    return gray;
}

void shift_image(image im, int c, float v) {
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float originalVal = get_pixel(im, i, j, c);
      set_pixel(im, i, j, c, originalVal + v);
    }
  }
}

void clamp_image(image im) {
  float originalVal, clampedVal;
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      for (int k = 0; k < im.c; k++) {
        originalVal = get_pixel(im, i, j, k);
        clampedVal = originalVal > 1 ? 1 : (originalVal < 0 ? 0 : originalVal);
        set_pixel(im, i, j, k, clampedVal);
      }
    }
  }
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im) {
  float red, green, blue;
  float hue, saturation, value;
  float minValue, C, Hprime;

  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      red   = get_pixel(im, i, j, 0);
      green = get_pixel(im, i, j, 1);
      blue  = get_pixel(im, i, j, 2);

      value    = three_way_max(red, green, blue);
      minValue = three_way_min(red, green, blue);
      C = value - minValue;

      saturation = 0;
      if (value != 0) {
        saturation = C / value;
      }

      if (C == 0) {
        Hprime = 0;
      } else if (value == red) {
        Hprime = (green - blue) / C;
      } else if (value == green) {
        Hprime = (blue - red) / C + 2;
      } else { // value == blue
        Hprime = (red - green) / C + 4;
      }

      hue = Hprime / 6;
      if (Hprime < 0) {
        hue += 1;
      }

      set_pixel(im, i, j, 0, hue);
      set_pixel(im, i, j, 1, saturation);
      set_pixel(im, i, j, 2, value);
    }
  }
}

void hsv_to_rgb(image im) {
  float hue, saturation, value;
  float red, green, blue;
  float minValue, chroma, Hprime, x;

  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      hue        = get_pixel(im, i, j, 0);
      saturation = get_pixel(im, i, j, 1);
      value      = get_pixel(im, i, j, 2);

      chroma = saturation * value;
      minValue = value - chroma;
      Hprime = hue * 6;
      x = chroma * (1 - fabs(fmod(Hprime, 2) - 1));

      if (Hprime < 1) {
        red = chroma;
        green = x;
        blue = 0;
      } else if (Hprime < 2) {
        red = x;
        green = chroma;
        blue = 0;
      } else if (Hprime < 3) {
        red = 0;
        green = chroma;
        blue = x;
      } else if (Hprime < 4) {
        red = 0;
        green = x;
        blue = chroma;
      } else if (Hprime < 5) {
        red = x;
        green = 0;
        blue = chroma;
      } else { // Hprime < 6
        red = chroma;
        green = 0;
        blue = x;
      }

      set_pixel(im, i, j, 0, red + minValue);
      set_pixel(im, i, j, 1, green + minValue);
      set_pixel(im, i, j, 2, blue + minValue);
    }
  }
}

void scale_image(image im, int c, float v) {
  for (int i = 0; i < im.w; i++) {
    for (int j = 0; j < im.h; j++) {
      float originalVal = get_pixel(im, i, j, c);
      set_pixel(im, i, j, c, originalVal * v);
    }
  }
}