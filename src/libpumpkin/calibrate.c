#include <PalmOS.h>

#include "sys.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "calibrate.h"
#include "debug.h"

static const char *msg = "Tap on the screen";

static void set_calibration(int32_t *lcdx, int32_t *lcdy, int32_t *tpx, int32_t *tpy, calibration_t *c) {
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

static void drawTarget(int i, uint32_t color, int width, int height, int radius, int *x, int *y, surface_t *surface) {
  switch (i) {
    case 0:
      *x = radius;
      *y = radius;
      break;
    case 1:
      *x = width - radius;
      *y = height / 2;
      break;
    case 2:
      *x = width / 2;
      *y = height - radius;
      break;
  }

  graphic_ellipse(surface->data, *x, *y, radius, radius, false, color, surface->setpixel, NULL);
  graphic_ellipse(surface->data, *x, *y, 2*radius/3, 2*radius/3, false, color, surface->setpixel, NULL);
  graphic_ellipse(surface->data, *x, *y, radius/3, radius/3, false, color, surface->setpixel, NULL);
  surface->setpixel(surface->data, *x, *y, color);
}

void calibrate(window_provider_t *wp, window_t *w, int width, int height, calibration_t *c) {
  surface_t *surface;
  texture_t *texture;
  uint8_t *raw;
  uint32_t red, white, black;
  int lcdx[3], lcdy[3], tpx[3], tpy[3];
  int i, x, y, radius, len, font, fw, fh, r;

  if ((surface = surface_create(width, height, SURFACE_ENCODING_RGB565)) != NULL) {
    texture = wp->create_texture(w, width, height);
    white = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0xff, 0xff, 0xff, 0xff);
    black = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0x00, 0x00, 0x00, 0xff);
    red = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, 0xff, 0x00, 0x00, 0xff);
    raw = (uint8_t *)surface->getbuffer(surface->data, &len);
    surface_rectangle(surface, 0, 0, width-1, height-1, 1, white);
    radius = width / 10;

    font = 6;
    fw = surface_font_height(font);
    fh = surface_font_width(font);
    x = (width - fw * sys_strlen(msg)) / 2;
    y = height / 2 + radius + fh;
    surface_print(surface, x, y, (char *)msg, font, black, white);
    wp->update_texture_rect(w, texture, raw, 0, 0, width, height);
    wp->draw_texture_rect(w, texture, 0, 0, width, height, 0, 0);
    if (wp->render) wp->render(w);

    for (i = 0; i < 3;) {
      debug(DEBUG_INFO, "TOUCH", "calibrating point %d", i+1);
      drawTarget(i, red, width, height, radius, &x, &y, surface);
      wp->update_texture_rect(w, texture, raw, x - radius, y - radius, radius*2, radius*2);
      wp->draw_texture_rect(w, texture, x - radius, y - radius, radius*2, radius*2, x - radius, y - radius);
      if (wp->render) wp->render(w);
      lcdx[i] = x;
      lcdy[i] = y;
      r = wp->average(w, &tpx[i], &tpy[i], -1);
      if (r == -1) break;
      if (r == 0) continue;
      debug(DEBUG_INFO, "TOUCH", "target (%3d,%3d) clicked (%3d,%3d)", lcdx[i], lcdy[i], tpx[i], tpy[i]);
      drawTarget(i, white, width, height, radius, &x, &y, surface);
      wp->update_texture_rect(w, texture, raw, x - radius, y - radius, radius*2, radius*2);
      wp->draw_texture_rect(w, texture, x - radius, y - radius, radius*2, radius*2, x - radius, y - radius);
      i++;
    }

    if (i == 3) {
      set_calibration(lcdx, lcdy, tpx, tpy, c);
    }

    surface_rectangle(surface, 0, 0, width-1, height-1, 1, white);
    wp->update_texture_rect(w, texture, raw, 0, 0, width, height);
    wp->draw_texture_rect(w, texture, 0, 0, width, height, 0, 0);
    if (wp->render) wp->render(w);

    wp->destroy_texture(w, texture);
    surface_destroy(surface);
  }
}
