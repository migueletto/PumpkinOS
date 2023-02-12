#define TAG_DISPLAY "display"

#define DISPLAY_LINE_LINE   1
#define DISPLAY_LINE_RECT   2
#define DISPLAY_LINE_FILLED 3

#define DISPLAY_ELLIPSE_LINE   1
#define DISPLAY_ELLIPSE_FILLED 2

typedef struct {
  char *tag;
  font_t *f;
  int width, height, depth;
  uint32_t fg, bg;
  int fixedfont;
  int x, y;

  void *data;
  int (*enable)(void *data, int enable);
  int (*contrast)(void *data, uint8_t contrast);
  int (*backlight)(void *data, int backlight);
  int (*printstr)(void *data, int x, int y, char *s, font_t *f, uint32_t fg, uint32_t bg);
  int (*printchar)(void *data, int x, int y, uint8_t c, font_t *f, uint32_t fg, uint32_t bg);
  int (*cls)(void *data, uint32_t bg);
  int (*draw)(void *data, int x, int y, image_provider_t *image, image_t *img);
  int (*drawf)(void *data, int x, int y, int encoding, int width, int height, unsigned char *frame);
  int (*line)(void *data, int x1, int y1, int x2, int y2, uint32_t fg, int style);
  int (*ellipse)(void *data, int x, int y, int rx, int ry, uint32_t fg, int style);
  uint32_t (*rgb)(void *data, int red, int green, int blue);
  void (*setpixel)(void *data, int x, int y, uint32_t fg);
  int (*event)(void *data, uint32_t us, int *arg1, int *arg2);
} libdisplay_t;
