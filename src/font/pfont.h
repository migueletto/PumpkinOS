#ifndef PIT_FONT_H
#define PIT_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int width, height;
  int min, max;
  uint8_t *font0, *font1;
} font_t;

#ifdef __cplusplus
}
#endif

#endif
