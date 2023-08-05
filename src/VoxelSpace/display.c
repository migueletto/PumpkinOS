#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "display.h"

void draw_pixel(uint16_t *framebuffer, uint16_t x, uint16_t y, uint16_t color) {
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
    return;
  }
  framebuffer[(SCREEN_WIDTH * y) + x] = color;
}

static int abs(int n) {
  return n < 0 ? -n : n;
}

void draw_line(uint16_t *framebuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  float err = ((dx > dy) ? dx : -dy) / 2.0;
  while (true) {
    draw_pixel(framebuffer, x0, y0, color);
    if (x0 == x1 && y0 == y1) {
      break;
    }
    float e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

void draw_rect(uint16_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
  for (size_t i = 0; i < width; i++) {
    for (size_t j = 0; j < height; j++) {
      uint16_t current_x = x + i;
      uint16_t current_y = y + j;
      draw_pixel(framebuffer, current_x, current_y, color);
    }
  }
}
