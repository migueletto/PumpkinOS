#include <PalmOS.h>

#include "resource.h"
#include "editsurf.h"
#include "debug.h"

typedef struct {
  BitmapType *bmp;
  Coord x, y;
} surface_edit_t;

static Boolean surfaceGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  RectangleType rect;
  surface_edit_t *data;
  UInt16 index;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, surfaceGad);
  FrmGetObjectBounds(frm, index, &rect);
  data = (surface_edit_t *)FrmGetGadgetData(frm, index);

  switch (cmd) {
    case formGadgetDrawCmd:
      if (data->bmp) {
        WinPaintBitmap(data->bmp, data->x, data->y);
      }
      break;
    case formGadgetHandleEventCmd:
      break;
  }

  return true;
}

static Boolean eventHandler(EventType *event) {
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

Boolean editSurface(FormType *frm, char *title, surface_t *surface) {
  WinHandle wh;
  BitmapType *bmp;
  RectangleType rect;
  surface_t *s;
  surface_edit_t data;
  UInt16 index, depth, width, height;
  void *bits, *buffer;
  int len;
  Err error;

  wh = WinGetDisplayWindow();
  bmp = WinGetBitmap(wh);
  depth = BmpGetBitDepth(bmp);
  index = FrmGetObjectIndex(frm, surfaceGad);
  FrmGetObjectBounds(frm, index, &rect);

  if (surface->width <= rect.extent.x && surface->height <= rect.extent.y) {
    width = surface->width;
    height = surface->height;
    s = surface;
  } else if (surface->width <= rect.extent.x) {
    height = rect.extent.y;
    width = (surface->width * height) / surface->height;
    s = NULL;
  } else if (surface->height <= rect.extent.y) {
    width = rect.extent.x;
    height = (surface->height * width) / surface->width;
    s = NULL;
  } else if (surface->width <= surface->height) {
    height = rect.extent.y;
    width = (surface->width * height) / surface->height;
    s = NULL;
  } else {
    width = rect.extent.x;
    height = (surface->height * width) / surface->width;
    s = NULL;
  }

  if (!s) {
    s = surface_create(width, height, SURFACE_ENCODING_RGB565);
    surface_scale(surface, s);
  }

  if (s) {
    MemSet(&data, sizeof(surface_edit_t), 0);
    data.bmp = BmpCreate3(width, height, 0, kDensityLow, depth, false, 0, NULL, &error);

    if (data.bmp) {
      data.x = rect.topLeft.x + (rect.extent.x - width) / 2;
      data.y = rect.topLeft.y + (rect.extent.y - height) / 2;

      FrmSetGadgetHandler(frm, index, surfaceGadgetCallback);
      FrmSetGadgetData(frm, index, &data);
      FrmSetEventHandler(frm, eventHandler);
      FrmSetTitle(frm, title);

      BmpSetLittleEndianBits(data.bmp, true);
      bits = BmpGetBits(data.bmp);
      buffer = surface_buffer(s, &len);
      MemMove(bits, buffer, len);

      FrmDoDialog(frm);
      BmpDelete(data.bmp);
    }
    if (s != surface) surface_destroy(s);
  }

  return false;
}
