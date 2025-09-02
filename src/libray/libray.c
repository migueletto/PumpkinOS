#include "sys.h"
#include "thread.h"
#include "script.h"
#include "pwindow.h"
#include "average.h"
#include "debug.h"
#include "raylib.h"

struct texture_t {
  int width, height;
  uint32_t *aux;
};

typedef struct {
  int width, height;
  int x, y, buttons;
  int numKeyDown;
  uint8_t keyDown[512];
  uint16_t charDown[512];
  uint32_t *aux;
  Texture2D texture;
} libray_window_t;

static window_provider_t window_provider;

static window_t *libray_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate,
     int fullscreen, int software, char *driver, void *data) {

  libray_window_t *window;
  Image image;

  if ((window = sys_calloc(1, sizeof(libray_window_t))) != NULL) {
    window->width = *width;
    window->height = *height;
    window->aux = sys_calloc(window->width * window->height, sizeof(uint32_t));
    InitWindow(window->width, window->height, "");
    image = GenImageColor(window->width, window->height, BLACK);
    window->texture = LoadTextureFromImage(image);
    UnloadImage(image);
  }

  return (window_t *)window;
}

static int libray_window_render(window_t *_window) {
  libray_window_t *window = (libray_window_t *)_window;
  int r = -1;

  if (window) {
    BeginDrawing();
    DrawTexture(window->texture, 0, 0, WHITE);
    EndDrawing();
    r = 0;
  }

  return r;
}

static int libray_window_erase(window_t *_window, uint32_t bg) {
  libray_window_t *window = (libray_window_t *)_window;
  int r = -1;

  if (window) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
    r = 0;
  }

  return r;
}

static texture_t *libray_window_create_texture(window_t *_window, int width, int height) {
  texture_t *texture;

  if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    texture->aux = sys_calloc(width * height, sizeof(uint32_t));
  }

  return texture;
}

static int libray_window_destroy_texture(window_t *_window, texture_t *texture) {
  int r = -1;

  if (texture) {
    sys_free(texture->aux);
    sys_free(texture);
    r = 0;
  }

  return r;
}

static int libray_window_background(window_t *_window, uint32_t *raw, int width, int height) {
  return -1;
}

static int libray_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *p, int tx, int ty, int w, int h) {
  uint32_t *src, *dst;
  int i, j, r = -1;

  if (texture && p) {
    src = (uint32_t *)p;
    src += ty * texture->width + tx;
    dst = texture->aux;
    dst += ty * texture->width + tx;
    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        uint32_t d = src[j];
        uint32_t a = (d >> 24) & 0xff;
        uint32_t r = (d >> 16) & 0xff;
        uint32_t g = (d >> 8) & 0xff;
        uint32_t b = d & 0xff; 
        // ABGR
        d = (a << 24) | (b << 16) | (g << 8) | r;
        dst[j] = d;
      }
      src += texture->width;
      dst += texture->width;
    }
    r = 0;
  }

  return r;
}

static int libray_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  int r = -1;

  if (texture && src) {
    r = libray_window_update_texture_rect(_window, texture, src, 0, 0, texture->width, texture->height);
  }

  return r;
}

static int libray_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libray_window_t *window = (libray_window_t *)_window;
  uint32_t *src, *dst;
  int i, j, r = -1;

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

      src = texture->aux;
      src += ty * texture->width + tx;
      dst = window->aux;
      dst += y * window->width + x;
      for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
          dst[j] = src[j];
        }
        src += texture->width;
        dst += window->width;
      } 
      UpdateTexture(window->texture, window->aux);
    }
    r = 0;
  }

  return r;
}

static int libray_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  int r = -1;

  if (_window && texture) {
    r = libray_window_draw_texture_rect(_window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return r;
}

static void libray_window_status(window_t *_window, int *x, int *y, int *buttons) {
  libray_window_t *window = (libray_window_t *)_window;
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

static void libray_window_title(window_t *window, char *title) {
  SetWindowTitle(title);
}

static char *libray_window_clipboard(window_t *window, char *clipboard, int len) {
  char *s;

  if (clipboard != NULL && len > 0) {
    if ((s = sys_calloc(1, len + 1)) != NULL) {
      sys_memcpy(s, clipboard, len);
      SetClipboardText(s);
      sys_free(s);
    }
  }

  return (char *)GetClipboardText();
}

static int libray_window_event(window_t *window, int wait, int remove, int *key, int *mods, int *buttons) {
  return 0;
}

int mapKey(int key) {
  switch (key) {
    case KEY_LEFT_SHIFT:    key = WINDOW_KEY_SHIFT; break;
    case KEY_RIGHT_SHIFT:   key = WINDOW_KEY_SHIFT; break;
    case KEY_LEFT_CONTROL:  key = WINDOW_KEY_CTRL; break;
    case KEY_RIGHT_CONTROL: key = WINDOW_KEY_RCTRL; break;
    case KEY_RIGHT:         key = WINDOW_KEY_RIGHT; break;
    case KEY_LEFT:          key = WINDOW_KEY_LEFT; break;
    case KEY_DOWN:          key = WINDOW_KEY_DOWN; break;
    case KEY_UP:            key = WINDOW_KEY_UP; break;
    case KEY_PAGE_UP:       key = WINDOW_KEY_PGUP; break;
    case KEY_PAGE_DOWN:     key = WINDOW_KEY_PGDOWN; break;
    case KEY_HOME:          key = WINDOW_KEY_HOME; break;
    case KEY_END:           key = WINDOW_KEY_END; break;
    case KEY_INSERT:        key = WINDOW_KEY_INS; break;
    case KEY_F1:            key = WINDOW_KEY_F1; break;
    case KEY_F2:            key = WINDOW_KEY_F2; break;
    case KEY_F3:            key = WINDOW_KEY_F3; break;
    case KEY_F4:            key = WINDOW_KEY_F4; break;
    case KEY_F5:            key = WINDOW_KEY_F5; break;
    case KEY_F6:            key = WINDOW_KEY_F6; break;
    case KEY_F7:            key = WINDOW_KEY_F7; break;
    case KEY_F8:            key = WINDOW_KEY_F8; break;
    case KEY_F9:            key = WINDOW_KEY_F9; break;
    case KEY_F10:           key = WINDOW_KEY_F10; break;
    case KEY_F11:           key = WINDOW_KEY_F11; break;
    case KEY_F12:           key = WINDOW_KEY_F12; break;
    case KEY_BACKSPACE:     key = 8; break;
    case KEY_TAB:           key = 9; break;
    case KEY_ENTER:         key = 13; break;
  }

  return key;
}

static int libray_window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  libray_window_t *window = (libray_window_t *)_window;
  int leftDown, rightDown, x, y, keyCode, key, i;
  uint32_t n = wait > 0 ? wait * 10 : 0; // n = (ms * 1000) / 100

  if (window) {
    for (i = 0;;) {
      if (thread_must_end()) return -1;
      if (i > 0) sys_usleep(100);
      PollInputEvents();

      leftDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
      rightDown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

      if (leftDown && !(window->buttons & 1)) {
        window->buttons |= 1;
        *arg1 = 1;
        return WINDOW_BUTTONDOWN;
      }

      if (!leftDown && (window->buttons & 1)) {
        window->buttons &= ~1;
        *arg1 = 1;
        return WINDOW_BUTTONUP;
      }

      if (rightDown && !(window->buttons & 2)) {
        window->buttons |= 2;
        *arg1 = 2;
        return WINDOW_BUTTONDOWN;
      }

      if (!rightDown && (window->buttons & 2)) {
        window->buttons &= ~2;
        *arg1 = 2;
        return WINDOW_BUTTONUP;
      }

      x = GetMouseX();
      y = GetMouseY();

      if (x != window->x || y != window->y) {
        *arg1 = window->x = x;
        *arg2 = window->y = y;
        return WINDOW_MOTION;
      }

      if (window->numKeyDown > 0) {
        for (keyCode = 1; keyCode < 512; keyCode++) {
          if (window->keyDown[keyCode] && !IsKeyDown(keyCode)) {
            key = window->charDown[keyCode];
            debug(2, "RAYLIB", "key up   '%c' %d (code %d)", key >= 32 && key < 127 ? key : ' ', key, keyCode);
            window->keyDown[keyCode] = 0;
            window->numKeyDown--;
            *arg1 = key;
            return WINDOW_KEYUP;
          }
        }
      }

      key = GetCharPressed();
      keyCode = GetKeyPressed();

      if (keyCode > 0 && keyCode < 512) {
        if (key) {
          key = mapKey(key);
          debug(2, "RAYLIB", "key down '%c' %d (code %d) C", key >= 32 && key < 127 ? key : ' ', key, keyCode);
        } else {
          key = mapKey(keyCode);
          debug(2, "RAYLIB", "key down '%c' %d (code %d)", key >= 32 && key < 127 ? key : ' ', key, keyCode);
        }

        if (key) {
          window->charDown[keyCode] = key;
          window->keyDown[keyCode] = 1;
          window->numKeyDown++;
          *arg1 = key;
          return WINDOW_KEYDOWN;
        }
      }

      if (wait == 0) break;
      i++;
      if (wait < 0) continue;
      if (i == n) break;
    }
  }

  return 0;
}

static int libray_window_update(window_t *_window, int x, int y, int width, int height) {
  return 0;
}

static int libray_window_destroy(window_t *_window) {
  libray_window_t *window = (libray_window_t *)_window;

  if (window) {
    UnloadTexture(window->texture);
    CloseWindow();
    sys_free(window->aux);
    sys_free(window);
  }

  return 0;
}

static int libray_window_average(window_t *window, int *x, int *y, int ms) {
  return average_click(&window_provider, window, x, y, ms);
}

static int libray_calib(int pe) {
  window_provider.average = libray_window_average;
  return 0;
}

int libray_load(void) {
  sys_memset(&window_provider, 0, sizeof(window_provider));
  window_provider.create = libray_window_create;
  window_provider.event = libray_window_event;
  window_provider.destroy = libray_window_destroy;
  window_provider.erase = libray_window_erase;
  window_provider.render = libray_window_render;
  window_provider.background = libray_window_background;
  window_provider.create_texture = libray_window_create_texture;
  window_provider.destroy_texture = libray_window_destroy_texture;
  window_provider.update_texture = libray_window_update_texture;
  window_provider.draw_texture = libray_window_draw_texture;
  window_provider.status = libray_window_status;
  window_provider.title = libray_window_title;
  window_provider.clipboard = libray_window_clipboard;
  window_provider.event2 = libray_window_event2;
  window_provider.update = libray_window_update;
  window_provider.draw_texture_rect = libray_window_draw_texture_rect;
  window_provider.update_texture_rect = libray_window_update_texture_rect;

  return 0;
}

static void traceLogCallback(int level, const char *fmt, va_list ap) {
  switch (level) {
    case LOG_DEBUG:   level = DEBUG_TRACE; break;
    case LOG_INFO:    level = DEBUG_INFO;  break;
    case LOG_WARNING: level = DEBUG_INFO;  break;
    case LOG_ERROR:   level = DEBUG_ERROR; break;
    case LOG_FATAL:   level = DEBUG_ERROR; break;
    default: return;
  }

  debugva_full("", "", 0, level, "RAYLIB", fmt, ap);
}

int libray_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "RAYLIB", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);
  script_add_iconst(pe, obj, "hdepth", 32);

  script_add_function(pe, obj, "calib", libray_calib);

  SetTraceLogCallback(traceLogCallback);

  return 0;
}
