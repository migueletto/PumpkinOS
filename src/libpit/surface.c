#include "sys.h"
#include "pfont.h"
#include "font5x8.h"
#include "font6x8coco.h"
#include "font6x8apple.h"
#include "font8x8.h"
#include "font10x16.h"
#include "font12x16coco.h"
#include "font16x16.h"
#include "font16x16cp437.h"
#include "graphic.h"
#include "surface.h"
#include "util.h"
#include "debug.h"
#include "xalloc.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct {
  int width, height, encoding;
  uint32_t rowBytes;
  uint8_t *buffer;
  surface_t *surface;
} buffer_surface_t;

static font_t *getfont(int font) {
  font_t *f;

  switch (font) {
    case 0: f = &font5x8; break;
    case 1: f = &font6x8coco; break;
    case 2: f = &font6x8apple; break;
    case 3: f = &font8x8; break;
    case 4: f = &font10x16; break;
    case 5: f = &font12x16coco; break;
    case 6: f = &font16x16; break;
    case 7: f = &font16x16_cp437; break;
    default: f = &font8x8; break;
  }

  return f;
}

int surface_font_width(int font) {
  font_t *f = getfont(font);
  return f->width;
}

int surface_font_height(int font) {
  font_t *f = getfont(font);
  return f->height;
}

void surface_print(surface_t *surface, int x, int y, char *s, int font, uint32_t fg, uint32_t bg) {
  font_t *f = getfont(font);
  int i;

  if (surface && s) {
    for (i = 0; s[i]; i++, x += f->width) {
      if (surface->printchar) {
        surface->printchar(surface->data, x, y, s[i], fg, bg);
      } else {
        graphic_printchar(surface->data, x, y, s[i], f, fg, bg, surface->setpixel);
      }
    }
  }
}

void surface_printvf(surface_t *surface, int x, int y, char *s, uint32_t color, double size, double angle, graphic_vfont_t *vf) {
  graphic_vfont_draw(vf, surface->data, s, x, y, color, size, (int)((angle * 180) / M_PI), surface->setpixel, surface->setarea);
}

void surface_sizevf(char *s, double size, int *dx, int *dy, graphic_vfont_t *vf) {
  graphic_vfont_size(vf, s, size, dx, dy);
}

void surface_ellipse(surface_t *surface, int x, int y, int rx, int ry, int filled, uint32_t color) {
  graphic_ellipse(surface->data, x, y, rx, ry, filled, color, surface->setpixel, surface->setarea);
}

void surface_line(surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color) {
  graphic_line(surface->data, x1, y1, x2, y2, color, surface->setpixel, surface->setarea);
}

void surface_rectangle(surface_t *surface, int x1, int y1, int x2, int y2, int filled, uint32_t color) {
  graphic_rectangle(surface->data, x1, y1, x2, y2, filled, color, surface->setpixel, surface->setarea);
}

void surface_polygon(surface_t *surface, point_t *points, int n, int filled, uint32_t color) {
  graphic_polygon(surface->data, points, n, filled, color, surface->setpixel, surface->setarea);
}

static uint8_t surface_best_color(surface_palette_t *palette, int npalette, int red, int green, int blue) {
  int32_t i, r, g, b, dr, dg, db, imin;
  uint32_t d, dmin;

  dmin = 0xffffffff;
  imin = 0;
  if (red   < 0) red   = 0; else if (red   > 255) red   = 255;
  if (green < 0) green = 0; else if (green > 255) green = 255;
  if (blue  < 0) blue  = 0; else if (blue  > 255) blue  = 255;

  for (i = 0; i < npalette; i++) {
    r = palette[i].red;
    g = palette[i].green;
    b = palette[i].blue;
    if (red == r && green == g && blue == b) {
      return i;
    }
    dr = red - r;
    dr = dr * dr;
    dg = green - g;
    dg = dg * dg;
    db = blue - b;
    db = db * db;
    d = dr + dg + db;
    if (d < dmin) {
      dmin = d;
      imin = i;
    }
  }

  return imin;
}

static uint8_t surface_gray_rgb(uint8_t red, uint8_t green, uint8_t blue) {
  return ((uint32_t)(red * 0.299 + green * 0.587 + blue * 0.114)) & 0xff;
}

void surface_draw(surface_t *dst, int dst_x, int dst_y, surface_t *src, int src_x, int src_y, int w, int h) {
  uint32_t color, transp;
  int32_t *pixels, oldpixel, newpixel, quant_error;
  int32_t *pixels_r, *pixels_g, *pixels_b, old_r, old_g, old_b, err_r, err_g, err_b;
  int i, j, k, red, green, blue, alpha, transparent;

  transparent = src->gettransp ? src->gettransp(src->data, &transp) : 0;

  if (dst->encoding == src->encoding && src->encoding != SURFACE_ENCODING_PALETTE) {
    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        color = src->getpixel(src->data, src_x + j, src_y + i);
        if (!transparent || color != transp) dst->setpixel(dst->data, dst_x + j, dst_y + i, color);
      }
    }
  } else if (dst->encoding == SURFACE_ENCODING_PALETTE) {
    // Floyd–Steinberg dithering

    if ((pixels_r = xcalloc(1, (w+1) * (h+1) * sizeof(int32_t))) != NULL &&
        (pixels_g = xcalloc(1, (w+1) * (h+1) * sizeof(int32_t))) != NULL &&
        (pixels_b = xcalloc(1, (w+1) * (h+1) * sizeof(int32_t))) != NULL) {

      for (i = 0, k = 0; i < h; i++) {
        for (j = 0; j < w; j++, k++) {
          color = src->getpixel(src->data, src_x + j, src_y + i);
          src->rgb_color(src->data, color, &red, &green, &blue, &alpha);
          if (alpha) {
            pixels_r[k] = red;
            pixels_g[k] = green;
            pixels_b[k] = blue;
          }
        }
      }

//int min_r = 1000, max_r = -1000;
      for (i = 0, k = 0; i < h; i++) {
        for (j = 0; j < w; j++, k++) {
          old_r = pixels_r[k];
          old_g = pixels_g[k];
          old_b = pixels_b[k];
if (old_r < 0) old_r = 0; else if (old_r > 255) old_r = 255;
if (old_g < 0) old_g = 0; else if (old_g > 255) old_g = 255;
if (old_b < 0) old_b = 0; else if (old_b > 255) old_b = 255;
          newpixel = surface_best_color(dst->palette, dst->npalette, old_r, old_g, old_b);
          dst->setpixel(dst->data, dst_x + j, dst_y + i, newpixel);
          dst->rgb_color(dst->data, newpixel, &red, &green, &blue, &alpha);
          err_r = old_r - red;
//if (err_g < min_r) min_r = err_g;
//if (err_g > max_r) max_r = err_g;
          err_g = old_g - green;
          err_b = old_b - blue;
          pixels_r[k + 1]     += err_r * 7 / 16;
          pixels_r[k + w - 1] += err_r * 3 / 16;
          pixels_r[k + w]     += err_r * 5 / 16;
          pixels_r[k + w + 1] += err_r * 1 / 16;
          pixels_g[k + 1]     += err_g * 7 / 16;
          pixels_g[k + w - 1] += err_g * 3 / 16;
          pixels_g[k + w]     += err_g * 5 / 16;
          pixels_g[k + w + 1] += err_g * 1 / 16;
          pixels_b[k + 1]     += err_b * 7 / 16;
          pixels_b[k + w - 1] += err_b * 3 / 16;
          pixels_b[k + w]     += err_b * 5 / 16;
          pixels_b[k + w + 1] += err_b * 1 / 16;
        }
      }
//debug(1, "XXX", "err %d %d", min_r, max_r);

      xfree(pixels_b);
      xfree(pixels_g);
      xfree(pixels_r);
    }
  } else if (dst->encoding == SURFACE_ENCODING_MONO) {
    if ((pixels = xcalloc(1, (w+1) * (h+1) * sizeof(int32_t))) != NULL) {
      if (src->encoding == SURFACE_ENCODING_GRAY) {
        for (i = 0, k = 0; i < h; i++) {
          for (j = 0; j < w; j++, k++) {
            pixels[k] = src->getpixel(src->data, src_x + j, src_y + i);
          }
        }
      } else {
        for (i = 0, k = 0; i < h; i++) {
          for (j = 0; j < w; j++, k++) {
            color = src->getpixel(src->data, src_x + j, src_y + i);
            src->rgb_color(src->data, color, &red, &green, &blue, &alpha);
            pixels[k] = alpha ? surface_gray_rgb(red, green, blue) : 0;
          }
        }
      }
      for (i = 0, k = 0; i < h; i++) {
        for (j = 0; j < w; j++, k++) {
          oldpixel = pixels[k];
          newpixel = oldpixel < 128 ? 0 : 255;
          dst->setpixel(dst->data, dst_x + j, dst_y + i, newpixel ? 1 : 0);
          quant_error = oldpixel - newpixel;
          pixels[k + 1]     += quant_error * 7 / 16;
          pixels[k + w - 1] += quant_error * 3 / 16;
          pixels[k + w]     += quant_error * 5 / 16;
          pixels[k + w + 1] += quant_error * 1 / 16;
        }
      }
      xfree(pixels);
    }
  } else {
    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        color = src->getpixel(src->data, src_x + j, src_y + i);
        if (!transparent || color != transp) {
          src->rgb_color(src->data, color, &red, &green, &blue, &alpha);
          color = dst->color_rgb(dst->data, red, green, blue, alpha);
          dst->setpixel(dst->data, dst_x + j, dst_y + i, color);
        }
      }
    }
  }
}

static uint32_t surface_mix_rgb(uint32_t c1, uint32_t c2) {
  uint8_t r1, g1, b1, r2, g2, b2, alpha;

  alpha = c2 >> 24;

  switch (alpha) {
    case 0x00:
      c2 = c1;
      break;
    case 0xFF:
      break;
    default:
      b2 = c2 >> 16;
      g2 = c2 >> 8;
      r2 = c2;
      b1 = c1 >> 16;
      g1 = c1 >> 8;
      r1 = c1;
      b2 = ((b2 * alpha) + (b1 * (255 - alpha))) / 255;
      g2 = ((g2 * alpha) + (g1 * (255 - alpha))) / 255;
      r2 = ((r2 * alpha) + (r1 * (255 - alpha))) / 255;
      alpha = 255;
      c2 = (alpha << 24) | (b2 << 16) | (g2 << 8) | r2;
      break;
  }

  return c2;
}

#define SETPIXEL1(x,y,c) \
  p = &b->buffer[y * b->rowBytes + ((x) >> 3)]; \
  m = 1 << ((x) & 7); \
  if (c) *p |= m; else *p &= (m ^ 0xff); \

#define SETPIXEL8(x,y,c) \
  p = &b->buffer[y * b->rowBytes + x]; \
  *p = c; \

#define SETPIXEL16(x,y,c) \
  p16 = (uint16_t *)&b->buffer[y * b->rowBytes + x * 2]; \
  *p16 = c; \

#define SETPIXEL32(x,y,c) \
  p32 = (uint32_t *)&b->buffer[y * b->rowBytes + x * 4]; \
  *p32 = surface_mix_rgb(*p32, c); \

static void bsurface_setpixel(void *data, int x, int y, uint32_t color) {
  buffer_surface_t *b = (buffer_surface_t *)data;
  uint8_t *p, m;
  uint16_t *p16;
  uint32_t *p32;

  if (x >= 0 && x < b->width && y >= 0 && y < b->height) {
    switch (b->encoding) {
      case SURFACE_ENCODING_MONO:
        SETPIXEL1(x, y, color)
        break;
      case SURFACE_ENCODING_GRAY:
      case SURFACE_ENCODING_PALETTE:
        SETPIXEL8(x, y, color)
        break;
      case SURFACE_ENCODING_RGB565:
        SETPIXEL16(x, y, color)
        break;
      case SURFACE_ENCODING_ARGB:
        SETPIXEL32(x, y, color)
        break;
    }
  }
}

static void bsurface_setarea(void *data, int x1, int y1, int x2, int y2, uint32_t color) {
  buffer_surface_t *b = (buffer_surface_t *)data;
  uint8_t *p, m;
  uint16_t *p16;
  uint32_t *p32;
  int x, y;

  if (x1 < 0) x1 = 0;
  else if (x1 >= b->width) x1 = b->width-1;
  if (x2 < 0) x2 = 0;
  else if (x2 >= b->width) x2 = b->width-1;
  if (y1 < 0) y1 = 0;
  else if (y1 >= b->height) y1 = b->height-1;
  if (y2 < 0) y2 = 0;
  else if (y2 >= b->height) y2 = b->height-1;

  switch (b->encoding) {
    case SURFACE_ENCODING_MONO:
      for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
          SETPIXEL1(x, y, color);
        }
      }
      break;
    case SURFACE_ENCODING_GRAY:
    case SURFACE_ENCODING_PALETTE:
      for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
          SETPIXEL8(x, y, color);
        }
      }
      break;
    case SURFACE_ENCODING_RGB565:
      for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
          SETPIXEL16(x, y, color);
        }
      }
      break;
    case SURFACE_ENCODING_ARGB:
      for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
          SETPIXEL32(x, y, color);
        }
      }
      break;
  }
}

static uint32_t surface_getpixelb(uint8_t *buffer, int encoding, int width, int x, int y) {
  uint8_t *p, m;
  uint16_t *p16;
  uint32_t *p32;
  uint32_t rowBytes, color = 0;

    switch (encoding) {
      case SURFACE_ENCODING_MONO:
        rowBytes = (width + 7) >> 3;
        p = &buffer[y * rowBytes + (x >> 3)];
        m = 1 << (x & 7);
        color = (*p & m) ? 1 : 0;
        break;
      case SURFACE_ENCODING_GRAY:
      case SURFACE_ENCODING_PALETTE:
        rowBytes = width;
        p = &buffer[y * rowBytes + x];
        color = *p;
        break;
      case SURFACE_ENCODING_RGB565:
        rowBytes = width << 1;
        p16 = (uint16_t *)&buffer[y * rowBytes + x * 2];
        color = *p16;
        break;
      case SURFACE_ENCODING_ARGB:
        rowBytes = width << 2;
        p32 = (uint32_t *)&buffer[y * rowBytes + x * 4];
        color = *p32;
        break;
    }

  return color;
}

static uint32_t bsurface_getpixel(void *data, int x, int y) {
  buffer_surface_t *b = (buffer_surface_t *)data;
  uint32_t color = 0;

  if (x >= 0 && x < b->width && y >= 0 && y < b->height) {
    color = surface_getpixelb(b->buffer, b->encoding, b->width, x, y);
  }

  return color;
}

void surface_rgb_color(int encoding, surface_palette_t *palette, int npalette, uint32_t color, int *red, int *green, int *blue, int *alpha) {
  if (alpha) *alpha = 0;
  if (red)   *red   = 0;
  if (green) *green = 0;
  if (blue)  *blue  = 0;

  switch (encoding) {
    case SURFACE_ENCODING_MONO:
      if (alpha) *alpha =  0xff;
      if (red)   *red   = color ? 0xff : 0x00;
      if (green) *green = color ? 0xff : 0x00;
      if (blue)  *blue  = color ? 0xff : 0x00;
      break;
    case SURFACE_ENCODING_GRAY:
      if (alpha) *alpha =  0xff;
      if (red)   *red   = color & 0xff; 
      if (green) *green = color & 0xff; 
      if (blue)  *blue  = color & 0xff; 
      break;
    case SURFACE_ENCODING_PALETTE:
      if (alpha) *alpha =  0xff;
      if (color < npalette) {
        if (red)   *red   = palette[color].red;
        if (green) *green = palette[color].green;
        if (blue)  *blue  = palette[color].blue;
      } else {
        if (red)   *red   = 0;
        if (green) *green = 0;
        if (blue)  *blue  = 0;
      }
      break;
    case SURFACE_ENCODING_RGB565:
      if (alpha) *alpha =  0xff;
      if (red)   *red   = ((color >> 11) & 0x1f) << 3; 
      if (green) *green = ((color >>  5) & 0x3f) << 2; 
      if (blue)  *blue  = ( color        & 0x1f) << 3; 
      break;
    case SURFACE_ENCODING_ARGB:
      if (alpha) *alpha =  color >> 24;
      if (red)   *red   = (color >> 16) & 0xff; 
      if (green) *green = (color >>  8) & 0xff;
      if (blue)  *blue  =  color        & 0xff;
      break;
  }
}

static void bsurface_rgb_color(void *data, uint32_t color, int *red, int *green, int *blue, int *alpha) {
  buffer_surface_t *b = (buffer_surface_t *)data;
  surface_rgb_color(b->surface->encoding, b->surface->palette, b->surface->npalette, color, red, green, blue, alpha);
}

uint32_t surface_color_rgb(int encoding, surface_palette_t *palette, int npalette, int red, int green, int blue, int alpha) {
  uint32_t color = 0;

  switch (encoding) {
    case SURFACE_ENCODING_MONO:
      color = (red | green | blue) ? 1 : 0;
      break;
    case SURFACE_ENCODING_GRAY:
      color = surface_gray_rgb(red, green, blue);
      break;
    case SURFACE_ENCODING_PALETTE:
      if (palette) {
        color = surface_best_color(palette, npalette, red, green, blue);
      }
      break;
    case SURFACE_ENCODING_RGB565:
      color = red >> 3;
      color <<= 6;
      color |= green >> 2;
      color <<= 5;
      color |= blue >> 3;
      break;
    case SURFACE_ENCODING_ARGB:
      color = blue | (((uint32_t)green) << 8) | (((uint32_t)red) << 16) | (((uint32_t)alpha) << 24);
      break;
  }

  return color;
}

static uint32_t bsurface_color_rgb(void *data, int red, int green, int blue, int alpha) {
  buffer_surface_t *b = (buffer_surface_t *)data;
  return surface_color_rgb(b->surface->encoding, b->surface->palette, b->surface->npalette, red, green, blue, alpha);
}

static void *bsurface_getbuffer(void *data, int *len) {
  buffer_surface_t *b = (buffer_surface_t *)data;
  *len = b->rowBytes * b->height;
  return b->buffer;
}

static void bsurface_destroy(void *data) {
  buffer_surface_t *b = (buffer_surface_t *)data;

  if (b) {
    if (b->buffer) xfree(b->buffer);
    xfree(b);
  }
}

int surface_event(surface_t *surface, uint32_t us, int *arg1, int *arg2) {
  return surface && surface->event ? surface->event(surface->data, us, arg1, arg2) : 0;
}

void surface_update(surface_t *surface) {
  if (surface && surface->update) surface->update(surface->data);
}

void *surface_buffer(surface_t *surface, int *len) {
  if (len) *len = 0;
  return (surface && surface->getbuffer) ? surface->getbuffer(surface->data, len) : NULL;
}

void surface_copy(surface_t *surface, uint8_t *src) {
  uint32_t color;
  uint8_t *dst;
  int i, j, len;

  if (surface && src) {
    if ((dst = surface_buffer(surface, &len)) != NULL) {
      xmemcpy(dst, src, len);
    } else {
      for (i = 0; i < surface->height; i++) {
        for (j = 0; j < surface->width; j++) {
          color = surface_getpixelb(src, surface->encoding, surface->width, j, i);
          surface->setpixel(surface->data, j, i, color);
        }
      }
    }
  }
}

surface_t *surface_create(int width, int height, int encoding) {
  surface_t *surface;
  buffer_surface_t *b;
  uint32_t rowBytes;

  debug(DEBUG_TRACE, "SURFACE", "surface_create %d,%d %d", width, height, encoding);

  switch (encoding) {
    case SURFACE_ENCODING_ARGB:    rowBytes = width * 4; break;
    case SURFACE_ENCODING_RGB565:  rowBytes = width * 2; break;
    case SURFACE_ENCODING_GRAY:    rowBytes = width; break;
    case SURFACE_ENCODING_PALETTE: rowBytes = width; break;
    case SURFACE_ENCODING_MONO:    rowBytes = (width + 7) / 8; break;
      break;
    default:
      debug(DEBUG_ERROR, "SURFACE", "surface_create: invalid encoding %d", encoding);
      return NULL;
  }

  if ((surface = xcalloc(1, sizeof(surface_t))) != NULL) {
    if ((b = xcalloc(1, sizeof(buffer_surface_t))) != NULL) {
      surface->tag = TAG_SURFACE;
      surface->width = width;
      surface->height = height;
      surface->encoding = encoding;
      surface->data = b;
      surface->setarea = bsurface_setarea;
      surface->setpixel = bsurface_setpixel;
      surface->getpixel = bsurface_getpixel;
      surface->color_rgb = bsurface_color_rgb;
      surface->rgb_color = bsurface_rgb_color;
      surface->getbuffer = bsurface_getbuffer;
      surface->destroy = bsurface_destroy;
      b->surface = surface;
      b->width = width;
      b->height = height;
      b->encoding = encoding;
      b->rowBytes = rowBytes;
      b->buffer = xcalloc(1, height * rowBytes);

      if (b->buffer == NULL) {
        xfree(b);
        xfree(surface);
        surface = NULL;
      }
    } else {
      xfree(surface);
      surface = NULL;
    }
  }

  return surface;
}

void surface_palette(surface_t *surface, int i, int red, int green, int blue) {
  if (surface && surface->encoding == SURFACE_ENCODING_PALETTE && i >= 0 && i < MAX_PALETTE) {
     if (!surface->palette) {
       surface->palette = xcalloc(MAX_PALETTE, sizeof(surface_palette_t));
     }
     if (surface->palette) {
       surface->palette[i].red = red;
       surface->palette[i].green = green;
       surface->palette[i].blue = blue;
       if (i >= surface->npalette) surface->npalette = i+1;
     }
  }
}

void surface_settitle(surface_t *surface, char *title) {
  if (surface && surface->settitle) surface->settitle(surface->data, title);
}

static void dither(uint8_t *buf, surface_t *surface) {
  int16_t *pixels, oldpixel, newpixel, quant_error;
  int i, j, k;

  if ((pixels = xcalloc(1, (surface->width+1) * (surface->height+1) * sizeof(int16_t))) != NULL) {
    for (i = 0, k = 0; i < surface->height; i++) {
      for (j = 0; j < surface->width; j++, k++) {
        pixels[k] = buf[k];
      }
    }

    for (i = 0, k = 0; i < surface->height; i++) {
      for (j = 0; j < surface->width; j++, k++) {
        oldpixel = pixels[k];
        newpixel = oldpixel < 128 ? 0 : 255;
        bsurface_setpixel(surface->data, j, i, newpixel ? 1 : 0);
        quant_error = oldpixel - newpixel;
        pixels[k + 1]                  += quant_error * 7 / 16;
        pixels[k + surface->width - 1] += quant_error * 3 / 16;
        pixels[k + surface->width]     += quant_error * 5 / 16;
        pixels[k + surface->width + 1] += quant_error * 1 / 16;
      }
    }

    xfree(pixels);
  }
}

static int surface_scale_down(surface_t *src, surface_t *dst) {
  uint32_t color, *rbin, *gbin, *bbin, *count;
  float fx, fy;
  int red, green, blue, alpha, len;
  int i, j, k, x, y;

  fx = (float)dst->width / (float)src->width;
  fy = (float)dst->height / (float)src->height;
  len = dst->width * dst->height;
  rbin = xcalloc(len, sizeof(uint32_t));
  gbin = xcalloc(len, sizeof(uint32_t));
  bbin = xcalloc(len, sizeof(uint32_t));
  count = xcalloc(len, sizeof(uint32_t));

  for (i = 0; i < src->height; i++) {
    for (j = 0; j < src->width; j++) {
      x = (int)(j * fx + 0.5);
      y = (int)(i * fy + 0.5);
      k = y * dst->width + x;
      if (k < len) {
        color = src->getpixel(src->data, j, i);
        src->rgb_color(src->data, color, &red, &green, &blue, &alpha);
        rbin[k] += red;
        gbin[k] += green;
        bbin[k] += blue;
        count[k]++;
      }
    }
  }

  for (i = 0, k = 0; i < dst->height; i++) {
    for (j = 0; j < dst->width; j++, k++) {
      color = count[k] ? dst->color_rgb(dst->data, rbin[k] / count[k], gbin[k] / count[k], bbin[k] / count[k], 255) : dst->color_rgb(dst->data, 0, 0, 0, 255);
      dst->setpixel(dst->data, j, i, color);
    }
  }

  xfree(count);
  xfree(bbin);
  xfree(gbin);
  xfree(rbin);

  return 0;
}

static int surface_scale_up(surface_t *src, surface_t *dst) {
  uint32_t color;
  float fx, fy;
  int red, green, blue, alpha;
  int i, j, x, y;

  fx = (float)src->width / (float)dst->width;
  fy = (float)src->height / (float)dst->height;

  for (i = 0; i < dst->height; i++) {
    for (j = 0; j < dst->width; j++) {
      x = (int)(j * fx + 0.5);
      y = (int)(i * fy + 0.5);
      if (x < src->width && y < src->height) {
        color = src->getpixel(src->data, x, y);
        src->rgb_color(src->data, color, &red, &green, &blue, &alpha);
        color = dst->color_rgb(dst->data, red, green, blue, alpha);
      } else {
        color = dst->color_rgb(dst->data, 0, 0, 0, 255);
      }
      dst->setpixel(dst->data, j, i, color);
    }
  }

  return 0;
}

int surface_scale(surface_t *src, surface_t *dst) {
  int r = -1;

  if (src && dst) {
    if (dst->width <= src->width && dst->height <= src->height) {
      r = surface_scale_down(src, dst);
    } else if (dst->width >= src->width && dst->height >= src->height) {
      r = surface_scale_up(src, dst);
    } else {
      debug(DEBUG_ERROR, "SURFACE", "can only scale either up or down");
    }
  }

  return r;
}

surface_t *surface_load(char *filename, int encoding) {
  surface_t *surface = NULL;
  uint8_t *buf, red, green, blue, alpha;
  uint32_t color;
  int width, height, icomp, ocomp, mono, len, i, j, k;

  // 1: grey
  // 2: grey, alpha
  // 3: red, green, blue
  // 4: red, green, blue, alpha

  switch (encoding) {
    case SURFACE_ENCODING_ARGB:   ocomp = 4; break;
    case SURFACE_ENCODING_RGB565: ocomp = 4; break;
    case SURFACE_ENCODING_GRAY:   ocomp = 1; break;
    case SURFACE_ENCODING_MONO:   ocomp = 1; break;
    default:
      debug(DEBUG_ERROR, "SURFACE", "surface_load: invalid encoding %d", encoding);
      return NULL;
  }

  if ((buf = stbi_load(filename, &width, &height, &icomp, ocomp)) != NULL) {
    mono = 0;
    if (icomp == 1) {
      len = width * height;
      for (k = 0, mono = 1; mono && k < len; k++) {
        mono = buf[k] == 0 || buf[k] == 255;
      }
    }
    if ((surface = surface_create(width, height, encoding)) != NULL) {
      if (encoding == SURFACE_ENCODING_MONO) {
        if (mono) {
          for (i = 0, k = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
              bsurface_setpixel(surface->data, j, i, buf[k] ? 1 : 0);
            }
          }
        } else {
          dither(buf, surface);
        }
      } else {
        for (i = 0, k = 0; i < height; i++) {
          for (j = 0; j < width; j++) {
            if (ocomp == 4) {
              red   = buf[k++];
              green = buf[k++];
              blue  = buf[k++];
              alpha = buf[k++];
              color = bsurface_color_rgb(surface->data, red, green, blue, alpha);
            } else {
              color = buf[k++]; // gray
            }
            bsurface_setpixel(surface->data, j, i, color);
          }
        }
      }
    }
    stbi_image_free(buf);
  }

  return surface;
}

int surface_save(surface_t *surface, char *filename, int quality) {
  char *ext;
  uint32_t color;
  uint8_t *buf;
  int red, green, blue, alpha;
  int ocomp, i, j, k, r = -1;

  // 1: grey
  // 2: grey, alpha
  // 3: red, green, blue
  // 4: red, green, blue, alpha

  if (surface) {
    switch (surface->encoding) {
      case SURFACE_ENCODING_ARGB:   ocomp = 3; break;
      case SURFACE_ENCODING_RGB565: ocomp = 3; break;
      case SURFACE_ENCODING_GRAY:   ocomp = 1; break;
      case SURFACE_ENCODING_MONO:   ocomp = 1; break;
      default:
        debug(DEBUG_ERROR, "SURFACE", "surface_save: invalid encoding %d", surface->encoding);
        return -1;
    }

    if ((ext = getext(filename)) != NULL) {
      buf = xcalloc(1, surface->width * surface->height * ocomp);

      if (ocomp >= 3) {
        for (i = 0, k = 0; i < surface->height; i++) {
          for (j = 0; j < surface->width; j++) {
            color = surface->getpixel(surface->data, j, i);
            surface->rgb_color(surface->data, color, &red, &green, &blue, &alpha);
            buf[k++] = red;
            buf[k++] = green;
            buf[k++] = blue;
            if (ocomp == 4) buf[k++] = alpha;
          }
        }
      } else {
        for (i = 0, k = 0; i < surface->height; i++) {
          for (j = 0; j < surface->width; j++) {
            buf[k++] = surface->getpixel(surface->data, j, i); // gray
          }
        }
      }

      if (!strcasecmp(ext, "png")) {
        r = stbi_write_png(filename, surface->width, surface->height, ocomp, buf, surface->width * ocomp);
      } else if (!strcasecmp(ext, "bmp")) {
        r = stbi_write_bmp(filename, surface->width, surface->height, ocomp, buf);
      } else if (!strcasecmp(ext, "jpg")) {
        r = stbi_write_jpg(filename, surface->width, surface->height, ocomp, buf, quality);
      } else {
       debug(DEBUG_ERROR, "SURFACE", "invalid type \"%s\"", filename);
      }
      xfree(buf);
    } else {
      debug(DEBUG_ERROR, "SURFACE", "invalid type \"%s\"", filename);
    }
  }

  return r;
}

int surface_destroy(surface_t *surface) {
  int r = -1;

  if (surface) {
    if (surface->destroy) surface->destroy(surface->data);
    if (surface->palette) xfree(surface->palette);
    xfree(surface);
    r = 0;
  }

  return r;
}
