#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pwindow.h"
#include "audio.h"
#include "ptr.h"
#include "average.h"
#include "debug.h"

typedef struct {
  int width, height;
} libwnull_window_t;

struct texture_t {
  int width, height;
  uint32_t *buf;
};

static window_provider_t window_provider;

static window_t *libwnull_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  libwnull_window_t *window;

  if ((window = sys_calloc(1, sizeof(libwnull_window_t))) != NULL) {
    window->width = *width;
    window->height = *height;
    debug(DEBUG_INFO, "WNULL", "create encoding=%d size=%dx%d w=%p", encoding, *width, *height, window);
  }

  return (window_t *)window;
}

static int libwnull_destroy(window_t *_window) {
  libwnull_window_t *window;
  int r = -1;

  window = (libwnull_window_t *)_window;
  debug(DEBUG_INFO, "WNULL", "destroy w=%p", window);

  if (window) {
    sys_free(window);
    r = 0;
  }

  return r;
}

static int libwnull_render(window_t *window) {
  debug(DEBUG_INFO, "WNULL", "render %p", window);
  return 0;
}

static texture_t *libwnull_create_texture(window_t *_window, int width, int height) {
  libwnull_window_t *window;
  texture_t *texture = NULL;

  window = (libwnull_window_t *)_window;

  if (window) {
    if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
      texture->width = width;
      texture->height = height;
      debug(DEBUG_INFO, "WNULL", "create_texture size=%dx%d t=%p", width, height, texture);
    }
  }

  return texture;
}

static int libwnull_destroy_texture(window_t *_window, texture_t *texture) {
  libwnull_window_t *window;
  int r = -1;

  window = (libwnull_window_t *)_window;

  if (window && texture) {
    debug(DEBUG_INFO, "WNULL", "destroy_texture t=%p", texture);
    sys_free(texture);
  }

  return r;
}

static int libwnull_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  debug(DEBUG_INFO, "WNULL", "update_texture t=%p raw=%p", texture, raw);
  return 0;
}

static int libwnull_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  debug(DEBUG_INFO, "WNULL", "draw_texture t=%p pos=%d,%d", texture, x, y);
  return 0;
}

static void libwnull_status(window_t *window, int *x, int *y, int *buttons) {
  debug(DEBUG_INFO, "WNULL", "status w=%p x=%p y=%p buttons=%p", window, x, y, buttons);
}

static int libwnull_event2(window_t *window, int wait, int *arg1, int *arg2) {
  //debug(DEBUG_INFO, "WNULL", "event2 w=%p wait=%d arg1=%p arg2=%p", window, wait, arg1, arg2);
  return 0;
}

static int libwnull_draw_texture_rect(window_t *window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  debug(DEBUG_INFO, "WNULL", "draw_texture_rect t=%p tpos=%d,%d tsize=%d,%d pos=%d,%d", texture, tx, ty, w, h, x, y);
  return 0;
}

static int libwnull_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  debug(DEBUG_INFO, "WNULL", "update_texture_rect t=%p src=%p tpos=%d,%d tsize=%d,%d", texture, src, tx, ty, w, h);
  return 0;
}

int libwnull_load(void) {
  sys_memset(&window_provider, 0, sizeof(window_provider));
  window_provider.create = libwnull_create;
  window_provider.destroy = libwnull_destroy;
  window_provider.render = libwnull_render;
  window_provider.create_texture = libwnull_create_texture;
  window_provider.destroy_texture = libwnull_destroy_texture;
  window_provider.update_texture = libwnull_update_texture;
  window_provider.draw_texture = libwnull_draw_texture;
  window_provider.draw_texture_rect = libwnull_draw_texture_rect;
  window_provider.update_texture_rect = libwnull_update_texture_rect;
  window_provider.status = libwnull_status;
  window_provider.event2 = libwnull_event2;

  return 0;
}

int libwnull_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "WNULL", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);
  script_add_iconst(pe, obj, "hdepth", 16);

  return 0;
}

int libwnull_unload(void) {
  return 0;
}
