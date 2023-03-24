#include <PalmOS.h>

#include "format.h"
#include "compass.h"
#include "trig.h"
#include "MathLib.h"

void DrawCompass(Int16 x, Int16 y, Int16 r, UInt8 labels)
{
  Int16 h, g, x1, y1, x2, y2;
  double dr, dr3;

  h = FntCharHeight();
  DrawCircle(x, y, r);

  if (labels & LABEL_N) WinDrawChar('N', x-FntCharWidth('N')/2, y-r-h);
  if (labels & LABEL_S) WinDrawChar('S', x-FntCharWidth('S')/2, y+r+1);
  if (labels & LABEL_W) WinDrawChar('W', x-r-FntCharWidth('W')-1, y-h/2);
  if (labels & LABEL_E) WinDrawChar('E', x+r+3, y-h/2);

  dr = (double)r;
  dr3 = (double)(r-3);

  for (g = 0; g <= 90; g += 15) {
    x1 = dr * sin_table[g];
    y1 = dr * cos_table[g];
    x2 = dr3 * sin_table[g];
    y2 = dr3 * cos_table[g];

    WinDrawLine(x+x1, y+y1, x+x2, y+y2);
    WinDrawLine(x+x1, y-y1, x+x2, y-y2);
    WinDrawLine(x-x1, y+y1, x-x2, y+y2);
    WinDrawLine(x-x1, y-y1, x-x2, y-y2);
  }
}

void DrawArrow(Int16 x, Int16 y, Int16 r, Int16 h, double g,
               Boolean full, Boolean draw)
{
  Int16 rx, ry, cx, cy, x1, y1, x2, y2;
  double a;
  RectangleType rect;

  g = -g+90.0;

  a = TORAD(g);
  ry = sin(a)*(r-6);
  rx = cos(a)*(r-6);

  RctSetRectangle(&rect, x-1, y-1, 3, 3);
  WinDrawRectangle(&rect, 0);

  if (full) {
    if (draw) {
      WinDrawLine(x, y, x-rx, y+ry);
      WinDrawLine(x, y, x+rx, y-ry);
    } else {
      WinEraseLine(x, y, x-rx, y+ry);
      WinEraseLine(x, y, x+rx, y-ry);
    }
  }

  cx = x+rx;
  cy = y-ry;

  a = TORAD(g-135.0);
  ry = sin(a)*h;
  rx = cos(a)*h;
  x1 = cx+rx;
  y1 = cy-ry;
  if (draw)
    WinDrawLine(cx, cy, x1, y1);
  else
    WinEraseLine(cx, cy, x1, y1);

  a = TORAD(g+135.0);
  ry = sin(a)*h;
  rx = cos(a)*h;
  x2 = cx+rx;
  y2 = cy-ry;
  if (draw)
    WinDrawLine(cx, cy, x2, y2);
  else
    WinEraseLine(cx, cy, x2, y2);

  if (!full) {
    if (draw)
      WinDrawLine(x1, y1, x2, y2);
    else
      WinEraseLine(x1, y1, x2, y2);
  }
}

void DrawCircle(Int16 x, Int16 y, Int16 r)
{
  Int16 g, x1, y1, x0 = 0, y0 = 0;
  double dr;

  dr = (double)r;

  for (g = 0; g <= 90; g += 1) {
    x1 = dr * sin_table[g];
    y1 = dr * cos_table[g];
    if (g == 0) {
      x0 = x1;
      y0 = y1;
    }
    WinDrawLine(x+x0, y+y0, x+x1, y+y1);
    WinDrawLine(x+x0, y-y0, x+x1, y-y1);
    WinDrawLine(x-x0, y+y0, x-x1, y+y1);
    WinDrawLine(x-x0, y-y0, x-x1, y-y1);
    x0 = x1;
    y0 = y1;
  }
}
