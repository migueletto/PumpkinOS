#include <PalmOS.h>
#include <time.h>
#include <sys/time.h>

#include "mutex.h"
#include "pumpkin.h"
#include "AppRegistry.h"
#include "storage.h"
#include "rgb.h"
#include "resource.h"
#include "resedit.h"
#include "bytes.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_TITLE 256
#define CELL_MIN 6
#define LIGHT_GRAY 0xE0

typedef enum {
  dragMode,
  drawMode,
  pickMode
} edit_mode_t;

typedef struct {
  char *prefix, title[MAX_TITLE];
  FormType *frm;
  BitmapType **bmps;
  MemHandle handle;
  int numBmps, index;
  int x, y, w, h, dw, dh;
  int iDown, jDown;
  int i0, j0;
  int width, height;
  Boolean truncated, popup, dirty;
  edit_mode_t mode;
  UInt8 color1, color2, color4, color8;
  RGBColorType rgb;
  Boolean down, drag, transpPixel, stop;
} bmp_edit_t;

static const UInt8 gray1[2]  = {0xff, 0x00};
static const UInt8 gray2[4]  = {0xff, 0xaa, 0x55, 0x00};
static const UInt8 gray4[16] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

static Boolean getBitN(BitmapType *bmp, UInt32 x, UInt32 y, int n, UInt32 *index, RGBColorType *rgb) {
  ColorTableType *colorTable;
  UInt32 transparentValue;
  UInt32 value;
  UInt8 b;
  Boolean transp = false;

  switch (n) {
    case  1:
      b = BmpGetPixelValue(bmp, x, y);
      *index = b;
      rgb->r = rgb->g = rgb->b = gray1[b];
      break;
    case  2:
      b = BmpGetPixelValue(bmp, x, y);
      *index = b;
      rgb->r = rgb->g = rgb->b = gray2[b];
      break;
    case  4:
      b = BmpGetPixelValue(bmp, x, y);
      *index = b;
      rgb->r = rgb->g = rgb->b = gray4[b];
      break;
    case  8:
      colorTable = BmpGetColortable(bmp);
      if (colorTable == NULL) {
        colorTable = pumpkin_defaultcolorTable();
      }
      b = BmpGetPixelValue(bmp, x, y);
      *index = b;
      rgb->r = colorTable->entry[b].r;
      rgb->g = colorTable->entry[b].g;
      rgb->b = colorTable->entry[b].b;
      transp = BmpGetTransparentValue(bmp, &transparentValue) && transparentValue == b;
      break;
    case 16:
      value = BmpGetPixelValue(bmp, x, y);
      *index = value;
      rgb->r = r565(value);
      rgb->g = g565(value);
      rgb->b = b565(value);
      transp = BmpGetTransparentValue(bmp, &transparentValue) && transparentValue == value;
      break;
  }

  return transp;
}

static Boolean putBitN(BitmapType *bmp, UInt32 x, UInt32 y, int n, UInt16 value, Boolean transpPixel) {
  UInt32 transparentValue;
  Boolean transp = false;

  switch (n) {
    case  8:
      if (transpPixel && BmpGetTransparentValue(bmp, &transparentValue)) {
        value = transparentValue;
        transp = true;
      }
      break;
    case 16:
      if (transpPixel && BmpGetTransparentValue(bmp, &transparentValue)) {
        value = transparentValue;
        transp = true;
      }
      break;
  }

  BmpSetPixel(bmp, x, y, value);

  return transp;
}

static Boolean getBit(BitmapType *bmp, UInt32 x, UInt32 y, UInt32 *index, RGBColorType *rgb) {
  BitmapTypeV1 *bmpV1;
  BitmapTypeV2 *bmpV2;
  BitmapTypeV3 *bmpV3;
  Boolean transp = false;

  switch (bmp->version) {
    case 0:
      transp = getBitN(bmp, x, y, 1, index, rgb);
      break;
    case 1:
      bmpV1 = (BitmapTypeV1 *)bmp;
      transp = getBitN(bmp, x, y, bmpV1->pixelSize, index, rgb);
      break;
    case 2:
      bmpV2 = (BitmapTypeV2 *)bmp;
      transp = getBitN(bmp, x, y, bmpV2->pixelSize, index, rgb);
      break;
    case 3:
      bmpV3 = (BitmapTypeV3 *)bmp;
      transp = getBitN(bmp, x, y, bmpV3->pixelSize, index, rgb);
      break;
    default:
      break;
  }

  return transp;
}

static Boolean putBit(BitmapType *bmp, UInt32 x, UInt32 y, UInt16 value, Boolean transpPixel) {
  BitmapTypeV1 *bmpV1;
  BitmapTypeV2 *bmpV2;
  BitmapTypeV3 *bmpV3;
  Boolean transp = false;

  switch (bmp->version) {
    case 0:
      putBitN(bmp, x, y, 1, value, false);
      break;
    case 1:
      bmpV1 = (BitmapTypeV1 *)bmp;
      putBitN(bmp, x, y, bmpV1->pixelSize, value, false);
      break;
    case 2:
      bmpV2 = (BitmapTypeV2 *)bmp;
      transp = putBitN(bmp, x, y, bmpV2->pixelSize, value, transpPixel);
      break;
    case 3:
      bmpV3 = (BitmapTypeV3 *)bmp;
      transp = putBitN(bmp, x, y, bmpV3->pixelSize, value, transpPixel);
      break;
    default:
      break;
  }

  return transp;
}

static void paintPixel(bmp_edit_t *data, int j, int i) {
  RGBColorType rgb, oldf;
  ColorTableType *colorTable;
  RectangleType pixel;
  Boolean transp;
  UInt16 depth, color;
  int x, y;

  DmSetDirty(data->handle);

  x = data->x + j * data->dw;
  y = data->y + i * data->dh;

  depth = BmpGetBitDepth(data->bmps[data->index]);
  switch (depth) {
    case  1:
      color = data->color1;
      rgb.r = rgb.g = rgb.b = gray1[color & 0x01];
      break;
    case  2:
      color = data->color2;
      rgb.r = rgb.g = rgb.b = gray2[color & 0x03];
      break;
    case  4:
      color = data->color4;
      rgb.r = rgb.g = rgb.b = gray4[color & 0x0F];
      break;
    case  8:
      color = data->color8;
      colorTable = BmpGetColortable(data->bmps[data->index]);
      if (colorTable == NULL) colorTable = pumpkin_defaultcolorTable();
      if (color < colorTable->numEntries) {
        rgb.r = colorTable->entry[color].r;
        rgb.g = colorTable->entry[color].g;
        rgb.b = colorTable->entry[color].b;
      } else {
        rgb.r = rgb.g = rgb.b = 0x00;
      }
      break;
    case 16:
      color = rgb565(data->rgb.r, data->rgb.g, data->rgb.b);
      rgb.r = data->rgb.r;
      rgb.g = data->rgb.g;
      rgb.b = data->rgb.b;
      break;
  }

  transp = putBit(data->bmps[data->index], data->j0 + j, data->i0 + i, color, data->transpPixel);

  if (transp) {
    rgb.r = rgb.g = rgb.b = 0xff;
    WinSetForeColorRGB(&rgb, &oldf);
    RctSetRectangle(&pixel, x+1, y+1, data->dw-1, data->dh-1);
    WinPaintRectangle(&pixel, 0);
    rgb.r = rgb.g = rgb.b = LIGHT_GRAY;
    WinSetForeColorRGB(&rgb, NULL);
    WinPaintPixel(x+data->dw/2, y+data->dh/2);

  } else {
    WinSetForeColorRGB(&rgb, &oldf);
    RctSetRectangle(&pixel, x+1, y+1, data->dw-1, data->dh-1);
    WinPaintRectangle(&pixel, 0);
  }

  WinSetForeColorRGB(&oldf, NULL);
}

static Boolean bitmapGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  RectangleType rect, aux;
  BitmapType *bmp;
  bmp_edit_t *data;
  RGBColorType rgb, oldf;
  PatternType oldp;
  UInt16 index;
  UInt32 c;
  int i, j, x, y, w, h, dw, dh;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, bitmapGad);
  FrmGetObjectBounds(frm, index, &rect);
  data = (bmp_edit_t *)FrmGetGadgetData(frm, index);
  bmp = data->bmps[data->index];

  data->width = bmp->width;
  data->height = bmp->height;
  w = rect.extent.x;
  h = rect.extent.y;
  dw = (w-1) / data->width;
  dh = (h-1) / data->height;
  if (dw < CELL_MIN) {
    dw = CELL_MIN;
    data->width = (w-1) / dw;
  }
  if (dh < CELL_MIN) {
    dh = CELL_MIN;
    data->height = (h-1) / dh;
  }
  if (dw < dh) dh = dw; else dw = dh;
  w = dw * data->width + 1;
  h = dh * data->height + 1;

  data->w = w;
  data->h = h;
  data->x = rect.topLeft.x + (rect.extent.x - w) / 2;
  data->y = rect.topLeft.y + (rect.extent.y - h) / 2;
  data->dw = dw;
  data->dh = dh;
  data->truncated = (data->height != bmp->height) || (data->width != bmp->width);

  switch (cmd) {
    case formGadgetDrawCmd:
      oldp = WinGetPatternType();
      WinSetPatternType(blackPattern);
      rgb.r = rgb.g = rgb.b = 0xff;
      WinSetForeColorRGB(&rgb, &oldf);

      RctSetRectangle(&aux, rect.topLeft.x, rect.topLeft.y, (rect.extent.x - w) / 2, rect.extent.y);
      WinPaintRectangle(&aux, 0);

      aux.extent.x = rect.extent.x;
      aux.extent.y = (rect.extent.y - h) / 2;
      WinPaintRectangle(&aux, 0);

      aux.topLeft.y += aux.extent.y + data->h;
      WinPaintRectangle(&aux, 0);

      RctSetRectangle(&aux, rect.topLeft.x + data->w + (rect.extent.x - w) / 2, rect.topLeft.y, (rect.extent.x - w) / 2, rect.extent.y);
      WinPaintRectangle(&aux, 0);

      rgb.r = rgb.g = rgb.b = LIGHT_GRAY;
      WinSetForeColorRGB(&rgb, &oldf);

      x = rect.topLeft.x + (rect.extent.x - w) / 2;
      y = rect.topLeft.y + (rect.extent.y - h) / 2;

      for (i = 0; i < data->height; i++) {
        WinPaintLine(x, y, x+w-1, y);
        y += dh;
      }
      WinPaintLine(x, y, x+w-1, y);

      y = rect.topLeft.y + (rect.extent.y - h) / 2;

      for (i = 0; i < data->width; i++) {
        WinPaintLine(x, y, x, y+h-1);
        x += dw;
      }
      WinPaintLine(x, y, x, y+h-1);

      for (i = 0; i < data->height; i++) {
        x = rect.topLeft.x + (rect.extent.x - w) / 2;
        for (j = 0; j < data->width; j++) {
          if (getBit(bmp, data->j0 + j, data->i0 + i, &c, &rgb)) {
            rgb.r = rgb.g = rgb.b = 0xff;
            WinSetForeColorRGB(&rgb, NULL);
            RctSetRectangle(&aux, x+1, y+1, dw-1, dh-1);
            WinPaintRectangle(&aux, 0);
            rgb.r = rgb.g = rgb.b = LIGHT_GRAY;
            WinSetForeColorRGB(&rgb, NULL);
            WinPaintPixel(x+dw/2, y+dh/2);
          } else {
            WinSetForeColorRGB(&rgb, NULL);
            RctSetRectangle(&aux, x+1, y+1, dw-1, dh-1);
            WinPaintRectangle(&aux, 0);
          }
          x += dw;
        }
        y += dh;
      }

      WinSetPatternType(oldp);
      WinSetForeColorRGB(&oldf, NULL);
      break;
    case formGadgetHandleEventCmd:
      break;
  }

  return true;
}

static void paintBitmap(UInt16 id, Coord x, Coord y) {
  MemHandle h;
  BitmapType *bmp;

  if ((h = DmGet1Resource(bitmapRsc, id)) != NULL) {
    if ((bmp = MemHandleLock(h)) != NULL) {
      WinPaintBitmap(bmp, x, y);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }
}

static Boolean paletteGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  EventType *event;
  FormType *frm;
  ListType *lst;
  BitmapType *bmp;
  RectangleType rect;
  RGBColorType rgb, oldf;
  IndexedColorType color;
  ColorTableType *colorTable;
  PatternType oldp;
  bmp_edit_t *data;
  UInt16 index, depth;
  Int16 i;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, paletteGad);
  FrmGetObjectBounds(frm, index, &rect);
  data = (bmp_edit_t *)FrmGetGadgetData(frm, index);
  oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  rgb.r = rgb.g = rgb.b = 0xff;
  WinSetForeColorRGB(&rgb, &oldf);
  bmp = data->bmps[data->index];
  depth = BmpGetBitDepth(bmp);

  switch (cmd) {
    case formGadgetDrawCmd:
      if (data->transpPixel) {
        paintBitmap(transpBmp, rect.topLeft.x, rect.topLeft.y);
      } else {
        switch (depth) {
          case  1:
            rgb.r = rgb.g = rgb.b = gray1[data->color1];
            break;
          case  2:
            rgb.r = rgb.g = rgb.b = gray2[data->color2];
            break;
          case  4:
            rgb.r = rgb.g = rgb.b = gray4[data->color4];
            break;
          case  8:
            colorTable = BmpGetColortable(bmp);
            if (colorTable == NULL) {
              colorTable = pumpkin_defaultcolorTable();
            }
            rgb.r = colorTable->entry[data->color8].r;
            rgb.g = colorTable->entry[data->color8].g;
            rgb.b = colorTable->entry[data->color8].b;
            break;
          case 16:
            rgb.r = data->rgb.r;
            rgb.g = data->rgb.g;
            rgb.b = data->rgb.b;
            break;
        }
        WinSetForeColorRGB(&rgb, NULL);
        WinPaintRectangle(&rect, 0);
        rgb.r = rgb.g = rgb.b = LIGHT_GRAY;
        WinSetForeColorRGB(&rgb, NULL);
        WinPaintLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y);
        WinPaintLine(rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
        WinPaintLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1);
        WinPaintLine(rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
      }
      break;
    case formGadgetEraseCmd:
      WinPaintRectangle(&rect, 0);
      break;
    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        switch (depth) {
          case  1:
            data->popup = true;
            index = FrmGetObjectIndex(frm, paletteLst);
            lst = (ListType *)FrmGetObjectPtr(frm, index);
            LstSetSelection(lst, data->color1);
            i = LstPopupList(lst);
            if (i != -1) data->color1 = i;
            paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
            data->popup = false;
            break;
          case  2:
            data->popup = true;
            index = FrmGetObjectIndex(frm, paletteLst);
            lst = (ListType *)FrmGetObjectPtr(frm, index);
            LstSetSelection(lst, data->color2);
            i = LstPopupList(lst);
            if (i != -1) data->color2 = i;
            paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
            data->popup = false;
            break;
          case  4:
            data->popup = true;
            index = FrmGetObjectIndex(frm, paletteLst);
            lst = (ListType *)FrmGetObjectPtr(frm, index);
            LstSetSelection(lst, data->color4);
            i = LstPopupList(lst);
            if (i != -1) data->color4 = i;
            paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
            data->popup = false;
            break;
          case  8:
            color = data->color8;
            if (UIPickColor(&color, NULL, UIPickColorStartPalette, "Choose Color", NULL)) {
              data->color8 = color;
              paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
            }
            break;
          case 16:
            rgb.r = data->rgb.r;
            rgb.g = data->rgb.g;
            rgb.b = data->rgb.b;
            if (UIPickColor(NULL, &rgb, UIPickColorStartRGB, "Choose Color", NULL)) {
              data->rgb.r = rgb.r;
              data->rgb.g = rgb.g;
              data->rgb.b = rgb.b;
              paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
            }
          break;
        }
      }
      break;
  }

  WinSetForeColorRGB(&oldf, NULL);
  WinSetPatternType(oldp);

  return true;
}

static Boolean toolsGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  EventType *event;
  FormType *frm;
  ListType *lst;
  //MemHandle h;
  //BitmapType *bmp;
  RectangleType rect;
  RGBColorType rgb, oldf;
  PatternType oldp;
  bmp_edit_t *data;
  UInt16 index, id;
  Int16 i;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, toolsGad);
  FrmGetObjectBounds(frm, index, &rect);
  data = (bmp_edit_t *)FrmGetGadgetData(frm, index);
  oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  rgb.r = rgb.g = rgb.b = 0xff;
  WinSetForeColorRGB(&rgb, &oldf);

  switch (cmd) {
    case formGadgetDrawCmd:
      WinPaintRectangle(&rect, 0);
      switch (data->mode) {
        case dragMode: id = moveBmp; break;
        case drawMode: id = drawBmp; break;
        case pickMode: id = pickBmp; break;
        default: id = 0; break;
      }
      paintBitmap(id, rect.topLeft.x, rect.topLeft.y);
      break;
    case formGadgetEraseCmd:
      WinPaintRectangle(&rect, 0);
      break;
    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        data->popup = true;
        index = FrmGetObjectIndex(frm, toolsLst);
        lst = (ListType *)FrmGetObjectPtr(frm, index);
        LstSetSelection(lst, data->mode);
        i = LstPopupList(lst);
        if (i != -1) {
          data->mode = i;
          toolsGadgetCallback(gad, formGadgetDrawCmd, NULL);
        }
        data->popup = false;
      }
      break;
  }

  WinSetForeColorRGB(&oldf, NULL);
  WinSetPatternType(oldp);

  return true;
}

static void getLabel(bmp_edit_t *data) {
  BitmapType *bmp;
  BitmapTypeV0 *bmpV0;
  BitmapTypeV1 *bmpV1;
  BitmapTypeV2 *bmpV2;
  BitmapTypeV3 *bmpV3;

  bmp = data->bmps[data->index];

  switch (bmp->version) {
    case 0:
      bmpV0 = (BitmapTypeV0 *)bmp;
      snprintf(data->title, MAX_TITLE-1, "%s: V0 %dx%d 1b", data->prefix, bmpV0->width, bmpV0->height);
      break;
    case 1:
      bmpV1 = (BitmapTypeV1 *)bmp;
      snprintf(data->title, MAX_TITLE-1, "%s: V1 %dx%d %db", data->prefix, bmpV1->width, bmpV1->height, bmpV1->pixelSize);
      break;
    case 2:
      bmpV2 = (BitmapTypeV2 *)bmp;
      snprintf(data->title, MAX_TITLE-1, "%s: V2 %dx%d %db", data->prefix, bmpV2->width, bmpV2->height, bmpV2->pixelSize);
      break;
    case 3:
      bmpV3 = (BitmapTypeV3 *)bmp;
      snprintf(data->title, MAX_TITLE-1, "%s: V3 %dx%d %db (%d)", data->prefix, bmpV3->width, bmpV3->height, bmpV3->pixelSize, bmpV3->density);
      break;
    default:
      snprintf(data->title, MAX_TITLE-1, "%s: V?", data->prefix);
      break;
  }
}

static void paletteListDraw(Int16 itemNum, RectangleType *bounds, Char **itemsText) {
  RGBColorType rgb, oldf;
  UInt8 *gray;

  gray = (UInt8 *)itemsText;
  rgb.r = rgb.g = rgb.b = gray[itemNum];
  WinSetForeColorRGB(&rgb, &oldf);
  WinPaintRectangle(bounds, 0);
  WinSetForeColorRGB(&oldf, NULL);
}

static void toolsListDraw(Int16 itemNum, RectangleType *bounds, Char **itemsText) {
  UInt16 id;

  switch (itemNum) {
    case 0: id = moveBmp; break;
    case 1: id = drawBmp; break;
    case 2: id = pickBmp; break;
    default: id = 0; break;
  }

  paintBitmap(id, bounds->topLeft.x, bounds->topLeft.y);
}

static Boolean eventHandler(EventType *event) {
  FormType *frm;
  FormGadgetTypeInCallback *gad;
  ControlType *ctl;
  ListType *lst;
  RectangleType rect;
  BitmapType *bmp;
  FontID old;
  RGBColorType rgb;
  UInt16 index, ctlIndex, depth;
  UInt32 c, transparentValue;
  bmp_edit_t *data;
  int x, y, i, j, i0, j0;
  Boolean changed, handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, bitmapGad);
      data = (bmp_edit_t *)FrmGetGadgetData(frm, index);
      depth = BmpGetBitDepth(data->bmps[data->index]);

      index = FrmGetObjectIndex(frm, prevCtl);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      ctl->attr.frame = noButtonFrame;
      FrmHideObject(frm, index);

      index = FrmGetObjectIndex(frm, nextCtl);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      ctl->attr.frame = noButtonFrame;
      if (data->numBmps == 1) {
        FrmHideObject(frm, index);
      }

      index = FrmGetObjectIndex(frm, paletteGad);
      FrmGetObjectBounds(frm, index, &rect);
      index = FrmGetObjectIndex(frm, paletteLst);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      lst->font = monoFont1;
      LstSetDrawFunction(lst, paletteListDraw);
      switch (depth) {
        case  1: LstSetListChoices(lst, (char **)gray1,  2); break;
        case  2: LstSetListChoices(lst, (char **)gray2,  4); break;
        case  4: LstSetListChoices(lst, (char **)gray4, 16); break;
      }
      old = FntSetFont(lst->font);
      LstSetPosition(lst, rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1 - LstGetNumberOfItems(lst) * FntCharHeight());
      FntSetFont(old);

      index = FrmGetObjectIndex(frm, toolsGad);
      FrmGetObjectBounds(frm, index, &rect);
      index = FrmGetObjectIndex(frm, toolsLst);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      lst->font = monoFont2;
      LstSetDrawFunction(lst, toolsListDraw);
      LstSetListChoices(lst, NULL, 3);
      old = FntSetFont(lst->font);
      LstSetPosition(lst, rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1 - LstGetNumberOfItems(lst) * FntCharHeight());
      FntSetFont(old);

      FrmDrawForm(frm);
      getLabel(data);
      FrmSetTitle(frm, data->title);
      handled = true;
      break;

    case penDownEvent:
    case penMoveEvent:
    case penUpEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, bitmapGad);
      data = (bmp_edit_t *)FrmGetGadgetData(frm, index);
      RctSetRectangle(&rect, data->x, data->y, data->w-1, data->h-1);
      if (!data->popup && RctPtInRectangle(event->screenX, event->screenY, &rect)) {
        x = event->screenX - rect.topLeft.x;
        y = event->screenY - rect.topLeft.y;
        i = y / data->dh;
        j = x / data->dw;
        switch (event->eType) {
          case penDownEvent:
            debug(DEBUG_INFO, "Launcher", "edit bmp penDown %d,%d pixel %d,%d mode %d", x, y, j, i, data->mode);
            data->iDown = i;
            data->jDown = j;
            data->down = true;
            break;
          case penMoveEvent:
            if (data->down) {
              switch (data->mode) {
                case drawMode:
                  if (i != data->iDown || j != data->jDown) {
                    debug(DEBUG_INFO, "Launcher", "edit bmp penMove draw %d,%d pixel %d,%d", x, y, j, i);
                    paintPixel(data, j, i);
                    data->dirty = true;
                    data->iDown = i;
                    data->jDown = j;
                    data->drag = true;
                  }
                  break;
                case dragMode:
                  if (data->truncated && (i != data->iDown || j != data->jDown)) {
                    bmp = data->bmps[data->index];
                    i0 = data->i0 + (data->iDown - i);
                    j0 = data->j0 + (data->jDown - j);
                    if (i0 < 0) i0 = 0;
                    if (j0 < 0) j0 = 0;
                    if (i0 >= bmp->height - data->height) i0 = bmp->height - data->height;
                    if (j0 >= bmp->width - data->width) j0 = bmp->width - data->width;
                    if (i0 != data->i0 || j0 != data->j0) {
                      debug(DEBUG_INFO, "Launcher", "edit bmp penMove drag %d,%d pixel %d,%d origin %d,%d", x, y, j, i, j0, i0);
                      data->i0 = i0;
                      data->j0 = j0;
                      data->iDown = i;
                      data->jDown = j;
                      data->drag = true;
                      index = FrmGetObjectIndex(frm, bitmapGad);
                      gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, index);
                      bitmapGadgetCallback(gad, formGadgetDrawCmd, NULL);
                    }
                  }
                  break;
                default:
                  break;
              }
            }
            break;
          case penUpEvent:
            debug(DEBUG_INFO, "Launcher", "edit bmp penUp %d,%d pixel %d,%d mode %d", x, y, j, i, data->mode);
            if (data->down && !data->drag) {
              switch (data->mode) {
                case drawMode:
                  paintPixel(data, j, i);
                  data->dirty = true;
                  break;
                case pickMode:
                  index = FrmGetObjectIndex(frm, paletteGad);
                  gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, index);
                  bmp = data->bmps[data->index];
                  depth = BmpGetBitDepth(bmp);
                  getBit(bmp, data->j0 + j, data->i0 + i, &c, &rgb);
                  switch (depth) {
                    case  1:
                      data->color1 = c;
                      paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
                      break;
                    case  2:
                      data->color2 = c;
                      paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
                      break;
                    case  4:
                      data->color4 = c;
                      paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
                      break;
                    case  8:
                      data->color8 = c;
                      data->transpPixel = BmpGetTransparentValue(bmp, &transparentValue) && transparentValue == c;
                      paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
                      break;
                    case 16:
                      data->rgb.r = rgb.r;
                      data->rgb.g = rgb.g;
                      data->rgb.b = rgb.b;
                      data->transpPixel = BmpGetTransparentValue(bmp, &transparentValue) && transparentValue == c;
                      paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
                      break;
                  }
                  break;
                default:
                  break;
              }
            }
            data->down = false;
            data->drag = false;
            break;
          default:
            break;
        }
        handled = (data->mode == drawMode || data->mode == dragMode);
      }
      break;

    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, bitmapGad);
      data = (bmp_edit_t *)FrmGetGadgetData(frm, index);

      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          data->stop = true;
          handled = true;
          break;
        case nextCtl:
        case prevCtl:
          changed = false;
          if (event->data.ctlSelect.controlID == nextCtl) {
            if (data->index < data->numBmps-1) {
              changed = true;
              data->index++;
              ctlIndex = FrmGetObjectIndex(frm, nextCtl);
              ctl = (ControlType *)FrmGetObjectPtr(frm, ctlIndex);
              CtlSetValue(ctl, 0);
              if (data->index == data->numBmps-1) {
                FrmHideObject(frm, ctlIndex);
              }
              ctlIndex = FrmGetObjectIndex(frm, prevCtl);
              ctl = (ControlType *)FrmGetObjectPtr(frm, ctlIndex);
              FrmShowObject(frm, ctlIndex);
              CtlSetValue(ctl, 0);
            }
          } else {
            if (data->index > 0) {
              changed = true;
              data->index--;
              ctlIndex = FrmGetObjectIndex(frm, prevCtl);
              ctl = (ControlType *)FrmGetObjectPtr(frm, ctlIndex);
              CtlSetValue(ctl, 0);
              if (data->index == 0) {
                ctlIndex = FrmGetObjectIndex(frm, prevCtl);
                FrmHideObject(frm, ctlIndex);
              }
              ctlIndex = FrmGetObjectIndex(frm, nextCtl);
              ctl = (ControlType *)FrmGetObjectPtr(frm, ctlIndex);
              FrmShowObject(frm, ctlIndex);
              CtlSetValue(ctl, 0);
            }
          }

          if (changed) {
            data->i0 = 0;
            data->j0 = 0;

            depth = BmpGetBitDepth(data->bmps[data->index]);
            index = FrmGetObjectIndex(frm, paletteGad);
            gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, index);
            paletteGadgetCallback(gad, formGadgetDrawCmd, NULL);
            FrmGetObjectBounds(frm, index, &rect);
            index = FrmGetObjectIndex(frm, paletteLst);
            lst = (ListType *)FrmGetObjectPtr(frm, index);
            switch (depth) {
              case  1: LstSetListChoices(lst, (char **)gray1,  2); break;
              case  2: LstSetListChoices(lst, (char **)gray2,  4); break;
              case  4: LstSetListChoices(lst, (char **)gray4, 16); break;
            }
            old = FntSetFont(lst->font);
            LstSetPosition(lst, rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1 - LstGetNumberOfItems(lst) * FntCharHeight());
            FntSetFont(old);

            index = FrmGetObjectIndex(frm, bitmapGad);
            gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, index);
            bitmapGadgetCallback(gad, formGadgetDrawCmd, NULL);
            getLabel(data);
            FrmSetTitle(frm, data->title);
          }
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

Boolean editBitmap(FormType *frm, char *title, MemHandle h) {
  bmp_edit_t data;
  FormType *previous;
  BitmapType *bmp, *aux;
  EventType event;
  UInt16 index;
  Err err;
  Boolean r = false;

  if ((bmp = (BitmapType *)MemHandleLock(h)) != NULL) {
    MemSet(&data, sizeof(bmp_edit_t), 0);
    data.frm = frm;
    data.handle = h;
    data.mode = dragMode;
    data.rgb.r = data.rgb.g = data.rgb.b = 0xff;
    data.prefix = title;

    for (aux = bmp; aux; aux = BmpGetNextBitmapAnyDensity(aux)) {
      data.numBmps++;
    }

    data.bmps = xcalloc(data.numBmps, sizeof(BitmapType *));
    for (aux = bmp, index = 0; aux; aux = BmpGetNextBitmapAnyDensity(aux), index++) {
      data.bmps[index] = aux;
    }

    index = FrmGetObjectIndex(frm, bitmapGad);
    FrmSetGadgetHandler(frm, index, bitmapGadgetCallback);
    FrmSetGadgetData(frm, index, &data);

    index = FrmGetObjectIndex(frm, paletteGad);
    FrmSetGadgetHandler(frm, index, paletteGadgetCallback);
    FrmSetGadgetData(frm, index, &data);

    index = FrmGetObjectIndex(frm, toolsGad);
    FrmSetGadgetHandler(frm, index, toolsGadgetCallback);
    FrmSetGadgetData(frm, index, &data);

    FrmSetEventHandler(frm, eventHandler);
    previous = FrmGetActiveForm();
    FrmSetActiveForm(frm);
    MemSet(&event, sizeof(EventType), 0);
    event.eType = frmOpenEvent;
    event.data.frmOpen.formID = frm->formId;
    FrmDispatchEvent(&event);

    do {
      EvtGetEvent(&event, 500);
      if (SysHandleEvent(&event)) continue;
      if (MenuHandleEvent(NULL, &event, &err)) continue;
      if (eventHandler(&event)) continue;
      FrmDispatchEvent(&event);
    } while (event.eType != appStopEvent && !data.stop);

    xfree(data.bmps);
    FrmEraseForm(frm);
    FrmSetActiveForm(previous);
    MemHandleUnlock(h);
    r = data.dirty;
  }

  return r;
}
