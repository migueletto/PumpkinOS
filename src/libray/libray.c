#include "sys.h"
#include "thread.h"
#include "script.h"
#include "pwindow.h"
#include "debug.h"
#include "raylib.h"

#define MAX_DRAW 256

struct texture_t {
  int width, height;
  Texture2D texture;
  uint32_t *aux;
};

typedef struct {
  texture_t *texture;
  Rectangle src;
  Vector2 pos;
} texture_draw_t;

typedef struct {
  int width, height;
  int x, y, buttons;
  int numKeyDown;
  uint8_t keyDown[512];
  uint16_t charDown[512];
  texture_draw_t draw[MAX_DRAW];
  int ndraw;
} libray_window_t;

static window_provider_t window_provider;

static window_t *libray_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate,
     int fullscreen, int software, char *driver, void *data) {

  libray_window_t *window;

  if ((window = sys_calloc(1, sizeof(libray_window_t))) != NULL) {
    window->width = *width;
    window->height = *height;
    InitWindow(window->width, window->height, "");
  }

  return (window_t *)window;
}

static int libray_window_render(window_t *_window) {
  libray_window_t *window = (libray_window_t *)_window;
  int i, r = -1;

  if (window) {
    BeginDrawing();
    ClearBackground(WHITE);
    for (i = 0; i < window->ndraw; i++) {
      DrawTextureRec(window->draw[i].texture->texture, window->draw[i].src, window->draw[i].pos, WHITE);
    }
    EndDrawing();
    window->ndraw = 0;
    r = 0;
  }

  return r;
}

static int libray_window_erase(window_t *_window, uint32_t bg) {
  libray_window_t *window = (libray_window_t *)_window;
  int r = -1;

  if (window) {
    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();
    r = 0;
  }

  return r;
}

static texture_t *libray_window_create_texture(window_t *_window, int width, int height) {
  texture_t *texture;
  Image image;

  if ((texture = sys_calloc(1, sizeof(texture_t))) != NULL) {
    image = GenImageColor(width, height, WHITE);
    texture->width = width;
    texture->height = height;
    texture->texture = LoadTextureFromImage(image);
    texture->aux = sys_calloc(width * height, sizeof(uint32_t));
    UnloadImage(image);
  }

  return texture;
}

static int libray_window_destroy_texture(window_t *_window, texture_t *texture) {
  int r = -1;

  if (texture) {
    UnloadTexture(texture->texture);
    sys_free(texture->aux);
    sys_free(texture);
    r = 0;
  }

  return r;
}

static int libray_window_background(window_t *_window, uint32_t *raw, int width, int height) {
  return -1;
}

static int libray_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  int r = -1;

  if (texture && src) {
    UpdateTexture(texture->texture, src);
    r = 0;
  }

  return r;
}

static int libray_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  Rectangle rec;
  uint32_t *p;
  int i, k, len, r = -1;

  if (texture && src) {
    if (tx == 0 && ty == 0 && w == texture->width && h == texture->height) {
      return libray_window_update_texture(_window, texture, src);
    }
    p = (uint32_t *)src;
    p += ty * texture->width + tx;
    len = w * sizeof(uint32_t);
    for (i = 0, k = 0; i < h; i++) {
      sys_memcpy(&texture->aux[k], p, len);
      k += w;
      p += texture->width;
    }
    rec.x = tx;
    rec.y = ty;
    rec.width = w;
    rec.height = h;
    UpdateTextureRec(texture->texture, rec, texture->aux);
    r = 0;
  }

  return r;
}

static int libray_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libray_window_t *window = (libray_window_t *)_window;
  int r = -1;

  if (window && texture && w > 0 && h > 0) {
    if (window->ndraw < MAX_DRAW) {
      window->draw[window->ndraw].texture = texture;
      window->draw[window->ndraw].src.x = tx;
      window->draw[window->ndraw].src.y = ty;
      window->draw[window->ndraw].src.width = w;
      window->draw[window->ndraw].src.height = h;
      window->draw[window->ndraw].pos.x = x;
      window->draw[window->ndraw].pos.y = y;
      window->ndraw++;
      r = 0;
    }
  }

  return r;
}

static int libray_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  libray_window_t *window = (libray_window_t *)_window;
  int r = -1;

  if (window && texture) {
    if (window->ndraw < MAX_DRAW) {
      window->draw[window->ndraw].texture = texture;
      window->draw[window->ndraw].src.x = 0;
      window->draw[window->ndraw].src.y = 0;
      window->draw[window->ndraw].src.width = texture->width;
      window->draw[window->ndraw].src.height = texture->height;
      window->draw[window->ndraw].pos.x = x;
      window->draw[window->ndraw].pos.y = y;
      window->ndraw++;
      r = 0;
    }
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

static int libray_window_destroy(window_t *window) {
  CloseWindow();
  return 0;
}

// fake "average" function just for testing the UI
static int libray_window_average(window_t *_window, int *x, int *y, int ms) {
  int arg1, arg2;

  for (;;) {
    if (thread_must_end()) return -1;

    switch (libray_window_event2(_window, 1, &arg1, &arg2)) {
      case WINDOW_BUTTONUP:
        return 1;
      case WINDOW_MOTION:
        *x = arg1;
        *y = arg2;
        break;
      case 0:
        if (ms == -1) continue;
        if (ms == 0) return 0;
        ms--;
        break;
      case -1:
        return -1;
    }
  }

  return -1;
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

  script_add_function(pe, obj, "calib", libray_calib);

  SetTraceLogCallback(traceLogCallback);

  return 0;
}
