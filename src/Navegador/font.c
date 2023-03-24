#include <PalmOS.h>

#include "trig.h"
#include "gui.h"
#include "font.h"

typedef struct {
  Int8 x1, y1, x2, y2;
} FontSeg;

typedef struct {
  UInt8 nsegs, dx, dy;
  FontSeg *seg;
} FontGlyph;

static Boolean hd;
static UInt16 factor;
static FontGlyph glyph[256];

static void FontDrawGlyph(FontGlyph *g, Int16 x, Int16 y, Int16 angle);

void FontInit(Boolean highDensity)
{
  hd = highDensity;
  factor = 1;
  MemSet(glyph, sizeof(glyph), 0);
}

Err FontSet(FontID f)
{
  UInt8 c, n;
  UInt16 i, j, k;
  MemHandle h;
  UInt8 *p;

  if ((h = DmGetResource('Font', lineFontId)) == NULL)
    return -1;

  if ((p = MemHandleLock(h)) == NULL) {
    DmReleaseResource(h);
    return -2;
  }

  for (i = 0;;) {
    c = p[i++];
    n = p[i++];

    if (c == 0 && n == 0)
      break;

    glyph[c].nsegs = n;
    if (glyph[c].seg == NULL)
      glyph[c].seg = MemPtrNew(n * sizeof(FontSeg));

    for (k = 0; k < n; k++) {
      glyph[c].seg[k].x1 = p[i++];
      glyph[c].seg[k].y1 = p[i++];
      glyph[c].seg[k].x2 = p[i++];
      glyph[c].seg[k].y2 = p[i++];
    }
  }

  MemHandleUnlock(h);
  DmReleaseResource(h);

  if (hd) {
    if (f == stdFont || f == boldFont)
      factor = 2;
    else
      factor = 1;
  } else {
    factor = 1;
  }

  for (i = 0; i < 256; i++) {
    glyph[i].dx = 0;
    glyph[i].dy = 0;

    for (j = 0; j < glyph[i].nsegs; j++) {
      if (hd) {
        glyph[i].seg[j].x1 *= factor;
        glyph[i].seg[j].x2 *= factor;
        glyph[i].seg[j].y1 *= factor;
        glyph[i].seg[j].y2 *= factor;
      }

      if (glyph[i].seg[j].x1 > glyph[i].dx)
        glyph[i].dx = glyph[i].seg[j].x1;
      if (glyph[i].seg[j].x2 > glyph[i].dx)
        glyph[i].dx = glyph[i].seg[j].x2;

      if (glyph[i].seg[j].y1 > glyph[i].dy)
        glyph[i].dy = glyph[i].seg[j].y1;
      if (glyph[i].seg[j].y2 > glyph[i].dy)
        glyph[i].dy = glyph[i].seg[j].y2;
    }
  }

  return 0;
}

void FontFinish(void)
{
  Int16 i;

  for (i = 0; i < 256; i++) {
    if (glyph[i].seg) {
      MemPtrFree(glyph[i].seg);
      glyph[i].seg = NULL;
    }
  }
  MemSet(glyph, sizeof(glyph), 0);
}

static void FontDrawGlyph(FontGlyph *g, Int16 x, Int16 y, Int16 angle)
{
  Int16 i, x1, x2, y1, y2;
  FontSeg *seg;
  double sin_t, cos_t;

  for (i = 0; i < g->nsegs; i++) {
    seg = &g->seg[i];

    if (angle >= 0) {
      cos_t = cos_table[angle];
      sin_t = sin_table[angle];
    } else {
      cos_t = cos_table[-angle];
      sin_t = -sin_table[-angle];
    }

    x1 = seg->x1 * cos_t - seg->y1 * sin_t;
    y1 = seg->x1 * sin_t + seg->y1 * cos_t;

    x2 = seg->x2 * cos_t - seg->y2 * sin_t;
    y2 = seg->x2 * sin_t + seg->y2 * cos_t;

    WinDrawLine(x + x1, y - y1, x + x2, y - y2);
  }
}

void FontStringSize(char *s, Int16 *dx, Int16 *dy)
{
  UInt8 c;
  Int16 i, tx, ty;

  *dx = *dy = 0;

  if (s == NULL)
    return;

  for (i = 0; i < s[i]; i++) {
    c = (UInt8)s[i];

    if (c == ' ') {
      tx = 2 * factor;
      ty = 0;
    } else {
      if (c >= 'a' && c <= 'z')
        c -= 32;

      if (glyph[c].nsegs == 0 || glyph[c].seg == NULL)
        c = 0;

      tx = glyph[c].dx;
      ty = glyph[c].dy;
    }

    if (ty > *dy)
      *dy = ty;

    if (i)
      *dx += 2 * factor;
    *dx += tx;
  }
}

void FontDrawString(char *s, Int16 x, Int16 y, Int16 angle)
{
  UInt8 c;
  Int16 i, dx, dg, x1, y1;
  double sin_t, cos_t;

  if (s == NULL)
    return;

  if (angle >= 0) {
    cos_t = cos_table[angle];
    sin_t = sin_table[angle];
  } else {
    cos_t = cos_table[-angle];
    sin_t = -sin_table[-angle];
  }

  dx = 0;

  for (i = 0; i < s[i]; i++) {
    c = (UInt8)s[i];

    if (c == ' ')
      dg = 2 * factor;
    else {
      if (c >= 'a' && c <= 'z')
        c -= 32;

      if (glyph[c].nsegs == 0 || glyph[c].seg == NULL)
        c = 0;

      x1 = x + dx * cos_t;
      y1 = y - dx * sin_t;
      FontDrawGlyph(&glyph[c], x1, y1, angle);
      dg = glyph[c].dx;
    }

    if (i)
      dx += 2 * factor;
    dx += dg;
  }
}
