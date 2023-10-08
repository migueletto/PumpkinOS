#include <PalmOS.h>

#include "sys.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "calibrate.h"
#include "debug.h"

#define RADIUS 36

static const char *msg = "Tap on the screen";

static void set_calibration(uint32_t *lcdx, uint32_t *lcdy, uint32_t *tpx, uint32_t *tpy, calibration_t *c) {
  c->div = ((tpx[0] - tpx[2]) * (tpy[1] - tpy[2])) -
           ((tpx[1] - tpx[2]) * (tpy[0] - tpy[2]));

  c->a = ((lcdx[0] - lcdx[2]) * (tpy[1] - tpy[2])) -
         ((lcdx[1] - lcdx[2]) * (tpy[0] - tpy[2]));

  c->b = ((tpx[0] - tpx[2]) * (lcdx[1] - lcdx[2])) -
         ((lcdx[0] - lcdx[2]) * (tpx[1] - tpx[2]));

  c->c = (tpx[2] * lcdx[1] - tpx[1] * lcdx[2]) * tpy[0] +
         (tpx[0] * lcdx[2] - tpx[2] * lcdx[0]) * tpy[1] +
         (tpx[1] * lcdx[0] - tpx[0] * lcdx[1]) * tpy[2];

  c->d = ((lcdy[0] - lcdy[2]) * (tpy[1] - tpy[2])) -
         ((lcdy[1] - lcdy[2]) * (tpy[0] - tpy[2]));

  c->e = ((tpx[0] - tpx[2]) * (lcdy[1] - lcdy[2])) -
         ((lcdy[0] - lcdy[2]) * (tpx[1] - tpx[2]));

  c->f = (tpx[2] * lcdy[1] - tpx[1] * lcdy[2]) * tpy[0] +
         (tpx[0] * lcdy[2] - tpx[2] * lcdy[0]) * tpy[1] +
         (tpx[1] * lcdy[0] - tpx[0] * lcdy[1]) * tpy[2];
}

static void drawTarget(int i, uint32_t color, int width, int height, int *x, int *y, surface_t *surface) {
  switch (i) {
    case 0:
      *x = RADIUS;
      *y = RADIUS;
      break;
    case 1:
      *x = width - RADIUS;
      *y = height / 2;
      break;
    case 2:
      *x = width / 2;
      *y = height - RADIUS;
      break;
  }

  graphic_ellipse(surface->data, *x, *y, RADIUS, RADIUS, false, color, surface->setpixel, NULL);
  graphic_ellipse(surface->data, *x, *y, 2*RADIUS/3, 2*RADIUS/3, false, color, surface->setpixel, NULL);
  graphic_ellipse(surface->data, *x, *y, RADIUS/3, RADIUS/3, false, color, surface->setpixel, NULL);
  surface->setpixel(surface->data, *x, *y, color);
}

void pumpkin_calibrate(window_provider_t *wp, window_t *w, int width, int height, calibration_t *c) {
  surface_t *surface;
  texture_t *texture;
  uint8_t *raw;
  uint32_t red, white, black;
  uint32_t lcdx[3], lcdy[3], tpx[3], tpy[3];
  uint64_t t0, t;
  int done[3];
  int i, x, y, len, font, fw, fh, ev, arg1, arg2, stop;

  if ((surface = surface_create(width, height, SURFACE_ENCODING_RGB565)) != NULL) {
    texture = wp->create_texture(w, width, height);
    white = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0xff, 0xff, 0xff, 0xff);
    black = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x00, 0x00, 0x00, 0xff);
    red = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0xff, 0x00, 0x00, 0xff);
    raw = (uint8_t *)surface->getbuffer(surface->data, &len);
    surface_rectangle(surface, 0, 0, width-1, height-1, 1, white);

    font = 6;
    fw = surface_font_height(font);
    fh = surface_font_width(font);
    x = (width - fw * sys_strlen(msg)) / 2;
    y = height / 2 + RADIUS + fh;
    surface_print(surface, x, y, (char *)msg, font, black, white);

    i = 0;
    drawTarget(i, red, width, height, &x, &y, surface);
    lcdx[0] = x;
    lcdy[0] = y;
debug(1, "XXX", "lcd %d %d,%d", i, lcdx[i], lcdy[i]);
    done[0] = done[1] = done[2] = 0;
    wp->update_texture_rect(w, texture, raw, 0, 0, width, height);
    wp->draw_texture_rect(w, texture, 0, 0, width, height, 0, 0);
    if (wp->render) wp->render(w);
    t0 = sys_get_clock();

    for (stop = 0; !stop;) {
      ev = wp->event2(w, -1, &arg1, &arg2);

      switch (ev) {
        case WINDOW_BUTTONDOWN:
//debug(1, "XXX", "button down %d", i);
          break;
        case WINDOW_MOTION:
          if (!done[i]) {
            tpx[i] = arg1;
            tpy[i] = arg2;
            done[i] = 1;
debug(1, "XXX", "tp %d %d,%d", i, tpx[i], tpy[i]);
          }
          break;
        case WINDOW_BUTTONUP:
//debug(1, "XXX", "button up %d", i);
          t = sys_get_clock();
          if ((t - t0) < 100000) break;
          t0 = t;
          i++;
          drawTarget(i-1, white, width, height, &x, &y, surface);
          wp->update_texture_rect(w, texture, raw, x - RADIUS, y - RADIUS, RADIUS*2, RADIUS*2);
          wp->draw_texture_rect(w, texture, x - RADIUS, y - RADIUS, RADIUS*2, RADIUS*2, x - RADIUS, y - RADIUS);
          if (i == 3) {
debug(1, "XXX", "set_calibration");
            set_calibration(lcdx, lcdy, tpx, tpy, c);
            stop = 1;
            break;
          }
          drawTarget(i, red, width, height, &x, &y, surface);
          lcdx[i] = x;
          lcdy[i] = y;
debug(1, "XXX", "lcd %d %d,%d", i, lcdx[i], lcdy[i]);
          wp->update_texture_rect(w, texture, raw, 0, 0, width, height);
          wp->draw_texture_rect(w, texture, x - RADIUS, y - RADIUS, RADIUS*2, RADIUS*2, x - RADIUS, y - RADIUS);
          if (wp->render) wp->render(w);
          break;
        case -1:
          stop = 1;
          break;
      }
    }

    surface_rectangle(surface, 0, 0, width-1, height-1, 1, white);
    wp->update_texture_rect(w, texture, raw, 0, 0, width, height);
    wp->draw_texture_rect(w, texture, 0, 0, width, height, 0, 0);
    if (wp->render) wp->render(w);

    wp->destroy_texture(w, texture);
    surface_destroy(surface);
  }
}
