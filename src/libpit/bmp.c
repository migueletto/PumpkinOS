#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "bmp.h"

#pragma pack(2)
typedef struct {
  uint16_t type;
  uint32_t fileSize;
  uint32_t reserved;
  uint32_t dataOffset;
  uint32_t headerSize;
  uint32_t width;
  uint32_t height;
  uint16_t planes;
  uint16_t bpp;
  uint32_t compression;
  uint32_t dataSize;
  uint32_t hRes;
  uint32_t vRes;
  uint32_t colors;
  uint32_t iColors;
} bmp_t;
#pragma pack()

int bmp_decode(uint8_t *bmp, uint8_t *chr, uint32_t cols, uint32_t rows, uint32_t width, uint32_t height) {
  bmp_t *header;
  uint8_t mask;
  int pitch, offset, i, j, k, col, row, x, y;

  header = (bmp_t *)bmp;
  if (header->type != 0x4D42) return -1;
  if (header->dataOffset != sizeof(bmp_t)) return -1;
  if (header->width != cols * width) return -1;
  if (header->height != rows * height) return -1;
  if (header->planes != 1) return -1;
  if (header->bpp != 24) return -1;
  if (header->compression != 0) return -1;

  bmp = &bmp[sizeof(bmp_t)];
  mask = 0;
  j = 0;
  k = 0;

  // BMPs are stored upside down
  pitch = header->width * 3; // 3=RGB
  i = header->height * pitch;

  for (y = 0; y < header->height; y++) {
    i -= pitch;
    row = y / height;
    for (x = 0;;) {
      col = x / width;
      offset = (row * cols + col) * height;
      // if first channel is non zero then 1, otherwise 0
      mask = (mask << 1) | (bmp[i] ? 1 : 0);
      x++;
      j++;
      i += 3; // 3=RGB, skip other two channels
      if (j == width) {
        chr[offset + k] = mask;
        mask = 0;
        j = 0;
        offset += height;
      }
      if (x == header->width) break;
    }
    i -= pitch;
    k++;
    if (k == height) {
      k = 0;
    }
  }

  return 0;
}
