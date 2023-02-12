#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "yuv.h"

static void yuv2rgb(int y, int u, int v, int *r, int *g, int *b) {
  int c, d, e;

  c = y - 16;
  d = u - 128;
  e = v - 128;

  *r = (298 * c + 409 * e + 128) >> 8;
  *g = (298 * c - 100 * d - 208 * e + 128) >> 8;
  *b = (298 * c + 516 * d + 128) >> 8;

  if (*r < 0) *r = 0; else if (*r > 255) *r = 255;
  if (*g < 0) *g = 0; else if (*g > 255) *g = 255;
  if (*b < 0) *b = 0; else if (*b > 255) *b = 255;
}

int i420_yuyv(unsigned char *i420, int i420_len, unsigned char *yuyv, int width) {
  int ylen, ulen, ustart, vstart;
  int i, j, k, even, next;

  ylen = (2 * i420_len) / 3;
  ulen = ylen / 4;
  ustart = ylen;
  vstart = ustart + ulen;
  next = width;
  even = 1;

  for (i = 0, j = 0, k = 0; i < ylen;) {
    yuyv[j+0] = i420[i++];
    yuyv[j+1] = i420[ustart + k];
    yuyv[j+2] = i420[i++];
    yuyv[j+3] = i420[vstart + k];
    j += 4;
    k++;
    if (i == next) {
      if (!even) {
        ustart += k;
        vstart += k;
      }
      k = 0;
      next += width;
      even = !even;
    }
  }

  return j;
}

int yuyv_i420(unsigned char *yuyv, int yuyv_len, unsigned char *i420, int width) {
  int ylen, ulen, ustart, vstart;
  int i, j, u, v, next, even;
  uint32_t aux1, aux2, *py, *pi;
  uint16_t *pu, *pv;

  ylen = yuyv_len / 2;
  ulen = yuyv_len / 8;
  ustart = ylen;
  vstart = ustart + ulen;

  ylen /= 2;
  py = (uint32_t *)yuyv;
  pi = (uint32_t *)i420;
  pu = (uint16_t *)(i420 + ustart);
  pv = (uint16_t *)(i420 + vstart);
  u = v = 0;
  next = width / 4;
  even = 1;

  for (i = 0, j = 0; i < ylen;) {
    aux1 = py[i++];
    aux2 = py[i++];
    pi[j++] = (aux1 & 0xFF) | ((aux1 & 0xFF0000) >> 8) | ((aux2 & 0xFF) << 16) | ((aux2 & 0xFF0000) << 8);
    if (even) {
      pu[u++] = ((aux1 & 0xFF00) >> 8) | (aux2 & 0xFF00);
      pv[v++] = (aux1 >> 24) | ((aux2 >> 16) & 0xFF00);
    }
    if (j == next) {
      next += width;
      even = !even;
    }
  }

  return vstart + ulen;
}

int old_yuyv_i420(unsigned char *yuyv, int yuyv_len, unsigned char *i420, int width) {
  int ylen, ulen, ustart, vstart;
  int i, j, even, next;

  ylen = yuyv_len / 2;
  ulen = yuyv_len / 8;
  ustart = ylen;
  vstart = ustart + ulen;
  next = width;
  even = 1;

  for (i = 0, j = 0; i < ylen;) {
    i420[i++] = yuyv[j+0];
    i420[i++] = yuyv[j+2];
    if (even) {
      i420[ustart++] = yuyv[j+1];
      i420[vstart++] = yuyv[j+3];
    }
    j += 4;
    if (i == next) {
      next += width;
      even = !even;
    }
  }

  return vstart;
}

void yuyv_gray(unsigned char *yuyv, int yuyv_len, unsigned char *gray) {
  uint32_t aux, y1, y2, *py;
  uint16_t *pg;
  int i, j;

  j = 0;
  py = (uint32_t *)yuyv;
  pg = (uint16_t *)gray;
  yuyv_len /= 4;

  for (i = 0; i < yuyv_len;) {
    aux = py[i++];
    y1 =  aux        & 0xFF;
    y2 = (aux >> 16) & 0xFF;
    pg[j++] = y1 | (y2 << 8);
  }
}

void yuyv_rgb(unsigned char *yuyv, int yuyv_len, unsigned char *rgb) {
  unsigned char y1, u, y2, v;
  uint32_t aux, *py, *p3;
  int i, j, r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;

  j = 0;
  py = (uint32_t *)yuyv;
  p3 = (uint32_t *)rgb;
  yuyv_len /= 4;

  for (i = 0; i < yuyv_len;) {
    aux = py[i++];
    y1 =  aux        & 0xFF;
    u  = (aux >>  8) & 0xFF;
    y2 = (aux >> 16) & 0xFF;
    v  = (aux >> 24) & 0xFF;

    yuv2rgb(y1, u, v, &r1, &g1, &b1);
    yuv2rgb(y2, u, v, &r2, &g2, &b2);

    aux = py[i++];
    y1 =  aux        & 0xFF;
    u  = (aux >>  8) & 0xFF;
    y2 = (aux >> 16) & 0xFF;
    v  = (aux >> 24) & 0xFF;

    yuv2rgb(y1, u, v, &r3, &g3, &b3);
    yuv2rgb(y2, u, v, &r4, &g4, &b4);

    p3[j++] = r1 | (g1 << 8) | (b1 << 16) | (r2 << 24);
    p3[j++] = g2 | (b2 << 8) | (r3 << 16) | (g3 << 24);
    p3[j++] = b3 | (r4 << 8) | (g4 << 16) | (b4 << 24);
  }
}

static void rgb2yuyv(int *y1, int *u, int *y2, int *v, int r1, int g1, int b1, int r2, int g2, int b2) {
  double c, d, e, f;

  e = (616.0 * r1 - 516.0 * g1 - 100.0 * b1) / 1403.40625;
  d = ((b1 << 8) - (r1 << 8) + 409.0 * e) / 516.0;
  f = 409.0 * e - 128.0;
  c = ((r1 << 8) - f) / 298.0;

  *y1 = (int)c + 16;
  *u = (int)d + 128;
  *v = (int)e + 128;

  c = ((r2 << 8) - f) / 298.0;
  *y2 = (int)c + 16;

  if (*y1 < 0) *y1 = 0; else if (*y1 > 255) *y1 = 255;
  if (*u < 0) *u = 0; else if (*u > 255) *u = 255;
  if (*y2 < 0) *y2 = 0; else if (*y2 > 255) *y2 = 255;
  if (*v < 0) *v = 0; else if (*v > 255) *v = 255;
}

void rgb_yuyv(unsigned char *rgb, int rgb_len, unsigned char *yuyv) {
  unsigned char r1, g1, b1, r2, g2, b2;
  unsigned char r3, g3, b3, r4, g4, b4;
  uint32_t rgb_r, gb_rg, b_rgb;
  uint32_t *p3, *py;
  int i, j, y1, u, y2, v;

  p3 = (uint32_t *)rgb;
  py = (uint32_t *)yuyv;
  rgb_len /= 4;

  for (i = 0, j = 0; i < rgb_len;) {
    rgb_r = p3[i+0];
    gb_rg = p3[i+1];
    b_rgb = p3[i+2];
    i += 3;

    r1 =  rgb_r        & 0xff;
    g1 = (rgb_r >>  8) & 0xff;
    b1 = (rgb_r >> 16) & 0xff;

    r2 =  rgb_r >> 24;
    g2 =  gb_rg        & 0xff;
    b2 = (gb_rg >>  8) & 0xff;

    r3 = (gb_rg >> 16) & 0xff;
    g3 =  gb_rg >> 24;
    b3 =  b_rgb        & 0xff;

    r4 = (b_rgb >>  8) & 0xff;
    g4 = (b_rgb >> 16) & 0xff;
    b4 =  b_rgb >> 24;

    rgb2yuyv(&y1, &u, &y2, &v, r1, g1, b1, r2, g2, b2);
    py[j+0] = y1 | (u << 8) | (y2 << 16) | (v << 24);

    rgb2yuyv(&y1, &u, &y2, &v, r3, g3, b3, r4, g4, b4);
    py[j+1] = y1 | (u << 8) | (y2 << 16) | (v << 24);
    j += 2;
  }
}

void rgb_gray(unsigned char *rgb, int rgb_len, unsigned char *gray) {
  unsigned char r1, g1, b1, r2, g2, b2;
  unsigned char r3, g3, b3, r4, g4, b4;
  uint32_t rgb_r, gb_rg, b_rgb;
  uint32_t *p3, *pg;
  uint32_t gray1, gray2, gray3, gray4;
  int i, j;

  p3 = (uint32_t *)rgb;
  pg = (uint32_t *)gray;
  rgb_len /= 4;

  for (i = 0, j = 0; i < rgb_len;) {
    rgb_r = p3[i+0];
    gb_rg = p3[i+1];
    b_rgb = p3[i+2];
    i += 3;

    r1 = rgb_r & 0xff;
    g1 = (rgb_r >>  8) & 0xff;
    b1 = (rgb_r >> 16) & 0xff;

    r2 = rgb_r >> 24;
    g2 = gb_rg & 0xff;
    b2 = (gb_rg >>  8) & 0xff;

    r3 = (gb_rg >> 16) & 0xff;
    g3 = gb_rg >> 24;
    b3 = b_rgb & 0xff;

    r4 = (b_rgb >>  8) & 0xff;
    g4 = (b_rgb >> 16) & 0xff;
    b4 = b_rgb >> 24;

    gray1 = (r1 + g1 + b1) / 3;
    gray2 = (r2 + g2 + b2) / 3;
    gray3 = (r3 + g3 + b3) / 3;
    gray4 = (r4 + g4 + b4) / 3;

    pg[j++] = gray1 | (gray2 << 8) | (gray3 << 16) | (gray4 << 24);
  }
}

void bgra_rgba(unsigned char *bgra, int bgra_len, unsigned char *rgba) {
  uint32_t *pb, *pr, d;
  uint32_t r, b;
  int i;

  pb = (uint32_t *)bgra;
  pr = (uint32_t *)rgba;
  bgra_len /= 4;

  for (i = 0; i < bgra_len; i++) {
    d = pb[i];
    r = (d & 0xff) << 16;
    b = (d >> 16) & 0xff;
    pr[i] = (d & 0xff000000) | r | (d & 0x0000ff00) | b;
  }
}

void bgra_rgb(unsigned char *bgra, int bgra_len, unsigned char *rgb) {
  uint32_t *p3, *p4, bgra1, bgra2, bgra3, bgra4;
  uint32_t r, b;
  int i, j;

  p3 = (uint32_t *)rgb;
  p4 = (uint32_t *)bgra;
  bgra_len /= 4;

  for (i = 0, j = 0; i < bgra_len;) {
    bgra1 = p4[i+0];
    r = (bgra1 & 0xff) << 16;
    b = (bgra1 >> 16) & 0xff;
    bgra1 = r | (bgra1 & 0x0000ff00) | b;

    bgra2 = p4[i+1];
    r = (bgra2 & 0xff) << 16;
    b = (bgra2 >> 16) & 0xff;
    bgra2 = r | (bgra2 & 0x0000ff00) | b;

    bgra3 = p4[i+2];
    r = (bgra3 & 0xff) << 16;
    b = (bgra3 >> 16) & 0xff;
    bgra3 = r | (bgra3 & 0x0000ff00) | b;

    bgra4 = p4[i+3];
    r = (bgra4 & 0xff) << 16;
    b = (bgra4 >> 16) & 0xff;
    bgra4 = r | (bgra4 & 0x0000ff00) | b;

    p3[j+0] = ( bgra1        & 0x00ffffff) | (bgra2 << 24);
    p3[j+1] = ((bgra2 >>  8) & 0x0000ffff) | (bgra3 << 16);
    p3[j+2] = ((bgra3 >> 16) & 0x000000ff) | (bgra4 <<  8);

    i += 4;
    j += 3;
  }
}

void rgba_yuyv(unsigned char *rgba, int rgba_len, unsigned char *yuyv) {
  unsigned char r1, g1, b1, r2, g2, b2;
  uint32_t rgba1, rgba2;
  uint32_t *p4, *py;
  int i, j, y1, u, y2, v;

  p4 = (uint32_t *)rgba;
  py = (uint32_t *)yuyv;
  rgba_len /= 4;

  for (i = 0, j = 0; i < rgba_len;) {
    rgba1 = p4[i+0];
    rgba2 = p4[i+1];
    i += 2;

    b1 = rgba1 & 0xff;
    g1 = (rgba1 >>  8) & 0xff;
    r1 = (rgba1 >> 16) & 0xff;

    b2 = rgba2 & 0xff;
    g2 = (rgba2 >>  8) & 0xff;
    r2 = (rgba2 >> 16) & 0xff;

    rgb2yuyv(&y1, &u, &y2, &v, r1, g1, b1, r2, g2, b2);
    py[j] = y1 | (u << 8) | (y2 << 16) | (v << 24);
    j++;
  }
}

void rgba_gray(unsigned char *rgba, int rgba_len, unsigned char *gray) {
  unsigned char r1, g1, b1, r2, g2, b2;
  unsigned char r3, g3, b3, r4, g4, b4;
  uint32_t rgba1, rgba2, rgba3, rgba4;
  uint32_t gray1, gray2, gray3, gray4;
  uint32_t *p4, *pg;
  int i, j;

  p4 = (uint32_t *)rgba;
  pg = (uint32_t *)gray;
  rgba_len /= 4;

  for (i = 0, j = 0; i < rgba_len;) {
    rgba1 = p4[i+0];
    rgba2 = p4[i+1];
    rgba3 = p4[i+2];
    rgba4 = p4[i+3];
    i += 4;

    b1 = rgba1 & 0xff;
    g1 = (rgba1 >>  8) & 0xff;
    r1 = (rgba1 >> 16) & 0xff;

    b2 = rgba2 & 0xff;
    g2 = (rgba2 >>  8) & 0xff;
    r2 = (rgba2 >> 16) & 0xff;

    b3 = rgba3 & 0xff;
    g3 = (rgba3 >>  8) & 0xff;
    r3 = (rgba3 >> 16) & 0xff;

    b4 = rgba4 & 0xff;
    g4 = (rgba4 >>  8) & 0xff;
    r4 = (rgba4 >> 16) & 0xff;

    gray1 = (r1 + g1 + b1) / 3;
    gray2 = (r2 + g2 + b2) / 3;
    gray3 = (r3 + g3 + b3) / 3;
    gray4 = (r4 + g4 + b4) / 3;

    pg[j++] = gray1 | (gray2 << 8) | (gray3 << 16) | (gray4 << 24);
  }
}

void i420_gray(unsigned char *i420, int i420_len, unsigned char *gray) {
  uint32_t *p, *q;
  int i, len;

  p = (uint32_t *)i420;
  q = (uint32_t *)gray;
  len = (i420_len * 2) / 3;
  len /= 4;

  for (i = 0; i < len; i++) {
    q[i] = p[i];
  }
}

void gray_rgb(unsigned char *gray, int gray_len, unsigned char *rgb) {
  uint32_t *pg, *p3, aux;
  int i, j, g1, g2, g3, g4;

  pg = (uint32_t *)gray;
  p3 = (uint32_t *)rgb;
  gray_len /= 4;

  for (i = 0, j = 0; i < gray_len; i++) {
    aux = pg[i];
    g1 = aux & 0xFF;
    g2 = (aux >> 8) & 0xFF;
    g3 = (aux >> 16) & 0xFF;
    g4 = (aux >> 24) & 0xFF;
    p3[j++] = g1 | (g1 << 8) | (g1 << 16) | (g2 << 24);
    p3[j++] = g2 | (g2 << 8) | (g3 << 16) | (g3 << 24);
    p3[j++] = g3 | (g4 << 8) | (g4 << 16) | (g4 << 24);
  }
}

void gray_rgba(unsigned char *gray, int gray_len, unsigned char *rgba) {
  uint32_t *pg, *p4, aux, g1, g2, g3, g4;
  int i, j;

  pg = (uint32_t *)gray;
  p4 = (uint32_t *)rgba;
  gray_len /= 4;

  for (i = 0, j = 0; i < gray_len; i++) {
    aux = pg[i];
    g1 = aux & 0xFF;
    g2 = (aux >> 8) & 0xFF;
    g3 = (aux >> 16) & 0xFF;
    g4 = (aux >> 24) & 0xFF;
    p4[j++] = g1 | (g1 << 8) | (g1 << 16) | 0xff000000;
    p4[j++] = g2 | (g2 << 8) | (g2 << 16) | 0xff000000;
    p4[j++] = g3 | (g3 << 8) | (g3 << 16) | 0xff000000;
    p4[j++] = g4 | (g4 << 8) | (g4 << 16) | 0xff000000;
  }
}

void gray_yuyv(unsigned char *gray, int gray_len, unsigned char *yuyv) {
  unsigned char g1, g2, g3, g4;
  uint32_t *pg, *py, aux;
  int i, j, y1, u, y2, v;

  pg = (uint32_t *)gray;
  py = (uint32_t *)yuyv;
  gray_len /= 4;

  for (i = 0, j = 0; i < gray_len; i++) {
    aux = pg[i];
    g1 = aux & 0xFF;
    g2 = (aux >> 8) & 0xFF;
    g3 = (aux >> 16) & 0xFF;
    g4 = (aux >> 24) & 0xFF;

    rgb2yuyv(&y1, &u, &y2, &v, g1, g1, g1, g2, g2, g2);

    py[j+0] = y1 | (u << 8) | (y2 << 16) | (v << 24);

    rgb2yuyv(&y1, &u, &y2, &v, g3, g3, g3, g4, g4, g4);

    py[j+1] = y1 | (u << 8) | (y2 << 16) | (v << 24);
    j += 2;
  }
}

void rgba_rgb(unsigned char *rgba, int rgba_len, unsigned char *rgb) {
  uint32_t *p3, *p4, rgba1, rgba2, rgba3, rgba4;
  int i, j;

  p3 = (uint32_t *)rgb;
  p4 = (uint32_t *)rgba;
  rgba_len /= 4;

  for (i = 0, j = 0; i < rgba_len;) {
    rgba1 = p4[i+0];
    rgba2 = p4[i+1];
    rgba3 = p4[i+2];
    rgba4 = p4[i+3];
    i += 4;

    p3[j+0] = ( rgba1        & 0x00ffffff) | (rgba2 << 24);
    p3[j+1] = ((rgba2 >>  8) & 0x0000ffff) | (rgba3 << 16);
    p3[j+2] = ((rgba3 >> 16) & 0x000000ff) | (rgba4 <<  8);

    j += 3;
  }
}

void rgb_rgba(unsigned char *rgb, int rgb_len, unsigned char *rgba) {
  int i, j;
  uint32_t *p3, *p4;
  uint32_t rgb_r, gb_rg, b_rgb;

  p3 = (uint32_t *)rgb;
  p4 = (uint32_t *)rgba;
  rgb_len /= 4;

  for (i = 0, j = 0; i < rgb_len;) {
    rgb_r = p3[i+0];
    gb_rg = p3[i+1];
    b_rgb = p3[i+2];
    i += 3;

    p4[j+0] = (rgb_r & 0x00ffffff) | 0xff000000;
    p4[j+1] = (rgb_r >> 24) | ((gb_rg <<  8) & 0x00ffff00) | 0xff000000;
    p4[j+2] = (gb_rg >> 16) | ((b_rgb << 16) & 0x0000ff00) | 0xff000000;
    p4[j+3] = (b_rgb >>  8) | 0xff000000;
    j += 4;
  }
}

void yuyv_rgba(unsigned char *yuyv, int yuyv_len, unsigned char *rgba) {
  unsigned char y1, u, y2, v;
  uint32_t aux, *py, *p4;
  int i, j, r, g, b;

  j = 0;
  py = (uint32_t *)yuyv;
  p4 = (uint32_t *)rgba;
  yuyv_len /= 4;

  for (i = 0; i < yuyv_len;) {
    aux = py[i++];
    y1 = aux;
    u  = aux >> 8;
    y2 = aux >> 16;
    v  = aux >> 24;

    yuv2rgb(y1, u, v, &r, &g, &b);
    p4[j++] = b | (g << 8) | (r << 16) | 0xff000000;

    yuv2rgb(y2, u, v, &r, &g, &b);
    p4[j++] = b | (g << 8) | (r << 16) | 0xff000000;
  }
}

void desaturate_i420(unsigned char *buf, int len) {
  int ylen = (len * 2) / 3;
  memset(buf + ylen, 0x80, ylen / 2);
}

void desaturate_yuyv(unsigned char *buf, int len) {
  uint32_t *p;
  int i;

  len /= 4;
  p = (uint32_t *)buf;
  for (i = 0; i < len; i++) {
    p[i] = (p[i] & 0x00FF00FF) | 0x80008000;
  }
}

void uyvy_yuyv(unsigned char *uyvy, int uyvy_len, unsigned char *yuyv) {
  uint32_t *pu, *py, aux, y1, u, y2, v;
  int i;

  uyvy_len /= 4;
  pu = (uint32_t *)uyvy;
  py = (uint32_t *)yuyv;

  for (i = 0; i < uyvy_len; i++) {
    aux = pu[i];
    u  =  aux        & 0xFF;
    y1 = (aux >>  8) & 0xFF;
    v  = (aux >> 16) & 0xFF;
    y2 = (aux >> 24) & 0xFF;
    py[i] = y1 | (u << 8) | (y2 << 16) | (v << 24);
  }
}

void yuyv_rgb565(unsigned char *yuyv, int yuyv_len, unsigned char *rgb) {
  unsigned char y1, u, y2, v;
  uint32_t aux, *py;
  uint16_t *p2, w;
  int i, j, r1, g1, b1, r2, g2, b2;

  j = 0;
  py = (uint32_t *)yuyv;
  p2 = (uint16_t *)rgb;
  yuyv_len /= 4;

  for (i = 0; i < yuyv_len; i++) {
    aux = py[i];
    y1 =  aux        & 0xFF;
    u  = (aux >>  8) & 0xFF;
    y2 = (aux >> 16) & 0xFF;
    v  = (aux >> 24) & 0xFF;

    yuv2rgb(y1, u, v, &r1, &g1, &b1);
    yuv2rgb(y2, u, v, &r2, &g2, &b2);

    w = r1 >> 3;
    w <<= 6;
    w |= g1 >> 2;
    w <<= 5;
    w |= b1 >> 3;
    p2[j++] = w;

    w = r2 >> 3;
    w <<= 6;
    w |= g2 >> 2;
    w <<= 5;
    w |= b2 >> 3;
    p2[j++] = w;
  }
}
