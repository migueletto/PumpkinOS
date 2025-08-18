#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "pwindow.h"
#include "bytes.h"
#include "debug.h"
#include "xalloc.h"

#include "cursor.c"

typedef struct {
  int fb_num, kbd_num, mouse_num;
  int width, height, depth;
  int fb_fd, kbd_fd, mouse_fd, len;
  int xmin, xmax, ymin, ymax;
  int x, y, buttons, button_down;
  int sx, sy, cursor;
  uint32_t saved[32 * 32 * sizeof(uint32_t)];
  uint16_t *p16;
  uint32_t *p32;
  void *p;
} fb_t;

struct texture_t {
  int width, height, depth, size;
  uint16_t *b16;
  uint32_t *b32;
};

static window_provider_t wp;
static fb_t fb;

static void save_behind(fb_t *fb, uint32_t x, uint32_t y) {
  uint32_t i, j, k, windex;

  windex = y * fb->width + x;

  for (i = 0, k = 0; i < cursor.height; i++) {
    for (j = 0; j < cursor.width; j++, k++) {
      fb->saved[k] = fb->p32[windex + j];
    }
    windex += fb->width;
  }
}

static void restore_behind(fb_t *fb, uint32_t x, uint32_t y) {
  uint32_t i, j, k, windex;

  windex = y * fb->width + x;

  for (i = 0, k = 0; i < cursor.height; i++) {
    for (j = 0; j < cursor.width; j++, k++) {
      fb->p32[windex + j] = fb->saved[k];
    }
    windex += fb->width;
  }
}

static void draw_cursor(fb_t *fb, uint32_t x, uint32_t y) {
  uint32_t *c = (uint32_t *)cursor.data;
  uint32_t i, j, windex, d;

  windex = y * fb->width + x;

  for (i = 0; i < cursor.height; i++) {
    for (j = 0; j < cursor.width; j++) {
      d = c[i * cursor.width + j];
      if (d & 0xff000000) {
        fb->p32[windex + j] = d;
      }
    }
    windex += fb->width;
  }
}

static uint16_t evt2key(uint16_t code) {
  switch (code) {
    case 106: code = WINDOW_KEY_RIGHT; break;
    case 105: code = WINDOW_KEY_LEFT; break;
    case 108: code = WINDOW_KEY_DOWN; break;
    case 103: code = WINDOW_KEY_UP; break;
    case 104: code = WINDOW_KEY_PGUP; break;
    case 109: code = WINDOW_KEY_PGDOWN; break;
    case 102: code = WINDOW_KEY_HOME; break;
    case 107: code = WINDOW_KEY_END; break;
    case 110: code = WINDOW_KEY_INS; break;
    case  59: code = WINDOW_KEY_F1; break;
    case  60: code = WINDOW_KEY_F2; break;
    case  61: code = WINDOW_KEY_F3; break;
    case  62: code = WINDOW_KEY_F4; break;
    case  63: code = WINDOW_KEY_F5; break;
    case  64: code = WINDOW_KEY_F6; break;
    case  65: code = WINDOW_KEY_F7; break;
    case  66: code = WINDOW_KEY_F8; break;
    case  67: code = WINDOW_KEY_F9; break;
    case  68: code = WINDOW_KEY_F10; break;
    case  87: code = WINDOW_KEY_F11; break;
    case  88: code = WINDOW_KEY_F12; break;
    default: code = 0; break;
  }

  return code;
}

// 2021-04-29 17:13:12.516 I 10910 PalmOS   INPUT: 0000: 28 E9 8A 60 3F C9 07 00 01 00 4A 01 01 00 00 00
// 2021-04-29 17:13:12.516 I 10910 PalmOS   INPUT: 0010: 28 E9 8A 60 3F C9 07 00 03 00 00 00 12 03 00 00
// 2021-04-29 17:13:12.516 I 10910 PalmOS   INPUT: 0020: 28 E9 8A 60 3F C9 07 00 03 00 01 00 2D 0D 00 00
// 2021-04-29 17:13:12.516 I 10910 PalmOS   INPUT: 0030: 28 E9 8A 60 3F C9 07 00 03 00 18 00 85 00 00 00
// 2021-04-29 17:13:12.516 I 10910 PalmOS   INPUT: 0040: 28 E9 8A 60 3F C9 07 00 00 00 00 00 00 00 00 00

static int input_event(fb_t *fb, uint32_t us, int *x, int *y, int *button, int *key) {
  uint8_t buf[24];
  uint32_t value;
  int32_t ivalue;
  uint16_t type, code;
  struct timeval tv;
  fd_set fds;
  int i, nfds, fd, len, nread, hask, hast, hasx, hasy, down, ev = -1;

  hask = hast = hasx = hasy = down = 0;
  len = sizeof(struct timeval) + 8; // struct timeval can be 16 bytes or 24 bytes
  *x = 0;
  *y = 0;
  *button = 0;

  for (; ev == -1;) {
    FD_ZERO(&fds);
    FD_SET(fb->kbd_fd, &fds);
    FD_SET(fb->mouse_fd, &fds);
    nfds = fb->kbd_fd > fb->mouse_fd ? fb->kbd_fd : fb->mouse_fd;
    tv.tv_sec = 0;
    tv.tv_usec = us;

    //switch (sys_read_timeout(fb->mouse_fd, buf, len, &nread, us)) {
    switch (select(nfds+1, &fds, NULL, NULL, &tv)) {
      case -1:
        debug(DEBUG_ERROR, "FB", "input_event error");
        return -1;
      case 0:
        ev = 0;
        break;
      default:
        nread = 0;
        if (FD_ISSET(fb->kbd_fd, &fds)) {
          sys_read_timeout(fb->kbd_fd, buf, len, &nread, 0);
          fd = fb->kbd_fd;
        } else {
          sys_read_timeout(fb->mouse_fd, buf, len, &nread, 0);
          fd = fb->mouse_fd;
        }
        debug(DEBUG_TRACE, "FB", "read %d bytes from fd %d", nread, fd);

        if (nread == len) {
          i = len - 8; // ignore struct timeval
          i += get2l(&type, buf, i);
          i += get2l(&code, buf, i);
          i += get4l(&value, buf, i);
          debug(DEBUG_TRACE, "FB", "type %u code %u value %d", type, code, value);

          // types and codes defined in /usr/include/linux/input-event-codes.h
          switch (type) {
            case 0x00: // EV_SYN
              if (hast) {
                ev = down ? WINDOW_BUTTONDOWN : WINDOW_BUTTONUP;
                *x = fb->x;
                *y = fb->y;
              } else if (hasx || hasy) {
                ev = WINDOW_MOTION;
                *x = fb->x;
                *y = fb->y;
              } else if (hask) {
                ev = down ? WINDOW_KEYDOWN : WINDOW_KEYUP;
              } else {
                ev = 0;
              }
              debug(DEBUG_TRACE, "FB", "EV_SYN event %d x=%d y=%d", ev, fb->x, fb->y);
              break;
            case 0x01: // EV_KEY
              debug(DEBUG_TRACE, "FB", "EV_KEY 0x%04X down=%d", code, value);
              switch (code) {
                case 0x110: // BTN_LEFT (for mouse)
                case 0x14A: // BTN_TOUCH (for touch screen)
                  *button = 1;
                  down = value ? 1 : 0;
                  hast = 1;
                  break;
                case 0x111: // BTN_RIGHT (for mouse)
                  *button = 2;
                  down = value ? 1 : 0;
                  hast = 1;
                  break;
                default:
                  code = evt2key(code);
                  if (code) {
                    *key = code;
                    down = value ? 1 : 0;
                    hask = 1;
                  }
                  break;
              }
              break;
            case 0x02: // EV_REL
                ivalue = value;
                switch (code) {
                  case 0x00: // REL_X
                    debug(DEBUG_TRACE, "FB", "EV_REL X %d: %d -> %d", ivalue, fb->x, fb->x + ivalue);
                    fb->x += ivalue;
                    if (fb->x < 0) fb->x = 0;
                    else if (fb->x >= fb->width) fb->x = fb->width-1;
                    if (fb->cursor) {
                      if (fb->sx >= 0) restore_behind(fb, fb->sx, fb->sy);
                      save_behind(fb, fb->x, fb->y);
                      draw_cursor(fb, fb->x, fb->y);
                      fb->sx = fb->x;
                      fb->sy = fb->y;
                    }
                    hasx = 1;
                    break;
                  case 0x01: // REL_Y
                    debug(DEBUG_TRACE, "FB", "EV_REL Y %d: %d -> %d", ivalue, fb->y, fb->y + ivalue);
                    fb->y += ivalue;
                    if (fb->y < 0) fb->y = 0;
                    else if (fb->y >= fb->height) fb->y = fb->height-1;
                    if (fb->cursor) {
                      if (fb->sx >= 0) restore_behind(fb, fb->sx, fb->sy);
                      save_behind(fb, fb->x, fb->y);
                      draw_cursor(fb, fb->x, fb->y);
                      fb->sx = fb->x;
                      fb->sy = fb->y;
                    }
                    hasy = 1;
                    break;
                }
                break;
              case 0x03: // EV_ABS
                switch (code) {
                  case 0x00: // ABS_X
                    debug(DEBUG_TRACE, "FB", "EV_ABS X %u", value);
                    value = ((value - fb->xmin) * fb->width) / (fb->xmax - fb->xmin);
                    if (value >= fb->width) value = fb->width-1;
                    fb->x = value;
                    hasx = 1;
                    break;
                  case 0x01: // ABS_Y
                    debug(DEBUG_TRACE, "FB", "EV_ABS Y %u", value);
                    value = ((value - fb->ymin) * fb->height) / (fb->ymax - fb->ymin);
                    if (value >= fb->height) value = fb->height-1;
                    fb->y = value;
                    hasy = 1;
                    break;
                  case 0x18: // ABS_PRESSURE
                    debug(DEBUG_TRACE, "FB", "EV_ABS pressure");
                    break;
                }
                break;
            }
        }
        break;
    }
  }

  return ev;
}

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  fb_t *fb = (fb_t *)data;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  struct input_absinfo abs_feat;
  int xmin, xmax, ymin, ymax;
  int fb_fd, kbd_fd, mouse_fd;
  char device[32];
  void *p;
  window_t *w = NULL;

  if (fb->fb_fd == 0) {
    snprintf(device, sizeof(device)-1, "/dev/fb%d", fb->fb_num);
    fb_fd = open(device, O_RDWR);
    if (fb_fd == -1) debug_errno("FB", "open fb");

    snprintf(device, sizeof(device)-1, "/dev/input/event%d", fb->kbd_num);
    kbd_fd = open(device, O_RDONLY);
    if (kbd_fd == -1) debug_errno("FB", "open keyboard");

    if (fb->mouse_num != fb->kbd_num) {
      snprintf(device, sizeof(device)-1, "/dev/input/event%d", fb->mouse_num);
      mouse_fd = open(device, O_RDONLY);
      if (mouse_fd == -1) debug_errno("FB", "open mouse");
    } else {
      mouse_fd = kbd_fd;
    }

    if (fb_fd != -1 && kbd_fd != -1 && mouse_fd != -1) {
      debug(DEBUG_INFO, "FB", "framebuffer %d open as fd %d", fb->fb_num, fb_fd);
      debug(DEBUG_INFO, "FB", "keyboard %d open as fd %d", fb->kbd_num, kbd_fd);
      debug(DEBUG_INFO, "FB", "mouse %d open as fd %d", fb->mouse_num, mouse_fd);

      if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) != -1 &&
          ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) != -1) {
        debug(DEBUG_INFO, "FB", "framebuffer %s %dx%d bpp %d type %d", finfo.id, vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, finfo.type);

        if (ioctl(mouse_fd, EVIOCGABS(ABS_X), &abs_feat) == 0) {
          debug(DEBUG_INFO, "FB", "input ABS_X: %d (min:%d max:%d flat:%d fuzz:%d)",
            abs_feat.value, abs_feat.minimum, abs_feat.maximum, abs_feat.flat, abs_feat.fuzz);
          xmin = abs_feat.minimum;
          xmax = abs_feat.maximum;
        } else {
          xmin = 0;
          xmax = vinfo.xres - 1;
        }
        if (ioctl(mouse_fd, EVIOCGABS(ABS_Y), &abs_feat) == 0) {
          debug(DEBUG_INFO, "FB", "input ABS_Y: %d (min:%d max:%d flat:%d fuzz:%d)",
            abs_feat.value, abs_feat.minimum, abs_feat.maximum, abs_feat.flat, abs_feat.fuzz);
          ymin = abs_feat.minimum;
          ymax = abs_feat.maximum;
        } else {
          ymin = 0;
          ymax = vinfo.yres - 1;
        }

        if ((p = (uint16_t *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0)) != NULL) {
          fb->fb_fd = fb_fd;
          fb->kbd_fd = kbd_fd;
          fb->mouse_fd = mouse_fd;
          fb->p = p;
          fb->width = vinfo.xres;
          fb->height = vinfo.yres;
          fb->xmin = xmin;
          fb->xmax = xmax;
          fb->ymin = ymin;
          fb->ymax = ymax;
          fb->x = fb->width / 2;
          fb->y = fb->height / 2;
          fb->sx = fb->sy = -1;
          fb->depth = vinfo.bits_per_pixel;
          fb->len = finfo.smem_len;
          if (fb->depth == 16) {
            fb->p16 = p;
          } else {
            fb->p32 = p;
          }
          *width = fb->width;
          *height = fb->height;
          w = (window_t *)fb;
        }
      } else {
        debug_errno("FB", "ioctl");
        close(fb_fd);
        close(kbd_fd);
        if (mouse_fd != kbd_fd) close(mouse_fd);
      }
    } else {
      if (fb_fd != -1) close(fb_fd);
      if (kbd_fd != -1) close(kbd_fd);
      if (mouse_fd != -1 && mouse_fd != kbd_fd) close(mouse_fd);
    }
  } else {
    debug(DEBUG_ERROR, "FB", "only one window can be created");
  }

  return w;
}

static int window_destroy(window_t *window) {
  fb_t *fb = (fb_t *)window;

  if (fb) {
    if (fb->fb_fd > 0) {
      munmap(fb->p, fb->len);
      close(fb->fb_fd);
    }
    if (fb->kbd_fd > 0) {
      close(fb->kbd_fd);
    }
    if (fb->mouse_fd > 0 && fb->mouse_fd != fb->kbd_fd) {
      close(fb->mouse_fd);
    }

    fb->fb_fd = 0;
    fb->kbd_fd = 0;
  }

  return 0;
}

static int window_render(window_t *window) {
  return 0;
}

static texture_t *window_create_texture(window_t *window, int width, int height) {
  fb_t *fb = (fb_t *)window;
  texture_t *texture;

  if ((texture = xcalloc(1, sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    texture->depth = fb->depth;

    switch (fb->depth) {
      case 16:
        texture->size = width * height * sizeof(uint16_t);
        texture->b16 = xmalloc(texture->size);
        break;
      case 32:
        texture->size = width * height * sizeof(uint32_t);
        texture->b32 = xmalloc(texture->size);
        break;
    }
  }

  return texture;
}

static int window_destroy_texture(window_t *window, texture_t *texture) {
  if (texture) {
    if (texture->b16) xfree(texture->b16);
    if (texture->b32) xfree(texture->b32);
    xfree(texture);
  }

  return 0;
}

static int window_update_texture_rect(window_t *window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  int i, j, tpitch, tindex;
  uint16_t *b16;
  uint32_t *b32;

  if (texture && src) {
    tpitch = texture->width;
    tindex = ty * tpitch + tx;
    b16 = (uint16_t *)src;
    b32 = (uint32_t *)src;

    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        switch (texture->depth) {
          case 16:
            texture->b16[tindex + j] = b16[tindex + j];
            break;
          case 32:
            texture->b32[tindex + j] = b32[tindex + j];
            break;
        }
      }
      tindex += tpitch;
    }
  }

  return 0;
}

static int window_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  if (texture && raw) {
    switch (texture->depth) {
      case 16:
        if (texture->b16) xmemcpy(texture->b16, raw, texture->size);
        break;
      case 32:
        if (texture->b32) xmemcpy(texture->b32, raw, texture->size);
        break;
    }
  }

  return 0;
}

static int window_draw_texture_rect(window_t *window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  fb_t *fb = (fb_t *)window;
  int i, j, d, tpitch, wpitch, tindex, windex, r = -1;

  if (texture && fb->p) {
    if (x < 0) {
      tx -= x;
      w += x;
      x = 0;
    }
    if (x+w >= fb->width) {
      d = x+w - fb->width;
      w -= d;
    }
    if (y < 0) {
      ty -= y;
      h += y;
      y = 0;
    }
    if (y+h >= fb->height) {
      d = y+h - fb->height;
      h -= d;
    }

    if (w > 0 && h > 0) {
      if (fb->cursor) {
        if (fb->sx >= 0) restore_behind(fb, fb->sx, fb->sy);
      }
      tpitch = texture->width;
      wpitch = fb->width;
      tindex = ty * tpitch + tx;
      windex = y * wpitch + x;

      for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
          switch (fb->depth) {
            case 16:
              fb->p16[windex + j] = texture->b16[tindex + j];
              break;
            case 32:
              fb->p32[windex + j] = texture->b32[tindex + j];
              break;
          }
        }
        tindex += tpitch;
        windex += wpitch;
      }
      if (fb->cursor) {
        save_behind(fb, fb->x, fb->y);
        draw_cursor(fb, fb->x, fb->y);
        fb->sx = fb->x;
        fb->sy = fb->y;
      }
      r = 0;
    }
  }

  return r;
}

static int window_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  int r = -1;

  if (texture) {
    r = window_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return r;
}

static int window_event2(window_t *window, int wait, int *arg1, int *arg2) {
  fb_t *fb = (fb_t *)window;
  int button, key, ev = 0;

  if (fb->mouse_fd || fb->kbd_fd > 0) {
    if (fb->button_down) {
      debug(DEBUG_TRACE, "FB", "restoring button down");
      ev = WINDOW_BUTTONDOWN;
      *arg1 = fb->button_down;
      *arg2 = 0;
      fb->button_down = 0;
    } else {
      ev = input_event(fb, wait, arg1, arg2, &button, &key);
      switch (ev) {
        case WINDOW_BUTTONDOWN:
          debug(DEBUG_TRACE, "FB", "changing first button down into motion");
          fb->button_down = button;
          ev = WINDOW_MOTION;
          break;
        case WINDOW_BUTTONUP:
          *arg1 = button;
          *arg2 = 0;
          break;
        case WINDOW_KEYDOWN:
        case WINDOW_KEYUP:
          *arg1 = key;
          *arg2 = 0;
          break;
      }
    }
  }
  if (ev) debug(DEBUG_TRACE, "FB", "window_event event %d x=%d y=%d", ev, *arg1, *arg2);

  return ev;
}

static void window_status(window_t *window, int *x, int *y, int *buttons) {
  fb_t *fb = (fb_t *)window;
  *x = fb->x;
  *y = fb->y;
  *buttons = fb->buttons;
}

static int libfb_setup(int pe) {
  script_int_t fb_num, kbd_num, mouse_num;
  int r = -1;

  if (script_get_integer(pe, 0, &fb_num) == 0 &&
      script_get_integer(pe, 1, &kbd_num) == 0 &&
      script_get_integer(pe, 2, &mouse_num) == 0) {

    fb.fb_num = fb_num;
    fb.kbd_num = kbd_num;
    fb.mouse_num = mouse_num;
    debug(DEBUG_INFO, "FB", "setup %d %d %d", fb.fb_num, fb.kbd_num, fb.mouse_num);
    r = 0;
  }

  return r;
}

static int libfb_cursor(int pe) {
  int on, r = -1;

  if (script_get_boolean(pe, 0, &on) == 0) {
    fb.cursor = on;
    r = 0;
  }

  return r;
}

int libfb_init(int pe, script_ref_t obj) {
  xmemset(&wp, 0, sizeof(window_provider_t));
  xmemset(&fb, 0, sizeof(fb_t));

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
  wp.data = &fb;

  debug(DEBUG_INFO, "FB", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &wp);

  script_add_iconst(pe, obj, "hdepth", 16);

  script_add_function(pe, obj, "setup",  libfb_setup);
  script_add_function(pe, obj, "cursor", libfb_cursor);

  return 0;
}
