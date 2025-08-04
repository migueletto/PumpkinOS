#include <PalmOS.h>

#define SSFN_IMPLEMENTATION

#define SSFN_memcmp  sys_memcmp
#define SSFN_memset  sys_memset
#define SSFN_realloc sys_realloc
#define SSFN_free    sys_free

#include "sys.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "ssfn_font.h"
#include "debug.h"

#include "ssfn.h"

FontTypeV2 *pumpkin_create_ssfn(void *h, uint8_t *p, uint32_t size, uint32_t *dsize, uint8_t family, uint8_t style, uint16_t fsize) {
  FontTypeV2 *font;
  ssfn_t *ctx;
  ssfn_buf_t buf;
  UInt32 *src, n;
  UInt8 *dst, mask;
  int i, j, mult, density, width, height, left, top;
  int widths[256], tops[256];
  char c[2];
  Err err;

  ctx = sys_calloc(1, sizeof(ssfn_t));
  if (ssfn_load(ctx, p) != SSFN_OK) {
    ssfn_free(ctx);
    debug(DEBUG_ERROR, "SSFN", "invalid ssfn font");
    debug_bytes(DEBUG_ERROR, "SSFN", p, size > 64 ? 64 : size);
    return NULL;
  }

  if ((font = StoNewDecodedResource(h, sizeof(FontTypeV2), 0, 0)) != NULL) {
    font->v = 2;
    font->size = size;
    font->fontType = 0x9200;
    font->version = 1;
    font->densityCount = 2;
    font->userdata = ctx;

    font->firstChar = -1;
    font->lastChar = -1;
    sys_memset(widths, 0, sizeof(widths));
    sys_memset(tops, 0, sizeof(tops));
    width = height = left = top = 0;

    for (density = 0; density < 2; density++, fsize <<= 1) {
      if (ssfn_select(ctx, family, NULL, style, fsize) != SSFN_OK) {
        debug(DEBUG_ERROR, "SSFN", "ssfn_select size %d failed", fsize);
        MemChunkFree(font);
        return NULL;
      }

      if (density == 0) {
        font->densities = sys_calloc(font->densityCount, sizeof(FontDensityType));
        font->pitch = sys_calloc(font->densityCount, sizeof(uint16_t));
        font->data = sys_calloc(font->densityCount, sizeof(uint8_t *));
        font->bmp = sys_calloc(font->densityCount, sizeof(BitmapType *));

        for (i = 0; i < 256; i++) {
          c[0] = i;
          c[1] = 0;
          if (ssfn_bbox(ctx, c, &width, &height, &left, &top) == SSFN_OK) {
            if (width > 0 && height > 0) {
              debug(DEBUG_TRACE, "SSFN", "char %d width %d height %d left %d top %d", i, width, height, left, top);
              widths[i] = width;
              tops[i] = top;
              if (width > font->fRectWidth) {
                font->fRectWidth = width;
              }
              if (height > font->fRectHeight) {
                font->fRectHeight = height;
              }
              if (font->firstChar == -1) {
                font->firstChar = i;
              }
              font->lastChar = i;
              font->totalWidth += width;
            }
          }
        }
      }

      if (font->totalWidth % 8) {
        font->totalWidth += 8 - (font->totalWidth % 8);
      }

      sys_memset(&buf, 0, sizeof(buf));

      if (density == 0) {
        debug(DEBUG_INFO, "SSFN", "chars %d to %d, rectWidth %d, rectheight %d",
          font->firstChar, font->lastChar, font->fRectWidth, font->fRectHeight);

        font->densities[density].density = kDensityLow;
        font->column = sys_calloc(font->lastChar - font->firstChar + 1, sizeof(uint16_t));
        font->width = sys_calloc(font->lastChar - font->firstChar + 1, sizeof(uint8_t));
        font->column[0] = 0;
        for (i = 0; i < font->lastChar - font->firstChar + 1; i++) {
          font->width[i] = widths[font->firstChar + i];
          if (i > 0) font->column[i] = font->column[i - 1] + font->width[i - 1];
        }
        mult = 1;
      } else {
        font->densities[density].density = kDensityDouble;
        mult = 2;
      }

      font->pitch[density] = (font->totalWidth * mult + 7) / 8;
      font->bmp[density] = BmpCreate3(font->totalWidth * mult, font->fRectHeight * mult, 0, font->densities[density].density, 1, true, 0, NULL, &err);
      n = font->totalWidth * mult * font->fRectHeight * mult * 4;
      font->data[density] = sys_calloc(1, n);
      sys_memset(font->data[density], 0xff, n);

      buf.ptr = font->data[density];
      buf.p = font->totalWidth * mult * 4;
      buf.fg = 0xff000000;
      buf.bg = 0xffffffff;
      buf.w = font->totalWidth * mult;
      buf.h = font->fRectHeight * mult;
      debug(DEBUG_INFO, "SSFN", "density %d fsize %d width %d pitch %d (%d) height %d", density, fsize, buf.w, buf.p, font->pitch[density] * 8, buf.h);
      for (i = font->firstChar; i <= font->lastChar; i++) {
        if (font->width[i - font->firstChar]) {
          buf.x = font->column[i - font->firstChar] * mult;
          buf.y = tops[i] * mult;
          c[0] = i;
          c[1] = 0;
          ssfn_render(ctx, &buf, c);
        }
      }

      src = (UInt32 *)font->data[density];
      dst = BmpGetBits(font->bmp[density]);
      BmpGetSizes(font->bmp[density], &n, NULL);
      sys_memset(dst, 0, n);
      n = font->totalWidth * mult * font->fRectHeight * mult;

      mask = 0x80;
      for (i = 0, j = 0; i < n; i++) {
        if (src[i] < 0xff400000) {
          dst[j] |= mask;
        }
        mask >>= 1;
        if (mask == 0x00) {
          mask = 0x80;
          j++;
        }
      }
    }
  }

  *dsize = sizeof(FontTypeV2);

  return font;
}

void pumpkin_destroy_ssfn(void *p) {
  FontTypeV2 *font;
  ssfn_t *ctx;
  int i;

  font = (FontTypeV2 *)p;
  if (font) {
    if (font->densities) sys_free(font->densities);
    if (font->pitch) sys_free(font->pitch);
    if (font->column) sys_free(font->column);
    if (font->width) sys_free(font->width);

    if (font->bmp) {
      for (i = 0; i < font->densityCount; i++) {
        if (font->bmp[i]) BmpDelete(font->bmp[i]);
      }
      sys_free(font->bmp);
    }
    if (font->data) {
      for (i = 0; i < font->densityCount; i++) {
        if (font->data) {
          sys_free(font->data[i]);
        }
      }
      sys_free(font->data);
    }
    if (font->userdata) {
      ctx = (ssfn_t *)font->userdata;
      ssfn_free(ctx);
    }
    MemChunkFree(font);
  }
}
