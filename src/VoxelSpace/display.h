#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 290

void draw_pixel(uint16_t *framebuffer, uint16_t x, uint16_t y, uint16_t color);
void draw_line(uint16_t *framebuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void draw_rect(uint16_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

#endif
