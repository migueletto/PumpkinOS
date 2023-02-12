#include <stdlib.h>
#include <stdint.h>

#include "bytes.h"

int put1(uint8_t w, uint8_t *buf, int i) {
  buf[i] = w;
  return 1;
}

int get1(uint8_t *w, uint8_t *buf, int i) {
  *w = buf[i];
  return 1;
}

int put2l(uint16_t w, uint8_t *buf, int i) {
  buf[i] = w;
  buf[i+1] = w >> 8;
  return 2;
}

int get2l(uint16_t *w, uint8_t *buf, int i) {
  *w = ((uint16_t)buf[i+1] << 8) | buf[i];
  return 2;
}

int put4l(uint32_t w, uint8_t *buf, int i) {
  buf[i] = w;
  buf[i+1] = w >> 8;
  buf[i+2] = w >> 16;
  buf[i+3] = w >> 24;
  return 4;
}

int get4l(uint32_t *w, uint8_t *buf, int i) {
  *w = ((uint32_t)buf[i+3] << 24) |
       ((uint32_t)buf[i+2] << 16) |
       ((uint32_t)buf[i+1] << 8)  | buf[i];
  return 4;
}

int put2b(uint16_t w, uint8_t *buf, int i) {
  buf[i] = w >> 8;
  buf[i+1] = w;
  return 2;
}

int get2b(uint16_t *w, uint8_t *buf, int i) {
  *w = ((uint16_t)buf[i] << 8) | buf[i+1];
  return 2;
}

int put4b(uint32_t w, uint8_t *buf, int i) {
  buf[i] = w >> 24;
  buf[i+1] = w >> 16;
  buf[i+2] = w >> 8;
  buf[i+3] = w;
  return 4;
}

int get4b(uint32_t *w, uint8_t *buf, int i) {
  *w = ((uint32_t)buf[i] << 24)   |
       ((uint32_t)buf[i+1] << 16) |
       ((uint32_t)buf[i+2] << 8)  | buf[i+3];
  return 4;
}

int putID(char *id, uint8_t *buf, int i) {
  buf[i] = id[0];
  buf[i+1] = id[1];
  buf[i+2] = id[2];
  buf[i+3] = id[3];
  return 4;
}

int getID(char *id, uint8_t *buf, int i) {
  id[0] = buf[i];
  id[1] = buf[i+1];
  id[2] = buf[i+2];
  id[3] = buf[i+3];
  return 4;
}
