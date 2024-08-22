#include "sys.h"
#include "rgb.h"

uint16_t rgb565(uint16_t r, uint16_t g, uint16_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

uint16_t r565(uint16_t rgb) {
  uint16_t c = ((rgb & 0xF800) >> 8);
  if (c > 0x0F) c |= 0x07;
  return c;
}

uint16_t g565(uint16_t rgb) {
  uint16_t c = ((rgb & 0x07E0) >> 3);
  if (c > 0x1F) c |= 0x03;
  return c;
}

uint16_t b565(uint16_t rgb) {
  uint16_t c = ((rgb & 0x001F) << 3);
  if (c > 0x0F) c |= 0x07;
  return c;
}

uint32_t rgb24(uint32_t r, uint32_t g, uint32_t b) {
  return (r << 16) | (g << 8) | b;
}

uint32_t r24(uint32_t rgb) {
  return rgb >> 16;
}

uint32_t g24(uint32_t rgb) {
  return (rgb >> 8) & 0xff;
}

uint32_t b24(uint32_t rgb) {
  return rgb & 0xff;
}

uint32_t rgb32(uint32_t r, uint32_t g, uint32_t b) {
  return (r << 16) | (g << 8) | b;
}

uint32_t rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
  return (a << 24) | (r << 16) | (g << 8) | b;
}

uint32_t a32(uint32_t rgba) {
  return rgba >> 24;
}

uint32_t r32(uint32_t rgb) {
  return (rgb >> 16) & 0xff;
}

uint32_t g32(uint32_t rgb) {
  return (rgb >> 8) & 0xff;
}

uint32_t b32(uint32_t rgb) {
  return rgb & 0xff;
}

uint8_t rgb2gray(uint8_t red, uint8_t green, uint8_t blue) {
  return ((uint32_t)(red * 0.299 + green * 0.587 + blue * 0.114)) & 0xff;
}
