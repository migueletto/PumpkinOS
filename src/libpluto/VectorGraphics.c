#include <PalmOS.h>

#include "VectorGraphics.h"
#include "plutosvg.h"
#include "pumpkin.h"
#include "rgb.h"
#include "debug.h"

struct VectorGraphicsType {
  plutovg_document_t *document;
  plutovg_surface_t *surface;
  UInt32 width, height;
  double dpi;
};

static void resize(VectorGraphicsType *vg, double factor) {
  double width, height;

  plutosvg_document_size(vg->document, &width, &height);
  vg->width = (UInt32)(factor * width + 0.5);
  vg->height = (UInt32)(factor * height + 0.5);
  if (vg->surface) plutovg_surface_destroy(vg->surface);
  vg->surface = NULL;
}

VectorGraphicsType *VgCreate(char *s, UInt32 size) {
  VectorGraphicsType *vg;

  if ((vg = MemPtrNew(sizeof(VectorGraphicsType))) != NULL) {
    if ((vg->document = plutosvg_parse_from_memory(s, size)) != NULL) {
      vg->dpi = 96.0;
      plutosvg_document_prepare(vg->document, vg->dpi);
      resize(vg, 1.0);
    }
  }

  return vg;
}

void VgSize(VectorGraphicsType *vg, double *width, double *height) {
  if (vg) {
    *width = vg->width;
    *height = vg->height;
  }
}

static void setBit(int x, int y, int red, int green, int blue, int alpha, void *data) {
  PointType *point = (PointType *)data;
  RGBColorType rgb, old;

  if (alpha > 0) {
    rgb.r = red;
    rgb.g = green;
    rgb.b = blue;
    WinSetForeColorRGB(&rgb, &old);
    WinPaintPixel(point->x + x, point->y + y);
    WinSetForeColorRGB(&old, NULL);
  }
}

void VgRender(VectorGraphicsType *vg, Coord x, Coord y) {
  PointType point;

  if (vg) {
    if (vg->surface == NULL) {
      vg->surface = plutovg_surface_create(vg->width, vg->height);
    }
    point.x = x;
    point.y = y;
    plutovg_surface_clear(vg->surface);
    plutosvg_document_render(vg->document, vg->surface, NULL, vg->dpi);
    plutovg_surface_write_to_memory(vg->surface, setBit, &point);
  }
}

void VgScale(VectorGraphicsType *vg, double factor) {
  if (vg && factor > 0) {
    plutosvg_document_reset(vg->document);
    plutosvg_document_scale(vg->document, factor);
    resize(vg, factor);
  }
}

void VgRotate(VectorGraphicsType *vg, double angle, double cx, double cy) {
  if (vg) {
    plutosvg_document_reset(vg->document);
    plutosvg_document_rotate(vg->document, angle, cx ,cy);
  }
}

void VgFreeze(VectorGraphicsType *vg) {
  if (vg) {
    plutosvg_document_freeze(vg->document);
  }
}

void VgDestroy(VectorGraphicsType *vg) {
  if (vg) {
    if (vg->surface) plutovg_surface_destroy(vg->surface);
    if (vg->document) plutosvg_document_destroy(vg->document);
    MemPtrFree(vg);
  }
}
