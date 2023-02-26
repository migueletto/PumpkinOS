#include <PalmOS.h>

#include <stdlib.h>
#include <string.h>

#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "media.h"
#include "sys.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "grail.h"
#include "dia.h"
#include "debug.h"
#include "xalloc.h"

#define DIA_WIDTH     160
#define DIA_HEIGHT    240
#define DIA_GHEIGHT   80
#define ALPHA_WIDTH   97
#define ALPHA_HEIGHT  65

#define CODE_CAPS   157
#define CODE_SHIFT  158

#define ICON_HOME     0
#define ICON_MENU     1
#define ICON_FIND     2
#define ICON_KBD      3
#define ICON_SHIFT    4
#define ICON_UP_DOWN 10

#define TRIGGER_UP   27
#define TRIGGER_DOWN 28

struct dia_t {
  window_provider_t *wp;
  window_t *w;
  int first, mode, sel;
  int width, height;
  int graffiti_height;
  int alpha_width;
  int alpha_height;
  int taskbar_height;
  int icon_width;
  int graffiti_alpha;
  uint64_t graffiti_t0;
  uint64_t last;
  int graffiti_drawn;
  int stroke_dirty;
  int graffiti_dirty;
  int graffiti_state;
  int grail_state;
  int trigger, state;
  int dbl;
  uint32_t fg, bg;
  surface_t *surface;
  WinHandle graffiti_wh;
  WinHandle lower_wh;
  WinHandle upper_wh;
  WinHandle number_wh;
  WinHandle mode_wh;
  WinHandle wh;
  texture_t *graffiti;
  int xmin, xmax, ymin, ymax;
  RectangleType bounds_lower[256];
  RectangleType bounds_upper[256];
  RectangleType bounds_number[256];
  RectangleType *bounds;
};

static void dia_invert_button(dia_t *dia, int i) {
  RectangleType rect;
  UInt16 prev;

  prev = WinSetCoordinateSystem(dia->dbl ? kCoordinatesDouble : kCoordinatesStandard);
  WinHandle old = WinSetDrawWindow(dia->wh);
  RctSetRectangle(&rect, i * dia->icon_width, dia->alpha_height, dia->icon_width, dia->taskbar_height);
  WinInvertRectangle(&rect, 0);
  WinSetDrawWindow(old);
  WinSetCoordinateSystem(prev);
  dia->first = 1;
}

static void dia_invert_key(dia_t *dia, int i) {
  RectangleType rect;
  UInt16 prev;

  prev = WinSetCoordinateSystem(dia->dbl ? kCoordinatesDouble : kCoordinatesStandard);
  WinHandle old = WinSetDrawWindow(dia->wh);
  MemMove(&rect, &dia->bounds[i], sizeof(RectangleType));
  WinInvertRectangle(&rect, 0);
  WinSetDrawWindow(old);
  WinSetCoordinateSystem(prev);
  dia->first = 1;
}

dia_t *dia_init(window_provider_t *wp, window_t *w, int encoding, int depth, int dbl) {
  dia_t *dia = NULL;
  RGBColorType fg, bg;

  debug(DEBUG_INFO, "DIA", "setting DIA");

  if ((dia = xcalloc(1, sizeof(dia_t))) != NULL) {
    dia->wp = wp;
    dia->w = w;
    dia->dbl = dbl;
    dia->width = DIA_WIDTH;
    dia->height = DIA_HEIGHT;
    dia->graffiti_height = DIA_GHEIGHT;
    dia->alpha_width = ALPHA_WIDTH;
    dia->alpha_height = ALPHA_HEIGHT;
    if (dbl) {
      dia->width *= 2;
      dia->height *= 2;
      dia->graffiti_height *= 2;
      dia->alpha_width *= 2;
      dia->alpha_height *= 2;
      dia->icon_width = 29;  // 11*29 + 1 = 320
    } else {
      dia->icon_width = 14;  // 11*14 + 1 = 155
    }

    dia_color(&fg, &bg);
    dia->surface = surface_create(dia->width, dia->graffiti_height, encoding);
    dia->fg = surface_color_rgb(encoding, dia->surface->palette, dia->surface->npalette, fg.r, fg.g, fg.b, 0xff);
    dia->bg = surface_color_rgb(encoding, dia->surface->palette, dia->surface->npalette, bg.r, bg.g, bg.b, 0xff);
    surface_rectangle(dia->surface, 0, 0, dia->width - 1, dia->graffiti_height - 1, 1, dia->bg);

    dia->graffiti = wp->create_texture(w, dia->width, dia->graffiti_height);
    dia->taskbar_height = dia->graffiti_height - dia->alpha_height;
    dia->graffiti_alpha = -1;
    dia->trigger = 1;
    dia->state = 1;
    dia->sel = -1;
    dia->mode = DIA_MODE_GRAF;
    dia->graffiti_dirty = 1;
    dia->xmin = dia->width;
    dia->ymin = dia->height;
    dia->xmax = dia->ymax = 0;
    dia->stroke_dirty = 0;
    dia->first = 1;
  }

  return dia;
}

static void dia_draw_bmp(UInt16 resID, Coord x, Coord y) {
  MemHandle h;
  BitmapType *bmp;

  if ((h = DmGetResource(bitmapRsc, resID)) != NULL) {
    if ((bmp = MemHandleLock(h)) != NULL) {
      WinPaintBitmap(bmp, x, y);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }
}

void dia_set_wh(dia_t *dia, int mode, WinHandle wh, RectangleType *bounds) {
  WinHandle old;
  UInt16 prev;
  RGBColorType fg, bg;
  RectangleType rect;
  Err err;

  switch (mode) {
    case DIA_MODE_LOWER:
      dia->lower_wh = wh;
      xmemcpy(dia->bounds_lower, bounds, sizeof(RectangleType)*256);
      break;
    case DIA_MODE_UPPER:
      dia->upper_wh = wh;
      xmemcpy(dia->bounds_upper, bounds, sizeof(RectangleType)*256);
      break;
    case DIA_MODE_NUMBER:
      dia->number_wh = wh;
      xmemcpy(dia->bounds_number, bounds, sizeof(RectangleType)*256);
      break;
  }

  prev = WinSetCoordinateSystem(dia->dbl ? kCoordinatesDouble : kCoordinatesStandard);
  if (dia->wh == NULL) {
    dia->wh = WinCreateOffscreenWindow(dia->width, dia->graffiti_height, nativeFormat, &err);
    old = WinSetDrawWindow(dia->wh);
    WinPushDrawState();
    dia_color(&fg, &bg);
    WinSetTextColorRGB(&fg, NULL);
    WinSetBackColorRGB(&bg, NULL);
    RctSetRectangle(&rect, 0, 0, dia->width, dia->graffiti_height);
    WinEraseRectangle(&rect, 0);
    WinPopDrawState();
    dia_draw_bmp(32501, 0, dia->alpha_height);
    WinSetDrawWindow(old);
  }

  if (dia->graffiti_wh == NULL) {
    if ((dia->graffiti_wh = WinCreateOffscreenWindow(dia->width, dia->graffiti_height - dia->taskbar_height, nativeFormat, &err)) != NULL) {
      old = WinSetDrawWindow(dia->graffiti_wh);
      dia_draw_bmp(32500, 0, 0);
      WinSetDrawWindow(old);
    }
  }
  WinSetCoordinateSystem(prev);
}

void dia_color(RGBColorType *fg, RGBColorType *bg) {
  fg->r = 255;
  fg->g = 255;
  fg->b = 255;
  bg->r = 49;
  bg->g = 48;
  bg->b = 49;
}

static void draw_symbol(dia_t *dia, int cond, int s, int i) {
  WinHandle prev;
  RGBColorType fg, bg;
  RectangleType rect;
  uint16_t fw, fh;
  uint8_t *raw;
  int x, y, len;

  prev = WinSetDrawWindow(dia->wh);
  WinPushDrawState();
  dia_color(&fg, &bg);
  WinSetTextColorRGB(&fg, NULL);
  WinSetBackColorRGB(&bg, NULL);
  if (dia->dbl) WinSetCoordinateSystem(kCoordinatesDouble);
  FntSetFont(symbolFont);
  fw = FntCharWidth(GRAFFITI_CAPS);
  fh = FntCharHeight();
  x = i * dia->icon_width + (dia->icon_width - fw) / 2;
  y = dia->alpha_height + (dia->taskbar_height - fh) / 2;
  RctSetRectangle(&rect, x, y, fw, fh);
  WinEraseRectangle(&rect, 0);
  if (cond) {
    WinPaintChar(s, x, y);
  }
  WinPopDrawState();
  WinSetDrawWindow(prev);

  BmpDrawSurface(WinGetBitmap(dia->wh), x, y, fw, fh, dia->surface, x, y, true);

  raw = (uint8_t *)dia->surface->getbuffer(dia->surface->data, &len);
  dia->wp->update_texture_rect(dia->w, dia->graffiti, raw, x, y, fw, fh);
  dia->wp->draw_texture_rect(dia->w, dia->graffiti, x, y, fw, fh, x, dia->height - dia->graffiti_height + y);
}

static void draw_graffiti_state(dia_t *dia) {
  draw_symbol(dia, dia->graffiti_state, dia->graffiti_state, ICON_SHIFT);
}

static void draw_trigger(dia_t *dia) {
  draw_symbol(dia, dia->trigger, dia->state ? TRIGGER_DOWN : TRIGGER_UP, ICON_UP_DOWN);
}

static void dia_set_mode(dia_t *dia, int mode) {
  switch (mode) {
    case DIA_MODE_GRAF:
      dia->bounds = NULL;
      break;
    case DIA_MODE_LOWER:
      dia->bounds = dia->bounds_lower;
      break;
    case DIA_MODE_UPPER:
      dia->bounds = dia->bounds_upper;
      break;
    case DIA_MODE_NUMBER:
      dia->bounds = dia->bounds_number;
      break;
  }

  dia->mode = mode;
  dia->graffiti_dirty = true;
  dia->first = true;
}

void dia_set_graffiti_state(dia_t *dia, uint16_t state) {
  dia->graffiti_state = state;
  dia->graffiti_dirty = true;
}

int dia_update(dia_t *dia) {
  uint64_t now;
  uint8_t *raw;
  UInt16 prev;
  WinHandle old;
  BitmapType *bmp;
  int len, r = 0;

  now = sys_get_clock();

  if (dia->graffiti_drawn && (now - dia->graffiti_t0) > 500000) {
    dia->graffiti_drawn = 0;
    dia->first = true;
  }

  if (dia->first) {
    dia->stroke_dirty = 0;

    switch (dia->mode) {
      case DIA_MODE_GRAF:
        bmp = WinGetBitmap(dia->graffiti_wh);
        break;
      case DIA_MODE_LOWER:
        bmp = WinGetBitmap(dia->lower_wh);
        break;
      case DIA_MODE_UPPER:
        bmp = WinGetBitmap(dia->upper_wh);
        break;
      case DIA_MODE_NUMBER:
        bmp = WinGetBitmap(dia->number_wh);
        break;
      default:
        bmp = NULL;
        break;
    }
    if (bmp) {
      prev = WinSetCoordinateSystem(dia->dbl ? kCoordinatesDouble : kCoordinatesStandard);
      old = WinSetDrawWindow(dia->wh);
      WinPaintBitmap(bmp, 0, 0);
      WinSetDrawWindow(old);
      WinSetCoordinateSystem(prev);
    }

    draw_graffiti_state(dia);
    draw_trigger(dia);

    BmpDrawSurface(WinGetBitmap(dia->wh), 0, 0, dia->width, dia->graffiti_height, dia->surface, 0, 0, true);

    raw = (uint8_t *)dia->surface->getbuffer(dia->surface->data, &len);
    dia->wp->update_texture(dia->w, dia->graffiti, raw);
    if (dia->state) {
      // show whole DIA
      dia->wp->draw_texture(dia->w, dia->graffiti, 0, dia->height - dia->graffiti_height);
    } else {
      // show only taskbar
      dia->wp->draw_texture_rect(dia->w, dia->graffiti, 0, dia->graffiti_height - dia->taskbar_height, dia->width, dia->taskbar_height, 0, dia->height - dia->taskbar_height);
    }
    dia->first = 0;
    r = 1;
  }

  if (dia->stroke_dirty) {
    if (dia->xmax > dia->xmin && dia->ymax > dia->ymin) {
      raw = (uint8_t *)dia->surface->getbuffer(dia->surface->data, &len);
      dia->wp->update_texture_rect(dia->w, dia->graffiti, raw, dia->xmin, dia->ymin, dia->xmax-dia->xmin+1, dia->ymax-dia->ymin+1);
      dia->wp->draw_texture_rect(dia->w, dia->graffiti, dia->xmin, dia->ymin, dia->xmax-dia->xmin+1, dia->ymax-dia->ymin+1, 0 + dia->xmin, dia->height - dia->graffiti_height + dia->ymin);
      dia->xmin = dia->width;
      dia->ymin = dia->height;
      dia->xmax = dia->ymax = 0;
      r = 1;
    }
    dia->stroke_dirty = 0;
  }

  if (dia->graffiti_dirty) {
    draw_graffiti_state(dia);
    draw_trigger(dia);
    dia->graffiti_dirty = 0;
    r = 1;
  }

  return r;
}

int dia_finish(dia_t *dia) {
  int r = -1;

  if (dia) {
    if (dia->graffiti) dia->wp->destroy_texture(dia->w, dia->graffiti);
    if (dia->surface) surface_destroy(dia->surface);

    if (dia->lower_wh) WinDeleteWindow(dia->lower_wh, false);
    if (dia->upper_wh) WinDeleteWindow(dia->upper_wh, false);
    if (dia->number_wh) WinDeleteWindow(dia->number_wh, false);
    if (dia->graffiti_wh) WinDeleteWindow(dia->graffiti_wh, false);
    if (dia->wh) WinDeleteWindow(dia->wh, false);

    xfree(dia);
    r = 0;
  }

  return r;
}

int dia_clicked(dia_t *dia, int current_task, int x, int y, int down) {
  uint32_t aux;
  int i, c, glyph, r = -1;

  if (y >= (dia->height - dia->taskbar_height) && y < dia->height && x >= 0 && x < dia->width) {
    // taskbar area
    i = x / dia->icon_width;

    switch (i) {
      case ICON_HOME:
      case ICON_MENU:
      case ICON_FIND:
      case ICON_KBD:
      case ICON_UP_DOWN:
        dia_invert_button(dia, i);
        break;
    }

    if (!down) {
      if (dia->sel != -1) {
        dia_invert_key(dia, dia->sel);
        dia->sel = -1;
      }

      switch (i) {
        case ICON_HOME:
          if (current_task >= 0 && pumpkin_is_launched()) {
            pumpkin_forward_event(current_task, MSG_KEY, WINDOW_KEY_HOME, 0, 0);
          }
          break;
        case ICON_MENU:
          if (current_task >= 0) {
            pumpkin_forward_event(current_task, MSG_KEY, WINDOW_KEY_F5, 0, 0);
          }
          break;
        case ICON_FIND:
          // not implemented
          //if (current_task >= 0) {
          //  pumpkin_forward_event(current_task, MSG_KEY, WINDOW_KEY_F6, 0, 0);
          //}
          break;
        case ICON_KBD:
          switch (dia->mode) {
            case DIA_MODE_GRAF:
              dia_set_mode(dia, (dia->graffiti_state == GRAFFITI_SHIFT || dia->graffiti_state == GRAFFITI_CAPS) ? DIA_MODE_UPPER : DIA_MODE_LOWER);
              break;
            case DIA_MODE_LOWER:
            case DIA_MODE_UPPER:
              dia_set_mode(dia, DIA_MODE_NUMBER);
              break;
            case DIA_MODE_NUMBER:
              dia_set_mode(dia, DIA_MODE_GRAF);
              break;
          }
          break;
        case ICON_UP_DOWN:
          if (dia->trigger) {
            pumpkin_dia_set_state(!dia->state);
          }
          break;
      }
    }
    r = 0;

  } else if (dia->state && y >= (dia->height - dia->graffiti_height) && y < (dia->height - dia->taskbar_height) && x >= 0 && x < dia->width) {

    if (dia->mode == DIA_MODE_GRAF) {
      // graffiti area

      if (down) {
        if (dia->graffiti_alpha == -1) {
          dia->graffiti_alpha = x < dia->alpha_width;
          grail_begin(x < dia->alpha_width);
        }
      } else {
        if (dia->sel != -1) {
          dia_invert_key(dia, dia->sel);
          dia->sel = -1;
        }
        if (dia->graffiti_alpha != -1) {
          glyph = grail_end(&aux);
          if (aux != dia->grail_state) {
            dia->grail_state = aux;
            if (dia->grail_state & GRAIL_CAPSLOCK) {
              if (dia->mode == DIA_MODE_LOWER) dia_set_mode(dia, DIA_MODE_UPPER);
              KbdGrfSetState(GRAFFITI_CAPS);
            } else if (dia->grail_state & GRAIL_CAPS) {
              if (dia->mode == DIA_MODE_LOWER) dia_set_mode(dia, DIA_MODE_UPPER);
              KbdGrfSetState(GRAFFITI_SHIFT);
            } else if (dia->grail_state & GRAIL_PUNCT) {
              KbdGrfSetState(GRAFFITI_PUNCT);
            } else {
              if (dia->mode == DIA_MODE_UPPER) dia_set_mode(dia, DIA_MODE_LOWER);
              KbdGrfSetState(0);
            }
          }
          if (glyph) {
            debug(DEBUG_TRACE, "DIA", "glyph '%c'", glyph);
            if (current_task >= 0) {
              pumpkin_forward_event(current_task, MSG_KEY, glyph, 0, 0);
            }
          }
        }
      }
      r = 0;

    } else {
      // keyboard area
      y -= (dia->height - dia->graffiti_height);

      if (down) {
        for (i = 0; i < 256; i++) {
          if (dia->bounds[i].extent.x && RctPtInRectangle(x, y, &dia->bounds[i])) {
            dia_invert_key(dia, i);
            dia->sel = i;
            break;
          }
        }
      } else {
        if (dia->sel != -1) {
          dia_invert_key(dia, dia->sel);
        }

        for (i = 0; i < 256; i++) {
          if (dia->bounds[i].extent.x && RctPtInRectangle(x, y, &dia->bounds[i]) && i == dia->sel) {
            switch (i) {
              case CODE_CAPS:
                grail_reset();
                dia->grail_state = 0;
                KbdGrfSetState(dia->graffiti_state == GRAFFITI_CAPS ? 0 : GRAFFITI_CAPS);
                dia_set_mode(dia, dia->graffiti_state == GRAFFITI_CAPS ? DIA_MODE_UPPER : DIA_MODE_LOWER);
                break;
              case CODE_SHIFT:
                grail_reset();
                dia->grail_state = 0;
                KbdGrfSetState(dia->graffiti_state == GRAFFITI_SHIFT ? 0 : GRAFFITI_SHIFT);
                dia_set_mode(dia, dia->graffiti_state == GRAFFITI_SHIFT ? DIA_MODE_UPPER : DIA_MODE_LOWER);
                break;
              default:
                c = i;
                if (current_task >= 0) {
                  pumpkin_forward_event(current_task, MSG_KEY, c, 0, 0);
                }
                if (dia->graffiti_state == GRAFFITI_SHIFT) {
                  KbdGrfSetState(0);
                  dia_set_mode(dia, DIA_MODE_LOWER);
                }
                break;
            }
            break;
          }
        }
        dia->sel = -1;
        r = 0;
      }
    }
  }

  if (!down) {
    dia->graffiti_alpha = -1;
  }

  return r;
}

int dia_set_trigger(dia_t *dia, int trigger) {
  int r = -1;

  if (dia) {
    if (dia->trigger != trigger) {
      dia->trigger = trigger;
      dia->graffiti_dirty = 1;
    }
    r = 0;
  }

  return r;
}

int dia_get_trigger(dia_t *dia) {
  return dia ? dia->trigger : -1;
}

int dia_set_state(dia_t *dia, int state) {
  uint8_t *raw;
  int len, r = -1;

  if (dia) {
    if (dia->state != state) {
      dia->state = state;
      if (dia->state == 0) {
        // erase DIA (minus taskbar) with white
        raw = (uint8_t *)dia->surface->getbuffer(dia->surface->data, &len);
        surface_rectangle(dia->surface, 0, 0, dia->width - 1, dia->graffiti_height - dia->taskbar_height - 1, 1, dia->bg);
        dia->wp->update_texture_rect(dia->w, dia->graffiti, raw, 0, 0, dia->width, dia->graffiti_height - dia->taskbar_height);
        dia->wp->draw_texture_rect(dia->w, dia->graffiti, 0, 0, dia->width, dia->graffiti_height - dia->taskbar_height, 0, dia->height - dia->graffiti_height);
      }
      debug(DEBUG_INFO, "DIA", "dia_set_state %d", state);
      dia->first = 1;
    }
    r = 0;
  }

  return r;
}

int dia_get_state(dia_t *dia) {
  return dia ? dia->state : -1;
}

int dia_get_main_dimension(dia_t *dia, int *width, int *height) {
  int r = -1;

  if (dia && width && height) {
    *width = dia->width;
    *height = dia->state ? dia->height - dia->graffiti_height : dia->height - dia->taskbar_height;
    r = 0;
  }

  return r;
}

int dia_get_graffiti_dimension(dia_t *dia, int *width, int *height) {
  int r = -1;

  if (dia && width && height) {
    *width = dia->width;
    *height = dia->graffiti_height - dia->taskbar_height;
    r = 0;
  }

  return r;
}

int dia_get_taskbar_dimension(dia_t *dia, int *width, int *height) {
  int r = -1;

  if (dia && width && height) {
    *width = dia->width;
    *height = dia->taskbar_height;
    r = 0;
  }

  return r;
}

int dia_stroke(dia_t *dia, int x, int y) {
  int r = 0;

  if (dia->graffiti_alpha != -1 &&
      y >= (dia->height - dia->graffiti_height) &&
      y <  (dia->height - dia->taskbar_height) && x < dia->width) {

    switch (dia->graffiti_alpha) {
      case 1:
        if (x < dia->alpha_width) {
          // alpha graffiti area
          grail_stroke(x, y - (dia->height - dia->graffiti_height));
        }
        break;
      case 0:
        if (x >= dia->alpha_width) {
          // numeric graffiti area
          grail_stroke(x - dia->alpha_width, y - (dia->height - dia->graffiti_height));
        }
        break;
    }
    r = 1;
  }

  return r;
}

void dia_draw_stroke(dia_t *dia, int x1, int y1, int x2, int y2, int alpha) {
  uint32_t c;
  int dx, rx, ry;

  dx = alpha ? 0 : dia->alpha_width;
  rx = x2 - x1;
  ry = y2 - y1;
  if (rx < 0) rx = -rx;
  if (ry < 0) ry = -ry;

  c = surface_color_rgb(dia->surface->encoding, dia->surface->palette, dia->surface->npalette, 0xff, 0x00, 0x00, 0xff);
  surface_line(dia->surface, dx+x1, y1, dx+x2, y2, c);
  if (rx > ry) {
    surface_line(dia->surface, dx+x1, y1-1, dx+x2, y2-1, c);
    surface_line(dia->surface, dx+x1, y1+1, dx+x2, y2+1, c);
  } else {
    surface_line(dia->surface, dx+x1-1, y1, dx+x1-1, y1, c);
    surface_line(dia->surface, dx+x1+1, y1, dx+x1+1, y1, c);
  }
  if (dx+x1 < dia->xmin) dia->xmin = dx+x1;
  else if (dx+x1 > dia->xmax) dia->xmax = dx+x1;
  if (dx+x2 < dia->xmin) dia->xmin = dx+x2;
  else if (dx+x2 > dia->xmax) dia->xmax = dx+x2;
  if (y1 < dia->ymin) dia->ymin = y1;
  else if (y1 > dia->ymax) dia->ymax = y1;
  if (y2 < dia->ymin) dia->ymin = y2;
  else if (y2 > dia->ymax) dia->ymax = y2;

  dia->graffiti_t0 = sys_get_clock();
  dia->graffiti_drawn = 1;
  dia->stroke_dirty = 1;
}
