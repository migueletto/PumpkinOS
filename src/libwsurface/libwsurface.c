#include "sys.h"
#include "script.h"
#include "media.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "ptr.h"
#include "debug.h"

struct texture_t {
  int width, height;
  surface_t *surface;
};

typedef struct {
  int width, height, encoding;
  int x, y, buttons;
} libwsurface_window_t;

static int surface_ptr;
static window_provider_t provider;

static window_t *libwsurface_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  libwsurface_window_t *window;
  surface_t *surface;
  int w, h, surface_encoding;

  if (surface_ptr == -1) {
    debug(DEBUG_ERROR, "WSURF", "invalid surface");
    return NULL;
  }
  if ((surface = ptr_lock(surface_ptr, TAG_SURFACE)) == NULL) {
    return NULL;
  }
  w = surface->width;
  h = surface->height;
  surface_encoding = surface->encoding;
  ptr_unlock(surface_ptr, TAG_SURFACE);

  switch (encoding) {
    case ENC_RGB565:
      if (surface_encoding != SURFACE_ENCODING_RGB565) {
        debug(DEBUG_ERROR, "WSURF", "invalid window encoding %s for surface encoding %d", video_encoding_name(encoding), surface_encoding);
        return NULL;
      }
      break;
    case ENC_RGBA:
      if (surface_encoding != SURFACE_ENCODING_ARGB) {
        debug(DEBUG_ERROR, "WSURF", "invalid window encoding %s for surface encoding %d", video_encoding_name(encoding), surface_encoding);
        return NULL;
      }
      break;
    default:
      debug(DEBUG_ERROR, "WSURF", "invalid window encoding %s", video_encoding_name(encoding));
      return NULL;
  }

  if (*width <= 0  || *width >= w)  *width = w;
  if (*height <= 0 || *height >= h) *height = h;

  if ((window = sys_calloc(1, sizeof(libwsurface_window_t))) != NULL) {
    window->encoding = surface_encoding;
    window->width = *width;
    window->height = *height;
  }

  return (window_t *)window;
}

static texture_t *libwsurface_window_create_texture(window_t *_window, int width, int height) {
  libwsurface_window_t *window = (libwsurface_window_t *)_window;
  texture_t *texture;

  if (window && (texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
    if ((texture->surface = surface_create(width, height, window->encoding)) != NULL) {
      texture->width = width;
      texture->height = height;
    } else {
      sys_free(texture);
      texture = NULL;
    }
  }

  return texture;
}

static int libwsurface_window_destroy_texture(window_t *_window, texture_t *texture) {
  int r = -1;

  if (texture) {
    if (texture->surface) surface_destroy(texture->surface);
    sys_free(texture);
    r = 0;
  }

  return r;
}

static int libwsurface_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  uint32_t *src32;
  uint16_t *src16;
  int i, j, k, r = -1;

  if (texture && texture->surface && src) {
    switch (texture->surface->encoding) {
      case SURFACE_ENCODING_RGB565:
        src16 = (uint16_t *)src;
        src16 = &src16[ty * texture->width + tx];
        for (i = 0, k = 0; i < h; i++) {
          for (j = 0; j < w; j++) {
            texture->surface->setpixel(texture->surface->data, j, i, src16[k + j]);
          }
          k += texture->width;
        }
        break;
      case SURFACE_ENCODING_ARGB:
        src32 = (uint32_t *)src;
        src32 = &src32[ty * texture->width + tx];
        for (i = 0, k = 0; i < h; i++) {
          for (j = 0; j < w; j++) {
            texture->surface->setpixel(texture->surface->data, j, i, src32[k + j]);
          }
          k += texture->width;
        }
        break;
    }
    r = 0;
  }

  return r;
}

static int libwsurface_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  return libwsurface_window_update_texture_rect(_window, texture, src, 0, 0, texture->width, texture->height);
}

static int libwsurface_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libwsurface_window_t *window = (libwsurface_window_t *)_window;
  surface_t *surface;
  int r = -1;

  if (window && texture && texture->surface && w > 0 && h > 0 && surface_ptr != -1) {
    if ((surface = ptr_lock(surface_ptr, TAG_SURFACE)) != NULL) {
      surface_draw(surface, x, y, texture->surface, tx, ty, w, h);
      ptr_unlock(surface_ptr, TAG_SURFACE);
      r = 0;
    }
  }

  return r;
}

static int libwsurface_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  return libwsurface_window_draw_texture_rect(_window, texture, 0, 0, texture->width, texture->height, x, y);
}

static void libwsurface_window_status(window_t *_window, int *x, int *y, int *buttons) {
  libwsurface_window_t *window = (libwsurface_window_t *)_window;

  if (window && x && y && buttons) {
    *x = window->x;
    *y = window->y;
    *buttons = window->buttons;
  }
}

static int libwsurface_window_event(window_t *window, int wait, int remove, int *key, int *mods, int *buttons) {
  return 0;
}

static int libwsurface_window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  libwsurface_window_t *window = (libwsurface_window_t *)_window;
  surface_t *surface;
  int ev = 0;

  if (window && surface_ptr != -1) {
    if ((surface = ptr_lock(surface_ptr, TAG_SURFACE)) != NULL) {
      ev = surface->event(surface->data, wait <= 0 ? wait : wait*1000, arg1, arg2);
      ptr_unlock(surface_ptr, TAG_SURFACE);

      switch (ev) {
        case SURFACE_EVENT_MOTION:
          window->x = *arg1;
          window->y = *arg2;
          break;
        case SURFACE_EVENT_BUTTONDOWN:
          window->buttons |= *arg1;
          break;
        case SURFACE_EVENT_BUTTONUP:
          window->buttons &= ~(*arg1);
          break;
      }
    }
  }

  return ev;
}

static int libwsurface_window_erase(window_t *_window, uint32_t bg) {
  libwsurface_window_t *window = (libwsurface_window_t *)_window;
  surface_t *surface;
  uint32_t c;
  int r = -1;

  if (window && surface_ptr != -1) {
    if ((surface = ptr_lock(surface_ptr, TAG_SURFACE)) != NULL) {
      c = surface_color_rgb(surface->encoding, surface->palette, surface->npalette, 0, 0, 0, 0xFF);
      surface_rectangle(surface, 0, 0, window->width - 1, window->height - 1, 1, c);
      ptr_unlock(surface_ptr, TAG_SURFACE);
      r = 0;
    }
  }

  return r;
}

static int libwsurface_window_destroy(window_t *_window) {
  libwsurface_window_t *window = (libwsurface_window_t *)_window;
  int r = -1;

  if (window) {
    sys_free(window);
    r = 0;
  }

  return r;
}

int libwsurface_surface(int pe) {
  script_int_t ptr;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    surface_ptr = ptr;
  }

  return 0;
}

int libwsurface_load(void) {
  sys_memset(&provider, 0, sizeof(window_provider_t));
  provider.create = libwsurface_window_create;
  provider.event = libwsurface_window_event;
  provider.destroy = libwsurface_window_destroy;
  provider.create_texture = libwsurface_window_create_texture;
  provider.destroy_texture = libwsurface_window_destroy_texture;
  provider.update_texture = libwsurface_window_update_texture;
  provider.update_texture_rect = libwsurface_window_update_texture_rect;
  provider.draw_texture = libwsurface_window_draw_texture;
  provider.draw_texture_rect = libwsurface_window_draw_texture_rect;
  provider.erase = libwsurface_window_erase;
  provider.status = libwsurface_window_status;
  provider.event2 = libwsurface_window_event2;

  surface_ptr = -1;

  return 0;
}

int libwsurface_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "WSURF", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &provider);

  script_add_function(pe, obj, "surface", libwsurface_surface);

  return 0;
}
