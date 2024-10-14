#include "sys.h"
#include "pwindow.h"
#include "rgb.h"
#include "debug.h"
#include "xalloc.h"

#include "wman.h"

#define MAX_AREAS 32

#define WMAN_BACK     -1
#define WMAN_HBORDER  -2
#define WMAN_VBORDER  -3
#define WMAN_HSBORDER -4
#define WMAN_VSBORDER -5

typedef struct {
  int x, y, width, height;
} rect_t;

typedef struct {
  int id;
  texture_t *t;
  rect_t r;
} wman_area_t;

struct wman_t {
  window_provider_t *wp;
  window_t *w;
  texture_t *background;
  texture_t *hborder, *hsborder, *hs0border, *hs1border;
  texture_t *vborder, *vsborder, *vs0border, *vs1border;
  rect_t r;
  int border, n;
  wman_area_t area[MAX_AREAS];
};

#define swap(a,b) { aux=a; a=b; b=aux; }
#define min(a,b) (a) < (b) ? (a) : (b)
#define max(a,b) (a) > (b) ? (a) : (b)

static int intersection(rect_t *a, rect_t *b, rect_t *r) {
  int x1, y1, x2, y2;
  int x3, y3, x4, y4;
  int x5, y5, x6, y6;
  int aux;

  x1 = a->x;
  y1 = a->y;
  x2 = a->x + a->width - 1;
  y2 = a->y + a->height - 1;
  if (x1 > x2) swap(x1, x2);
  if (y1 > y2) swap(y1, y2);

  x3 = b->x;
  y3 = b->y;
  x4 = b->x + b->width - 1;
  y4 = b->y + b->height - 1;
  if (x3 > x4) swap(x3, x4);
  if (y3 > y4) swap(y3, y4);

  x5 = max(x1, x3);
  y5 = max(y1, y3);
  x6 = min(x2, x4);
  y6 = min(y2, y4);

  if (x6 > x5 && y6 > y5) {
    r->x = x5;
    r->y = y5;
    r->width = x6 - x5 + 1;
    r->height = y6 - y5 + 1;
    return 1;
  }

  return 0;
}

static int contains(rect_t *a, rect_t *b) {
  return (b->x >= a->x) && (b->y >= a->y) && (b->x + b->width <= a->x + a->width) && (b->y + b->height <= a->y + a->height);
}

static int intersects(rect_t *a, rect_t *b) {
  return !((b->x + b->width <= a->x)  ||
           (b->y + b->height <= a->y) ||
           (b->x >= a->x + a->width)  ||
           (b->y >= a->y + a->height));
}

static int difference(rect_t *a, rect_t *b, rect_t *r) {
  int rectCount = 0;

  if (contains(b, a)) {
    return 0;
  }

  if (!intersects(a, b)) {
    xmemcpy(r, a, sizeof(rect_t));
    return 1;
  }

  // compute the top rectangle
  int raHeight = b->y - a->y;
  if (raHeight > 0) {
    r[rectCount].x = a->x;
    r[rectCount].y = a->y;
    r[rectCount].width = a->width;
    r[rectCount].height = raHeight;
    rectCount++;
  }

  // compute the bottom rectangle
  int rbY = b->y + b->height;
  int rbHeight = a->height - (rbY - a->y);
  if (rbHeight > 0 && rbY < a->y + a->height) {
    r[rectCount].x = a->x;
    r[rectCount].y = rbY;
    r[rectCount].width = a->width;
    r[rectCount].height = rbHeight;
    rectCount++;
  }

  int rectAYH = a->y+a->height;
  int y1 = b->y > a->y ? b->y : a->y;
  int y2 = rbY < rectAYH ? rbY : rectAYH;
  int rcHeight = y2 - y1;

  // compute the left rectangle
  int rcWidth = b->x - a->x;
  if (rcWidth > 0 && rcHeight > 0) {
    r[rectCount].x = a->x;
    r[rectCount].y = y1;
    r[rectCount].width = rcWidth;
    r[rectCount].height = rcHeight;
    rectCount++;
  }

  // compute the right rectangle
  int rbX = b->x + b->width;
  int rdWidth = a->width - (rbX - a->x);
  if (rdWidth > 0) {
    r[rectCount].x = rbX;
    r[rectCount].y = y1;
    r[rectCount].width = rdWidth;
    r[rectCount].height = rcHeight;
    rectCount++;
  }

  return rectCount;
}

static void set_rect(rect_t *r, int x, int y, int width, int height, char *label) {
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    //debug(DEBUG_ERROR, "WMAN", "%s invalid rect %d,%d,%d,%d", label, x, y, width, height);
  }
  r->x = x;
  r->y = y;
  r->width = width;
  r->height = height;
}

static void set_top_border(wman_t *wm, int i, rect_t *b) {
  set_rect(b, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->area[i].r.width + 2*wm->border, wm->border, "set_top_border");
}

static void set_bottom_border(wman_t *wm, int i, rect_t *b) {
  set_rect(b, wm->area[i].r.x - wm->border, wm->area[i].r.y + wm->area[i].r.height, wm->area[i].r.width + 2*wm->border, wm->border, "set_bottom_border");
}

static void set_left_border(wman_t *wm, int i, rect_t *b) {
  set_rect(b, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->border, wm->area[i].r.height + 2*wm->border, "set_left_border");
}

static void set_right_border(wman_t *wm, int i, rect_t *b) {
  set_rect(b, wm->area[i].r.x + wm->area[i].r.width, wm->area[i].r.y - wm->border, wm->border, wm->area[i].r.height + 2*wm->border, "set_right_border");
}

static texture_t *solid_texture(wman_t *wm, int depth, int width, int height, uint8_t r, uint8_t g, uint8_t b) {
  texture_t *t = NULL;
  uint32_t i, n;
  uint32_t *b32;
  uint16_t *b16;
  void *bg;

  if (wm->w) {
    if ((t = wm->wp->create_texture(wm->w, width, height)) != NULL) {
      n = width * height;

      switch (depth) {
        case 16:
          b16 = xcalloc(1, n * 2);
          for (i = 0; i < n; i++) {
            b16[i] = rgb565(r, g, b);
          }
          bg = (uint8_t *)b16;
          break;
        case 32:
          b32 = xcalloc(1, n * 4);
          for (i = 0; i < n; i++) {
            b32[i] = rgba32(r, g, b, 0xFF);
          }
          bg = (uint8_t *)b32;
          break;
        default:
          bg = NULL;
          break;
      }
  
      if (bg) {
        wm->wp->update_texture(wm->w, t, bg);
        xfree(bg);
      }
    }
  }

  return t;
}

int wman_set_image_background(wman_t *wm, int depth, void *image) {
  uint32_t n;
  void *bg;
  int res = -1;

  if (wm->w && wm->background && image) {
    switch (depth) {
      case 16:
        n = wm->r.width * wm->r.height * 2;
        bg = xcalloc(1, n);
        sys_memcpy(bg, image, n);
        break;
      case 32:
        n = wm->r.width * wm->r.height * 4;
        bg = xcalloc(1, n);
        sys_memcpy(bg, image, n);
        break;
      default:
        bg = NULL;
        break;
    }

    if (bg) {
      wm->wp->update_texture(wm->w, wm->background, bg);
      xfree(bg);
      res = 0;
    }
  }

  return res;
}

int wman_set_background(wman_t *wm, int depth, uint8_t r, uint8_t g, uint8_t b) {
  int res = -1;

  if (wm) {
    if (wm->background) wm->wp->destroy_texture(wm->w, wm->background);
    wm->background = solid_texture(wm, depth, wm->r.width, wm->r.height, r, g, b);
    res = 0;
  }

  return res;
}

int wman_set_border(wman_t *wm, int depth, int size, uint8_t rsel, uint8_t gsel, uint8_t bsel, uint8_t rlck, uint8_t glck, uint8_t blck, uint8_t r, uint8_t g, uint8_t b) {
  int sel0, res = -1;

  if (wm) {
    wm->border = size;

    sel0 = wm->hsborder == wm->hs0border;
    if (wm->hs0border) wm->wp->destroy_texture(wm->w, wm->hs0border);
    if (wm->hs1border) wm->wp->destroy_texture(wm->w, wm->hs1border);
    if (wm->hborder)   wm->wp->destroy_texture(wm->w, wm->hborder);
    if (wm->vs0border) wm->wp->destroy_texture(wm->w, wm->vs0border);
    if (wm->vs1border) wm->wp->destroy_texture(wm->w, wm->vs1border);
    if (wm->vborder)   wm->wp->destroy_texture(wm->w, wm->vborder);

    wm->hs0border = solid_texture(wm, depth, wm->r.width, size, rsel, gsel, bsel);
    wm->hs1border = solid_texture(wm, depth, wm->r.width, size, rlck, glck, blck);
    wm->hborder   = solid_texture(wm, depth, wm->r.width, size, r, g, b);

    wm->vs0border = solid_texture(wm, depth, size, wm->r.height, rsel, gsel, bsel);
    wm->vs1border = solid_texture(wm, depth, size, wm->r.height, rlck, glck, blck);
    wm->vborder   = solid_texture(wm, depth, size, wm->r.height, r, g, b);

    wm->hsborder  = sel0 ? wm->hs0border : wm->hs1border;
    wm->vsborder  = sel0 ? wm->vs0border : wm->vs1border;

    res = 0;
  }

  return res;
}

int wman_choose_border(wman_t *wm, int i) {
  int res = -1;

  if (wm) {
    if (i == 0) {
      wm->hsborder = wm->hs0border;
      wm->vsborder = wm->vs0border;
    } else {
      wm->hsborder = wm->hs1border;
      wm->vsborder = wm->vs1border;
    }
    res = 0;
  }

  return res;
}

static void wman_draw(wman_t *wm, int i, int x, int y, int w, int h, int dstX, int dstY) {
  if (w > 0 && h > 0) {
    debug(DEBUG_TRACE, "WMAN", "draw i=%d x=%d y=%d w=%d h=%d", i, x, y, w, h);

    switch (i) {
      case WMAN_BACK:
        if (wm->background) {
          wm->wp->draw_texture_rect(wm->w, wm->background, x, y, w, h, dstX, dstY);
        }
        break;
      case WMAN_HBORDER:
        if (wm->hborder) {
          wm->wp->draw_texture_rect(wm->w, wm->hborder, x, y, w, h, dstX, dstY);
        }
        break;
      case WMAN_VBORDER:
        if (wm->vborder) {
          wm->wp->draw_texture_rect(wm->w, wm->vborder, x, y, w, h, dstX, dstY);
        }
        break;
      case WMAN_HSBORDER:
        if (wm->hsborder) {
          wm->wp->draw_texture_rect(wm->w, wm->hsborder, x, y, w, h, dstX, dstY);
        }
        break;
      case WMAN_VSBORDER:
        if (wm->vsborder) {
          wm->wp->draw_texture_rect(wm->w, wm->vsborder, x, y, w, h, dstX, dstY);
        }
        break;
      default:
        if (i < wm->n) {
          debug(DEBUG_TRACE, "WMAN", "area i=%d x=%d y=%d w=%d h=%d dx=%d dy=%d", i, x, y, w, h, dstX, dstY);
          wm->wp->draw_texture_rect(wm->w, wm->area[i].t, x, y, w, h, dstX, dstY);
        }
        break;
    }
  }
}

wman_t *wman_init(window_provider_t *wp, window_t *w, int width, int height) {
  wman_t *wm;

  if ((wm = xcalloc(1, sizeof(wman_t))) != NULL) {
    wm->wp = wp;
    wm->w = w;
    set_rect(&wm->r, 0, 0, width, height, "wman_init");
  }

  return wm;
}

static void callback_if_visible(wman_t *wm, int id, rect_t *b) {
  rect_t r, d[4];
  int i, n;

  i = wm->n-1;
  set_rect(&r, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->area[i].r.width + 2*wm->border, wm->area[i].r.height + 2*wm->border, "callback_if_visible");
  n = difference(b, &r, d);

  for (i = 0; i < n; i++) {
    wman_draw(wm, id, 0, 0, d[i].width, d[i].height, d[i].x ,d[i].y);
  }
}

static void draw_border(wman_t *wm, int i, int selected) {
  rect_t b;

  set_top_border(wm, i, &b);
  wman_draw(wm, selected ? WMAN_HSBORDER : WMAN_HBORDER, 0, 0, b.width, b.height, b.x, b.y);
  set_bottom_border(wm, i, &b);
  wman_draw(wm, selected ? WMAN_HSBORDER : WMAN_HBORDER, 0, 0, b.width, b.height, b.x, b.y);
  set_left_border(wm, i, &b);
  wman_draw(wm, selected ? WMAN_VSBORDER : WMAN_VBORDER, 0, 0, b.width, b.height, b.x, b.y);
  set_right_border(wm, i, &b);
  wman_draw(wm, selected ? WMAN_VSBORDER : WMAN_VBORDER, 0, 0, b.width, b.height, b.x, b.y);
}

static void change_top(wman_t *wm, int area, int select, int unselect) {
  rect_t b;
  int i = wm->n-1;

  if (area) {
    // paint top area
    wman_draw(wm, i, 0, 0, wm->area[i].r.width, wm->area[i].r.height, wm->area[i].r.x, wm->area[i].r.y);
  }

  if (wm->border) {
    if (select) {
      // select top borders
      draw_border(wm, i, 1);
    }

    if (unselect && wm->n > 1) {
      // unselect previous top borders
      i = wm->n-2;
      set_top_border(wm, i, &b);
      callback_if_visible(wm, WMAN_HBORDER, &b);
      set_bottom_border(wm, i, &b);
      callback_if_visible(wm, WMAN_HBORDER, &b);
      set_left_border(wm, i, &b);
      callback_if_visible(wm, WMAN_VBORDER, &b);
      set_right_border(wm, i, &b);
      callback_if_visible(wm, WMAN_VBORDER, &b);
    }
  }
}

void wman_clear(wman_t *wm) {
  wman_draw(wm, WMAN_BACK, 0, 0, wm->r.width, wm->r.height, 0, 0);
}

int wman_add(wman_t *wm, int id, texture_t *t, int x, int y, int w, int h) {
  int i, r = -1;

  if (wm && w > 0 && h > 0 && wm->n < MAX_AREAS) {
    i = wm->n++;
    wm->area[i].id = id;
    wm->area[i].t = t;
    if (x == -1) x = i ? (wm->r.width - w) / 2 : wm->border;
    if (y == -1) y = i ? (wm->r.height - h) / 2 : wm->border;
    set_rect(&wm->area[i].r, x, y, w, h, "wman_add");
    change_top(wm, 0, 1, 1);
    r = 0;
  }

  return r;
}

int wman_texture(wman_t *wm, int id, texture_t *t, int w, int h) {
  int i, r = -1;

  if (wm) {
    for (i = wm->n-1; i >= 0; i--) {
      if (wm->area[i].id == id) {
        wm->area[i].t = t;
        wm->area[i].r.width = w;
        wm->area[i].r.height = h;
        r = 0;
        break;
      }
    }
  }

  return r;
}

static void update(wman_t *wm, int i0, int x0, int y0, rect_t *r, int i) {
  rect_t d[4], a;
  int j, n;

  if (i > wm->n) {
    // there is nothing on top of this area, draw it
    wman_draw(wm, i0, r->x - x0, r->y - y0, r->width, r->height, r->x, r->y);
    return;
  }

  set_rect(&a, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->area[i].r.width + 2*wm->border, wm->area[i].r.height + 2*wm->border, "update");
  n = difference(r, &a, d);
  if (n == 0) {
    // the area is completely obscured by other area, return without drawing
    return;
  }

  for (j = 0; j < n; j++) {
    update(wm, i0, x0, y0, &d[j], i+1);
  }
}

// x,y: task relative coordinates
int wman_update(wman_t *wm, int id, int x, int y, int w, int h) {
  rect_t r;
  int i, res = -1;

  if (wm && wm->n) {
    if (wm->area[wm->n-1].id == id) {
      i = wm->n-1;
      wman_draw(wm, i, x, y, w, h, wm->area[wm->n-1].r.x + x, wm->area[wm->n-1].r.y + y);
      res = 0;
    } else {
      for (i = 0; i < wm->n-1; i++) {
        if (wm->area[i].id == id) {
//debug(1, "XXX", "wman_update %d: %d,%d,%d,%d", i, x, y, w, h);
          set_rect(&r, wm->area[i].r.x + x, wm->area[i].r.y + y, w, h, "wman_update");
          update(wm, i, wm->area[i].r.x, wm->area[i].r.y, &r, i+1);
          res = 0;
          break;
        }
      }
    }
  }

  return res;
}

int wman_raise(wman_t *wm, int id) {
  int i, found, r = -1;
  wman_area_t aux;

  if (wm && wm->n > 0) {
    if (wm->area[wm->n-1].id == id) {
      change_top(wm, 1, 1, 1);
      return 0;
    }
    found = 0;
    for (i = 0; i < wm->n-1; i++) {
      if (found) {
        wm->area[i] = wm->area[i+1];
      } else if (wm->area[i].id == id) {
        aux = wm->area[i];
        wm->area[i] = wm->area[i+1];
        found = 1;
      }
    }
    if (found) {
      wm->area[i] = aux;
      change_top(wm, 1, 1, 1);
      r = 0;
    }
  }

  return r;
}

static void check_rect(wman_t *wm, rect_t *r) {
  rect_t b, rr;
  int i;

  if (r->x < 0) {
    r->width += r->x;
    r->x = 0;
  } else if (r->x + r->width >= wm->r.width) {
    r->width = wm->r.width - r->x;
  }
  if (r->y < 0) {
    r->height += r->y;
    r->y = 0;
  } else if (r->y + r->height >= wm->r.height) {
    r->height = wm->r.height - r->y;
  }
  if (r->width > 0 && r->height > 0) {
    wman_draw(wm, WMAN_BACK, r->x, r->y, r->width, r->height, r->x, r->y);
  }

  for (i = 0; i < wm->n-1; i++) {
    if (intersection(r, &wm->area[i].r, &rr)) {
//debug(1, "XXX", "wman_move (%d,%d,%d,%d) intersection (%d,%d,%d,%d) = (%d,%d,%d,%d)", r->x, r->y, r->width, r->height, wm->area[i].r.x, wm->area[i].r.y, wm->area[i].r.width, wm->area[i].r.height, rr.x, rr.y, rr.width, rr.height);
//debug(1, "XXX", "wman_move uncover %d,%d,%d,%d", rr.x - wm->area[i].r.x, rr.y - wm->area[i].r.y, rr.width, rr.height);
      wman_draw(wm, i, rr.x - wm->area[i].r.x, rr.y - wm->area[i].r.y, rr.width, rr.height, rr.x, rr.y);
    }

    if (wm->border) {
      set_top_border(wm, i, &b);
      if (intersection(r, &b, &rr)) {
        wman_draw(wm, WMAN_HBORDER, 0, 0, rr.width, rr.height, rr.x, rr.y);
      }
      set_bottom_border(wm, i, &b);
      if (intersection(r, &b, &rr)) {
        wman_draw(wm, WMAN_HBORDER, 0, 0, rr.width, rr.height, rr.x, rr.y);
      }
      set_left_border(wm, i, &b);
      if (intersection(r, &b, &rr)) {
        wman_draw(wm, WMAN_VBORDER, 0, 0, rr.width, rr.height, rr.x, rr.y);
      }
      set_right_border(wm, i, &b);
      if (intersection(r, &b, &rr)) {
        wman_draw(wm, WMAN_VBORDER, 0, 0, rr.width, rr.height, rr.x, rr.y);
      }
    }
  }
}

int wman_move(wman_t *wm, int id, int dx, int dy) {
  rect_t r;
  int i, res = -1;

  // only the topmost area can move
  if (wm && wm->n > 0 && wm->area[wm->n-1].id == id && (dx != 0 || dy != 0)) {
    i = wm->n-1;

    if (wm->wp->move) {
//debug(1, "XXX", "move %d,%d,%d,%d %d,%d", wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->area[i].r.width + 2*wm->border, wm->area[i].r.height + 2*wm->border, dx, dy);
      wm->wp->move(wm->w, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->area[i].r.width + 2*wm->border, wm->area[i].r.height + 2*wm->border, dx, dy);
    }

//debug(1, "XXX", "wman_move dx=%d dy=%d", dx, dy);
    if (dx != 0) {
      if (dx > 0) {
//debug(1, "XXX", "wman_move dx cur r=%d,%d,%d,%d", wm->area[i].r.x, wm->area[i].r.y, wm->area[i].r.width, wm->area[i].r.height);
        set_rect(&r, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, dx+1, wm->area[i].r.height + 2*wm->border, "wman_move");
//debug(1, "XXX", "wman_move dx pos r=%d,%d,%d,%d", r.x, r.y, r.width, r.height);
      } else {
        set_rect(&r, wm->area[i].r.x + wm->area[i].r.width + wm->border + dx, wm->area[i].r.y - wm->border, -dx+1, wm->area[i].r.height + 2*wm->border, "wman_move");
//debug(1, "XXX", "wman_move dx neg r=%d,%d,%d,%d", r.x, r.y, r.width, r.height);
      }
      check_rect(wm, &r);
    }

    if (dy != 0) {
      if (dy > 0) {
        set_rect(&r, wm->area[i].r.x - wm->border, wm->area[i].r.y - wm->border, wm->area[i].r.width + 2*wm->border, dy+1, "wman_move");
      } else {
        set_rect(&r, wm->area[i].r.x - wm->border, wm->area[i].r.y + wm->area[i].r.height + wm->border + dy, wm->area[i].r.width + 2*wm->border, -dy+1, "wman_move");
      }
      check_rect(wm, &r);
    }

    wm->area[i].r.x += dx;
    wm->area[i].r.y += dy;

    if (!wm->wp->move) {
      change_top(wm, 1, 1, 0);
    }
    res = 0;
  }

  return res;
}

int wman_remove(wman_t *wm, int id, int remove) {
  wman_area_t aux;
  rect_t r, rr;
  int i, found, res = -1;

  if (wm && wm->n > 0) {
    if (wm->area[wm->n-1].id == id) {
      aux = wm->area[wm->n-1];
      found = 1;
    } else {
      found = 0;
      for (i = 0; i < wm->n-1; i++) {
        if (found) {
          wm->area[i] = wm->area[i+1];
        } else if (wm->area[i].id == id) {
          aux = wm->area[i];
          wm->area[i] = wm->area[i+1];
          found = 1;
        }
      }
    }
    if (found) {
      wm->n--;
      set_rect(&r, aux.r.x - wm->border, aux.r.y - wm->border, aux.r.width + 2*wm->border, aux.r.height + 2*wm->border, "wman_remove");
      wman_draw(wm, WMAN_BACK, r.x, r.y, r.width, r.height, r.x, r.y);
      for (i = 0; i < wm->n; i++) {
        if (intersection(&r, &wm->area[i].r, &rr)) {
          wman_draw(wm, i, 0, 0, wm->area[i].r.width, wm->area[i].r.height, wm->area[i].r.x, wm->area[i].r.y);
          draw_border(wm, i, i == wm->n-1);
        }
      }
      if (!remove) wm->n++;
      res = 0;
    }
  }

  return res;
}

int wman_clicked(wman_t *wm, int x, int y, int *tx, int *ty) {
  int i;

  if (wm) {
    for (i = wm->n-1; i >= 0; i--) {
      if (x >= wm->area[i].r.x &&
          x <  wm->area[i].r.x + wm->area[i].r.width &&
          y >= wm->area[i].r.y &&
          y <  wm->area[i].r.y + wm->area[i].r.height) {

        //*tx = wm->area[i].r.x - x;
        //*ty = wm->area[i].r.y - y;
        *tx = x - wm->area[i].r.x;
        *ty = y - wm->area[i].r.y;

        return wm->area[i].id;
      }
    }
  }

  return -1;
}

int wman_xy(wman_t *wm, int id, int *x, int *y) {
  int i, r = -1;

  if (wm && x && y) {
    for (i = 0; i < wm->n; i++) {
      if (wm->area[i].id == id) {
        *x = wm->area[i].r.x;
        *y = wm->area[i].r.y;
        r = 0;
        break;
      }
    }
  }

  return r;
}

int wman_draw_all(wman_t *wm) {
  int i, r = -1;

  if (wm) {
    for (i = 0; i < wm->n; i++) {
      draw_border(wm, i, i == wm->n-1);
      wm->wp->draw_texture_rect(wm->w, wm->area[i].t, 0, 0, wm->area[i].r.width, wm->area[i].r.height, wm->area[i].r.x, wm->area[i].r.y);
    }
    r = 0;
  }

  return r;
}

int wman_finish(wman_t *wm) {
  int r = -1;

  if (wm) {
    if (wm->background) wm->wp->destroy_texture(wm->w, wm->background);
    if (wm->hborder)    wm->wp->destroy_texture(wm->w, wm->hborder);
    if (wm->hs0border)  wm->wp->destroy_texture(wm->w, wm->hs0border);
    if (wm->hs1border)  wm->wp->destroy_texture(wm->w, wm->hs1border);
    if (wm->vborder)    wm->wp->destroy_texture(wm->w, wm->vborder);
    if (wm->vs0border)  wm->wp->destroy_texture(wm->w, wm->vs0border);
    if (wm->vs1border)  wm->wp->destroy_texture(wm->w, wm->vs1border);
    xfree(wm);
    r = 0;
  }

  return r;
}
