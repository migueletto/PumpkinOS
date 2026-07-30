#include "sys.h"
#include "endianness.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pwindow.h"
#include "audio.h"
#include "ptr.h"
#include "rgb.h"
#include "debug.h"

#define PORT 65432

#define RGB 3

typedef struct {
  int fd, encoding, width, height, spixel;
  int x, y, last_key;
  uint32_t buttons;
  uint64_t last_timestamp;
  uint8_t *buf;
} libwnull_window_t;

struct texture_t {
  int width, height;
  uint8_t *buf;
};

static window_provider_t window_provider;

#define CMD_WINDOW  1
#define CMD_FINISH  2
#define CMD_MOTION  3
#define CMD_BUTTON  4
#define CMD_KEYDOWN 5
#define CMD_KEYUP   6
#define CMD_DRAW    7

static int send_window_cmd(libwnull_window_t *window) {
  uint16_t cmd[4];
  int r = -1;

  if (window) {
    cmd[0] = sys_htole16(CMD_WINDOW);
    cmd[1] = sys_htole16(window->encoding);
    cmd[2] = sys_htole16(window->width);
    cmd[3] = sys_htole16(window->height);
    r = sys_write(window->fd, (uint8_t *)cmd, 8) == 8 ? 0 : -1;
  }

  return r;
}

static int send_draw_cmd(libwnull_window_t *window, int x, int y, int w, int h) {
  uint16_t cmd[5];
  int r = -1;

  if (window) {
    cmd[0] = sys_htole16(CMD_DRAW);
    cmd[1] = sys_htole16(x);
    cmd[2] = sys_htole16(y);
    cmd[3] = sys_htole16(w);
    cmd[4] = sys_htole16(h);
    r = sys_write(window->fd, (uint8_t *)cmd, 10) == 10 ? 0 : -1;
  }

  return r;
}

static int send_finish_cmd(libwnull_window_t *window) {
  uint16_t cmd;
  int r = -1;

  if (window) {
    cmd = sys_htole16(CMD_FINISH);
    if (sys_write(window->fd, (uint8_t *)&cmd, 2) == 2) {
      r = 0;
    }
  }

  return r;
}

static window_t *libwnull_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  libwnull_window_t *window = NULL;
  sys_timeval_t tv;
  char host[256];
  int port, fd;

  if (encoding == ENC_RGB565) {
    if ((window = sys_calloc(1, sizeof(libwnull_window_t))) != NULL) {

      if (window_provider.data) {
        // working as a client
        window->fd = sys_socket_open_connect((char *)window_provider.data, PORT, IP_STREAM);
      } else {
        // working as a server
        window->fd = -1;
        port = PORT;
        if ((fd = sys_socket_bind("0.0.0.0", &port, IP_STREAM)) != -1) {
          tv.tv_sec = 15;
          tv.tv_usec = 0;
          debug(DEBUG_INFO, "WNULL", "waiting for client to connect ...");
          window->fd = sys_socket_accept(fd, host, sizeof(host), &port, &tv);
          if (window->fd > 0) {
            debug(DEBUG_INFO, "WNULL", "client %s connected", host);
          } else if (window->fd == 0) {
            debug(DEBUG_INFO, "WNULL", "no client did connect");
            window->fd = -1;
          }
          sys_close(fd);
        }
      }

      if (window->fd != -1) {
        window->encoding = encoding;
        window->width = *width;
        window->height = *height;
        window->spixel = sizeof(uint16_t);
        if ((window->buf = sys_calloc(window->width * window->height, window->spixel)) != NULL) {
          if (send_window_cmd(window) == 0) {
            debug(DEBUG_INFO, "WNULL", "create encoding=%d size=%dx%d w=%p", encoding, window->width, window->height, window);
          } else {
            sys_close(window->fd);
            sys_free(window);
            window = NULL;
          }
        } else {
          sys_free(window);
          window = NULL;
        }
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
    send_finish_cmd(window);
    if (window->fd > 0) sys_close(window->fd);
    if (window->buf) sys_free(window->buf);
    sys_free(window);
    r = 0;
  }

  return r;
}

static int libwnull_render(window_t *_window) {
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
      send_draw_cmd(window, x, y, w, h);
      for (i = 0; i < h; i++) {
        sys_memcpy(d, s, len);
        sys_write(window->fd, (uint8_t *)d, len);
        s += spitch;
        d += dpitch;
      }
      r = 0;
    } else {
      debug(DEBUG_ERROR, "WNULL", "invalid libwnull w/d %d,%d %dx%d %d,%d", tx, ty, w, h, x, y);
    }
  } else {
    debug(DEBUG_ERROR, "WNULL", "invalid libwnull %d,%d %dx%d %d,%d", tx, ty, w, h, x, y);
  }

  return r;
}

static int libwnull_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  return libwnull_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, 0, 0);
}

static void libwnull_status(window_t *_window, int *x, int *y, int *buttons) {
  libwnull_window_t *window = (libwnull_window_t *)_window;

  debug(DEBUG_INFO, "WNULL", "status w=%p x=%d y=%d buttons=%u", window, window->x, window->y, window->buttons);
  if (x) *x = window->x;
  if (y) *y = window->y;
  if (buttons) *buttons = window->buttons;
}

static int libwnull_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  libwnull_window_t *window = (libwnull_window_t *)_window;
  uint16_t cmd, args[2];
  uint64_t timestamp;
  int nread, r = 0;

  if (window && window->fd > 0) {
    if ((r = sys_read_timeout(window->fd, (uint8_t *)&cmd, 2, &nread, wait < 0 ? -1 : wait * 1000)) == 1 && nread == 2) {
      cmd = sys_le16toh(cmd);
      //debug(DEBUG_INFO, "WNULL", "event2 w=%p wait=%d cmd=%u", window, wait, cmd);
      r = 0;

      switch (cmd) {
        case CMD_MOTION:
          if ((r = sys_read_timeout(window->fd, (uint8_t *)args, 4, &nread, 0)) == 1 && nread == 4) {
            window->x = sys_le16toh(args[0]);
            window->y = sys_le16toh(args[1]);
            //debug(DEBUG_INFO, "WNULL", "event2 w=%p motion %d,%d", window, window->x, window->y);
            *arg1 = window->x;
            *arg2 = window->y;
            r = WINDOW_MOTION;
          } else {
            debug(DEBUG_ERROR, "WNULL", "event2 w=%p invalid argument size %u for CMD_MOTION", window, nread);
          }
          break;
        case CMD_BUTTON:
          if ((r = sys_read_timeout(window->fd, (uint8_t *)args, 2, &nread, 0)) == 1 && nread == 2) {
            args[0] = sys_le16toh(args[0]);
            *arg1 = args[0] & 0x0F;
            if (args[0] & 0x8000) {
              window->buttons |= *arg1;
              //debug(DEBUG_INFO, "WNULL", "event2 w=%p button %d down", window, *arg1);
              r = WINDOW_BUTTONDOWN;
            } else {
              window->buttons &= !(*arg1);
              //debug(DEBUG_INFO, "WNULL", "event2 w=%p button %d up", window, *arg1);
              r = WINDOW_BUTTONUP;
            }
          } else {
            debug(DEBUG_ERROR, "WNULL", "event2 w=%p invalid argument size %u for CMD_BUTTON", window, nread);
          }
          break;
        case CMD_KEYDOWN:
          if ((r = sys_read_timeout(window->fd, (uint8_t *)args, 2, &nread, 0)) == 1 && nread == 2) {
            timestamp = sys_get_clock();
            args[0] = sys_le16toh(args[0]);
            *arg1 = args[0];
            debug(DEBUG_INFO, "WNULL", "event2 w=%p key %d down", window, *arg1);
            // try to avoid multiple key press events when holding down a key
            if (*arg1 != window->last_key || timestamp - window->last_timestamp > 200000) {
              if (*arg1) r = WINDOW_KEYDOWN;
              window->last_timestamp = timestamp;
              window->last_key = *arg1;
            }
          }
          break;
        case CMD_KEYUP:
          if ((r = sys_read_timeout(window->fd, (uint8_t *)args, 2, &nread, 0)) == 1 && nread == 2) {
            args[0] = sys_le16toh(args[0]);
            *arg1 = args[0];
            debug(DEBUG_INFO, "WNULL", "event2 w=%p key %d up", window, *arg1);
            if (*arg1) r = WINDOW_KEYUP;
            window->last_timestamp = 0;
          }
          break;
        default:
          debug(DEBUG_ERROR, "WNULL", "event2 w=%p invalid cmd %u", window, cmd);
          break;
      }
    }
  }

  return r;
}

static int libwnull_setup(int pe) {
  char *host = NULL;
  int r = -1;

  if (script_get_string(pe, 0, &host) == 0) {
    window_provider.data = sys_strdup(host);
    r = 0;
  }

  if (host) sys_free(host);

  return r;
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

  script_add_function(pe, obj, "setup",  libwnull_setup);

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);
  script_add_iconst(pe, obj, "hdepth", 16);

  return 0;
}

int libwnull_unload(void) {
  return 0;
}
