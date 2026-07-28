#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pwindow.h"
#include "audio.h"
#include "ptr.h"
#include "rgb.h"
#include "debug.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

#define RGB 3

typedef struct {
  int width, height;
  int spixel;
  uint8_t *buf;
  uint8_t *png;
  uint32_t count;
} libwnull_window_t;

struct texture_t {
  int width, height;
  uint8_t *buf;
};

static window_provider_t window_provider;

static window_t *libwnull_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  libwnull_window_t *window = NULL;

  if (encoding == ENC_RGB565) {
    if ((window = sys_calloc(1, sizeof(libwnull_window_t))) != NULL) {
      window->width = *width;
      window->height = *height;
      window->spixel = sizeof(uint16_t);
      if ((window->buf = sys_calloc(window->width * window->height, sizeof(uint16_t))) != NULL) {
        window->png = sys_calloc(1, window->width * window->height * RGB);
        debug(DEBUG_INFO, "WNULL", "create encoding=%d size=%dx%d w=%p", encoding, *width, *height, window);
      } else {
        sys_free(window);
        window = NULL;
      }
    }
  }

  return (window_t *)window;
}

static int libwnull_destroy(window_t *_window) {
  libwnull_window_t *window;
  int r = -1;

  window = (libwnull_window_t *)_window;
  debug(DEBUG_INFO, "WNULL", "destroy w=%p", window);

  if (window) {
    if (window->buf) sys_free(window->buf);
    if (window->png) sys_free(window->png);
    sys_free(window);
    r = 0;
  }

  return r;
}

static void window_stbi_write_func(void *context, void *data, int size) {
  int fd = *(int *)context;
  sys_write(fd, data, size);
}

static int libwnull_render(window_t *_window) {
  libwnull_window_t *window = (libwnull_window_t *)_window;
  char filename[64];
  uint16_t color;
  int i, j, k, l, fd, r = -1;

  debug(DEBUG_INFO, "WNULL", "render %p", window);
  if (window && window->png) {
    for (i = 0, l = 0, k = 0; i < window->height; i++) {
      for (j = 0; j < window->width; j++) {
        color = (window->buf[l+1] << 8) | window->buf[l];
        l += 2;
        window->png[k++] = r565(color);
        window->png[k++] = g565(color);
        window->png[k++] = b565(color);
      }
    }

    sys_snprintf(filename, sizeof(filename)-1, "w%05u.png", window->count++);
    if ((fd = sys_create(filename, SYS_WRITE | SYS_TRUNC, 0644)) != -1) {
    //if ((fd = sys_socket_open_connect("127.0.0.1", 65432, IP_STREAM)) != -1) {
      r = !stbi_write_png_to_func(window_stbi_write_func, &fd, window->width, window->height, RGB, window->png, window->width * RGB);
      sys_close(fd);
    }
  }

  return r;
}

static texture_t *libwnull_create_texture(window_t *_window, int width, int height) {
  libwnull_window_t *window;
  texture_t *texture = NULL;

  window = (libwnull_window_t *)_window;

  if (window) {
    if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
      texture->width = width;
      texture->height = height;
      if ((texture->buf = sys_calloc(texture->width * texture->height, window->spixel)) != NULL) {
        debug(DEBUG_INFO, "WNULL", "create_texture size=%dx%d t=%p", width, height, texture);
      } else {
        sys_free(texture);
        texture = NULL;
      }
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
    if (texture->buf) sys_free(texture->buf);
    sys_free(texture);
  }

  return r;
}

static int libwnull_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  libwnull_window_t *window;
  uint8_t *s, *d;
  int pitch, len, i, r = -1;

  debug(DEBUG_INFO, "WNULL", "update_texture_rect t=%p src=%p tpos=%d,%d tsize=%d,%d", texture, src, tx, ty, w, h);
  if (_window && texture && src && w > 0 && h > 0 && tx >= 0 && ty >= 0 && (tx+w) <= texture->width && (ty+h) <= texture->height) {
    window = (libwnull_window_t *)_window;
    s = &src[(ty * texture->width + tx) * window->spixel];
    d = &texture->buf[(ty * texture->width + tx) * window->spixel];
    pitch = texture->width * window->spixel;
    len = w * window->spixel;
    for (i = 0; i < h; i++) {
      sys_memcpy(d, s, len);
      s += pitch;
      d += pitch;
    } 
    r = 0;
  }
  
  return r;
}

static int libwnull_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  debug(DEBUG_INFO, "WNULL", "update_texture t=%p raw=%p", texture, raw);
  if (texture) {
    sys_memcpy(texture->buf, raw, texture->width * texture->height * sizeof(uint16_t));
  }

  return 0;
}

static int libwnull_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libwnull_window_t *window = (libwnull_window_t *)_window;
  uint8_t *s, *d;
  int spitch, dpitch, len, i, r = -1;

  debug(DEBUG_INFO, "WNULL", "draw_texture_rect t=%p tpos=%d,%d tsize=%d,%d pos=%d,%d", texture, tx, ty, w, h, x, y);
  if (window && texture && w > 0 && h > 0 && tx >= 0 && ty >= 0 && tx+w <= texture->width && ty+h <= texture->height &&
      x < window->width && y < window->height && x+w > 0 && y+h > 0) {

    if (x < 0) {
      tx -= x;
      w += x;
      x = 0;
    }

    if (y < 0) {
      ty -= y;
      h += y;
      y = 0;
    }

    if (w > 0 && h > 0) {
      if (x + w > window->width) {
        w = window->width - x;
      }

      if (y + h > window->height) {
        h = window->height - y;
      }

      s = (uint8_t *)&texture->buf[(ty * texture->width + tx) * window->spixel];
      d = (uint8_t *)&window->buf[(y * window->width + x) * window->spixel];
      spitch = texture->width * window->spixel;
      dpitch = window->width * window->spixel;
      len = w * window->spixel;
      for (i = 0; i < h; i++) {
        sys_memcpy(d, s, len);
        s += spitch;
        d += dpitch;
      }
      r = 0;
    } else {
      debug(DEBUG_ERROR, "WNULL", "invalid libwnull %d,%d %dx%d %d,%d", tx, ty, w, h, x, y);
    }
  }

  return r;
}

static int libwnull_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  return libwnull_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, 0, 0);
}

static void libwnull_status(window_t *window, int *x, int *y, int *buttons) {
  debug(DEBUG_INFO, "WNULL", "status w=%p x=%p y=%p buttons=%p", window, x, y, buttons);
  if (x) *x = 0;
  if (y) *y = 0;
  if (buttons) *buttons = 0;
}

static int libwnull_event2(window_t *window, int wait, int *arg1, int *arg2) {
  //debug(DEBUG_INFO, "WNULL", "event2 w=%p wait=%d arg1=%p arg2=%p", window, wait, arg1, arg2);
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
