#ifndef PIT_IMAGE_H
#define PIT_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_IMAGE "image"

#define IMAGE_PROVIDER "image_provider"

typedef struct image_t image_t;

typedef struct {
  image_t *(*load)(char *filename, int depth);
  image_t *(*create)(int width, int height, int depth);
  image_t *(*copy)(image_t *src);
  image_t *(*copy_resize)(image_t *src, int dst_width, int dst_height);
  image_t *(*slice)(image_t *image, int x, int y, int width, int height);
  int (*clear)(image_t *image);
  int (*fill_rgb)(image_t *image, unsigned char *rgb);
  int (*fill_rgba)(image_t *image, unsigned char *rgba);
  int (*fill_rgb565)(image_t *image, unsigned char *rgb565, int w, int h, int scale);
  int (*extract_rgb)(image_t *image, unsigned char *rgb);
  int (*extract_rgba)(image_t *image, unsigned char *rgba);
  int (*extract_rgb565)(image_t *image, unsigned char *rgb565);
  int (*extract_raw)(image_t *image, unsigned char *raw);
  int (*overlay_rgba)(image_t *image, unsigned char *rgba, int len);
  int (*size)(image_t *image, int *x, int *y);
  int (*blend)(image_t *image, int blend);
  int (*rgb)(image_t *image, int c, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha);
  int (*color)(image_t *image, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
  int (*icolor)(image_t *image, int index, uint8_t red, uint8_t green, uint8_t blue);
  int (*setcolor)(image_t *image, int color);
  int (*getpixel)(image_t *image, int x, int y);
  int (*setpixel)(image_t *image, int x, int y, int color);
  int (*gettransp)(image_t *image);
  int (*settransp)(image_t *image, int color);
  int (*line)(image_t *image, int x1, int y1, int x2, int y2);
  int (*lineto)(image_t *image, int x, int y);
  int (*rectangle)(image_t *image, int x1, int y1, int x2, int y2, int filled);
  int (*polygon)(image_t *image, point_t *points, int n, int filled);
  int (*ellipse)(image_t *image, int x, int y, int rx, int ry, int filled);
  int (*clip)(image_t *image, int x1, int y1, int x2, int y2);
  int (*draw)(image_t *image, int x, int y, image_t *obj);
  int (*draw2)(image_t *image, int x, int y, image_t *obj, int srcX, int srcY, int w, int h);
  int (*text)(image_t *image, int x, int y, char *font, double size, double angle, char *text, int *bounds);
  int (*save)(image_t *image, char *filename);
  int (*save_exif)(image_t *image, char *filename, int mode, time_t ts, double lon, double lat, double alt);
  int (*destroy)(image_t *image);
  uint8_t *(*raw)(image_t *image);
} image_provider_t;

#ifdef __cplusplus
}
#endif

#endif
