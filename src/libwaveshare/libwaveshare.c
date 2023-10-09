#include "sys.h"
#include "thread.h"
#include "script.h"
#include "pwindow.h"
#include "gpio.h"
#include "pspi.h"
#include "ili9486.h"
#include "ads7846.h"
#include "debug.h"

#define MIN_PRESSURE 25

typedef struct {
  spi_provider_t *spip;
  gpio_provider_t *gpiop;
  gpio_t *gpio;
  spi_t *tft_spi;
  spi_t *touch_spi;
  int width, height, len;
  int tft_cs, touch_cs, dc_pin, rst_pin, spi_speed;
  int x, y, down, button_down;
  uint16_t *p16;
  uint16_t *buf;
  uint16_t *aux;
  uint16_t xcal[5], ycal[5];
} libwaveshare_t;

struct texture_t {
  int width, height, size;
  uint16_t *p16;
};

static window_provider_t wp;
static libwaveshare_t ws;

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, void *data) {
  window_t *w = NULL;

  if ((ws.tft_spi = ws.spip->open(ws.tft_cs, ws.spi_speed, ws.spip->data)) != NULL &&
      (ws.touch_spi = ws.spip->open(ws.touch_cs, ws.spi_speed, ws.spip->data)) != NULL &&
      (ws.gpio = ws.gpiop->open(ws.gpiop->data)) != NULL) {

    if (ili9486_begin(ws.spip, ws.tft_spi, ws.gpiop, ws.gpio, ws.dc_pin, ws.rst_pin) == 0) {
      ili9486_enable(ws.spip, ws.tft_spi, ws.gpiop, ws.gpio, ws.dc_pin, 1);

      *width = ws.width;
      *height = ws.height;
      ws.len = ws.width * ws.height * sizeof(uint16_t);
      ws.p16 = sys_calloc(1, ws.len);
      ws.buf = sys_calloc(1, ws.len);
      ws.aux = sys_calloc(1, ws.len);
      w = (window_t *)&ws;
    }
  }

  return w;
}

static int window_destroy(window_t *window) {
  libwaveshare_t *ws = (libwaveshare_t *)window;

  if (ws) {
    ili9486_cls(ws->spip, ws->tft_spi, ws->gpiop, ws->gpio, ws->dc_pin, 0, 0, 0, ws->width, ws->height, ws->aux);
    ili9486_enable(ws->spip, ws->tft_spi, ws->gpiop, ws->gpio, ws->dc_pin, 0);
    ws->spip->close(ws->tft_spi);
    ws->spip->close(ws->touch_spi);
    ws->gpiop->close(ws->gpio);
    sys_free(ws->p16);
    sys_free(ws->buf);
    sys_free(ws->aux);
    ws->tft_spi = NULL;
    ws->touch_spi = NULL;
    ws->gpio = NULL;
  }

  return 0;
}

static int window_render(window_t *window) {
  libwaveshare_t *ws = (libwaveshare_t *)window;

  if (ws) {
    //ili9486_draw(ws->spip, ws->tft_spi, ws->gpiop, ws->gpio, ws->dc_pin, ws->p16, 0, 0, ws->width, ws->height, ws->aux);
  }

  return 0;
}

static texture_t *window_create_texture(window_t *window, int width, int height) {
  texture_t *texture;

  if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    texture->size = width * height * sizeof(uint16_t);
    texture->p16 = sys_malloc(texture->size);
  }

  return texture;
}

static int window_destroy_texture(window_t *window, texture_t *texture) {
  if (texture) {
    if (texture->p16) sys_free(texture->p16);
    sys_free(texture);
  }

  return 0;
}

static int window_update_texture_rect(window_t *window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  int i, j, tpitch, tindex;
  uint16_t *p16;

  if (texture && src) {
    tpitch = texture->width;
    tindex = ty * tpitch + tx;
    p16 = (uint16_t *)src;

    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        texture->p16[tindex + j] = p16[tindex + j];
      }
      tindex += tpitch;
    }
  }

  return 0;
}

static int window_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  if (texture && texture->p16 && raw) {
    sys_memcpy(texture->p16, raw, texture->size);
  }

  return 0;
}

static int window_draw_texture_rect(window_t *window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libwaveshare_t *ws = (libwaveshare_t *)window;
  int i, j, k, d, tpitch, wpitch, tindex, windex, r = -1;

  if (ws && texture) {
    if (x < 0) {
      tx -= x;
      w += x;
      x = 0;
    }
    if (x+w >= ws->width) {
      d = x+w - ws->width;
      w -= d;
    }
    if (y < 0) {
      ty -= y;
      h += y;
      y = 0;
    }
    if (y+h >= ws->height) {
      d = y+h - ws->height;
      h -= d;
    }

    if (w > 0 && h > 0) {
      tpitch = texture->width;
      wpitch = ws->width;
      tindex = ty * tpitch + tx;
      windex = y * wpitch + x;

      for (i = 0, k = 0; i < h; i++) {
        for (j = 0; j < w; j++, k++) {
          ws->p16[windex + j] = texture->p16[tindex + j];
          ws->buf[k] = texture->p16[tindex + j];
        }
        tindex += tpitch;
        windex += wpitch;
      }
      ili9486_draw(ws->spip, ws->tft_spi, ws->gpiop, ws->gpio, ws->dc_pin, ws->buf, x, y, w, h, ws->aux);
      r = 0;
    }
  }

  return r;
}

static int window_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  return window_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, x, y);
}

// return -1: error
// return  0: no screen tap
// return  1: screen tap at x,y

static int window_average(window_t *window, int *x, int *y, int ms) {
  libwaveshare_t *ws = (libwaveshare_t *)window;
  int pressure, ax, ay, tx, ty, n, down;

  down = 0;
  tx = ty = n = 0;

  for (;!thread_must_end();) {
    if (thread_must_end()) return -1;

    pressure = ads7846_pressure(ws->spip, ws->touch_spi);
    if (down) {
      if (pressure <= MIN_PRESSURE) {
        if (n == 0) {
          debug(DEBUG_ERROR, "WAVESHARE", "pressure %d, no points collected", pressure);
          return -1;
        }
        *x = tx / n;
        *y = ty / n;
        debug(DEBUG_ERROR, "WAVESHARE", "pressure %d, %d points collected, x=%d y=%d", pressure, n, *x, *y);
        return 1;
      }
      ay = ads7846_y(ws->spip, ws->touch_spi);
      ax = ads7846_x(ws->spip, ws->touch_spi);
      if (ax && ay) {
        tx += ax;
        ty += ay;
        n++;
      }
    } else {
      if (pressure > MIN_PRESSURE) {
        debug(DEBUG_INFO, "WAVESHARE", "pressure %d, collecting points", pressure);
        down = 1;
      } else {
        if (ms == -1) continue;
        if (ms == 0) return 0;
        ms--;
        sys_usleep(1000);
      }
    }
  }

  return -1;
}

static int window_event2(window_t *window, int wait, int *arg1, int *arg2) {
  libwaveshare_t *ws = (libwaveshare_t *)window;
  int r, x, y;

  if (ws->button_down) {
    ws->button_down = 0;
    *arg1 = 1;
    *arg2 = 0;
    debug(DEBUG_TRACE, "WAVESHARE", "button up");
    return WINDOW_BUTTONUP;
  }
  
  r = window_average(window, &x, &y, wait);
  if (r <= 0) return r;
  ws->button_down = 1;
  ws->x = x;
  ws->y = y;
  *arg1 = 1;
  *arg2 = 0;

  debug(DEBUG_TRACE, "WAVESHARE", "button down");
  return WINDOW_BUTTONDOWN;
}

static void window_status(window_t *window, int *x, int *y, int *buttons) {
  libwaveshare_t *ws = (libwaveshare_t *)window;

  if (ws) {
    *x = ws->x;
    *y = ws->y;
    *buttons = ws->down ? 1 : 0;
  }
}

static int setup(int pe) {
  script_int_t width, height, tft_cs, touch_cs, dc_pin, rst_pin, spi_speed;
  int r = -1;

  if (script_get_integer(pe, 0, &width) == 0 &&
      script_get_integer(pe, 1, &height) == 0 &&
      script_get_integer(pe, 2, &tft_cs) == 0 &&
      script_get_integer(pe, 3, &touch_cs) == 0 &&
      script_get_integer(pe, 4, &dc_pin) == 0 &&
      script_get_integer(pe, 5, &rst_pin) == 0 &&
      script_get_integer(pe, 6, &spi_speed) == 0) {

    ws.width = width;
    ws.height = height;
    ws.tft_cs = tft_cs;
    ws.touch_cs = touch_cs;
    ws.dc_pin = dc_pin;
    ws.rst_pin = rst_pin;
    ws.spi_speed = spi_speed;
    r = 0;
  }

  return r;
}

int libwaveshare_init(int pe, script_ref_t obj) {
  sys_memset(&wp, 0, sizeof(window_provider_t));
  sys_memset(&ws, 0, sizeof(libwaveshare_t));

  if ((ws.gpiop = script_get_pointer(pe, GPIO_PROVIDER)) == NULL) {
    debug(DEBUG_ERROR, "WAVESHARE", "gpio provider not found");
    return -1;
  }

  if ((ws.spip = script_get_pointer(pe, SPI_PROVIDER)) == NULL) {
    debug(DEBUG_ERROR, "WAVESHARE", "spi provider not found");
    return -1;
  }

  wp.create = window_create;
  wp.destroy = window_destroy;
  wp.render = window_render;
  wp.create_texture = window_create_texture;
  wp.destroy_texture = window_destroy_texture;
  wp.update_texture = window_update_texture;
  wp.draw_texture = window_draw_texture;
  wp.update_texture_rect = window_update_texture_rect;
  wp.draw_texture_rect = window_draw_texture_rect;
  wp.event2 = window_event2;
  wp.status = window_status;
  wp.average = window_average;
  wp.data = &ws;

  debug(DEBUG_INFO, "WAVESHARE", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &wp);

  script_add_function(pe, obj, "setup", setup);

  return 0;
}
