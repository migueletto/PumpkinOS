#ifndef PIT_GRAPHIC_H
#define PIT_GRAPHIC_H

#ifdef __cplusplus
extern "C" {
#endif

#define VFONT "vfont"

typedef struct {
  int x, y;
} point_t;

typedef struct graphic_vfont_t graphic_vfont_t;

typedef void (*setpixel_f)(void *data, int x, int y, uint32_t color);
typedef void (*setarea_f)(void *data, int x1, int y1, int x2, int y2, uint32_t color);

void graphic_line(void *data, int x1, int y1, int x2, int y2, uint32_t color, setpixel_f p, setarea_f a);
void graphic_rectangle(void *data, int x1, int y1, int x2, int y2, int filled, uint32_t color, setpixel_f p, setarea_f a);
void graphic_polygon(void *data, point_t *points, int n, int filled, uint32_t color, setpixel_f p, setarea_f a);
void graphic_ellipse(void *data, int x, int y, int rx, int ry, int filled, uint32_t color, setpixel_f p, setarea_f a);
void graphic_curve(void *data, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, setpixel_f p, setarea_f a);
void graphic_printchar(void *data, int x, int y, uint8_t c, font_t *f, uint32_t fg, uint32_t bg, setpixel_f p);

graphic_vfont_t *graphic_vfont_init(void);
void graphic_vfont_finish(graphic_vfont_t *f);
void graphic_vfont_draw(graphic_vfont_t *f, void *data, char *s, int x, int y, uint32_t color, double size, int angle, setpixel_f p, setarea_f a);
void graphic_vfont_size(graphic_vfont_t *f, char *s, double size, int *dx, int *dy);

#ifdef __cplusplus
}
#endif

#endif
