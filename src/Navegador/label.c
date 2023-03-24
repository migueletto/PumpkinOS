#include <PalmOS.h>
#include <BmpGlue.h>

#include "map.h"
#include "gpc.h"
#include "trig.h"
#include "font.h"
#include "label.h"
#include "format.h"
#include "ndebug.h"

static Int16 nlabels;
static MapLabelType *label;

void LabelInit(void)
{
  nlabels = 0;
  label = MemPtrNew(MAX_LABELS * sizeof(MapLabelType));
}

int LabelAdd(Int16 type, Int16 x, Int16 y, FontID font, UInt16 color,
             float angle, char *s)
{
  if (nlabels == MAX_LABELS || label == NULL)
    return -1;

  label[nlabels].type = type;
  label[nlabels].x = x;
  label[nlabels].y = y;
  label[nlabels].font = font;
  label[nlabels].color = color;
  label[nlabels].label = s;
  label[nlabels].angle = angle;

  label[nlabels].layer = 0;	// not used

  nlabels++;
  return 0;
}

void LabelFinish(Boolean highDensity)
{
  Int16 i, x, y, dx, dy, dx1, dy1, len, angle, ok;
  BitmapPtr bmp;
  gpc_polygon *p;

  if (label == NULL)
    return;

  WinPushDrawState();
  WinSetDrawMode(winOverlay);

  for (i = nlabels-1, p = NULL; i >= 0; i--) {
    switch (label[i].type) {
      case -1:
        bmp = (BitmapPtr)label[i].label;
        BmpGlueGetDimensions(bmp, &dx, &dy, NULL);
        if (WinGetCoordinateSystem() == kCoordinatesDouble) {
          dx <<= 1;
          dy <<= 1;
        }

        p = LabelBox(p, label[i].x, label[i].y, dx, dy, &ok);

        if (ok)
          WinDrawBitmap(bmp, label[i].x, label[i].y);
        break;
      case MAP_POINT:
        FntSetFont(label[i].font);
        len = StrLen(label[i].label);
        dy = FntCharHeight();
        dx = FntCharsWidth(label[i].label, len);

        p = LabelBox(p, label[i].x, label[i].y, dx, dy, &ok);

        if (ok) {
          //RectangleType rect;
          //RctSetRectangle(&rect, label[i].x, label[i].y, dx, dy);
          //WinSetForeColor(label[i].color);
          //WinDrawRectangleFrame(simpleFrame, &rect);

          WinSetTextColor(label[i].color);
          WinPaintChars(label[i].label, len, label[i].x, label[i].y);
        }
        break;
      case MAP_PLINE:
        FontStringSize(label[i].label, &dx, &dy);

        angle = (Int16)TODEG(label[i].angle);
        if (angle < -90) angle = -90;
        else if (angle > 90) angle = 90;

        dx1 = dx * cos_table[angle];

        if (angle >= 0)
          dy1 = dx * sin_table[angle];
        else
          dy1 = -dx * sin_table[-angle];

        if (dy1 < 0) dy1 = -dy1;

        if (dy1 < dx1) {
          if (dy1 < dy) dy1 = dy;
        } else {
          if (dx1 < dy) dx1 = dy;
        }

        x = label[i].x - dx1/2;
        y = label[i].y - dy1/2;

        p = LabelBox(p, x, y, dx1, dy1, &ok);

        if (ok) {
          if (angle > 0) y += dy1;
          WinSetForeColor(label[i].color);
          FontDrawString(label[i].label, x, y, angle);
        }
    }
  }

  if (p) {
    gpc_free_polygon(p);
    MemPtrFree(p);
  }

  WinPopDrawState();
  MemPtrFree(label);
}

gpc_polygon *LabelBox(gpc_polygon *p, Int16 x, Int16 y, Int16 dx, Int16 dy,
                      Int16 *ok)
{
  gpc_polygon q, *r;

  if (p == NULL) {
    *ok = 1;
    p = MemPtrNew(sizeof(gpc_polygon));
    MemSet(p, sizeof(gpc_polygon), 0);
    return p;
  }

  if (dx == 0 || dy == 0) {
    *ok = 0;
    return p;
  }

  MemSet(&q, sizeof(gpc_polygon), 0);
  q.num_contours = 1;
  q.hole = MemPtrNew(sizeof(int));
  q.hole[0] = 0;
  q.contour = MemPtrNew(sizeof(gpc_vertex_list));
  MemSet(q.contour, sizeof(gpc_vertex_list), 0);
  q.contour[0].num_vertices = 4;
  q.contour[0].vertex = MemPtrNew(4 * sizeof(gpc_vertex));
  MemSet(q.contour[0].vertex, 4 * sizeof(gpc_vertex), 0);
  q.contour[0].vertex[0].x = x;
  q.contour[0].vertex[0].y = y;
  q.contour[0].vertex[1].x = x + dx - 1;
  q.contour[0].vertex[1].y = y;
  q.contour[0].vertex[2].x = x + dx - 1;
  q.contour[0].vertex[2].y = y + dy - 1;
  q.contour[0].vertex[3].x = x;
  q.contour[0].vertex[3].y = y + dy - 1;

  r = MemPtrNew(sizeof(gpc_polygon));
  MemSet(r, sizeof(gpc_polygon), 0);
  gpc_polygon_clip(GPC_INT, p, &q, r);

  if (r->num_contours > 0) {
    gpc_free_polygon(&q);
    gpc_free_polygon(r);
    MemPtrFree(r);
    *ok = 0;
    return p;
  }

  gpc_free_polygon(r);
  gpc_polygon_clip(GPC_UNION, p, &q, r);

  gpc_free_polygon(&q);
  gpc_free_polygon(p);
  MemPtrFree(p);

  *ok = 1;
  return r;
}
