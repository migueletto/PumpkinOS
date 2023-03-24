#include <PalmOS.h>

#include "map.h"
#include "fill.h"

#define MAXVERTICAL 1024
#define SGN(x) ((x) < 0 ? -1 : 1)

typedef struct Edge {
  struct Edge *next;
  Int32 yTop, yBot;
  Int32 xNowWhole, xNowNum, xNowDen, xNowDir;
  Int32 xNowNumStep;
} Edge;

static Edge *edgeTable[MAXVERTICAL];
static Int32 xCoords[MAXVERTICAL];

static void FillEdges(Int32 n, MapICoordType *p)
{
  Int32 i, j;
  MapICoordType *p1, *p2, *p3;
  Edge *e;

  for (i = 0; i < n; i++) {
    p1 = &p[i];
    p2 = &p[(i + 1) % n];

    if (p1->y == p2->y)
      continue;   // Skip horiz. edges

    // Find next vertex not level with p2
    for (j = (i + 2) % n; ; j = (j + 1) % n) {
      p3 = &p[j];
      if (p2->y != p3->y)
        break;
    }

    e = MemPtrNew(sizeof(Edge));
    e->xNowNumStep = p1->x - p2->x;
    if (e->xNowNumStep < 0) e->xNowNumStep = -e->xNowNumStep;

    if (p2->y > p1->y) {
      e->yTop = p1->y;
      e->yBot = p2->y;
      e->xNowWhole = p1->x;
      e->xNowDir = SGN(p2->x - p1->x);
      e->xNowDen = e->yBot - e->yTop;
      e->xNowNum = (e->xNowDen >> 1);
      if (p3->y > p2->y)
        e->yBot--;
    } else {
      e->yTop = p2->y;
      e->yBot = p1->y;
      e->xNowWhole = p2->x;
      e->xNowDir = SGN(p1->x - p2->x);
      e->xNowDen = e->yBot - e->yTop;
      e->xNowNum = (e->xNowDen >> 1);
      if (p3->y < p2->y) {
        e->yTop++;
        e->xNowNum += e->xNowNumStep;
        while (e->xNowNum >= e->xNowDen) {
          e->xNowWhole += e->xNowDir;
          e->xNowNum -= e->xNowDen;
        }
      }
    }
    e->next = edgeTable[e->yTop];
    edgeTable[e->yTop] = e;
  }
}

// UpdateActive first removes any edges which curY has entirely
// passed by.  The removed edges are freed.
// It then removes any edges from the edge table at curY and
// places them on the active list.

static Edge *UpdateActive(Edge *active, Int32 curY)
{
  Edge *e, **ep;

  for (ep = &active, e = *ep; e != NULL; e = *ep) {
    if (e->yBot < curY) {
      *ep = e->next;
      MemPtrFree(e);
    } else
      ep = &e->next;
  }

  *ep = edgeTable[curY];
  return active;
}

// DrawRuns first uses an insertion sort to order the X
// coordinates of each active edge.  It updates the X coordinates
// for each edge as it does this.
// Then it draws a run between each pair of coordinates,
// using the specified fill pattern.
// This routine is very slow and it would not be that
// difficult to speed it way up.

static void DrawRuns(Edge *active, Int32 curY)
{
  Edge *e;
  Int32 i, numCoords = 0;

  for (e = active; e != NULL; e = e->next) {
    for (i = numCoords; i > 0 && xCoords[i - 1] > e->xNowWhole; i--)
      xCoords[i] = xCoords[i - 1];

    xCoords[i] = e->xNowWhole;
    numCoords++;
    e->xNowNum += e->xNowNumStep;

    while (e->xNowNum >= e->xNowDen) {
      e->xNowWhole += e->xNowDir;
      e->xNowNum -= e->xNowDen;
    }
  }

  if (numCoords % 2)  // Protect from degenerate polygons
    xCoords[numCoords] = xCoords[numCoords - 1], numCoords++;

  for (i = 0; i < numCoords; i += 2) {
    // All we need is to draw a horizontal line
    // from (xCoords[i], curY) to (xCoords[i + 1], curY)

    WinDrawLine(xCoords[i], curY, xCoords[i+1], curY);
  }
}

// This version of the fill takes a fill pattern argument
// It may be removed throughout for straight single-color fills

void MapFillPolygon(Int32 n, MapICoordType *p)
{
  Int32 i, curY;
  Edge *active;

  if (n < 3)
    return;

  for (i = 0; i < MAXVERTICAL; i++)
    edgeTable[i] = NULL;

  FillEdges(n, p);

  for (curY = 0; edgeTable[curY] == NULL; curY++)
    if (curY == MAXVERTICAL - 1)
      return;     // No edges in polygon

  for (active = NULL;
       (active = UpdateActive(active, curY)) != NULL; curY++)
    DrawRuns(active, curY);
}
