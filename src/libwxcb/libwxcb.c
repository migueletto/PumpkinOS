#include "sys.h"
#include "script.h"
#include "media.h"
#include "pwindow.h"
#include "average.h"
#include "debug.h"

#include <stdlib.h>
#include <poll.h>
#include <locale.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#define MAX_CLIPBOARD 1024

struct texture_t {
  xcb_pixmap_t pixmap;
  int width, height;
  uint8_t *buf;
};

typedef struct {
  xcb_atom_t selection;
  xcb_atom_t target;
  xcb_atom_t property;
  char buf[MAX_CLIPBOARD];
} xcb_clipboard_t;

typedef struct {
  int width, height, spixel;
  int x, y, buttons, mods, other;
  int64_t shift_up;
  xcb_connection_t *c;
  const xcb_setup_t *setup;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_gcontext_t gc;
  xcb_get_keyboard_mapping_reply_t *kb_map;
  struct xkb_context *ctx;
  struct xkb_keymap *keymap;
  struct xkb_compose_state *compose_state;
  struct xkb_state *xkb_state;
  texture_t *background;
  xcb_clipboard_t clipboard;
} libwxcb_window_t;

static window_provider_t window_provider;

static void libwxcb_status(libwxcb_window_t *window, int *x, int *y, int *buttons) {
  if (window) {
    *x = window->x;
    *y = window->y;
    *buttons = window->buttons;
  }
}

static void libwxcb_title(libwxcb_window_t *window, char *title) {
  if (window && title) {
    debug(DEBUG_TRACE, "XCB", "set title [%s]", title);
    xcb_change_property(window->c, XCB_PROP_MODE_REPLACE, window->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, sys_strlen(title), title);
    xcb_change_property(window->c, XCB_PROP_MODE_REPLACE, window->window, XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8, sys_strlen(title), title);
  }
}

// https://stackoverflow.com/questions/37295904/clipboard-and-xcb
static char *libwxcb_clipboard(libwxcb_window_t *_window, char *clipboard, int len) {
  libwxcb_window_t *window;
  xcb_generic_event_t *event;
  xcb_get_property_cookie_t cookie;
  xcb_get_property_reply_t *reply;
  char *r = NULL;

  if (_window) {
    window = (libwxcb_window_t *)_window;

    if (clipboard && len > 0) {
      // copy to clipboard not implemented
    }

    // paste from clipboard
    xcb_convert_selection(window->c, window->window, window->clipboard.selection, window->clipboard.target, window->clipboard.property, XCB_CURRENT_TIME);
    xcb_flush(window->c);

    if ((event = xcb_wait_for_event(window->c)) != NULL) {
      cookie = xcb_get_property(window->c, 0, window->window, window->clipboard.property, window->clipboard.target, 0, MAX_CLIPBOARD - 1);
      reply = xcb_get_property_reply(window->c, cookie, NULL);
      if (reply) {
        len = xcb_get_property_value_length(reply);
        sys_memset(window->clipboard.buf, 0, MAX_CLIPBOARD);
        if (len > 0) {
          if (len > MAX_CLIPBOARD - 1) len = MAX_CLIPBOARD - 1;
          sys_memcpy(window->clipboard.buf, xcb_get_property_value(reply), len);
          r = window->clipboard.buf;
        }
        free(reply);
      }
      xcb_delete_property(window->c, window->window, window->clipboard.property);
      free(event);
    }
  }

  return r;
}

static int map_button(xcb_button_press_event_t *ev) {
  switch (ev->detail) {
    case 1: return 1;
    case 3: return 2;
  }

  return 0;
}

static int map_key(libwxcb_window_t *window, xcb_key_press_event_t *ev) {
  xcb_keycode_t keycode;
  xcb_keysym_t *keysyms;
  enum xkb_compose_status compose_status;
  int key, shift, index, keysyms_per_keycode;

  keycode = ev->detail;
  shift = (ev->state & XCB_MOD_MASK_SHIFT) ? 1 : 0;
  //ctrl = (ev->state & XCB_MOD_MASK_CONTROL) ? 1 : 0;
  //alt = (ev->state & XCB_MOD_MASK_1) ? 1 : 0;
  key = xkb_state_key_get_one_sym(window->xkb_state, keycode);

  if (shift) {
    keysyms_per_keycode = window->kb_map->keysyms_per_keycode;
    keysyms = xcb_get_keyboard_mapping_keysyms(window->kb_map);
    index = (keycode - window->setup->min_keycode) * keysyms_per_keycode + shift;
    key = keysyms[index];
  }

  if (window->compose_state) {
    xkb_compose_state_feed(window->compose_state, key);
    compose_status = xkb_compose_state_get_status(window->compose_state);
    switch (compose_status) {
     case XKB_COMPOSE_NOTHING:
       break;
     case XKB_COMPOSE_COMPOSING:
       key = 0;
       break;
     case XKB_COMPOSE_CANCELLED:
       xkb_compose_state_reset(window->compose_state);
       key = 0;
       break;
     case XKB_COMPOSE_COMPOSED:
       key = xkb_compose_state_get_one_sym(window->compose_state);
       xkb_compose_state_reset(window->compose_state);
       break;
    }
  }

  if (key) {
    switch (key) {
      case XKB_KEY_BackSpace: key = 8; break;
      case XKB_KEY_Return:    key = 10; break;
      case XKB_KEY_Right:     key = WINDOW_KEY_RIGHT; break;
      case XKB_KEY_Left:      key = WINDOW_KEY_LEFT; break;
      case XKB_KEY_Down:      key = WINDOW_KEY_DOWN; break;
      case XKB_KEY_Up:        key = WINDOW_KEY_UP; break;
      case XKB_KEY_Page_Up:   key = WINDOW_KEY_PGUP; break;
      case XKB_KEY_Page_Down: key = WINDOW_KEY_PGDOWN; break;
      case XKB_KEY_Home:      key = WINDOW_KEY_HOME; break;
      case XKB_KEY_End:       key = WINDOW_KEY_END; break;
      case XKB_KEY_Insert:    key = WINDOW_KEY_INS; break;
      case XKB_KEY_Delete:    key = WINDOW_KEY_DEL; break;
      case XKB_KEY_F1:        key = WINDOW_KEY_F1; break;
      case XKB_KEY_F2:        key = WINDOW_KEY_F2; break;
      case XKB_KEY_F3:        key = WINDOW_KEY_F3; break;
      case XKB_KEY_F4:        key = WINDOW_KEY_F4; break;
      case XKB_KEY_F5:        key = WINDOW_KEY_F5; break;
      case XKB_KEY_F6:        key = WINDOW_KEY_F6; break;
      case XKB_KEY_F7:        key = WINDOW_KEY_F7; break;
      case XKB_KEY_F8:        key = WINDOW_KEY_F8; break;
      case XKB_KEY_F9:        key = WINDOW_KEY_F9; break;
      case XKB_KEY_F10:       key = WINDOW_KEY_F10; break;
      case XKB_KEY_F11:       key = WINDOW_KEY_F11; break;
      case XKB_KEY_F12:       key = WINDOW_KEY_F12; break;
      default:
        if (key > 0xff) key = 0;
        break;
    }
  }

  return key;
}

static int libwxcb_event2(libwxcb_window_t *window, int wait, int *arg1, int *arg2) {
  xcb_generic_event_t *event;
  xcb_expose_event_t *expose_event;
  xcb_key_press_event_t *key_event;
  xcb_button_press_event_t *button_event;
  xcb_motion_notify_event_t *motion_event;
  struct pollfd pfd;
  int poll_result, r = 0;

  if (!window) return -1;

  pfd.fd = xcb_get_file_descriptor(window->c);
  pfd.events = POLLIN;
  pfd.revents = 0;
  poll_result = poll(&pfd, 1, wait);

  if (poll_result == -1) return -1;
  if (poll_result == 0) return 0;
  if (!(pfd.revents & POLLIN)) return 0;
  if ((event = xcb_wait_for_event(window->c)) == NULL) return -1;

  switch (event->response_type & ~0x80) {
    case XCB_EXPOSE:
      expose_event = (xcb_expose_event_t *)event;
      debug(DEBUG_TRACE, "XCB", "expose window %d,%d %dx%d", expose_event->x, expose_event->y, expose_event->width, expose_event->height);
      r = WINDOW_EXPOSE;
      break;
    case XCB_KEY_PRESS:
      key_event = (xcb_key_press_event_t *)event;
      *arg1 = map_key(window, key_event);
      if (*arg1) r = WINDOW_KEYDOWN;
      break;
    case XCB_KEY_RELEASE:
      key_event = (xcb_key_press_event_t *)event;
      *arg1 = map_key(window, key_event);
      if (*arg1) r = WINDOW_KEYUP;
      break;
    case XCB_BUTTON_PRESS:
      button_event = (xcb_button_press_event_t *)event;
      *arg1 = map_button(button_event);
      if (*arg1) {
        window->buttons |= *arg1;
        r = WINDOW_BUTTONDOWN;
      }
      break;
    case XCB_BUTTON_RELEASE:
      button_event = (xcb_button_press_event_t *)event;
      *arg1 = map_button(button_event);
      if (*arg1) {
        window->buttons &= ~(*arg1);
        r = WINDOW_BUTTONUP;
      }
      break;
    case XCB_MOTION_NOTIFY:
      motion_event = (xcb_motion_notify_event_t *)event;
      window->x = motion_event->event_x;
      window->y = motion_event->event_y;
      *arg1 = window->x;
      *arg2 = window->y;
      r = WINDOW_MOTION;
      break;
  }

  free(event);

  return r;
}

static int libwxcb_window_show_cursor(window_t *window, int show) {
  return 0;
}

static window_t *libwxcb_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate,
     int fullscreen, int software, char *driver, void *data) {

  libwxcb_window_t *window;
  xcb_connection_t *c;
  const xcb_setup_t *setup;
  xcb_screen_t *screen;
  xcb_screen_iterator_t iter;
  struct xkb_compose_table *compose_table;
  xcb_intern_atom_cookie_t selection, target, property;
  xcb_intern_atom_reply_t *reply_selection, *reply_target, *reply_property;
  const char *locale;
  uint32_t spixel;
  uint32_t mask, values[3];
  int32_t device_id;

  switch (encoding) {
    case ENC_RGBA:
      spixel = sizeof(uint32_t);
      break;
    default:
      debug(DEBUG_ERROR, "XCB", "invalid encoding %s", video_encoding_name(encoding));
      return NULL;
  }

  if ((c = xcb_connect(NULL, NULL)) == NULL) {
    debug(DEBUG_ERROR, "XCB", "xcb_connect failed");
    return NULL;
  }

  if (xcb_connection_has_error(c)) {
    xcb_disconnect(c);
    debug(DEBUG_ERROR, "XCB", "xcb connection has error");
    return NULL;
  }

  if ((setup = xcb_get_setup(c)) == NULL) {
    xcb_disconnect(c);
    debug(DEBUG_ERROR, "XCB", "xcb_get_setup failed");
    return NULL;
  }

  iter = xcb_setup_roots_iterator(setup);
  screen = iter.data;

  if (screen->root_depth != 24) {
    xcb_disconnect(c);
    debug(DEBUG_ERROR, "XCB", "screen depth is %d bits, but only 24 is supported", screen->root_depth);
    return NULL;
  }

  if (*width == 0 || *height == 0) {
    *width = screen->width_in_pixels;
    *height = screen->height_in_pixels;
    debug(DEBUG_INFO, "XCB", "using whole display %dx%d", *width, *height);
  }

  if ((window = sys_calloc(1, sizeof(libwxcb_window_t))) != NULL) {
    window->c = c;
    window->setup = setup;
    window->screen = screen;
    window->window = xcb_generate_id(window->c);
    debug(DEBUG_INFO, "XCB", "windows id 0x%08X", window->window);

    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = window->screen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION;

    xcb_create_window(window->c, XCB_COPY_FROM_PARENT, window->window, window->screen->root,
        0, 0, *width, *height, 4, XCB_WINDOW_CLASS_INPUT_OUTPUT, window->screen->root_visual, mask, values);

    xcb_map_window(window->c, window->window);
    xcb_flush(window->c);

    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = window->screen->black_pixel;
    values[1] = window->screen->white_pixel;
    values[2] = 0;

    window->gc = xcb_generate_id(window->c);
    xcb_create_gc(window->c, window->gc, window->window, mask, values);

    if (!xkb_x11_setup_xkb_extension(window->c, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION,
           XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, NULL, NULL, NULL, NULL)) {
      debug(DEBUG_ERROR, "XCB", "xkb_x11_setup_xkb_extension failed");
    }

    window->ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    window->kb_map = xcb_get_keyboard_mapping_reply(window->c,
        xcb_get_keyboard_mapping(window->c, window->setup->min_keycode, window->setup->max_keycode - window->setup->min_keycode + 1), NULL);

    window->compose_state = NULL;
    locale = setlocale(LC_CTYPE, NULL);
    compose_table = xkb_compose_table_new_from_locale(window->ctx, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (compose_table) {
      window->compose_state = xkb_compose_state_new(compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
    } else {
      debug(DEBUG_ERROR, "XCB", "xkb_compose_table_new_from_locale failed");
    }

    if ((device_id = xkb_x11_get_core_keyboard_device_id(window->c)) != -1) {
      window->keymap = xkb_x11_keymap_new_from_device(window->ctx, window->c, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
      window->xkb_state = xkb_x11_state_new_from_device(window->keymap, window->c, device_id);
    } else {
      debug(DEBUG_ERROR, "XCB", "xkb_x11_get_core_keyboard_device_id failed");
    }

    selection = xcb_intern_atom(window->c, 0, 9, "CLIPBOARD");
    target = xcb_intern_atom(window->c, 0, 6, "STRING");
    property = xcb_intern_atom(window->c, 0, 7, "CONTENT");
    reply_selection = xcb_intern_atom_reply(window->c, selection, NULL);
    reply_target = xcb_intern_atom_reply(window->c, target, NULL);
    reply_property = xcb_intern_atom_reply(window->c, property, NULL);
    window->clipboard.selection = reply_selection->atom;
    window->clipboard.target = reply_target->atom;
    window->clipboard.property = reply_property->atom;
    free(reply_selection);
    free(reply_target);
    free(reply_property);

    window->width = *width;
    window->height = *height;
    window->spixel = spixel;
  } else {
    xcb_disconnect(c);
  }

  return (window_t *)window;
}

static int libwxcb_window_render(window_t *_window) {
  return 0;
}

static int libwxcb_window_erase(window_t *_window, uint32_t bg) {
  libwxcb_window_t *window;
  int r = -1;

  if (_window) {
    window = (libwxcb_window_t *)_window;
    xcb_clear_area(window->c, 1, window->window, 0, 0, window->width, window->height);

    if (window->background) {
       window_provider.draw_texture(_window, window->background, 0, 0);
    }

    r = 0;
  }

  return r;
}

static texture_t *libwxcb_window_create_texture(window_t *_window, int width, int height) {
  libwxcb_window_t *window;
  uint8_t *buf;
  texture_t *texture = NULL;

  window = (libwxcb_window_t *)_window;

  if (window && width > 0 && height > 0) {
    if ((buf = sys_calloc(width * height, window->spixel)) != NULL) {
      if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
        texture->pixmap = xcb_generate_id(window->c);
        xcb_create_pixmap(window->c, window->screen->root_depth, texture->pixmap, window->window, width, height);
        texture->width = width;
        texture->height = height;
        texture->buf = buf;
        debug(DEBUG_TRACE, "XCB", "libwxcb_window_create_texture %dx%d pixmap %d", width, height, texture->pixmap);
      } else {
        sys_free(buf);
      }
    }
  }

  return texture;
}

static int libwxcb_window_destroy_texture(window_t *_window, texture_t *texture) {
  libwxcb_window_t *window;
  int r = -1;

  window = (libwxcb_window_t *)_window;

  if (window && texture) {
    debug(DEBUG_TRACE, "XCB", "libwxcb_window_destroy_texture %dx%d pixmap %d", texture->width, texture->height, texture->pixmap);
    sys_free(texture->buf);
    sys_free(texture);
  }

  return r;
}

static int libwxcb_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  libwxcb_window_t *window;
  xcb_image_t *image;
  uint8_t *s, *d;
  int pitch, len, i, r = -1;

  if (_window && texture && w > 0 && h > 0 && tx >= 0 && ty >= 0 && (tx+w) <= texture->width && (ty+h) <= texture->height) {
    window = (libwxcb_window_t *)_window;
    s = &src[(ty * texture->width + tx) * window->spixel];
    d = texture->buf;
    pitch = texture->width * window->spixel;
    len = w * window->spixel;
    for (i = 0; i < h; i++) {
      sys_memcpy(d, s, len);
      s += pitch;
      d += len;
    }

    if ((image = xcb_image_create_native(window->c, w, h, XCB_IMAGE_FORMAT_Z_PIXMAP,
                     24, NULL, w * h * window->spixel, texture->buf)) != NULL) {
      xcb_image_put(window->c, texture->pixmap, window->gc, image, tx, ty, 0);
      xcb_image_destroy(image);
      debug(DEBUG_TRACE, "XCB", "libwxcb_window_update_texture_rect %d,%d %dx%d (%dx%d)", tx, ty, w, h, texture->width, texture->height);
      r = 0;
    }
  }

  return r;
}

static int libwxcb_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  libwxcb_window_t *window;
  xcb_image_t *image;
  int r = -1;

  if (_window && texture && src) {
    window = (libwxcb_window_t *)_window;
    sys_memcpy(texture->buf, src, texture->width * texture->height * window->spixel);

    if ((image = xcb_image_create_native(window->c, texture->width, texture->height, XCB_IMAGE_FORMAT_Z_PIXMAP,
                     24, NULL, texture->width * texture->height * window->spixel, texture->buf)) != NULL) {
      xcb_image_put(window->c, texture->pixmap, window->gc, image, 0, 0, 0);
      xcb_image_destroy(image);
      debug(DEBUG_TRACE, "XCB", "libwxcb_window_update_texture (%dx%d)", texture->width, texture->height);
      r = 0;
    }
  }

  return r;
}

static int libwxcb_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libwxcb_window_t *window;
  int r = -1;

  if (_window && texture && w > 0 && h > 0 && tx >= 0 && ty >= 0 && (tx+w) <= texture->width && (ty+h) <= texture->height) {
    window = (libwxcb_window_t *)_window;
    xcb_copy_area(window->c, texture->pixmap, window->window, window->gc, tx, ty, x, y, w, h);
    xcb_flush(window->c);
    debug(DEBUG_TRACE, "XCB", "libwxcb_window_draw_texture_rect %d,%d %dx%d, %d,%d", tx, ty, w, h, x, y);
    r = 0;
  } else {
    debug(DEBUG_ERROR, "XCB", "invalid libwxcb_window_draw_texture_rect %d,%d %dx%d, %d,%d", tx, ty, w, h, x, y);
  }

  return r;
}

static int libwxcb_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  int r = -1;

  if (texture) {
    r = libwxcb_window_draw_texture_rect(_window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return r;
}

static int libwxcb_window_background(window_t *_window, uint32_t *raw, int width, int height) {
  libwxcb_window_t *window;
  int r = -1;

  if (_window && raw) {
    window = (libwxcb_window_t *)_window;
    debug(DEBUG_TRACE, "XCB", "background %dx%d", width, height);
    if (window->background) {
      libwxcb_window_destroy_texture(_window, window->background);
    }
    window->background = libwxcb_window_create_texture(_window, width, height);
    libwxcb_window_update_texture(_window, window->background, (uint8_t *)raw);
    r = 0;
  }

  return r;
}

static void libwxcb_window_status(window_t *window, int *x, int *y, int *buttons) {
  libwxcb_status((libwxcb_window_t *)window, x, y, buttons);
}

static void libwxcb_window_title(window_t *window, char *title) {
  libwxcb_title((libwxcb_window_t *)window, title);
}

static char *libwxcb_window_clipboard(window_t *window, char *clipboard, int len) {
  return libwxcb_clipboard((libwxcb_window_t *)window, clipboard, len);
}

static int libwxcb_window_event2(window_t *window, int wait, int *arg1, int *arg2) {
  return libwxcb_event2((libwxcb_window_t *)window, wait, arg1, arg2);
}

static int libwxcb_window_update(window_t *_window, int x, int y, int width, int height) {
  return 0;
}

static int libwxcb_window_destroy(window_t *_window) {
  libwxcb_window_t *window;
  int r = -1;

  if (_window) {
    window = (libwxcb_window_t *)_window;
    if (window->background) {
      libwxcb_window_destroy_texture(_window, window->background);
    }
    xcb_unmap_window(window->c, window->window);
    xcb_disconnect(window->c);
    sys_free(window);
    r = 0;
  }

  return r;
}

static int libwxcb_window_average(window_t *window, int *x, int *y, int ms) {
  return average_click(&window_provider, window, x, y, ms);
}

static int libwxcb_calib(int pe) {
  window_provider.average = libwxcb_window_average;
  return 0;
}

int libwxcb_load(void) {
  sys_memset(&window_provider, 0, sizeof(window_provider));
  window_provider.create = libwxcb_window_create;
  window_provider.destroy = libwxcb_window_destroy;
  window_provider.erase = libwxcb_window_erase;
  window_provider.render = libwxcb_window_render;
  window_provider.background = libwxcb_window_background;
  window_provider.create_texture = libwxcb_window_create_texture;
  window_provider.destroy_texture = libwxcb_window_destroy_texture;
  window_provider.update_texture = libwxcb_window_update_texture;
  window_provider.draw_texture = libwxcb_window_draw_texture;
  window_provider.status = libwxcb_window_status;
  window_provider.title = libwxcb_window_title;
  window_provider.clipboard = libwxcb_window_clipboard;
  window_provider.event2 = libwxcb_window_event2;
  window_provider.update = libwxcb_window_update;
  window_provider.draw_texture_rect = libwxcb_window_draw_texture_rect;
  window_provider.update_texture_rect = libwxcb_window_update_texture_rect;
  window_provider.show_cursor = libwxcb_window_show_cursor;

  return 0;
}

int libwxcb_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "XCB", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);

  script_add_function(pe, obj, "calib", libwxcb_calib);

  return 0;
}

int libwxcb_unload(void) {
  return 0;
}
