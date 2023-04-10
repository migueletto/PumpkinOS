#include <PalmOS.h>

#include "debug.h"

#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

void RctSetRectangle(RectangleType *rP, Coord left, Coord top, Coord width, Coord height) {
  if (rP) {
    rP->topLeft.x = left;
    rP->topLeft.y = top;
    rP->extent.x = width;
    rP->extent.y = height;
  }
}

void RctInsetRectangle(RectangleType *rP, Coord insetAmt) {
  if (rP) {
    rP->topLeft.x += insetAmt;
    rP->topLeft.y += insetAmt;
    rP->extent.x -= 2*insetAmt;
    rP->extent.y -= 2*insetAmt;
  }
}

void RctOffsetRectangle(RectangleType *rP, Coord deltaX, Coord deltaY) {
  if (rP) {
    rP->topLeft.x += deltaX;
    rP->topLeft.y += deltaY;
  }
}

Boolean RctPtInRectangle(Coord x, Coord y, const RectangleType *rP) {
  Boolean r = false;

  if (rP) {
    r = x >= rP->topLeft.x && x < rP->topLeft.x+rP->extent.x && y >= rP->topLeft.y && y < rP->topLeft.y+rP->extent.y;
  }

  return r;
}

void RctCopyRectangle(const RectangleType *srcRectP, RectangleType *dstRectP) {
  if (srcRectP && dstRectP) {
    dstRectP->topLeft.x = srcRectP->topLeft.x;
    dstRectP->topLeft.y = srcRectP->topLeft.y;
    dstRectP->extent.x = srcRectP->extent.x;
    dstRectP->extent.y = srcRectP->extent.y;
  }
}

void RctGetIntersection(const RectangleType *r1P, const RectangleType *r2P, RectangleType *r3P) {
  Coord ax1, ax2, ay1, ay2;
  Coord bx1, bx2, by1, by2;
  Coord cx1, cx2, cy1, cy2;
  Coord aux;

  ax1 = r1P->topLeft.x;
  ax2 = r1P->topLeft.x + r1P->extent.x - 1;
  ay1 = r1P->topLeft.y;
  ay2 = r1P->topLeft.y + r1P->extent.y - 1;
  if (ax1 > ax2) {
    aux = ax1;
    ax1 = ax2;
    ax2 = aux;
  }
  if (ay1 > ay2) {
    aux = ay1;
    ay1 = ay2;
    ay2 = aux;
  }

  bx1 = r2P->topLeft.x;
  bx2 = r2P->topLeft.x + r2P->extent.x - 1;
  by1 = r2P->topLeft.y;
  by2 = r2P->topLeft.y + r2P->extent.y - 1;
  if (bx1 > bx2) {
    aux = bx1;
    bx1 = bx2;
    bx2 = aux;
  }
  if (by1 > by2) {
    aux = by1;
    by1 = by2;
    by2 = aux;
  }

  cx1 = max(ax1, bx1);
  cy1 = max(ay1, by1);
  cx2 = min(ax2, bx2);
  cy2 = min(ay2, by2);

  if (cx1 <= cx2 && cy1 <= cy2) {
    r3P->topLeft.x = cx1;
    r3P->topLeft.y = cy1;
    r3P->extent.x = cx2 - cx1 + 1;
    r3P->extent.y = cy2 - cy1 + 1;
  } else {
    r3P->topLeft.x = 0;
    r3P->topLeft.y = 0;
    r3P->extent.x = 0;
    r3P->extent.y = 0;
  }
}

void RctRectToAbs(const RectangleType *rP, AbsRectType *arP) {
  if (rP && arP) {
    arP->left = rP->topLeft.x;
    arP->right = rP->topLeft.x + rP->extent.x - 1;
    arP->top = rP->topLeft.y;
    arP->bottom = rP->topLeft.y + rP->extent.y - 1;
  }
}

void RctAbsToRect(const AbsRectType *arP, RectangleType *rP) {
  if (rP && arP) {
    rP->topLeft.x = arP->left;
    rP->extent.x = arP->right - arP->left + 1;
    rP->topLeft.y = arP->top;
    rP->extent.x = arP->bottom - arP->top + 1;
  }
}

static int contains(const RectangleType *a, const RectangleType *b) {
  return (b->topLeft.x >= a->topLeft.x) &&
         (b->topLeft.y >= a->topLeft.y) &&
         (b->topLeft.x + b->extent.x <= a->topLeft.x + a->extent.x) &&
         (b->topLeft.y + b->extent.y <= a->topLeft.y + a->extent.y);
}

static int intersects(const RectangleType *a, const RectangleType *b) {
  return !((b->topLeft.x + b->extent.x <= a->topLeft.x)  ||
           (b->topLeft.y + b->extent.y <= a->topLeft.y) ||
           (b->topLeft.x >= a->topLeft.x + a->extent.x)  ||
           (b->topLeft.y >= a->topLeft.y + a->extent.y));
}

UInt16 RctGetDifference(const RectangleType *a, const RectangleType *b, RectangleType *r) {
  UInt16 rectCount = 0;

  if (contains(b, a)) {
    return 0;
  }

  if (!intersects(a, b)) {
    MemMove(r, a, sizeof(RectangleType));
    return 1;
  }

  // compute the top rectangle
  int raHeight = b->topLeft.y - a->topLeft.y;
  if (raHeight > 0) {
    r[rectCount].topLeft.x = a->topLeft.x;
    r[rectCount].topLeft.y = a->topLeft.y;
    r[rectCount].extent.x = a->extent.x;
    r[rectCount].extent.y = raHeight;
    rectCount++;
  }

  // compute the bottom rectangle
  int rbY = b->topLeft.y + b->extent.y;
  int rbHeight = a->extent.y - (rbY - a->topLeft.y);
  if (rbHeight > 0 && rbY < a->topLeft.y + a->extent.y) {
    r[rectCount].topLeft.x = a->topLeft.x;
    r[rectCount].topLeft.y = rbY;
    r[rectCount].extent.x = a->extent.x;
    r[rectCount].extent.y = rbHeight;
    rectCount++;
  }

  int rectAYH = a->topLeft.y+a->extent.y;
  int y1 = b->topLeft.y > a->topLeft.y ? b->topLeft.y : a->topLeft.y;
  int y2 = rbY < rectAYH ? rbY : rectAYH;
  int rcHeight = y2 - y1;

  // compute the left rectangle
  int rcWidth = b->topLeft.x - a->topLeft.x;
  if (rcWidth > 0 && rcHeight > 0) {
    r[rectCount].topLeft.x = a->topLeft.x;
    r[rectCount].topLeft.y = y1;
    r[rectCount].extent.x = rcWidth;
    r[rectCount].extent.y = rcHeight;
    rectCount++;
  }

  // compute the right rectangle
  int rbX = b->topLeft.x + b->extent.x;
  int rdWidth = a->extent.x - (rbX - a->topLeft.x);
  if (rdWidth > 0) {
    r[rectCount].topLeft.x = rbX;
    r[rectCount].topLeft.y = y1;
    r[rectCount].extent.x = rdWidth;
    r[rectCount].extent.y = rcHeight;
    rectCount++;
  }

  return rectCount;
}
