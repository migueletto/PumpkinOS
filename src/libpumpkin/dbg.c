#include <PalmOS.h>

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "pumpkin.h"
#include "bytes.h"
#include "sys.h"
#include "rgb.h"
#include "debug.h"
#include "xalloc.h"

#include "dbg.h"

#define WIDTH  960
#define HEIGHT 640

#define MAX_PER_ROW 8

typedef struct dbg_bitmap_t {
  BitmapType *bmp;
  texture_t *texture;
  uint16_t *buf;
  int col, row;
  int x, y, updated;
  uint64_t last;
} dbg_bitmap_t;

typedef struct dbg_row_t {
  dbg_bitmap_t bitmap[MAX_PER_ROW];
  int num;
} dbg_row_t;

typedef struct dbg_t {
  dbg_row_t row[2];
} dbg_t;

static const UInt8 gray2values[4] = {0xff, 0xaa, 0x55, 0x00};
static const UInt8 gray4values[16] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

static window_provider_t *wp = NULL;
static window_t *w = NULL;
static dbg_t dbg;
static uint64_t last = 0;
static int updated = 0;

int dbg_init(window_provider_t *_wp, int encoding) {
  int width, height;

  wp = _wp;
  width = WIDTH;
  height = HEIGHT;
  w = wp->create(encoding, &width, &height, 1, 1, 0, 0, 0, wp->data);
  xmemset(&dbg, 0, sizeof(dbg_t));
//debug(1, "XXX", "dbg_init %p %d %p", wp, encoding, w);

  return w ? 0 : -1;
}

void dbg_finish(void) {
  if (w) {
//debug(1, "XXX", "dbg_finish");
    wp->destroy(w);
    w = NULL;
  }
}

static void dbg_draw(dbg_bitmap_t *bitmap) {
  BitmapType *bmp;
  ColorTableType *colorTable;
  UInt16 pixel;
  UInt32 transparentValue;
  Boolean isTransparent, transparent;
  UInt8 *bits;
  UInt32 depth, i, x, y;

  bmp = bitmap->bmp;
  depth = BmpGetBitDepth(bmp);
  bits = BmpGetBits(bmp);

  colorTable = BmpGetColortable(bmp);
  if (colorTable == NULL) {
    colorTable = pumpkin_defaultcolorTable();
  }

  isTransparent = BmpGetTransparentValue(bmp, &transparentValue);

  for (y = 0, i = 0; y < bmp->height; y++) {
    for (x = 0; x < bmp->width; x++) {
      pixel = 0;
      switch (depth) {
        case 1:
          pixel = bits[y * bmp->rowBytes + (x >> 3)];
          pixel = (pixel >> (7 - (x & 0x07))) & 1;
          transparent = isTransparent ? (pixel == transparentValue) : false;
          if (!transparent) pixel = pixel ? 0x0000 : 0xFFFF;
          break;
        case 2:
          pixel = bits[y * bmp->rowBytes + (x >> 2)];
          pixel = (pixel >> ((3 - (x & 0x03)) << 1)) & 0x03;
          transparent = isTransparent ? (pixel == transparentValue) : false;
          if (!transparent) pixel = rgb565(gray2values[pixel], gray2values[pixel], gray2values[pixel]);
          break;
        case 4:
          pixel = bits[y * bmp->rowBytes + (x >> 1)];
          pixel = (x & 0x01) ? pixel & 0x0F : pixel >> 4;
          transparent = isTransparent ? (pixel == transparentValue) : false;
          if (!transparent) pixel = rgb565(gray4values[pixel], gray4values[pixel], gray4values[pixel]);
          break;
        case 8:
          pixel = bits[y * bmp->rowBytes + x];
          transparent = isTransparent ? (pixel == transparentValue) : false;
          if (!transparent) pixel = rgb565(colorTable->entry[pixel].r, colorTable->entry[pixel].g, colorTable->entry[pixel].b);
          break;
        case 16:
          get2b(&pixel, bits, y * bmp->rowBytes + x*2);
          transparent = isTransparent ? (pixel == transparentValue) : false;
          break;
      }

      if (!transparent) {
        bitmap->buf[i++] = pixel;
      }
    }
  }

  bitmap->updated = 1;
}

void dbg_poll(void) {
  int col, row;
  uint64_t now;
  dbg_bitmap_t *bitmap;

  if (w && updated) {
    now = sys_get_clock();
    if ((now - last) > 50000) {
      for (row = 0; row < 2; row++) {
        for (col = 0; col < dbg.row[row].num; col++) {
          bitmap = &dbg.row[row].bitmap[col];
          if (bitmap->updated && (now - bitmap->last) > 50000) {
            dbg_draw(bitmap);
            wp->update_texture(w, bitmap->texture, (unsigned char *)bitmap->buf);
            bitmap->last = now;
            bitmap->updated = 0;
          }
          wp->draw_texture(w, bitmap->texture, bitmap->x, bitmap->y);
        }
      }
      wp->render(w);
      updated = 0;
      last = now;
    }
  }
}

static dbg_bitmap_t *dbg_get_bitmap(BitmapType *bmp) {
  BitmapTypeV0 *bmpV0;
  BitmapTypeV1 *bmpV1;
  BitmapTypeV2 *bmpV2;
  BitmapTypeV3 *bmpV3;
  dbg_bitmap_t *bitmap = NULL;

  if (bmp) {
    switch (bmp->version) {
      case 0:
        bmpV0 = (BitmapTypeV0 *)bmp;
        bitmap = (dbg_bitmap_t *)bmpV0->ext;
        break;
      case 1:
        bmpV1 = (BitmapTypeV1 *)bmp;
        bitmap = (dbg_bitmap_t *)bmpV1->ext;
        break;
      case 2:
        bmpV2 = (BitmapTypeV2 *)bmp;
        bitmap = (dbg_bitmap_t *)bmpV2->ext;
        break;
      case 3:
        bmpV3 = (BitmapTypeV3 *)bmp;
        bitmap = (dbg_bitmap_t *)bmpV3->ext;
        break;
    }
  }

  return bitmap;
}

void dbg_add(int row, BitmapType *bmp) {
  BitmapTypeV0 *bmpV0;
  BitmapTypeV1 *bmpV1;
  BitmapTypeV2 *bmpV2;
  BitmapTypeV3 *bmpV3;
  dbg_bitmap_t *bitmap;
  int col;

  row = 0;

  if (w && bmp && dbg.row[row].num < MAX_PER_ROW) {
    if (bmp->width < 320) return;
    if (dbg_get_bitmap(bmp) != NULL) return;
    col = dbg.row[row].num;
    bitmap = &dbg.row[row].bitmap[col];
    bitmap->bmp = bmp;
    bitmap->texture = wp->create_texture(w, bmp->width, bmp->height);
    bitmap->buf = xcalloc(bmp->width * bmp->height, 2);
    bitmap->x = col == 0 ? 0 : dbg.row[row].bitmap[col-1].x + dbg.row[row].bitmap[col-1].bmp->width;
    bitmap->y = col == 0 ? 0 : dbg.row[row].bitmap[col-1].y;
    if (bitmap->x >= WIDTH) {
      bitmap->x = 0;
      bitmap->y += 320;
    }
//debug(1, "XXX", "dbg_add %p %p row %d, col %d, %dx%d %d,%d depth %d, density %d", bmp, bitmap, row, col, bmp->width, bmp->height, bitmap->x, bitmap->y, BmpGetBitDepth(bmp), BmpGetDensity(bmp));
    bitmap->col = col;
    bitmap->row = row;
    bitmap->updated = 1;
    dbg.row[row].num++;

    switch (bmp->version) {
      case 0:
        bmpV0 = (BitmapTypeV0 *)bmp;
        bmpV0->ext = bitmap;
        break;
      case 1:
        bmpV1 = (BitmapTypeV1 *)bmp;
        bmpV1->ext = bitmap;
        break;
      case 2:
        bmpV2 = (BitmapTypeV2 *)bmp;
        bmpV2->ext = bitmap;
        break;
      case 3:
        bmpV3 = (BitmapTypeV3 *)bmp;
        bmpV3->ext = bitmap;
        break;
    }

    updated = 1;
  }
}

static void dbg_set_bitmap(BitmapType *bmp) {
  BitmapTypeV0 *bmpV0;
  BitmapTypeV1 *bmpV1;
  BitmapTypeV2 *bmpV2;
  BitmapTypeV3 *bmpV3;

  if (bmp) {
    switch (bmp->version) {
      case 0:
        bmpV0 = (BitmapTypeV0 *)bmp;
        bmpV0->ext = NULL;
        break;
      case 1:
        bmpV1 = (BitmapTypeV1 *)bmp;
        bmpV1->ext = NULL;
        break;
      case 2:
        bmpV2 = (BitmapTypeV2 *)bmp;
        bmpV2->ext = NULL;
        break;
      case 3:
        bmpV3 = (BitmapTypeV3 *)bmp;
        bmpV3->ext = NULL;
        break;
    }
  }
}

void dbg_delete(BitmapType *bmp) {
  dbg_bitmap_t *bitmap;
  int col, row;

  if (w && bmp) {
    if ((bitmap = dbg_get_bitmap(bmp)) != NULL) {
      xmemset(bitmap->buf, 0, bmp->width * bmp->height * 2);
      wp->update_texture(w, bitmap->texture, (unsigned char *)bitmap->buf);
      wp->draw_texture(w, bitmap->texture, bitmap->x, bitmap->y);
      if (bitmap->texture) wp->destroy_texture(w, bitmap->texture);
      if (bitmap->buf) xfree(bitmap->buf);
      bitmap->bmp = NULL;
      bitmap->texture = NULL;
      bitmap->buf = NULL;
      dbg_set_bitmap(bmp);
      row = bitmap->row;
//debug(1, "XXX", "dbg_delete %p row %d, col %d, %dx%d", bmp, row, bitmap->col, bmp->width, bmp->height);
      for (col = bitmap->col; col < dbg.row[row].num-1; col++) {
        xmemcpy(&dbg.row[row].bitmap[col], &dbg.row[row].bitmap[col+1], sizeof(dbg_bitmap_t));
      }
      dbg.row[row].num--;
      dbg.row[row].bitmap[0].col = 0;
      dbg.row[row].bitmap[0].x = 0;
      for (col = 1; col < dbg.row[row].num; col++) {
        dbg.row[row].bitmap[col].col = col;
        dbg.row[row].bitmap[col].x = dbg.row[row].bitmap[col-1].x + dbg.row[row].bitmap[col].bmp->width;
      }
    }
    updated = 1;
  }
}

void dbg_update(BitmapType *bmp) {
  dbg_bitmap_t *bitmap;

  if (w && bmp) {
    if ((bitmap = dbg_get_bitmap(bmp)) != NULL) {
      bitmap->updated = 1;
      updated = 1;
    }
  }
}
