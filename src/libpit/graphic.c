#include "sys.h"
#include "script.h"
#include "pwindow.h"
#include "ptr.h"
#include "media.h"
#include "pfont.h"
#include "graphic.h"
#include "debug.h"
#include "xalloc.h"

#define MAXVERTICAL 1024
#define SGN(x) ((x) < 0 ? -1 : 1)

typedef struct {
  double x1, y1, x2, y2;
} font_seg_t;

typedef struct {
  int ch, nsegs, seg0;
  double dx, dy;
} font_glyph_t;

struct graphic_vfont_t {
  double cos_table[360];
  double sin_table[360];
  font_glyph_t font[256];
  setpixel_f p;
};

typedef struct edge_t {
  struct edge_t *next;
  int yTop, yBot;
  int xNowWhole, xNowNum, xNowDen, xNowDir;
  int xNowNumStep;
} edge_t;

void graphic_line(void *data, int x1, int y1, int x2, int y2, uint32_t color, setpixel_f p, setarea_f a) {
  int i, aux, dx, dy, sx, sy, err, e2;

  if (y1 == y2) {
    if (x1 > x2) { aux = x1; x1 = x2; x2 = aux; }
    if (a) {
      a(data, x1, y1, x2, y1, color);
    } else {
      for (i = x1; i <= x2; i++) {
        p(data, i, y1, color);
      }
    }
  } else if (x1 == x2) {
    if (y1 > y2) { aux = y1; y1 = y2; y2 = aux; }
    if (a) {
      a(data, x1, y1, x1, y2, color);
    } else {
      for (i = y1; i <= y2; i++) {
        p(data, x1, i, color);
      }
    }
  } else {
    dx = x2 - x1;
    if (dx < 0) dx = -dx;
    sx = x1 < x2 ? 1 : -1;
    dy = y2 - y1;
    if (dy > 0) dy = -dy;
    sy = y1 < y2 ? 1 : -1;
    err = dx + dy;

    for (;;) {
      p(data, x1, y1, color);
      if (x1 == x2 && y1 == y2) break;

      e2 = 2*err;
      if (e2 >= dy) {
        err += dy;
        x1 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y1 += sy;
      }
    }
  }
}

void graphic_rectangle(void *data, int x1, int y1, int x2, int y2, int filled, uint32_t color, setpixel_f p, setarea_f a) {
  int i, aux;

  if (x1 > x2) { aux = x1; x1 = x2; x2 = aux; }
  if (y1 > y2) { aux = y1; y1 = y2; y2 = aux; }

  if (filled) {
    if (a) {
      a(data, x1, y1, x2, y2, color);
    } else {
      for (i = y1; i <= y2; i++) {
        graphic_line(data, x1, i, x2, i, color, p, a);
      }
    }
  } else {
    graphic_line(data, x1, y1, x2, y1, color, p, a);
    graphic_line(data, x1, y2, x2, y2, color, p, a);
    graphic_line(data, x1, y1, x1, y2, color, p, a);
    graphic_line(data, x2, y1, x2, y2, color, p, a);
  }
}

static void fill_edges(int n, point_t *points, edge_t **edgeTable) {
  int i, j;
  point_t *p1, *p2, *p3;
  edge_t *e;

  for (i = 0; i < n; i++) {
    p1 = &points[i];
    p2 = &points[(i + 1) % n];

    if (p1->y == p2->y)
      continue;   // Skip horiz. edges

    // Find next vertex not level with p2
    for (j = (i + 2) % n; ; j = (j + 1) % n) {
      p3 = &points[j];
      if (p2->y != p3->y)
        break;
    }

    e = xcalloc(1, sizeof(edge_t));
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

static edge_t *update_active(edge_t *active, int curY, edge_t **edgeTable) {
  edge_t *e, **ep;

  for (ep = &active, e = *ep; e != NULL; e = *ep) {
    if (e->yBot < curY) {
      *ep = e->next;
      xfree(e);
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

static void draw_runs(void *data, edge_t *active, int curY, int *xCoords, uint32_t color, setpixel_f p, setarea_f a) {
  edge_t *e;
  int i, numCoords = 0;

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
    graphic_line(data, xCoords[i], curY, xCoords[i+1], curY, color, p, a);
  }
}

void graphic_polygon(void *data, point_t *points, int n, int filled, uint32_t color, setpixel_f p, setarea_f a) {
  edge_t *active;
  edge_t **edgeTable;
  int i, curY, *xCoords;

  if (n > 0 && points) {
    if (filled) {
      if (n >= 3) {
        edgeTable = xcalloc(MAXVERTICAL, sizeof(edge_t));
        xCoords = xcalloc(MAXVERTICAL, sizeof(int));

        fill_edges(n, points, edgeTable);

        for (curY = 0; edgeTable[curY] == NULL; curY++) {
          if (curY == MAXVERTICAL - 1) {
            xfree(xCoords);
            xfree(edgeTable);
            return; // No edges in polygon
          }
        }

        for (active = NULL; (active = update_active(active, curY, edgeTable)) != NULL; curY++) {
          draw_runs(data, active, curY, xCoords, color, p, a);
        }

        xfree(xCoords);
        xfree(edgeTable);
      }
    } else {
      for (i = 1; i < n; i++) {
        graphic_line(data, points[i-1].x, points[i-1].y, points[i].x, points[i].y, color, p, a);
      }
      graphic_line(data, points[n-1].x, points[n-1].y, points[0].x, points[0].y, color, p, a);
    }
  }
}

void graphic_ellipse(void *data, int x, int y, int rx, int ry, int filled, uint32_t color, setpixel_f p, setarea_f a) {
  double d;
  int i, k;

  if (filled) {
    for (i = 0; i < ry; i++) {
      d = (double)i / (double)ry;
      d = sys_sqrt(1.0 - d * d) * (double)rx;
      k = (int)d;
      graphic_line(data, x, y+i, x+k, y+i, color, p, a);
      graphic_line(data, x, y+i, x-k, y+i, color, p, a);
      graphic_line(data, x, y-i, x+k, y-i, color, p, a);
      graphic_line(data, x, y-i, x-k, y-i, color, p, a);
    }
  } else {
    for (i = 0; i < ry; i++) {
      d = (double)i / (double)ry;
      d = sys_sqrt(1.0 - d * d) * (double)rx;
      k = (int)d;
      p(data, x+k, y+i, color);
      p(data, x-k, y+i, color);
      p(data, x+k, y-i, color);
      p(data, x-k, y-i, color);
    }
    for (i = 0; i < rx; i++) {
      d = (double)i / (double)rx;
      d = sys_sqrt(1.0 - d * d) * (double)ry;
      k = (int)d;
      p(data, x+i, y+k, color);
      p(data, x+i, y-k, color);
      p(data, x-i, y+k, color);
      p(data, x-i, y-k, color);
    }
  }
}

static void graphic_char(void *data, int x, int y, uint8_t c, int i, int h, font_t *f, uint32_t fg, uint32_t bg, setpixel_f p) {
  uint8_t *d, b, mask;
  int j, k, width;

  if (f->index) {
    d = f->font + i * f->len + f->index[c - f->min];
    width = f->cwidth[c - f->min];
  } else {
    d = f->font + i * f->len + (c - f->min) * f->width;
    width = f->width;
  }

//char buf[10];
//debug(1, "XXX", "graphic_char '%c' i=%d h=%d y=%d len=%d width=%d index=%d", c, i, h, y, f->len, width, d - f->font);
  for (j = 0; j < width; j++) {
    b = d[j];
    mask = 0x01;
    for (k = 0; k < h; k++) {
//buf[k] = (b & mask) ? '1' : '0';
      p(data, x+j, y+k, (b & mask) ? fg : bg);
      mask <<= 1;
    }
//buf[k] = 0;
//debug(1, "XXX", "%2d 0x%02X %s", j, b, buf);
  }
}

void graphic_printchar(void *data, int x, int y, uint8_t c, font_t *f, uint32_t fg, uint32_t bg, setpixel_f p) {
  int i, h, n;

  n = (f->height + 7) / 8;
  h = f->height;
  for (i = n - 1; i >= 0; i--, h -= 8, y += 8) {
    graphic_char(data, x, y, c, i, i == 0 ? h : 8, f, fg, bg, p);
  }
/*
  if (f->height <= 8) {
    graphic_char(data, x, y, c, 0, f->height, f, fg, bg, p);
  } else {
    graphic_char(data, x, y,   c, 1, 8, f, fg, bg, p);
    graphic_char(data, x, y+8, c, 0, f->height - 8, f, fg, bg, p);
  }
*/
}

#define SPACE 4

static font_seg_t segments[] = {
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 0
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 1
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 2
  {   3.000000,   0.000000,   0.000000,   0.000000 },  // 3
  {   0.000000,   3.000000,   3.000000,   3.000000 },  // 4
  {   0.000000,   0.000000,   0.000000,   0.000000 },  // 5
  {   0.000000,   0.000000,   3.000000,   6.000000 },  // 6
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 7
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 8
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 9
  {   3.000000,   0.000000,   0.000000,   0.000000 },  // 10
  {   0.000000,   0.000000,   3.000000,   6.000000 },  // 11
  {   1.000000,   0.000000,   1.000000,   6.000000 },  // 12
  {   1.000000,   6.000000,   0.000000,   5.000000 },  // 13
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 14
  {   3.000000,   6.000000,   3.000000,   4.000000 },  // 15
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 16
  {   0.000000,   4.000000,   0.000000,   0.000000 },  // 17
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 18
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 19
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 20
  {   3.000000,   0.000000,   0.000000,   0.000000 },  // 21
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 22
  {   0.000000,   6.000000,   0.000000,   4.000000 },  // 23
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 24
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 25
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 26
  {   3.000000,   0.000000,   3.000000,   4.000000 },  // 27
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 28
  {   0.000000,   4.000000,   0.000000,   6.000000 },  // 29
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 30
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 31
  {   3.000000,   0.000000,   3.000000,   4.000000 },  // 32
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 33
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 34
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 35
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 36
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 37
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 38
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 39
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 40
  {   3.000000,   0.000000,   0.000000,   0.000000 },  // 41
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 42
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 43
  {   3.000000,   0.000000,   3.000000,   6.000000 },  // 44
  {   3.000000,   6.000000,   0.000000,   6.000000 },  // 45
  {   0.000000,   6.000000,   0.000000,   4.000000 },  // 46
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 47
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 48
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 49
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 50
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 51
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 52
  {   0.000000,   6.000000,   2.000000,   6.000000 },  // 53
  {   2.000000,   6.000000,   3.000000,   4.000000 },  // 54
  {   3.000000,   4.000000,   2.000000,   3.000000 },  // 55
  {   2.000000,   3.000000,   3.000000,   2.000000 },  // 56
  {   3.000000,   2.000000,   2.000000,   0.000000 },  // 57
  {   2.000000,   0.000000,   0.000000,   0.000000 },  // 58
  {   0.000000,   3.000000,   2.000000,   3.000000 },  // 59
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 60
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 61
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 62
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 63
  {   0.000000,   6.000000,   2.000000,   6.000000 },  // 64
  {   2.000000,   6.000000,   3.000000,   4.000000 },  // 65
  {   3.000000,   4.000000,   3.000000,   2.000000 },  // 66
  {   3.000000,   2.000000,   2.000000,   0.000000 },  // 67
  {   0.000000,   0.000000,   2.000000,   0.000000 },  // 68
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 69
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 70
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 71
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 72
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 73
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 74
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 75
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 76
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 77
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 78
  {   3.000000,   0.000000,   3.000000,   3.000000 },  // 79
  {   3.000000,   3.000000,   2.000000,   3.000000 },  // 80
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 81
  {   3.000000,   0.000000,   3.000000,   6.000000 },  // 82
  {   0.000000,   4.000000,   3.000000,   4.000000 },  // 83
  {   1.000000,   0.000000,   1.000000,   6.000000 },  // 84
  {   0.000000,   0.000000,   2.000000,   0.000000 },  // 85
  {   0.000000,   6.000000,   2.000000,   6.000000 },  // 86
  {   3.000000,   0.000000,   3.000000,   6.000000 },  // 87
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 88
  {   0.000000,   0.000000,   0.000000,   2.000000 },  // 89
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 90
  {   0.000000,   4.000000,   3.000000,   6.000000 },  // 91
  {   0.000000,   4.000000,   3.000000,   0.000000 },  // 92
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 93
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 94
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 95
  {   4.000000,   0.000000,   4.000000,   6.000000 },  // 96
  {   0.000000,   6.000000,   2.000000,   4.000000 },  // 97
  {   2.000000,   4.000000,   4.000000,   6.000000 },  // 98
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 99
  {   0.000000,   6.000000,   3.000000,   0.000000 },  // 100
  {   3.000000,   0.000000,   3.000000,   6.000000 },  // 101
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 102
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 103
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 104
  {   3.000000,   0.000000,   0.000000,   0.000000 },  // 105
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 106
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 107
  {   3.000000,   6.000000,   3.000000,   4.000000 },  // 108
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 109
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 110
  {   0.000000,   6.000000,   4.000000,   6.000000 },  // 111
  {   4.000000,   6.000000,   4.000000,   0.000000 },  // 112
  {   4.000000,   0.000000,   0.000000,   0.000000 },  // 113
  {   2.000000,   1.000000,   2.000000,   0.000000 },  // 114
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 115
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 116
  {   3.000000,   6.000000,   3.000000,   4.000000 },  // 117
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 118
  {   1.000000,   4.000000,   3.000000,   0.000000 },  // 119
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 120
  {   3.000000,   0.000000,   3.000000,   4.000000 },  // 121
  {   3.000000,   4.000000,   0.000000,   4.000000 },  // 122
  {   0.000000,   4.000000,   0.000000,   6.000000 },  // 123
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 124
  {   2.000000,   0.000000,   2.000000,   6.000000 },  // 125
  {   0.000000,   6.000000,   4.000000,   6.000000 },  // 126
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 127
  {   3.000000,   6.000000,   3.000000,   0.000000 },  // 128
  {   3.000000,   0.000000,   0.000000,   0.000000 },  // 129
  {   0.000000,   6.000000,   2.000000,   0.000000 },  // 130
  {   2.000000,   0.000000,   4.000000,   6.000000 },  // 131
  {   0.000000,   0.000000,   0.000000,   6.000000 },  // 132
  {   4.000000,   0.000000,   4.000000,   6.000000 },  // 133
  {   0.000000,   2.000000,   2.000000,   2.000000 },  // 134
  {   2.000000,   2.000000,   4.000000,   0.000000 },  // 135
  {   0.000000,   0.000000,   3.000000,   6.000000 },  // 136
  {   0.000000,   6.000000,   3.000000,   0.000000 },  // 137
  {   2.000000,   0.000000,   2.000000,   3.000000 },  // 138
  {   2.000000,   3.000000,   0.000000,   6.000000 },  // 139
  {   2.000000,   3.000000,   4.000000,   6.000000 },  // 140
  {   0.000000,   6.000000,   3.000000,   6.000000 },  // 141
  {   3.000000,   6.000000,   0.000000,   0.000000 },  // 142
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 143
  {   0.000000,   0.000000,   3.000000,   0.000000 },  // 144
};

static font_glyph_t glyph[] = {
  { 0x00, 4, 0, 3.000000, 6.000000 },
  { '-' , 1, 4, 3.000000, 3.000000 },
  { '.' , 1, 5, 0.000000, 0.000000 },
  { '/' , 1, 6, 3.000000, 6.000000 },
  { '0' , 5, 7, 3.000000, 6.000000 },
  { '1' , 2, 12, 1.000000, 6.000000 },
  { '2' , 5, 14, 3.000000, 6.000000 },
  { '3' , 4, 19, 3.000000, 6.000000 },
  { '4' , 3, 23, 3.000000, 6.000000 },
  { '5' , 5, 26, 3.000000, 6.000000 },
  { '6' , 5, 31, 3.000000, 6.000000 },
  { '7' , 2, 36, 3.000000, 6.000000 },
  { '8' , 5, 38, 3.000000, 6.000000 },
  { '9' , 5, 43, 3.000000, 6.000000 },
  { 'A' , 4, 48, 3.000000, 6.000000 },
  { 'B' , 8, 52, 3.000000, 6.000000 },
  { 'C' , 3, 60, 3.000000, 6.000000 },
  { 'D' , 6, 63, 3.000000, 6.000000 },
  { 'E' , 4, 69, 3.000000, 6.000000 },
  { 'F' , 3, 73, 3.000000, 6.000000 },
  { 'G' , 5, 76, 3.000000, 6.000000 },
  { 'H' , 3, 81, 3.000000, 6.000000 },
  { 'I' , 3, 84, 2.000000, 6.000000 },
  { 'J' , 3, 87, 3.000000, 6.000000 },
  { 'K' , 3, 90, 3.000000, 6.000000 },
  { 'L' , 2, 93, 3.000000, 6.000000 },
  { 'M' , 4, 95, 4.000000, 6.000000 },
  { 'N' , 3, 99, 3.000000, 6.000000 },
  { 'O' , 4, 102, 3.000000, 6.000000 },
  { 'P' , 4, 106, 3.000000, 6.000000 },
  { 'Q' , 5, 110, 4.000000, 6.000000 },
  { 'R' , 5, 115, 3.000000, 6.000000 },
  { 'S' , 5, 120, 3.000000, 6.000000 },
  { 'T' , 2, 125, 4.000000, 6.000000 },
  { 'U' , 3, 127, 3.000000, 6.000000 },
  { 'V' , 2, 130, 4.000000, 6.000000 },
  { 'W' , 4, 132, 4.000000, 6.000000 },
  { 'X' , 2, 136, 3.000000, 6.000000 },
  { 'Y' , 3, 138, 4.000000, 6.000000 },
  { 'Z' , 3, 141, 3.000000, 6.000000 },
  { '_' , 1, 144, 3.000000, 0.000000 },
  { 0, 0 }
};

graphic_vfont_t *graphic_vfont_init(void) {
  graphic_vfont_t *f;
  font_seg_t *seg;
  double a;
  int i, j;

  if ((f = xcalloc(1, sizeof(graphic_vfont_t))) != NULL) {
    for (i = 0; i < 360; i++) {
      a = (i * sys_pi()) / 180;
      f->cos_table[i] = sys_cos(a);
      f->sin_table[i] = sys_sin(a);
    }

    for (i = 0; glyph[i].nsegs; i++) {
      if (glyph[i].ch < 256) {
        f->font[glyph[i].ch] = glyph[i];

        for (j = 0; j < glyph[i].nsegs; j++) {
          seg = (font_seg_t *)&segments[glyph[i].seg0 + j];
          if (seg->x1 < 0.0) seg->x1 = 0.0;
          if (seg->x2 < 0.0) seg->x2 = 0.0;
          if (seg->y1 < 0.0) seg->y1 = 0.0;
          if (seg->y2 < 0.0) seg->y2 = 0.0;
        }
      }
    }
  }

  return f;
}

void graphic_vfont_finish(graphic_vfont_t *f) {
  if (f) xfree(f);
}

static void graphic_vfont_draw_line(graphic_vfont_t *f, void *data, int x1, int y1, int x2, int y2, int width, uint32_t color, setpixel_f p, setarea_f a) {
  int tx, ty;

  switch (width) {
    case 1:
      graphic_line(data, x1, y1, x2, y2, color, p, a);
      break;
    case 2:
      graphic_line(data, x1, y1, x2, y2, color, p, a);
      tx = x2 > x1 ? x2-x1 : x1-x2;
      ty = y2 > y1 ? y2-y1 : y1-y2;
      if (tx < ty) {
        graphic_line(data, x1-1, y1, x2-1, y2, color, p, a);
      } else {
        graphic_line(data, x1, y1-1, x2, y2-1, color, p, a);
      }
      break;
  }
}

static void graphic_vfont_draw_glyph(graphic_vfont_t *f, void *data, font_glyph_t *g, int x, int y, uint32_t color, double size, double cos_t, double sin_t, setpixel_f p, setarea_f a) {
  int i, x1, x2, y1, y2;
  double sx1, sy1, sx2, sy2;
  font_seg_t *seg;

  for (i = 0; i < g->nsegs; i++) {
    seg = (font_seg_t *)&segments[g->seg0 + i];
    sx1 = seg->x1 * size;
    sy1 = seg->y1 * size;
    sx2 = seg->x2 * size;
    sy2 = seg->y2 * size;

    x1 = sx1 * cos_t - sy1 * sin_t;
    y1 = sx1 * sin_t + sy1 * cos_t;

    x2 = sx2 * cos_t - sy2 * sin_t;
    y2 = sx2 * sin_t + sy2 * cos_t;

    graphic_vfont_draw_line(f, data, x + x1, y - y1, x + x2, y - y2, 2, color, p, a);
  }
}

void graphic_vfont_size(graphic_vfont_t *f, char *s, double size, int *dx, int *dy) {
  uint8_t c;
  int i, tx, ty;

  *dx = *dy = 0;

  if (f && s) {
    for (i = 0; i < s[i]; i++) {
      c = (uint8_t)s[i];

      if (c == ' ') {
        tx = SPACE;
        ty = 0;
      } else {
        if (c >= 'a' && c <= 'z' && !f->font[c].nsegs) c -= 32;
        tx = f->font[c].dx * size;
        ty = f->font[c].dy * size;
      }

      if (ty > *dy) *dy = ty;
      *dx += SPACE;
      *dx += tx;
    }
  }
}

uint8_t graphic_vfont_mapchar(uint8_t c) {
  if (c >= 0x80) {
    switch (c) {
      case 0xC0:
      case 0xC1:
      case 0xC2:
      case 0xC3:
      case 0xC4:
      case 0xC5:
      case 0xC6:
      case 0xE0:
      case 0xE1:
      case 0xE2:
      case 0xE3:
      case 0xE4:
      case 0xE5:
      case 0xE6:
        c = 'A';
        break;
      case 0xC7:
      case 0xE7:
        c = 'C';
        break;
      case 0xC8:
      case 0xC9:
      case 0xCA:
      case 0xCB:
      case 0xE8:
      case 0xE9:
      case 0xEA:
      case 0xEB:
        c = 'E';
        break;
      case 0xCC:
      case 0xCD:
      case 0xCE:
      case 0xCF:
      case 0xEC:
      case 0xED:
      case 0xEE:
      case 0xEF:
        c = 'I';
        break;
      case 0xD0:
      case 0xF0:
        c = 'D';
        break;
      case 0xD1:
      case 0xF1:
        c = 'N';
        break;
      case 0xD2:
      case 0xD3:
      case 0xD4:
      case 0xD5:
      case 0xD6:
      case 0xF2:
      case 0xF3:
      case 0xF4:
      case 0xF5:
      case 0xF6:
        c = 'O';
        break;
      case 0xD9:
      case 0xDA:
      case 0xDB:
      case 0xDC:
      case 0xF9:
      case 0xFA:
      case 0xFB:
      case 0xFC:
        c = 'U';
        break;
      case 0xDD:
      case 0xFD:
        c = 'Y';
        break;
    }
  }

  return c;
}

void graphic_vfont_draw(graphic_vfont_t *f, void *data, char *s, int x, int y, uint32_t color, double size, int angle, setpixel_f p, setarea_f a) {
  uint8_t c;
  int i, dx, dg, x1, y1;
  double sin_t, cos_t;

  if (f && s) {
    for (; angle <= -360; angle += 360);
    for (; angle >=  360; angle -= 360);

    if (angle >= 0) {
      cos_t = f->cos_table[angle];
      sin_t = f->sin_table[angle];
    } else {
      cos_t = f->cos_table[-angle];
      sin_t = -f->sin_table[-angle];
    }

    dx = 0;

    for (i = 0; s[i]; i++) {
      c = (uint8_t)s[i];
      c = graphic_vfont_mapchar(c);
      if (c == ' ') {
        dg = SPACE;
      } else {
        if (c >= 'a' && c <= 'z' && !f->font[c].nsegs) c -= 32;
        x1 = x + dx * cos_t;
        y1 = y - dx * sin_t;
        graphic_vfont_draw_glyph(f, data, &f->font[c], x1, y1, color, size, cos_t, sin_t, p, a);
        dg = f->font[c].dx * size;
      }

      dx += SPACE;
      dx += dg;
    }
  }
}
