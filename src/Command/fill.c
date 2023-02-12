#include <PalmOS.h>

#include "fill.h"
#include "color.h"
#include "thread.h"
#include "xalloc.h"

typedef struct {
  Int16 x0, y0;
  Int16 x1, y1;
} FillWindow;

// Filled horizontal segment of scanline y for xl<=x<=xr.
// Parent segment was on line y-dy.  dy=1 or -1

typedef struct {
  Int16 y, xl, xr, dy;
} FillSegment;

#define MAX_STACK 4096	// max depth of stack

#define PUSH(Y, XL, XR, DY) \
    if (sp<stack+MAX_STACK && Y+(DY)>=win->y0 && Y+(DY)<=win->y1) \
    {sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define POP(Y, XL, XR, DY) \
    {sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

static UInt32 ReadPixel(Int16 x, Int16 y) {
  RGBColorType rgb;

  WinGetPixelRGB(x, y, &rgb);

  return RGBToLong(&rgb);
}

static UInt32 SetColor(UInt32 c) {
  RGBColorType rgb, old;

  LongToRGB(c, &rgb);
  WinSetForeColorRGB(&rgb, &old);

  return RGBToLong(&old);
}

static void WritePixel(Int16 x, Int16 y) {
  WinPaintPixel(x, y);
}

Err SeedFill(Int16 x, Int16 y, RectangleType *rect, RGBColorType *c) {
  UInt32 fillColor, areaColor, oldColor;
  Int16 l, x1, x2, dy;
  FillWindow window, *win;
  FillSegment *stack, *sp;
  WinHandle wh, old;
  UInt32 tmp;
  Err err;

  if ((wh = WinCreateOffscreenWindow(16, 16, nativeFormat, &err)) == NULL) {
    return -1;
  }
  old = WinSetDrawWindow(wh);

  fillColor = RGBToLong(c);
  tmp = SetColor(fillColor);
  WritePixel(0, 0);
  fillColor = ReadPixel(0, 0);
  SetColor(tmp);

  WinSetDrawWindow(old);
  WinDeleteWindow(wh, false);

  win = &window;
  win->x0 = rect->topLeft.x;
  win->y0 = rect->topLeft.y;
  win->x1 = rect->topLeft.x + rect->extent.x - 1;
  win->y1 = rect->topLeft.y + rect->extent.y - 1;

  areaColor = ReadPixel(x, y);

  if (areaColor == fillColor || x < win->x0 || x > win->x1 || y < win->y0 || y > win->y1) {
    return 0;
  }

  if ((stack = xcalloc(1, 2 * MAX_STACK * sizeof(FillSegment))) == NULL) {
    return -1;
  }

  sp = stack;

  oldColor = SetColor(fillColor);

  PUSH(y, x, x, 1);	// needed in some cases
  PUSH(y+1, x, x, -1);	// seed segment (popped 1st)

  while (sp > stack && sp < stack+MAX_STACK) {
    if (thread_must_end()) break;

    // pop segment off stack and fill a neighboring scan line
    POP(y, x1, x2, dy);

    // segment of scan line y-dy for x1<=x<=x2 was previously filled,
    // now explore adjacent pixels in scan line y

    for (x = x1; x >= win->x0 && ReadPixel(x, y) == areaColor; x--) {
      WritePixel(x, y);
    }

    if (x >= x1) {
      goto skip;
    }
    l = x+1;

    if (l < x1) {
      PUSH(y, l, x1-1, -dy);	// leak on left ?
    }
    x = x1+1;

    do {
      for (; x <= win->x1 && ReadPixel(x, y) == areaColor; x++) {
        WritePixel(x, y);
      }

      PUSH(y, l, x-1, dy);

      if (x > x2+1) {
        PUSH(y, x2+1, x-1, -dy);	// leak on right ?
      }

skip:
      for (x++; x <= x2 && ReadPixel(x, y) != areaColor; x++);
      l = x;
    } while (x <= x2 && sp < stack+MAX_STACK);
  }

  SetColor(oldColor);
  xfree(stack);

  return 0;
}
