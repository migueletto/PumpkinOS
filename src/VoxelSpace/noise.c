#include <stdio.h>
//#include <math.h>

#include "noise.h"

static int permutation[] = {
  151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 
  103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 
  26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 
  87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 
  77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 
  46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 
  187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 
  198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 
  255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 
  170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 
  172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 
  104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 
  241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 
  157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 
  93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static int noise2(int x, int y, int seed) {
  //int tmp = hash[(y + seed) % 256];
  //return hash[(tmp + x) % 256];
  int i = (y + seed) % 256;
  if (i < 0) i += 256;
  int j = (permutation[i] + x) % 256;
  if (j < 0) j += 256;
  return permutation[j];
}

static float lin_inter(float x, float y, float s) {
  return x + s * (y-x);
}

static float smooth_inter(float x, float y, float s) {
  return lin_inter(x, y, s * s * (3-2*s));
}

static float noise2d(float x, float y, int seed) {
  int x_int = x;
  int y_int = y;
  if (x < 0) x_int--;
  if (y < 0) y_int--;
  //int x_int = floorf(x);
  //int y_int = floorf(y);
  float x_frac = x - x_int;
  float y_frac = y - y_int;
  int s = noise2(x_int, y_int, seed);
  int t = noise2(x_int+1, y_int, seed);
  int u = noise2(x_int, y_int+1, seed);
  int v = noise2(x_int+1, y_int+1, seed);
  float low = smooth_inter(s, t, x_frac);
  float high = smooth_inter(u, v, x_frac);
  return smooth_inter(low, high, y_frac);
}

float noise(float x, float y, float freq, int depth, int seed) {
  float xa = x * freq;
  float ya = y * freq;
  float amp = 1.0;
  float fin = 0;
  float div = 0;
  int i;

  for (i = 0; i < depth; i++) {
    div += 256 * amp;
    fin += noise2d(xa, ya, seed) * amp;
    amp /= 2;
    xa *= 2;
    ya *= 2;
  }

  return fin / div;
}
