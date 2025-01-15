#include <SDL2/SDL.h>

#ifdef SDL_OPENGL
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#endif

#ifdef SDL_MIXER
#include <SDL2/SDL_mixer.h>
#endif

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "pwindow.h"
#include "audio.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define SDL_BUTTON1   1
#define SDL_BUTTON2   2

#define MAX_DRIVER 128

#define MAX_SHADER 16
#define MAX_VARS   16

struct texture_t {
  SDL_Texture *t;
  int width, height;
  SDL_Texture *blur;
  uint32_t *buf;
};

typedef struct {
  uint8_t from;
  uint8_t mods;
  uint8_t to;
} libsdl_keymap_t;

typedef struct {
  int width, height, xfactor, yfactor, rotate, fullscreen, software, spixel;
  int x, y, buttons, mods, other;
  uint32_t format;
  char driver[MAX_DRIVER];
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *background;
  libsdl_keymap_t keymap[0x1000];
  int64_t shift_up;
  int opengl;
#ifdef SDL_OPENGL
  GLuint programId[MAX_SHADER];
  texture_t *texture;
  float (*getvar)(char *name, void *data);
  void *data;
#endif
} libsdl_window_t;

static libsdl_keymap_t keymap[] = {
  { '\'', WINDOW_MOD_SHIFT, '"' },
  { '1', WINDOW_MOD_SHIFT, '!' },
  { '2', WINDOW_MOD_SHIFT, '@' },
  { '3', WINDOW_MOD_SHIFT, '#' },
  { '4', WINDOW_MOD_SHIFT, '$' },
  { '5', WINDOW_MOD_SHIFT, '%' },
  { '7', WINDOW_MOD_SHIFT, '&' },
  { '8', WINDOW_MOD_SHIFT, '*' },
  { '9', WINDOW_MOD_SHIFT, '(' },
  { '0', WINDOW_MOD_SHIFT, ')' },
  { '-', WINDOW_MOD_SHIFT, '_' },
  { '=', WINDOW_MOD_SHIFT, '+' },
  { '\\', WINDOW_MOD_SHIFT, '|' },
  { '[', WINDOW_MOD_SHIFT, '{' },
  { ']', WINDOW_MOD_SHIFT, '}' },
  { '~', WINDOW_MOD_SHIFT, '^' },
  { '/', WINDOW_MOD_SHIFT, '?' },
  { ';', WINDOW_MOD_SHIFT, ':' },
  { '.', WINDOW_MOD_SHIFT, '>' },
  { ',', WINDOW_MOD_SHIFT, '<' },

  // teclas com AltGr do acer
  { 'q', WINDOW_MOD_RALT, '/' },
  { 'w', WINDOW_MOD_RALT, '?' },

  { 'a', WINDOW_MOD_SHIFT, 'A' },
  { 'b', WINDOW_MOD_SHIFT, 'B' },
  { 'c', WINDOW_MOD_SHIFT, 'C' },
  { 'd', WINDOW_MOD_SHIFT, 'D' },
  { 'e', WINDOW_MOD_SHIFT, 'E' },
  { 'f', WINDOW_MOD_SHIFT, 'F' },
  { 'g', WINDOW_MOD_SHIFT, 'G' },
  { 'h', WINDOW_MOD_SHIFT, 'H' },
  { 'i', WINDOW_MOD_SHIFT, 'I' },
  { 'j', WINDOW_MOD_SHIFT, 'J' },
  { 'k', WINDOW_MOD_SHIFT, 'K' },
  { 'l', WINDOW_MOD_SHIFT, 'L' },
  { 'm', WINDOW_MOD_SHIFT, 'M' },
  { 'n', WINDOW_MOD_SHIFT, 'N' },
  { 'o', WINDOW_MOD_SHIFT, 'O' },
  { 'p', WINDOW_MOD_SHIFT, 'P' },
  { 'q', WINDOW_MOD_SHIFT, 'Q' },
  { 'r', WINDOW_MOD_SHIFT, 'R' },
  { 's', WINDOW_MOD_SHIFT, 'S' },
  { 't', WINDOW_MOD_SHIFT, 'T' },
  { 'u', WINDOW_MOD_SHIFT, 'U' },
  { 'v', WINDOW_MOD_SHIFT, 'V' },
  { 'w', WINDOW_MOD_SHIFT, 'W' },
  { 'x', WINDOW_MOD_SHIFT, 'X' },
  { 'y', WINDOW_MOD_SHIFT, 'Y' },
  { 'z', WINDOW_MOD_SHIFT, 'Z' },

  { 'a', WINDOW_MOD_CTRL, 1 },
  { 'b', WINDOW_MOD_CTRL, 2 },
  { 'c', WINDOW_MOD_CTRL, 3 },
  { 'd', WINDOW_MOD_CTRL, 4 },
  { 'e', WINDOW_MOD_CTRL, 5 },
  { 'f', WINDOW_MOD_CTRL, 6 },
  { 'g', WINDOW_MOD_CTRL, 7 },
  { 'h', WINDOW_MOD_CTRL, 8 },
  { 'i', WINDOW_MOD_CTRL, 9 },
  { 'j', WINDOW_MOD_CTRL, 10 },
  { 'k', WINDOW_MOD_CTRL, 11 },
  { 'l', WINDOW_MOD_CTRL, 12 },
  { 'm', WINDOW_MOD_CTRL, 13 },
  { 'n', WINDOW_MOD_CTRL, 14 },
  { 'o', WINDOW_MOD_CTRL, 15 },
  { 'p', WINDOW_MOD_CTRL, 16 },
  { 'q', WINDOW_MOD_CTRL, 17 },
  { 'r', WINDOW_MOD_CTRL, 18 },
  { 's', WINDOW_MOD_CTRL, 19 },
  { 't', WINDOW_MOD_CTRL, 20 },
  { 'u', WINDOW_MOD_CTRL, 21 },
  { 'v', WINDOW_MOD_CTRL, 22 },
  { 'w', WINDOW_MOD_CTRL, 23 },
  { 'x', WINDOW_MOD_CTRL, 24 },
  { 'y', WINDOW_MOD_CTRL, 25 },
  { 'z', WINDOW_MOD_CTRL, 26 },

  { 0, 0, 0 }
};

static window_provider_t window_provider;
static audio_provider_t audio_provider;

static int libsdl_init_audio(void) {
  int r = -1;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
    debug(DEBUG_ERROR, "SDL", "init audio failed: %s", SDL_GetError());
  } else {
#ifdef SDL_MIXER
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
      debug(DEBUG_ERROR, "SDL", "mixer open failed: %s", SDL_GetError());
    } else {
      debug(DEBUG_INFO, "SDL", "audio inited");
      r = 0;
    }
#else
    r = 0;
#endif
  }

  return r;
}

// SDL_RENDERER_SOFTWARE: The renderer is a software fallback
// SDL_RENDERER_ACCELERATED: The renderer uses hardware acceleration
// SDL_RENDERER_PRESENTVSYNC: Present is synchronized with the refresh
// SDL_RENDERER_TARGETTEXTURE: renderer supports rendering to texture

static void libsdl_render_info(char *label, SDL_RendererInfo *info) {
  debug(DEBUG_INFO, "SDL", "%s driver \"%s\", flags:%s%s%s%s", label, info->name,
    (info->flags & SDL_RENDERER_SOFTWARE)      ? " Software" : "",
    (info->flags & SDL_RENDERER_ACCELERATED)   ? " Accelerated" : "",
    (info->flags & SDL_RENDERER_PRESENTVSYNC)  ? " PresentVsync" : "",
    (info->flags & SDL_RENDERER_TARGETTEXTURE) ? " TargetTexture" : "");
}

static int libsdl_init_video(void) {
  SDL_RendererInfo info;
  SDL_Cursor *cursor;
  int flags, i, n, r = -1;
 
  flags = SDL_INIT_VIDEO;
#ifdef SDL_OPENGL
  flags |= SDL_VIDEO_OPENGL;
#endif

  if (SDL_InitSubSystem(flags) != 0) {
    debug(DEBUG_ERROR, "SDL", "init video failed: %s", SDL_GetError());

  } else {
    if ((n = SDL_GetNumRenderDrivers()) == -1) {
      debug(DEBUG_ERROR, "SDL", "SDL_GetNumRenderDrivers failed: %s", SDL_GetError());
    } else {
      for (i = 0; i < n; i++) {
        if (SDL_GetRenderDriverInfo(i, &info) == -1) {
          debug(DEBUG_ERROR, "SDL", "SDL_GetRenderDriverInfo %d failed: %s", i, SDL_GetError());
        } else {
          libsdl_render_info("available", &info);
          if ((cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND)) != NULL) {
            SDL_SetCursor(cursor);
          }
          //SDL_ShowCursor(SDL_DISABLE);
          r = 0;
        }
      }
    }
  }

  return r;
}

static void libsdl_status(libsdl_window_t *window, int *x, int *y, int *buttons) {
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

static void libsdl_title(libsdl_window_t *window, char *title) {
  SDL_SetWindowTitle(window->window, title);
}

static char *libsdl_clipboard(libsdl_window_t *window, char *clipboard, int len) {
  char *s;

  if (clipboard != NULL && len > 0) {
    if ((s = xcalloc(1, len + 1)) != NULL) {
      xmemcpy(s, clipboard, len);
      SDL_SetClipboardText(s);  // SDL_SetClipboardText puts UTF-8 text into the clipboard
      xfree(s);
    }
  }

  return SDL_GetClipboardText();
}

static int map_key(libsdl_window_t *window, SDL_Event *ev) {
  int index, shift = 0, key = 0;

  switch (ev->key.keysym.sym) {
    case SDLK_LCTRL:
      if (ev->type == SDL_KEYDOWN) {
        window->mods |= WINDOW_MOD_CTRL;
      } else {
        window->mods &= ~WINDOW_MOD_CTRL;
      }
      key = WINDOW_KEY_CTRL;
      break;
    case SDLK_RCTRL:
      if (ev->type == SDL_KEYDOWN) {
        window->mods |= WINDOW_MOD_RCTRL;
      } else {
        window->mods &= ~WINDOW_MOD_RCTRL;
      }
      key = WINDOW_KEY_RCTRL;
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      if (ev->type == SDL_KEYDOWN) {
        window->mods |= WINDOW_MOD_SHIFT;
        window->other = 0;
      } else {
        window->mods &= ~WINDOW_MOD_SHIFT;
      }
      key = WINDOW_KEY_SHIFT;
      break;
    case SDLK_LALT:
      if (ev->type == SDL_KEYDOWN) {
        window->mods |= WINDOW_MOD_LALT;
      } else {
        window->mods &= ~WINDOW_MOD_LALT;
      }
      key = WINDOW_KEY_LALT;
      break;
    case SDLK_RALT:
      if (ev->type == SDL_KEYDOWN) {
        window->mods |= WINDOW_MOD_RALT;
      } else {
        window->mods &= ~WINDOW_MOD_RALT;
      }
      key = WINDOW_KEY_RALT;
      break;

    default:
      if (ev->type == SDL_KEYDOWN) {
        if (window->mods & WINDOW_MOD_SHIFT) window->other = 1;
      } else {
        if (window->other && !(window->mods & WINDOW_MOD_SHIFT)) {
          window->mods |= WINDOW_MOD_SHIFT;
          shift = 1;
        }
        window->other = 0;
      }

      if (ev->key.keysym.sym < 256) {
        index = (window->mods << 8) | ev->key.keysym.sym;
        if (window->keymap[index].to && (window->keymap[index].mods & window->mods)) {
          key = window->keymap[index].to;
        } else {
          key = ev->key.keysym.sym;
        }
      } else {
        switch (ev->key.keysym.sym) {
          case SDLK_RIGHT: key = WINDOW_KEY_RIGHT; break;
          case SDLK_LEFT: key = WINDOW_KEY_LEFT; break;
          case SDLK_DOWN: key = WINDOW_KEY_DOWN; break;
          case SDLK_UP: key = WINDOW_KEY_UP; break;
          case SDLK_PAGEUP: key = WINDOW_KEY_PGUP; break;
          case SDLK_PAGEDOWN: key = WINDOW_KEY_PGDOWN; break;
          case SDLK_HOME: key = WINDOW_KEY_HOME; break;
          case SDLK_END: key = WINDOW_KEY_END; break;
          case SDLK_INSERT: key = WINDOW_KEY_INS; break;
          case SDLK_F1: key = WINDOW_KEY_F1; break;
          case SDLK_F2: key = WINDOW_KEY_F2; break;
          case SDLK_F3: key = WINDOW_KEY_F3; break;
          case SDLK_F4: key = WINDOW_KEY_F4; break;
          case SDLK_F5: key = WINDOW_KEY_F5; break;
          case SDLK_F6: key = WINDOW_KEY_F6; break;
          case SDLK_F7: key = WINDOW_KEY_F7; break;
          case SDLK_F8: key = WINDOW_KEY_F8; break;
          case SDLK_F9: key = WINDOW_KEY_F9; break;
          case SDLK_F10: key = WINDOW_KEY_F10; break;
          case SDLK_F11: key = WINDOW_KEY_F11; break;
          case SDLK_F12: key = WINDOW_KEY_F12; break;
          case 0x40000000:
            switch (ev->key.keysym.scancode) {
              case 0x34:
                key = (window->mods & WINDOW_MOD_SHIFT) ? '^' : '~';
                break;
              case 0x87:
                key = (window->mods & WINDOW_MOD_SHIFT) ? '?' : '/';
                break;
              default:
                debug(DEBUG_ERROR, "SDL", "unknown scancode 0x%02X", ev->key.keysym.scancode);
                key = 0;
                break;
            }
            break;
          default:
            debug(DEBUG_ERROR, "SDL", "unmapped sym 0x%08X scancode 0x%08X", ev->key.keysym.sym, ev->key.keysym.scancode);
            break;
        }
      }

      if (shift) {
        window->mods &= ~WINDOW_MOD_SHIFT;
      }
  }

  return key;
}

static int map_button(libsdl_window_t *window, SDL_Event *ev) {
  switch (ev->button.button) {
    case 1: return SDL_BUTTON1;
    case 3: return SDL_BUTTON2;
  }

  return 0;
}

static int libsdl_event2(libsdl_window_t *window, int wait, int *arg1, int *arg2) {
  SDL_Event ev;
  int has_ev, r = 0;

  has_ev = wait < 0 ? SDL_WaitEvent(&ev) : SDL_WaitEventTimeout(&ev, wait);

  if (has_ev) {
    switch (ev.type) {
      case SDL_KEYDOWN:
        *arg1 = map_key(window, &ev);
        if (*arg1) r = WINDOW_KEYDOWN;
        break;
      case SDL_KEYUP:
        *arg1 = map_key(window, &ev);
        if (*arg1) r = WINDOW_KEYUP;
        break;
      case SDL_MOUSEBUTTONDOWN:
        *arg1 = map_button(window, &ev);
        if (*arg1) {
          window->buttons |= *arg1;
          r = WINDOW_BUTTONDOWN;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        *arg1 = map_button(window, &ev);
        if (*arg1) {
          window->buttons &= ~(*arg1);
          r = WINDOW_BUTTONUP;
        }
        break;
      case SDL_MOUSEMOTION:
        window->x = ev.motion.x / window->xfactor;
        window->y = ev.motion.y / window->yfactor;
        *arg1 = window->x;
        *arg2 = window->y;
        r = WINDOW_MOTION;
        break;
      case SDL_MOUSEWHEEL:
        *arg1 = ev.wheel.x;
        *arg2 = ev.wheel.y;
        r = WINDOW_WHEEL;
        break;
    case SDL_WINDOWEVENT:
        switch (ev.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            r = -1;
            break;
          default:
            break;
        }
        break;
      case SDL_QUIT:
        r = -1;
        break;
    }
  }

  return r;
}

static int libsdl_event(libsdl_window_t *window, int wait, int remove, int *ekey, int *mods, int *ebuttons) {
  uint16_t index;
  int buttons, key;
  SDL_Event ev;
  int has_ev, shift, r = 0;

  buttons = window->buttons;

  for (;;) {
    if (wait) {
      // wait: timeout in ms, or -1 to wait forever
      has_ev = wait < 0 ? SDL_WaitEvent(&ev) : SDL_WaitEventTimeout(&ev, wait);
    } else {
      if (remove) {
        has_ev = SDL_PollEvent(&ev);
      } else {
        SDL_PumpEvents();
        has_ev = SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
      }
    }

    if (has_ev) {
      switch (ev.type) {
      case SDL_KEYDOWN:
        switch (ev.key.keysym.sym) {
          case SDLK_LCTRL:
            window->mods |= WINDOW_MOD_CTRL;
            debug(DEBUG_TRACE, "SDL", "left ctrl down");
            break;
          case SDLK_RCTRL:
            window->mods |= WINDOW_MOD_RCTRL;
            debug(DEBUG_TRACE, "SDL", "right ctrl down");
            break;
          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            window->mods |= WINDOW_MOD_SHIFT;
            debug(DEBUG_TRACE, "SDL", "shift down");
            break;
          case SDLK_LALT:
            window->mods |= WINDOW_MOD_LALT;
            debug(DEBUG_TRACE, "SDL", "lalt down");
            break;
          case SDLK_RALT:
            window->mods |= WINDOW_MOD_RALT;
            debug(DEBUG_TRACE, "SDL", "ralt down");
            break;
        }
        if (remove == 0) SDL_PollEvent(&ev);
        continue;

      case SDL_KEYUP:
        switch (ev.key.keysym.sym) {
          case SDLK_LCTRL:
          case SDLK_RCTRL:
            window->mods &= ~WINDOW_MOD_CTRL;
            debug(DEBUG_TRACE, "SDL", "ctrl up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            window->shift_up = sys_get_clock();
            window->mods &= ~WINDOW_MOD_SHIFT;
            debug(DEBUG_TRACE, "SDL", "shift up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          case SDLK_LALT:
            window->mods &= ~WINDOW_MOD_LALT;
            debug(DEBUG_TRACE, "SDL", "lalt up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          case SDLK_RALT:
            window->mods &= ~WINDOW_MOD_RALT;
            debug(DEBUG_TRACE, "SDL", "ralt up");
            if (remove == 0) SDL_PollEvent(&ev);
            continue;
          default:
            shift = 0;
            if (!(window->mods & WINDOW_MOD_SHIFT)) {
              if ((sys_get_clock() - window->shift_up) < 100000) {
                window->mods |= WINDOW_MOD_SHIFT;
                shift = 1;
              }
            }
            debug(DEBUG_TRACE, "SDL", "keyup sym 0x%08X (0x%02X)", ev.key.keysym.sym, window->mods);
            if (ev.key.keysym.sym < 256) {
              index = (window->mods << 8) | ev.key.keysym.sym;
              if (window->keymap[index].to && (window->keymap[index].mods & window->mods)) {
                key = window->keymap[index].to;
                debug(DEBUG_TRACE, "SDL", "change key '%c' -> '%c'", ev.key.keysym.sym, key);
              } else {
                key = ev.key.keysym.sym;
                debug(DEBUG_TRACE, "SDL", "key '%c'", key);
              }
            } else {
              switch (ev.key.keysym.sym) {
                case SDLK_RIGHT: key = WINDOW_KEY_RIGHT; break;
                case SDLK_LEFT: key = WINDOW_KEY_LEFT; break;
                case SDLK_DOWN: key = WINDOW_KEY_DOWN; break;
                case SDLK_UP: key = WINDOW_KEY_UP; break;
                case SDLK_PAGEUP: key = WINDOW_KEY_PGUP; break;
                case SDLK_PAGEDOWN: key = WINDOW_KEY_PGDOWN; break;
                case SDLK_HOME: key = WINDOW_KEY_HOME; break;
                case SDLK_END: key = WINDOW_KEY_END; break;
                case SDLK_F1: key = WINDOW_KEY_F1; break;
                case SDLK_F2: key = WINDOW_KEY_F2; break;
                case SDLK_F3: key = WINDOW_KEY_F3; break;
                case SDLK_F4: key = WINDOW_KEY_F4; break;
                case SDLK_F5: key = WINDOW_KEY_F5; break;
                case SDLK_F6: key = WINDOW_KEY_F6; break;
                case SDLK_F7: key = WINDOW_KEY_F7; break;
                case SDLK_F8: key = WINDOW_KEY_F8; break;
                case SDLK_F9: key = WINDOW_KEY_F9; break;
                case SDLK_F10: key = WINDOW_KEY_F10; break;
                case 0x40000000:
                  switch (ev.key.keysym.scancode) {
                    case 0x34:
                      key = (window->mods & WINDOW_MOD_SHIFT) ? '^' : '~';
                      debug(DEBUG_TRACE, "SDL", "scancode 0x%02X -> '%c'", ev.key.keysym.scancode, key);
                      break;
                    default:
                      debug(DEBUG_TRACE, "SDL", "unmapped scancode 0x%02X", ev.key.keysym.scancode);
                      key = 0;
                      break;
                  }
                  break;
                default:
                  debug(DEBUG_TRACE, "SDL", "unmapped sym 0x%08X", ev.key.keysym.sym);
                  key = 0;
                  break;
              }
            }
            if (key) {
              *ekey = key;
              *mods = window->mods;
              r = WINDOW_KEY;
            }
            if (shift) {
              window->mods &= ~WINDOW_MOD_SHIFT;
            }
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        switch (ev.button.button) {
          case 1:
            buttons |= SDL_BUTTON1;
            break;
          case 3:
            buttons |= SDL_BUTTON2;
            break;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        switch (ev.button.button) {
          case 1:
            buttons &= ~SDL_BUTTON1;
            break;
          case 3:
            buttons &= ~SDL_BUTTON2;
            break;
        }
        break;

      case SDL_MOUSEMOTION:
        window->x = ev.motion.x;
        window->y = ev.motion.y;
        *ekey = window->x;
        *mods = window->y;
        *ebuttons = buttons;
        r = WINDOW_MOTION;
        break;

    case SDL_WINDOWEVENT:
        switch (ev.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            r = -1;
            break;
          default:
            break;
        }
        break;

      case SDL_QUIT:
        r = -1;
        break;

      default:
        if (wait == 0 && remove == 0) {
          SDL_PollEvent(&ev);
          continue;
        }
      }

      if (buttons != window->buttons) {
        window->buttons = buttons;
        *ebuttons = buttons;
        r = WINDOW_BUTTON;
      }
    }

    if (wait < 0) {
      if (r) break;
    } else {
      break;
    }
  }

  return r;
}

static int libsdl_window_show_cursor(window_t *window, int show) {
  SDL_SetRelativeMouseMode(show ? SDL_FALSE : SDL_TRUE);
  return 0;
}

/*
  SDL_CreateWindow():
  a) Fullscreen window at the current desktop resolution: flags=SDL_WINDOW_FULLSCREEN_DESKTOP, ignores width and height parameters.
     Optional: to map a logical resolution into the real resolution:
     SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
     SDL_RenderSetLogicalSize(window->renderer, width, height);
  b) Fullscreen window at specified resolution: flags=SDL_WINDOW_FULLSCREEN, uses width and height parameters.
  c) Windowed mode at specified resolution: flags=0, uses width and height parameters.
*/

static int libsdl_video_setup(libsdl_window_t *window) {
  SDL_RendererInfo info;
  uint16_t index;
  uint32_t n, i, c, w, h;

  w = window->width;
  h = window->height;

  if (window->fullscreen) {
    c = 0;
  } else {
    c = SDL_WINDOWPOS_CENTERED;
    w *= window->xfactor;
    h *= window->yfactor;
  }

  i = -1;

  if (window->driver[0]) {
    if ((n = SDL_GetNumRenderDrivers()) != -1) {
      for (i = 0; i < n; i++) {
        if (SDL_GetRenderDriverInfo(i, &info) == 0) {
          if (!sys_strcmp(info.name, window->driver)) {
            debug(DEBUG_INFO, "SDL", "requested driver \"%s\" found", window->driver);
            break;
          }
        }
      }
      if (i == n) {
        debug(DEBUG_ERROR, "SDL", "requested driver \"%s\" not found, using default", window->driver);
        i = -1;
      }
    }
  }

  debug(DEBUG_INFO, "SDL", "creating window (%d x %d) fullscreen %d", w, h, window->fullscreen);
  window->window = SDL_CreateWindow("", c, c, w, h, window->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
  if (window->window == NULL) {
    debug(DEBUG_ERROR, "SDL", "SDL_CreateWindow failed: %s", SDL_GetError());
    return -1;
  }

SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

  debug(DEBUG_INFO, "SDL", "creating renderer");
  // index of the rendering driver to initialize, or -1 to initialize the first one supporting the requested flags.
  //window->renderer = SDL_CreateRenderer(window->window, i, window->software ? SDL_RENDERER_SOFTWARE : 0);
  window->renderer = SDL_CreateRenderer(window->window, i, window->software ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if (window->renderer == NULL) {
    SDL_DestroyWindow(window->window);
    window->window = NULL;
    return -1;
  }

  if (SDL_GetRendererInfo(window->renderer, &info) != 0) {
    debug(DEBUG_ERROR, "SDL", "SDL_GetRendererInfo failed: %s", SDL_GetError());
    SDL_DestroyRenderer(window->renderer);
    window->renderer = NULL;
    SDL_DestroyWindow(window->window);
    window->window = NULL;
    return -1;
  }
  libsdl_render_info("using", &info);

  if (window->fullscreen) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(window->renderer, w, h);
  } else {
    debug(DEBUG_INFO, "SDL", "set window border");
    SDL_SetWindowBordered(window->window, SDL_TRUE);
    if (window->xfactor > 1 || window->yfactor > 1) {
      SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
      SDL_RenderSetLogicalSize(window->renderer, w, h);
    }
  }

  SDL_SetRenderDrawBlendMode(window->renderer, SDL_BLENDMODE_BLEND);

  SDL_RenderClear(window->renderer);
  SDL_RenderPresent(window->renderer);

  for (i = 0; keymap[i].from; i++) {
    index = (keymap[i].mods << 8) | keymap[i].from;
    window->keymap[index] = keymap[i];
  }

  return 0;
}

static int libsdl_video_close(libsdl_window_t *window) {
  if (window->background) SDL_DestroyTexture(window->background);
  if (window->renderer) SDL_DestroyRenderer(window->renderer);
  if (window->window) SDL_DestroyWindow(window->window);
  xfree(window);

  return 0;
}

static window_t *libsdl_window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate,
     int fullscreen, int software, char *driver, void *data) {

  libsdl_window_t *window;
  SDL_Rect rect;
  uint32_t format, spixel;

  switch (encoding) {
/*
    case ENC_I420:
      format = SDL_PIXELFORMAT_IYUV;
      spixel = sizeof(uint16_t); // XXX ?
      break;
*/
    case ENC_UYVY:
      format = SDL_PIXELFORMAT_UYVY;
      spixel = sizeof(uint16_t);
      break;
    case ENC_YUYV:
      format = SDL_PIXELFORMAT_YUY2;
      spixel = sizeof(uint16_t);
      break;
    case ENC_RGB565:
      format = SDL_PIXELFORMAT_RGB565;
      spixel = sizeof(uint16_t);
      break;
    case ENC_RGB:
      format = SDL_PIXELFORMAT_BGR24;
      spixel = 3*sizeof(uint8_t);
      break;
    case ENC_RGBA:
      format = SDL_PIXELFORMAT_ARGB8888;
      spixel = sizeof(uint32_t);
      break;
    default:
      debug(DEBUG_ERROR, "SDL", "invalid encoding %s", video_encoding_name(encoding));
      return NULL;
  }

  if (*width == 0 || *height == 0) {
    if (fullscreen) {
      SDL_GetDisplayBounds(0, &rect);
    } else {
      SDL_GetDisplayUsableBounds(0, &rect);
    }
    *width = rect.w;
    *height = rect.h;
    debug(DEBUG_INFO, "SDL", "using whole display %dx%d", *width, *height);
  }

  if ((window = xcalloc(1, sizeof(libsdl_window_t))) != NULL) {
    window->width = *width;
    window->height = *height;
    window->xfactor = xfactor;
    window->yfactor = yfactor;
    window->fullscreen = fullscreen;
    window->software = software;
    window->format = format;
    window->spixel = spixel;
    window->rotate = rotate;
    sys_strncpy(window->driver, driver, MAX_DRIVER);

    if (libsdl_video_setup(window) == 0) {
      debug(DEBUG_INFO, "SDL", "window created");

    } else {
      xfree(window);
      window = NULL;
    }
  }

  return (window_t *)window;
}

#ifdef SDL_OPENGL
static void presentBackBuffer(libsdl_window_t *window, texture_t *t, GLuint programId) {
  GLint oldProgramId, numUniforms, index, size, loc;
 	GLenum type;
 	GLsizei length;
  GLfloat minx, miny, maxx, maxy;
  GLfloat minu, maxu, minv, maxv;
  GLfloat value;
  char var[256];
  PFNGLUSEPROGRAMPROC glUseProgram;

  glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");

  SDL_SetRenderTarget(window->renderer, NULL);
  SDL_RenderClear(window->renderer);

  if (programId != 0) {
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
    glUseProgram(programId);
  }

  if (window->getvar) {
    glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (index = 0; index < numUniforms; index++) {
      glGetActiveUniform(programId, index, sizeof(var), &length, &size, &type, var);
      if (type == GL_FLOAT && sys_strcmp(var, "u_texture") != 0) {
        value = window->getvar(var, window->data);
        debug(DEBUG_TRACE, "SDL", "shader var \"%s\" = %f", var, value);
        glUniform1f(index, value);
      }
    }
  }

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  SDL_GL_BindTexture(t->t, NULL, NULL);
  loc = glGetUniformLocation(programId, "u_texture");
  glUniform1i(loc, 0);

  minx = -1.0f;
  maxx = 1.0f;
  miny = 1.0f;
  maxy = -1.0f;

  minu = 0.0f;
  maxu = 1.0f;
  minv = 0.0f;
  maxv = 1.0f;

  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(minu, minv);
  glVertex2f(minx, miny);
  glTexCoord2f(maxu, minv);
  glVertex2f(maxx, miny);
  glTexCoord2f(minu, maxv);
  glVertex2f(minx, maxy);
  glTexCoord2f(maxu, maxv);
  glVertex2f(maxx, maxy);
  glEnd();

  SDL_GL_SwapWindow(window->window);

  if (programId != 0) {
    glUseProgram(oldProgramId);
  }
}
#endif

static int libsdl_window_render(window_t *_window) {
  libsdl_window_t *window = (libsdl_window_t *)_window;
  int r = -1;

  if (window) {
#ifdef SDL_OPENGL
    if (window->opengl && window->texture) {
      presentBackBuffer(window, window->texture, window->programId[0]);
    } else {
#endif
      SDL_RenderPresent(window->renderer);
#ifdef SDL_OPENGL
    }
#endif
    r = 0;
  }

  return r;
}

static int libsdl_window_erase(window_t *_window, uint32_t bg) {
  libsdl_window_t *window;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window) {
    SDL_RenderClear(window->renderer);

    if (window->background) {
      SDL_RenderCopy(window->renderer, window->background, NULL, NULL);
    }

    r = 0;
  }

  return r;
}

static texture_t *libsdl_window_create_texture(window_t *_window, int width, int height) {
  libsdl_window_t *window;
  texture_t *texture = NULL;

  window = (libsdl_window_t *)_window;

  if (window) {
    if ((texture = xcalloc(1, sizeof(texture_t))) != NULL) {
      if ((texture->t = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_STREAMING, width, height)) != NULL) {
        SDL_SetTextureBlendMode(texture->t, SDL_BLENDMODE_BLEND);
        texture->width = width;
        texture->height = height;
        texture->blur = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture->blur, SDL_BLENDMODE_BLEND);
        texture->buf = xcalloc(1, width * height * 4);
      } else {
        debug(DEBUG_ERROR, "SDL", "SDL_CreateTexture format 0x%08X failed: %s", window->format, SDL_GetError());
        xfree(texture);
        texture = NULL;
      }
    }
  }

  return texture;
}

static int libsdl_window_destroy_texture(window_t *_window, texture_t *texture) {
  libsdl_window_t *window;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && texture) {
    if (texture->t) SDL_DestroyTexture(texture->t);
    if (texture->blur) SDL_DestroyTexture(texture->blur);
    if (texture->buf) xfree(texture->buf);
    xfree(texture);
  }

  return r;
}

static int libsdl_window_background(window_t *_window, uint32_t *raw, int width, int height) {
  libsdl_window_t *window;
  SDL_Rect rect;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && raw) {
    if ((window->background = SDL_CreateTexture(window->renderer, window->format, SDL_TEXTUREACCESS_STREAMING, width, height)) != NULL) {
      rect.x = 0;
      rect.y = 0;
      rect.w = width;
      rect.h = height;
      SDL_UpdateTexture(window->background, &rect, (uint8_t *)raw, width * window->spixel);
      r = 0;
    }
  }

  return r;
}

static int libsdl_window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  libsdl_window_t *window;
  SDL_Rect rect;
  void *pixels;
  uint8_t *dst;
  int pitch, spitch, len, i, r = -1;

  window = (libsdl_window_t *)_window;

  rect.x = tx;
  rect.y = ty;
  rect.w = w;
  rect.h = h;

  if (SDL_LockTexture(texture->t, &rect, &pixels, &pitch) == 0) {
    src = &src[(ty * texture->width + tx) * window->spixel];
    dst = (uint8_t *)pixels;
    len = w * window->spixel;
    spitch = texture->width * window->spixel;
    for (i = 0; i < rect.h; i++) {
      xmemcpy(dst, src, len);
      src += spitch;
      dst += pitch;
    }
    r = 0;
    SDL_UnlockTexture(texture->t);
  }

  return r;
}

static int libsdl_window_update_texture(window_t *_window, texture_t *texture, uint8_t *src) {
  return libsdl_window_update_texture_rect(_window, texture, src, 0, 0, texture->width, texture->height);
}

static int libsdl_window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  libsdl_window_t *window;
  SDL_Rect src, dst;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && texture && w > 0 && h > 0) {
#ifdef SDL_OPENGL
    if (window->opengl) {
      window->texture = texture;
      return 0;
    }
#endif

    src.x = tx;
    src.y = ty;
    src.w = w;
    src.h = h;

    dst.x = x * window->xfactor;
    dst.y = y * window->yfactor;
    dst.w = w * window->xfactor;
    dst.h = h * window->yfactor;

    //r = SDL_RenderCopyEx(window->renderer, texture->t, &src, &dst, 0, NULL, SDL_FLIP_NONE);
    r = SDL_RenderCopy(window->renderer, texture->t, &src, &dst);

    if (r != 0)  {
      debug(DEBUG_ERROR, "SDL", "SDL_RenderCopy failed");
      r = -1;
    }
  }

  return r;
}

static int libsdl_window_draw_texture(window_t *_window, texture_t *texture, int x, int y) {
  libsdl_window_t *window;
  SDL_Rect dst;
  int r = -1;

  window = (libsdl_window_t *)_window;

  if (window && texture) {
    dst.x = x * window->xfactor;
    dst.y = y * window->yfactor;
    dst.w = texture->width  * window->xfactor;
    dst.h = texture->height * window->yfactor;

    //r = SDL_RenderCopyEx(window->renderer, texture->t, NULL, &dst, 90, &center, SDL_FLIP_NONE);
    r = SDL_RenderCopy(window->renderer, texture->t, NULL, &dst);

    if (r != 0)  {
      debug(DEBUG_ERROR, "SDL", "SDL_RenderCopy failed");
      r = -1;
    }
  }

  return r;
}

static int libsdl_mixer_init(void) {
  int r = -1;
#ifdef SDL_MIXER
  int i, n;
  char *s;

  if (Mix_Init(MIX_INIT_MID) == MIX_INIT_MID) {
    n = Mix_GetNumChunkDecoders();
    for (i = 0; i < n; i++) {
      s = (char *)Mix_GetChunkDecoder(i);
      if (s) debug(DEBUG_INFO, "SDL", "mixer chunk decoder %s", s);
    }
    n = Mix_GetNumMusicDecoders();
    for (i = 0; i < n; i++) {
      s = (char *)Mix_GetMusicDecoder(i);
      if (s) debug(DEBUG_INFO, "SDL", "mixer music decoder %s", s);
    }
    r = 0;
  }
  debug(DEBUG_INFO, "SDL", "mixer init %d", r);
#endif

  return r;
}

static int libsdl_mixer_play(uint8_t *buf, uint32_t len, int volume) {
#ifdef SDL_MIXER
  SDL_RWops *rwops;
  Mix_Music *music;

  debug(DEBUG_INFO, "SDL", "mixer play begin");
  if ((rwops = SDL_RWFromMem(buf, len)) != NULL) {
    debug(DEBUG_INFO, "SDL", "mixer play rwops done");
    if ((music = Mix_LoadMUSType_RW(rwops, MUS_MID, 0)) != NULL) {
      debug(DEBUG_INFO, "SDL", "mixer play load music done");
      if (volume >= 0) {
        Mix_VolumeMusic(volume); // 0-128
      }
      Mix_PlayMusic(music, 0);
      debug(DEBUG_INFO, "SDL", "mixer play play music done");
      for (; Mix_PlayingMusic() && !thread_must_end();) {
        sys_usleep(1000);
      }
      debug(DEBUG_INFO, "SDL", "mixer play loop done");
      Mix_FreeMusic(music);
      debug(DEBUG_INFO, "SDL", "mixer free music done");
    }
    SDL_RWclose(rwops);
  }
#endif

  return 0;
}

static int libsdl_mixer_stop(void) {
#ifdef SDL_MIXER
  debug(DEBUG_INFO, "SDL", "mixer stop");
  Mix_HaltMusic();
#endif
  return 0;
}

void libsdl_midi_finish(void) {
#ifdef SDL_MIXER
  Mix_Quit();
#endif
}

static void libsdl_window_status(window_t *window, int *x, int *y, int *buttons) {
  libsdl_status((libsdl_window_t *)window, x, y, buttons);
}

static void libsdl_window_title(window_t *window, char *title) {
  libsdl_title((libsdl_window_t *)window, title);
}

static char *libsdl_window_clipboard(window_t *window, char *clipboard, int len) {
  return libsdl_clipboard((libsdl_window_t *)window, clipboard, len);
}

static int libsdl_window_event(window_t *window, int wait, int remove, int *key, int *mods, int *buttons) {
  return libsdl_event((libsdl_window_t *)window, wait, remove, key, mods, buttons);
}

static int libsdl_window_event2(window_t *window, int wait, int *arg1, int *arg2) {
  return libsdl_event2((libsdl_window_t *)window, wait, arg1, arg2);
}

static int libsdl_window_update(window_t *_window, int x, int y, int width, int height) {
  return 0;
}

#ifdef SDL_OPENGL
static GLuint compileShader(const char *source, GLint len, GLuint shaderType) {
  GLuint result;
  GLint shaderCompiled, logLength;
  GLchar *log;
  char *type;
  PFNGLCREATESHADERPROC glCreateShader;
  PFNGLSHADERSOURCEPROC glShaderSource;
  PFNGLCOMPILESHADERPROC glCompileShader;
  PFNGLGETSHADERIVPROC glGetShaderiv;
  PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
  PFNGLDELETESHADERPROC glDeleteShader;

  glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
  glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
  result = 0;

  if (glCreateShader && glShaderSource && glCompileShader && glGetShaderiv && glGetShaderInfoLog && glDeleteShader) {
    type = shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment";
    result = glCreateShader(shaderType);
    glShaderSource(result, 1, &source, &len);
    glCompileShader(result);

    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      log = (GLchar *)sys_malloc(logLength);
      glGetShaderInfoLog(result, logLength, &logLength, log);
      debug(DEBUG_INFO, "SDL", "%s shader compilation log: %s", type, log);
      sys_free(log);
    }

    shaderCompiled = GL_FALSE;
    glGetShaderiv(result, GL_COMPILE_STATUS, &shaderCompiled);
    if (!shaderCompiled) {
      debug(DEBUG_ERROR, "SDL", "error compiling %s shader: %d", type, result);
      glDeleteShader(result);
      result = 0;
    } else {
      debug(DEBUG_INFO, "SDL", "%s shader compiled successfully", type);
    }
  } else {
    debug(DEBUG_ERROR, "SDL", "compileShader: one or more OpenGL functions are not available");
  }

  return result;
}

static int libsdl_window_shader(window_t *_window, int i, char *vertex, int vlen, char *fragment, int flen, float (*getvar)(char *name, void *data), void *data) {
  libsdl_window_t *window = (libsdl_window_t *)_window;
  GLint logLen, linkStatus, validateStatus;
  GLuint vtxShaderId, fragShaderId;
  char *log;
  int r = -1;
  PFNGLCREATEPROGRAMPROC glCreateProgram;
  PFNGLLINKPROGRAMPROC glLinkProgram;
  PFNGLVALIDATEPROGRAMPROC glValidateProgram;
  PFNGLGETPROGRAMIVPROC glGetProgramiv;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
  PFNGLATTACHSHADERPROC glAttachShader;
  PFNGLDELETESHADERPROC glDeleteShader;

  glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
  glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
  glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
  glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");

  if (glCreateProgram && glLinkProgram && glValidateProgram && glGetProgramiv && glGetProgramInfoLog && glAttachShader && glDeleteShader) {
    window->programId[i] = glCreateProgram();
    vtxShaderId = compileShader(vertex, vlen, GL_VERTEX_SHADER);
    fragShaderId = compileShader(fragment, flen, GL_FRAGMENT_SHADER);

    if (vtxShaderId && fragShaderId) {
      glAttachShader(window->programId[i], vtxShaderId);
      glAttachShader(window->programId[i], fragShaderId);
      glLinkProgram(window->programId[i]);
      glGetProgramiv(window->programId[i], GL_LINK_STATUS, &linkStatus);
      glValidateProgram(window->programId[i]);
      glGetProgramiv(window->programId[i], GL_VALIDATE_STATUS, &validateStatus);
      window->getvar = getvar;
      window->data = data;
      debug(DEBUG_INFO, "SDL", "program link status %d, validate status %d", linkStatus, validateStatus);
      window->opengl = linkStatus && validateStatus;
      r = window->opengl ? 0 : -1;

      glGetProgramiv(window->programId[i], GL_INFO_LOG_LENGTH, &logLen);
      if (logLen > 0) {
        log = (char *)sys_malloc(logLen * sizeof(char));
        glGetProgramInfoLog(window->programId[i], logLen, &logLen, log);
        debug(DEBUG_INFO, "SDL", "shader program log: %s", log);
        sys_free(log);
      }
    }

    if (vtxShaderId) {
      glDeleteShader(vtxShaderId);
    }

    if (fragShaderId) {
      glDeleteShader(fragShaderId);
    }
  } else {
    debug(DEBUG_ERROR, "SDL", "libsdl_window_shader: one or more OpenGL functions are not available");
  }

  return r;
}
#endif

static int libsdl_window_destroy(window_t *window) {
  return libsdl_video_close((libsdl_window_t *)window);
}

// fake "average" function just for testing the UI
static int libsdl_window_average(window_t *_window, int *x, int *y, int ms) {
  libsdl_window_t *window = (libsdl_window_t *)_window;
  int arg1, arg2;

  for (;;) {
    if (thread_must_end()) return -1;

    switch (libsdl_event2(window, 1, &arg1, &arg2)) {
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

static int libsdl_calib(int pe) {
  window_provider.average = libsdl_window_average;
  return 0;
}

static int set_video_driver(char *video_driver) {
#ifdef SDL_HINT_VIDEODRIVER
  if (video_driver) debug(DEBUG_INFO, "SDL", "hint video driver \"%s\"", video_driver);
  SDL_SetHint(SDL_HINT_VIDEODRIVER, video_driver);
#else
  if (video_driver) debug(DEBUG_INFO, "SDL", "setenv video driver \"%s\"", video_driver);
  SDL_setenv("SDL_VIDEODRIVER", video_driver, 1);
#endif
  return 1;
}

int liblsdl2_load(void) {
  SDL_version version;
  int set = 0;

  SDL_VERSION(&version);
  debug(DEBUG_INFO, "SDL", "version %d.%d.%d", version.major, version.minor, version.patch);

  if (!SDL_getenv("SDL_VIDEODRIVER") && SDL_getenv("WAYLAND_DISPLAY") != NULL) {
    set = set_video_driver("wayland");
  }

  libsdl_init_audio();

  if (libsdl_init_video() != 0) {
    SDL_Quit();
    return -1;
  }

  if (set) {
    set_video_driver(NULL);
  }

  xmemset(&window_provider, 0, sizeof(window_provider));
  window_provider.create = libsdl_window_create;
  window_provider.event = libsdl_window_event;
  window_provider.destroy = libsdl_window_destroy;
  window_provider.erase = libsdl_window_erase;
  window_provider.render = libsdl_window_render;
  window_provider.background = libsdl_window_background;
  window_provider.create_texture = libsdl_window_create_texture;
  window_provider.destroy_texture = libsdl_window_destroy_texture;
  window_provider.update_texture = libsdl_window_update_texture;
  window_provider.draw_texture = libsdl_window_draw_texture;
  window_provider.status = libsdl_window_status;
  window_provider.title = libsdl_window_title;
  window_provider.clipboard = libsdl_window_clipboard;
  window_provider.event2 = libsdl_window_event2;
  window_provider.update = libsdl_window_update;
  window_provider.draw_texture_rect = libsdl_window_draw_texture_rect;
  window_provider.update_texture_rect = libsdl_window_update_texture_rect;
  window_provider.show_cursor = libsdl_window_show_cursor;
#ifdef SDL_OPENGL
  window_provider.shader = libsdl_window_shader;
#endif

  xmemset(&audio_provider, 0, sizeof(audio_provider));
  audio_provider.mixer_init = libsdl_mixer_init;
  audio_provider.mixer_play = libsdl_mixer_play;
  audio_provider.mixer_stop = libsdl_mixer_stop;

  return 0;
}

int liblsdl2_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "SDL", "registering provider %s", WINDOW_PROVIDER);
  script_set_pointer(pe, WINDOW_PROVIDER, &window_provider);

  debug(DEBUG_INFO, "SDL", "registering provider %s", AUDIO_PROVIDER);
  script_set_pointer(pe, AUDIO_PROVIDER, &audio_provider);

  script_add_iconst(pe, obj, "motion", WINDOW_MOTION);
  script_add_iconst(pe, obj, "down", WINDOW_BUTTONDOWN);
  script_add_iconst(pe, obj, "up", WINDOW_BUTTONUP);

  script_add_function(pe, obj, "calib", libsdl_calib);

  return 0;
}

int liblsdl2_unload(void) {
#ifdef SDL_MIXER
  Mix_CloseAudio();
#endif
  SDL_Quit();
  return 0;
}
