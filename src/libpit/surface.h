#ifndef PIT_SURFACE_H
#define PIT_SURFACE_H

#define TAG_SURFACE "surface"

#define SURFACE_ENCODING_ARGB    1  // A R G B
#define SURFACE_ENCODING_RGB565  2  // R(5) G(6) B(5)
#define SURFACE_ENCODING_GRAY    3  // Y (8 bits)
#define SURFACE_ENCODING_MONO    4  // Y (1 bit)
#define SURFACE_ENCODING_PALETTE 5
#define SURFACE_ENCODING_ABGR    6  // A B G R

#define SURFACE_EVENT_MOTION      3
#define SURFACE_EVENT_KEYDOWN     4
#define SURFACE_EVENT_KEYUP       5
#define SURFACE_EVENT_BUTTONDOWN  6
#define SURFACE_EVENT_BUTTONUP    7
#define SURFACE_EVENT_WHEEL       8

#define MAX_PALETTE 256

typedef struct {
  uint8_t red, green, blue, pad;
} surface_palette_t;

typedef struct {
  char *tag;
  int width, height, encoding, npalette;
  int x, y;
  surface_palette_t *palette;
  void *data;
  void *(*getbuffer)(void *data, int *len);
  void (*setpixel)(void *data, int x, int y, uint32_t color);
  void (*setarea)(void *data, int x1, int y1, int x2, int y2, uint32_t color);
  void (*printchar)(void *data, int x, int y, uint8_t c, uint32_t fg, uint32_t bg);
  int (*gettransp)(void *data, uint32_t *transp);
  uint32_t (*getpixel)(void *data, int x, int y);
  uint32_t (*color_rgb)(void *data, int red, int green, int blue, int alpha);
  void (*rgb_color)(void *data, uint32_t color, int *red, int *green, int *blue, int *alpha);
  int (*event)(void *data, uint32_t us, int *arg1, int *arg2);
  void (*update)(void *data);
  void (*settitle)(void *data, char *title);
  void (*setoption)(void *data, int op, int value);
  void (*destroy)(void *data);
  void *udata;
} surface_t;

int surface_font_height(font_t *f, int font);
int surface_font_width(font_t *f, int font);
int surface_font_char_width(font_t *f, int font, int c);
int surface_font_chars_width(font_t *f, int font, char *s, int len);
void surface_print(surface_t *surface, int x, int y, char *s, font_t *f, int font, uint32_t fg, uint32_t bg);
void surface_printvf(surface_t *surface, int x, int y, char *s, uint32_t color, double size, double angle, graphic_vfont_t *vf);
void surface_sizevf(char *s, double size, int *dx, int *dy, graphic_vfont_t *vf);
void surface_ellipse(surface_t *surface, int x, int y, int rx, int ry, int filled, uint32_t color);
void surface_line(surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color);
void surface_rectangle(surface_t *surface, int x1, int y1, int x2, int y2, int filled, uint32_t color);
void surface_polygon(surface_t *surface, point_t *points, int n, int filled, uint32_t color);
void surface_curve(surface_t *surface, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void surface_draw(surface_t *dst, int dst_x, int dst_y, surface_t *src, int src_x, int src_y, int w, int h);
void surface_draw_alpha(surface_t *dst, int dst_x, int dst_y, surface_t *src, int src_x, int src_y, int w, int h);
void surface_update(surface_t *surface);
int surface_event(surface_t *surface, uint32_t us, int *arg1, int *arg2);
void surface_settitle(surface_t *surface, char *title);
void *surface_buffer(surface_t *surface, int *len);
void surface_copy(surface_t *surface, uint8_t *src);
void surface_rgb_color(int encoding, surface_palette_t *palette, int npalette, uint32_t color, int *red, int *green, int *blue, int *alpha);
uint32_t surface_color_rgb(int encoding, surface_palette_t *palette, int npalette, int red, int green, int blue, int alpha);
void surface_dither(surface_t *dst, int dst_x, int dst_y, surface_t *src, int src_x, int src_y, int w, int h, int mono);

surface_t *surface_create(int width, int height, int encoding);
surface_t *surface_load(char *filename, int encoding);
surface_t *surface_load_mem(uint8_t *buffer, int size, int encoding);
int surface_save(surface_t *surface, char *filename, int quality);
int surface_save_mem(surface_t *surface, int quality, void *context, void (*callback)(void *context, void *data, int size));
int surface_scale(surface_t *src, surface_t *dst);
int surface_rotate(surface_t *src, surface_t *dst, double angle);
int surface_destroy(surface_t *surface);
void surface_palette(surface_t *surface, int i, int red, int green, int blue);

#endif
