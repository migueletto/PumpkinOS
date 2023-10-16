#ifndef PIT_FONT_H
#define PIT_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int width, height;
  int min, max, len;
  uint8_t *font;
  uint8_t *cwidth;
  int *index;
} font_t;

#ifdef __cplusplus
}
#endif

#endif
