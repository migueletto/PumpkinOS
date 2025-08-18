#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pwindow.h"
#include "average.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <poll.h>
#include <locale.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <linux/input-event-codes.h>

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"
#include "shm.h"

#define MARGIN 4

struct texture_t {
  int width, height;
  uint8_t *buf;
};

typedef struct {
  int width, height, spixel;
  int old_x, old_y, x, y, old_buttons, buttons, mods, other;
  uint32_t old_key, key;
  uint32_t modifiers;
  int64_t shift_up;
  struct wl_display *display;
  struct wl_shm *shm;
  struct wl_compositor *compositor;
  struct wl_seat *seat;
  struct xdg_wm_base *xdg_wm_base;
  struct xdg_surface *xdg_surface;
  struct wl_surface *surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_buffer *buffer;
  struct wl_surface *cursor_surface;
  struct wl_cursor_image *cursor_image;
	struct wl_buffer *cursor_buffer;
	struct xkb_state *xkb_state;
	struct xkb_keymap *xkb_keymap;
	struct xkb_context *xkb_context;
  struct xkb_compose_state *compose_state;
  struct wl_keyboard *keyboard;
  struct zxdg_decoration_manager_v1 *decoration_manager;
  struct zxdg_toplevel_decoration_v1 *decoration;
  uint8_t *shm_data;
  int configured;
  int running;
  texture_t *background;
} libwwayland_window_t;

static window_provider_t window_provider;

static void noop() {
} 

static int map_button(uint32_t button) {
  switch (button) {
    case BTN_LEFT:  return 1;
    case BTN_RIGHT: return 2;
  }

  return 0;
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;
  int x, y;

  x = surface_x >> 8;
  y = surface_y >> 8;

  if (x >= MARGIN && y >= MARGIN && x < MARGIN + window->width && y < MARGIN + window->height) {
    window->x = x - MARGIN;
    window->y = y - MARGIN;
  }
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  button = map_button(button);

  if (button) {
    switch (state) {
      case WL_POINTER_BUTTON_STATE_PRESSED:
        if (button == 2 && (window->modifiers & 1)) {
          // shift right click, move the wayland window and don't pass the event to the app
          xdg_toplevel_move(window->xdg_toplevel, window->seat, serial);
        } else {
          window->buttons |= button;
        }
        break;
      case WL_POINTER_BUTTON_STATE_RELEASED:
        window->buttons &= ~button;
        break;
    }
  }
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  wl_pointer_set_cursor(wl_pointer, serial, window->cursor_surface, window->cursor_image->hotspot_x, window->cursor_image->hotspot_y);
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
}

static const struct wl_pointer_listener pointer_listener = {
  .enter = pointer_handle_enter,
  .leave = pointer_handle_leave,
  .motion = pointer_handle_motion,
  .button = pointer_handle_button,
  .axis = noop,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;
  char *map_shm;

  debug(DEBUG_INFO, "WAYLAND", "keyboard keymap format %u fd %d size %u", format, fd, size);
  map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  window->xkb_keymap = xkb_keymap_new_from_string(window->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm, size);
  close(fd);
  window->xkb_state = xkb_state_new(window->xkb_keymap);
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;
  uint32_t *key;
  char buf[128];

  debug(DEBUG_INFO, "WAYLAND", "keyboard enter");
  wl_array_for_each(key, keys) {
    xkb_keysym_t sym = xkb_state_key_get_one_sym(window->xkb_state, *key + 2*MARGIN);
    xkb_keysym_get_name(sym, buf, sizeof(buf));
    debug(DEBUG_INFO, "WAYLAND", "sym: %-12s (%d), ", buf, sym);
    xkb_state_key_get_utf8(window->xkb_state, *key + 2*MARGIN, buf, sizeof(buf));
    debug(DEBUG_INFO, "WAYLAND", "utf8: '%s'\n", buf);
  }
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
  debug(DEBUG_INFO, "WAYLAND", "keyboard leave");
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  debug(DEBUG_INFO, "WAYLAND", "keyboard modifiers 0x%08X", mods_depressed);
  xkb_state_update_mask(window->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
  window->modifiers = mods_depressed;
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
  debug(DEBUG_INFO, "WAYLAND", "keyboard repeat info");
}

static int map_key(libwwayland_window_t *window, uint32_t keycode) {
  enum xkb_compose_status compose_status;
  int key;

  //shift = (window->modifiers & 1) ? 1 : 0;
  //ctrl  = (window->modifiers & 4) ? 1 : 0;
  //alt   = (window->modifiers & 8) ? 1 : 0;
  key = xkb_state_key_get_one_sym(window->xkb_state, keycode);

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
      case XKB_KEY_BackSpace: key = 8;  break;
      case XKB_KEY_Return:    key = 10; break;
      case XKB_KEY_Escape:    key = 27; break;
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

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  key = map_key(window, key + 2*MARGIN); 
  if (key) {
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
      debug(DEBUG_TRACE, "WAYLAND", "key down %d", key);
      window->key = key;
    } else {
      debug(DEBUG_TRACE, "WAYLAND", "key up %d", window->key);
      window->key = 0;
    }
  }
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
  .keymap = wl_keyboard_keymap,
  .enter = wl_keyboard_enter,
  .leave = wl_keyboard_leave,
  .key = wl_keyboard_key,
  .modifiers = wl_keyboard_modifiers,
  .repeat_info = wl_keyboard_repeat_info,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;
  struct wl_pointer *pointer;

  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, window);
  }

  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    window->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(window->keyboard, &wl_keyboard_listener, window);
  }
}

static const struct wl_seat_listener seat_listener = {
  .capabilities = seat_handle_capabilities,
};

static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}   

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
  .ping = xdg_wm_base_handle_ping,
};

static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  debug(DEBUG_INFO, "WAYLAND", "registry name %u interface \"%s\" version %u", name, interface, version);

  if (sys_strcmp(interface, wl_shm_interface.name) == 0) {
    debug(DEBUG_INFO, "WAYLAND", "binding shm interface");
    window->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);

  } else if (sys_strcmp(interface, wl_seat_interface.name) == 0) {
    debug(DEBUG_INFO, "WAYLAND", "binding seat interface");
    window->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    wl_seat_add_listener(window->seat, &seat_listener, window);

  } else if (sys_strcmp(interface, wl_compositor_interface.name) == 0) {
    debug(DEBUG_INFO, "WAYLAND", "binding compositor interface");
    window->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);

  } else if (sys_strcmp(interface, xdg_wm_base_interface.name) == 0) {
    debug(DEBUG_INFO, "WAYLAND", "binding xdg wm base interface");
    window->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(window->xdg_wm_base, &xdg_wm_base_listener, NULL);

  } else if (sys_strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
    debug(DEBUG_INFO, "WAYLAND", "binding zxdg decoration manager interface");
    window->decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    window->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(window->decoration_manager, window->xdg_toplevel);
    zxdg_toplevel_decoration_v1_set_mode(window->decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
  } 
}

static void handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
}

static const struct wl_registry_listener registry_listener = {
  .global = handle_global,
  .global_remove = handle_global_remove,
};  
  
static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  xdg_surface_ack_configure(xdg_surface, serial);

  if (window->configured) {
    wl_surface_commit(window->surface);
  }

  window->configured = 1;
} 

static const struct xdg_surface_listener xdg_surface_listener = {
  .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
  libwwayland_window_t *window = (libwwayland_window_t *)data;

  window->running = 0;
} 

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
  .configure = noop,
  .close = xdg_toplevel_handle_close,
};

static void libwwayland_status(libwwayland_window_t *window, int *x, int *y, int *buttons) {
  if (window) {
    *x = window->x;
    *y = window->y;
    *buttons = window->buttons;
  }
}

static void libwwayland_title(libwwayland_window_t *window, char *title) {
  if (window && title) {
    debug(DEBUG_TRACE, "WAYLAND", "set title [%s]", title);
    //xdg_toplevel_set_title(window->xdg_toplevel, title);
  }
}

static char *libwwayland_clipboard(libwwayland_window_t *_window, char *clipboard, int len) {
  //libwwayland_window_t *window;
  char *r = NULL;

  if (_window) {
    //window = (libwwayland_window_t *)_window;

    if (clipboard && len > 0) {
      // copy to clipboard not implemented
    }

    // paste from clipboard not implemented
  }

  return r;
}

static int libwwayland_event2(libwwayland_window_t *window, int wait, int *arg1, int *arg2) {
  struct pollfd pfd;
  int poll_result, e, r = 0;

  if (!window) return -1;
  if (!window->running) return -1;
  if (thread_must_end()) return -1;

  wl_display_flush(window->display);

  pfd.fd = wl_display_get_fd(window->display);
  pfd.events = POLLIN;
  pfd.revents = 0;
  poll_result = poll(&pfd, 1, wait);
  if (poll_result == -1) return -1;
  if (poll_result == 0) return 0;
  if (!(pfd.revents & POLLIN)) return 0;

  if ((e = wl_display_dispatch(window->display)) == -1) {
    debug(DEBUG_ERROR, "WAYLAND", "wl_display_dispatch failed");
    return -1;
  }

  if (window->buttons != window->old_buttons) {
    if (!(window->old_buttons & 1) && window->buttons & 1) {
      *arg1 = 1;
      r = WINDOW_BUTTONDOWN;
    } else if (window->old_buttons & 1 && !(window->buttons & 1)) {
      *arg1 = 1;
      r = WINDOW_BUTTONUP;
    } else if (!(window->old_buttons & 2) && window->buttons & 2) {
      *arg1 = 2;
      r = WINDOW_BUTTONDOWN;
    } else if (window->old_buttons & 2 && !(window->buttons & 2)) {
      *arg1 = 2;
      r = WINDOW_BUTTONUP;
    }
    window->old_buttons = window->buttons;

  } else if (window->x != window->old_x || window->y != window->old_y) {
    *arg1 = window->x;
    *arg2 = window->y;
    window->old_x = window->x;
    window->old_y = window->y;
    r = WINDOW_MOTION;

  } else if (window->key != window->old_key) {
    if (window->old_key == 0 && window->key != 0) {
      *arg1 = window->key;
      r = WINDOW_KEYDOWN;
    } else if (window->old_key != 0 && window->key == 0) {
      *arg1 = window->old_key;
      r = WINDOW_KEYUP;
    }
    window->old_key = window->key;
  }

  return r;
}

static int libwwayland_window_show_cursor(window_t *_window, int show) {
  //libwwayland_window_t *window;
  int r = -1;

  if (_window) {
    //window = (libwwayland_window_t *)_window;
    r = 0;
  }

  return r;
}

static struct wl_buffer *create_buffer(libwwayland_window_t *window, int width, int height) {
  struct wl_shm_pool *pool;
  struct wl_buffer *buffer;
  int stride = width * window->spixel;
  int size = stride * height;
  uint32_t *p;
  int fd, i;
  
  fd = create_shm_file(size);
  if (fd < 0) {
    debug(DEBUG_ERROR, "WAYLAND", "creating a buffer file for %d B failed: %m", size);
    return NULL;
  }
  
  window->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (window->shm_data == MAP_FAILED) {
    debug(DEBUG_ERROR, "WAYLAND", "mmap failed: %m");
    close(fd);
    return NULL;
  } 
    
  p = (uint32_t *)window->shm_data;
  for (i = 0; i < width * height; i++) {
    p[i] = 0xff404080;
  }

  pool = wl_shm_create_pool(window->shm, fd, size);
  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);

  return buffer;
}

static window_t *libwwayland_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate,
     int fullscreen, int software, char *driver, void *data) {

  libwwayland_window_t *window;
  struct wl_display *display;
  struct wl_registry *registry;
	struct wl_cursor_theme *cursor_theme;
	struct wl_cursor *cursor;
  struct xkb_compose_table *compose_table;
  uint32_t spixel;
  const char *locale;

  switch (encoding) {
    case ENC_RGBA:
      spixel = sizeof(uint32_t);
      break;
    default:
      debug(DEBUG_ERROR, "WAYLAND", "invalid encoding %s", video_encoding_name(encoding));
      return NULL;
  }

  if ((display = wl_display_connect(NULL)) == NULL) {
    debug(DEBUG_ERROR, "WAYLAND", "wl_display_connect failed");
    return NULL;
  }

  if ((window = sys_calloc(1, sizeof(libwwayland_window_t))) != NULL) {
    window->display = display;
    window->width = *width;
    window->height = *height;
    window->spixel = spixel;
    window->running = 1;

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, window);
    if (wl_display_roundtrip(display) == -1) {
      debug(DEBUG_ERROR, "WAYLAND", "wl_registry_add_listener failed");
      sys_free(window);
      return NULL;
    } 

    if (window->shm == NULL || window->compositor == NULL || window->xdg_wm_base == NULL) {
      debug(DEBUG_ERROR, "WAYLAND", "no wl_shm, wl_compositor or xdg_wm_base support");
      sys_free(window);
      return NULL;
    }

    window->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    window->compose_state = NULL;
    locale = setlocale(LC_CTYPE, NULL);
    compose_table = xkb_compose_table_new_from_locale(window->xkb_context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (compose_table) {
      window->compose_state = xkb_compose_state_new(compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
    } else { 
      debug(DEBUG_ERROR, "WAYLAND", "xkb_compose_table_new_from_locale failed");
    }

	  cursor_theme = wl_cursor_theme_load(NULL, 24, window->shm);
	  cursor = wl_cursor_theme_get_cursor(cursor_theme, "left_ptr");
	  window->cursor_image = cursor->images[0];
	  window->cursor_buffer = wl_cursor_image_get_buffer(window->cursor_image);
	  window->cursor_surface = wl_compositor_create_surface(window->compositor);
	  wl_surface_attach(window->cursor_surface, window->cursor_buffer, 0, 0);
	  wl_surface_commit(window->cursor_surface);

    window->surface = wl_compositor_create_surface(window->compositor);
    window->xdg_surface = xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->surface);
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);

    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
    xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

    wl_surface_commit(window->surface);
    while (wl_display_dispatch(window->display) != -1 && !window->configured);

    if ((window->buffer = create_buffer(window, window->width + 2*MARGIN, window->height + 2*MARGIN)) == NULL) {
      sys_free(window);
      return NULL;
    }

    wl_surface_attach(window->surface, window->buffer, 0, 0);
    wl_surface_commit(window->surface);

    xdg_surface_set_window_geometry(window->xdg_surface, 0, 0, window->width + 2*MARGIN, window->height + 2*MARGIN);
    wl_surface_commit(window->surface);
  }

  return (window_t *)window;
}

static int libwwayland_window_render(window_t *_window) {
  return 0;
}

static int libwwayland_window_erase(window_t *_window, uint32_t bg) {
  libwwayland_window_t *window;
  int r = -1;

  if (_window) {
    window = (libwwayland_window_t *)_window;
    sys_memset(window->shm_data, 0, window->width * window->height * window->spixel);

    if (window->background) {
      window_provider.draw_texture(_window, window->background, 0, 0);
    }

    r = 0;
  }

  return r;
}

static texture_t *libwwayland_window_create_texture(window_t *_window, int width, int height) {
  libwwayland_window_t *window;
  uint8_t *buf;
  texture_t *texture = NULL;

  window = (libwwayland_window_t *)_window;

  if (window && width > 0 && height > 0) {
    if ((buf = sys_calloc(width * height, window->spixel)) != NULL) {
      if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
        texture->width = width;
        texture->height = height;
        texture->buf = buf;
      } else {
        sys_free(buf);
      }
    }
  }

  return texture;
}

static int libwwayland_window_destroy_texture(window_t *_window, texture_t *texture) {
  libwwayland_window_t *window;
  int r = -1;

  window = (libwwayland_window_t *)_window;

  if (window && texture) {
    sys_free(texture->buf);
    sys_free(texture);
  }

  return r;
}

static int libwwayland_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  libwwayland_window_t *window;
  uint8_t *s, *d;
  int pitch, len, i, r = -1;

  if (_window && texture && w > 0 && h > 0 && tx >= 0 && ty >= 0 && (tx+w) <= texture->width && (ty+h) <= texture->height) {
    window = (libwwayland_window_t *)_window;
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

static int libwwayland_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  libwwayland_window_t *window;
  int r = -1;

  if (_window && texture && src) {
    window = (libwwayland_window_t *)_window;
    sys_memcpy(texture->buf, src, texture->width * texture->height * window->spixel);
    r = 0;
  }

  return r;
}

static int libwwayland_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libwwayland_window_t *window = (libwwayland_window_t *)_window;
  uint8_t *s, *d;
  int spitch, dpitch, len, i, r = -1;

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

      x += MARGIN;
      y += MARGIN;

      s = &texture->buf[(ty * texture->width + tx) * window->spixel];
      d = &window->shm_data[(y * (window->width + 2*MARGIN) + x) * window->spixel];
      spitch = texture->width * window->spixel;
      dpitch = (window->width + 2*MARGIN) * window->spixel;
      len = w * window->spixel;
      for (i = 0; i < h; i++) {
        sys_memcpy(d, s, len);
        s += spitch;
        d += dpitch;
      }
      wl_surface_attach(window->surface, window->buffer, 0, 0);
      wl_surface_damage(window->surface, x, y, w, h);
      wl_surface_commit(window->surface);
      r = 0;
    } else {
      debug(DEBUG_ERROR, "WAYLAND", "invalid libwwayland_window_draw_texture_rect %d,%d %dx%d %d,%d", tx, ty, w, h, x, y);
    }
  }

  return r;
}

static int libwwayland_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  int r = -1;

  if (texture) {
    r = libwwayland_window_draw_texture_rect(_window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return r;
}

static int libwwayland_window_background(window_t *_window, uint32_t *raw, int width, int height) {
  libwwayland_window_t *window;
  int r = -1;

  if (_window && raw) {
    window = (libwwayland_window_t *)_window;
    debug(DEBUG_TRACE, "WAYLAND", "background %dx%d", width, height);
    if (window->background) {
      libwwayland_window_destroy_texture(_window, window->background);
    }
    window->background = libwwayland_window_create_texture(_window, width, height);
    libwwayland_window_update_texture(_window, window->background, (uint8_t *)raw);
    r = 0;
  }

  return r;
}

static void libwwayland_window_status(window_t *window, int *x, int *y, int *buttons) {
  libwwayland_status((libwwayland_window_t *)window, x, y, buttons);
}

static void libwwayland_window_title(window_t *window, char *title) {
  libwwayland_title((libwwayland_window_t *)window, title);
}

static char *libwwayland_window_clipboard(window_t *window, char *clipboard, int len) {
  return libwwayland_clipboard((libwwayland_window_t *)window, clipboard, len);
}

static int libwwayland_window_event2(window_t *window, int wait, int *arg1, int *arg2) {
  return libwwayland_event2((libwwayland_window_t *)window, wait, arg1, arg2);
}

static int libwwayland_window_update(window_t *_window, int x, int y, int width, int height) {
  return 0;
}

static int libwwayland_window_destroy(window_t *_window) {
  libwwayland_window_t *window;
  int r = -1;

  if (_window) {
    window = (libwwayland_window_t *)_window;
    if (window->background) {
      libwwayland_window_destroy_texture(_window, window->background);
    }
	  xdg_toplevel_destroy(window->xdg_toplevel);
	  xdg_surface_destroy(window->xdg_surface);
	  wl_surface_destroy(window->surface);
	  wl_buffer_destroy(window->buffer);
    sys_free(window);
    r = 0;
  }

  return r;
}

static int libwwayland_window_average(window_t *window, int *x, int *y, int ms) {
  return average_click(&window_provider, window, x, y, ms);
}

static int libwwayland_calib(int pe) {
  window_provider.average = libwwayland_window_average;
  return 0;
}

int libwwayland_load(void) {
  sys_memset(&window_provider, 0, sizeof(window_provider));
  window_provider.create = libwwayland_window_create;
  window_provider.destroy = libwwayland_window_destroy;
  window_provider.erase = libwwayland_window_erase;
  window_provider.render = libwwayland_window_render;
  window_provider.background = libwwayland_window_background;
  window_provider.create_texture = libwwayland_window_create_texture;
  window_provider.destroy_texture = libwwayland_window_destroy_texture;
  window_provider.update_texture = libwwayland_window_update_texture;
  window_provider.draw_texture = libwwayland_window_draw_texture;
  window_provider.status = libwwayland_window_status;
  window_provider.title = libwwayland_window_title;
  window_provider.clipboard = libwwayland_window_clipboard;
  window_provider.event2 = libwwayland_window_event2;
  window_provider.update = libwwayland_window_update;
  window_provider.draw_texture_rect = libwwayland_window_draw_texture_rect;
  window_provider.update_texture_rect = libwwayland_window_update_texture_rect;
  window_provider.show_cursor = libwwayland_window_show_cursor;

  return 0;
}

int libwwayland_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "WAYLAND", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);
  script_add_iconst(pe, obj, "hdepth", 32);

  script_add_function(pe, obj, "calib", libwwayland_calib);

  return 0;
}

int libwwayland_unload(void) {
  return 0;
}
