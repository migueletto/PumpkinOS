#include <PalmOS.h>

#include <string.h>
#include <math.h>

#include "grail.h"
#include "debug.h"

#define MAX_POINTS   512
#define MAX_SPOINTS   32

typedef struct {
  char *path;
  char *glyph;
  char *dis0, *dis1, *dis2;
} grail_t;

static grail_t alpha_glyphs[] = {
  { "R",         " "    },
  { "L",         "\b\r", "-0", "-+" },
  { "U",         "\x01" }, // Caps
  { "UD",        "a"    },
  { "URDLRDL",   "b"    },
  { "DURDLRDL",  "b"    },
  { "LR",        "c"    },
  { "LDR",       "c"    },
  { "DURDL",     "d"    },
  { "RLRL",      "e"    },
  { "LDRLDR",    "e"    },
  { "LD",        "f"    },
  { "LDRUL",     "go",   "-+", "00" },
  { "DURD",      "h"    },
  { "D",         "i"    },
  { "DL",        "j"    },
  { "DLUR",      "k"    },
  { "DLURD",     "k"    },
  { "DR",        "l"    },
  { "UDUD",      "m"    },
  { "UDU",       "n"    },
  { "URDL",      "p"    },
  { "LDRULR",    "q"    },
  { "URDLR",     "r"    },
  { "URDLD",     "r"    },
  { "LDRDL",     "s"    },
  { "RD",        "t"    },
  { "DRU",       "u"    },
  { "DU",        "v"    },
  { "DUDU",      "w"    },
  { "DRUL",      "x"    },
  { "DRULD",     "x"    },
  { "DRUDU",     "y"    },
  { "RDR",       "z"    },
  { "RLR",       "z"    },
  { NULL }
};

static grail_t numeric_glyphs[] = {
  { "R",         " "    },
  { "L",         "\b\r", "-0", "-+" },

  { "D",         "1"    },
  { "RDLR",      "2"    },
  { "URDLR",     "2"    },
  { "RLRL",      "3"    },
  { "RDLRDL",    "3"    },
  { "DR",        "4"    },
  { "LDRDL",     "5"    },
  { "LDRUL",     "60",   "-+", "00" },
  { "RD",        "7"    },
  { "LDRDLURUL", "8"    },
  { "LDRUD",     "9"    },
  { NULL }
};

static grail_t punctuation_glyphs[] = {
  { "L",         ","    },
  { "D",         "\'"   },
  { "LR",        "`<;",  "00D", "0-D", "00U" },
  { "RDLD",      "?"    },
  { "URDLD",     "?"    },
  { "R",         "-/\\", "+0", "+-", "++" },
  { "U",         "!"    },
  { "RDL",       "("    },
  { "LDR",       ")"    },
  { "LDRDL",     "$"    },
  { "RDLURDLUR", "%"    },
  { "LDRUL",     "@"    },
  { "DUD",       "#~",   "++", "-+" },
  { "UD",        "^|",   "+0", "00" },
  { "LDRDLURUL", "&"    },
  { "DRUL",      "*"    },
  { "DRULD",     "*"    },
  { "RL",        ">_",   "0-", "00" },
  { "LRLR",      "{[",   "0+", "0-" },
  { "LDRLDR",    "{"  },
  { "RLRL",      "}]",   "0+", "0-" },
  { "RDLRDL",    "}"    },
  { "RULRUL",    "]"    },
  { "DLUR",      "+"    },
  { "DLURD",     "+"    },
  { "RDR",       "="    },
  { "RLR",       "="    },
  { "DU",        ":"    },
  { "UDU",       "\""   },
  { "UR",        "\t"   },
  { NULL }
};

static PointType points[MAX_POINTS];
static int caps = 0, capslock = 0;
static int punctuation = 0;
static int alpha, num;

static double perpendicular_distance(PointType *p, PointType *p1, PointType *p2) {
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  double d = sqrt(dx * dx + dy * dy);
  return fabs(p->x * dy - p->y * dx + p2->x * p1->y - p2->y * p1->x) / d;
}

// https://rosettacode.org/wiki/Ramer-Douglas-Peucker_line_simplification#C

static int simplify_points(PointType *points, int n, double epsilon, PointType *dest, int destlen) {
  double max_dist = 0;
  int index = 0;
  for (int i = 1; i + 1 < n; ++i) {
    double dist = perpendicular_distance(&points[i], &points[0], &points[n - 1]);
    if (dist > max_dist) {
      max_dist = dist;
      index = i;
    }
  }

  if (max_dist > epsilon) {
    int n1 = simplify_points(points, index + 1, epsilon, dest, destlen);
    if (destlen >= n1 - 1) {
      destlen -= n1 - 1;
      dest += n1 - 1;
    } else {
      destlen = 0;
    }
    int n2 = simplify_points(points + index, n - index, epsilon, dest, destlen);

    return n1 + n2 - 1;
  }

  if (destlen >= 2) {
    dest[0] = points[0];
    dest[1] = points[n - 1];
  }

  return 2;
}

int grail_begin(int _alpha) {
  alpha = _alpha;
  num = 0;
  debug(DEBUG_TRACE, "GRAIL", "begin");

  return 0;
}

int grail_stroke(int x, int y) {
  int r = -1;

  if (num < MAX_POINTS) {
    if (num > 0) {
      grail_draw_stroke(x, y, points[num-1].x, points[num-1].y);
    }
    points[num].x = x;
    points[num].y = y;
    num++;
    r = 0;
  } else {
    debug(DEBUG_ERROR, "GRAIL", "max points exceeded");
  }

  return r;
}

static int grail_match(char *path, PointType *p0, PointType *p1, int gx, int gy, grail_t glyphs[]) {
  char dis[8];
  int i, j, dx, dy, gm, g = 0;

  gm = (gx > gy) ? gx : gy;
  if (gm == 0) return 0;

  dx = p1->x - p0->x;
  dy = p1->y - p0->y;
  dx = (dx * 100) / gm;
  dy = (dy * 100) / gm;

  for (i = 0; glyphs[i].path; i++) {
    for (j = 0;; j++) {
      if (glyphs[i].path[j] == 0) break;
      if (path[j] == 0) break;
      if (glyphs[i].path[j] != path[j]) break;
    }

    if (glyphs[i].path[j] == 0 && path[j] == 0) {
      if (glyphs[i].glyph[1] == 0) {
        g = glyphs[i].glyph[0];

      } else {
        // resolve ambiguities

        if (dx > 20) {
          dis[0] = '+';
        } else if (dx < -20) {
          dis[0] = '-';
        } else {
          dis[0] = '0';
        }

        if (dy > 20) {
          dis[1] = '+';
        } else if (dy < -20) {
          dis[1] = '-';
        } else {
          dis[1] = '0';
        }

        dis[2] = 0;
        debug(DEBUG_TRACE, "GRAIL", "dis \"%s\" dx=%d dy=%d", dis, dx, dy);

        if (!strcmp(dis, glyphs[i].dis0)) {
          debug(DEBUG_TRACE, "GRAIL", "dis0 \"%s\"", dis);
          g = glyphs[i].glyph[0];
        } else if (!strcmp(dis, glyphs[i].dis1)) {
          debug(DEBUG_TRACE, "GRAIL", "dis1 \"%s\"", dis);
          g = glyphs[i].glyph[1];
        } else if (glyphs[i].dis2 && !strcmp(dis, glyphs[i].dis2)) {
          debug(DEBUG_TRACE, "GRAIL", "dis2 \"%s\"", dis);
          g = glyphs[i].glyph[2];
        }
      }
      break;
    }
  }

  return g;
}

int grail_end(uint32_t *state) {
  PointType p[MAX_POINTS];
  char path[MAX_SPOINTS+1], dir, last;
  int i, n, dx, dy, gx, gy, xmin, xmax, ymin, ymax, np, glyph;

  debug(DEBUG_TRACE, "GRAIL", "end num=%d", num);

  if (num == 0 && !punctuation) {
    caps = capslock = 0;
    punctuation = 1;
    *state = 0;
    *state |= GRAIL_PUNCT;
    return 0;
  }

  n = simplify_points(points, num, 8.0, p, MAX_SPOINTS);
  np = 0;
  glyph = 0;
  debug(DEBUG_TRACE, "GRAIL", "simplify n=%d", n);

  if (n > 1) {
    last = ' ';
    xmin = xmax = p[0].x;
    ymin = ymax = p[0].y;

    for (i = 1; i < n; i++) {
      if (p[i].x < xmin) xmin = p[i].x;
      else if (p[i].x > xmax) xmax = p[i].x;
      if (p[i].y < ymin) ymin = p[i].y;
      else if (p[i].y > ymax) ymax = p[i].y;

      dy = p[i].y - p[i-1].y;
      dx = p[i].x - p[i-1].x;
      if (dy < 0) dy = -dy;
      if (dx < 0) dx = -dx;
      if (dx < dy) {
        if (p[i].y > p[i-1].y) {
          dir = 'D';
        } else {
          dir = 'U';
        }
      } else {
        if (p[i].x > p[i-1].x) {
          dir = 'R';
        } else {
          dir = 'L';
        }
      }
      if (dir != last && np < MAX_SPOINTS) {
        path[np++] = dir;
        last = dir;
      }
    }
    debug(DEBUG_TRACE, "GRAIL", "np=%d x:%d->%d y:%d->%d", np, xmin, xmax, ymin, ymax);

    if (np > 0) {
      gx = xmax - xmin;
      gy = ymax - ymin;

      path[np] = 0;
      debug(DEBUG_TRACE, "GRAIL", "path \"%s\" gx=%d gy=%d", path, gx, gy);

      if (punctuation) {
        glyph = (num == 0) ? '.' : grail_match(path, &p[0], &p[n-1], gx, gy, punctuation_glyphs);
        punctuation = 0;
      } else {
        if (alpha) {
          glyph = grail_match(path, &p[0], &p[n-1], gx, gy, alpha_glyphs);
          if (glyph == 0x01) {
            if (!caps) {
              debug(DEBUG_TRACE, "GRAIL", "caps on");
              caps = 1;
            } else if (!capslock) {
              debug(DEBUG_TRACE, "GRAIL", "capslock on");
              capslock = 1;
            } else {
              debug(DEBUG_TRACE, "GRAIL", "caps/capslock off");
              caps = capslock = 0;
            }
            glyph = 0;
          } else if (caps) {
            if (glyph >= 'a' && glyph <= 'z') glyph &= 0xDF;
            caps = capslock;
          }
        } else {
          glyph = grail_match(path, &p[0], &p[n-1], gx, gy, numeric_glyphs);
        }
      }
    }
  }
    
  *state = 0;
  if (caps) *state |= GRAIL_CAPS;
  if (capslock) *state |= GRAIL_CAPSLOCK;
  if (punctuation) *state |= GRAIL_PUNCT;

  return glyph;
}

void grail_reset(void) {
  caps = capslock = punctuation = 0;
}
