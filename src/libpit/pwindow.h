#ifndef PIT_WINDOW_H
#define PIT_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#define WINDOW_PROVIDER "window_provider"

#define WINDOW_KEY         1
#define WINDOW_BUTTON      2
#define WINDOW_MOTION      3
#define WINDOW_KEYDOWN     4
#define WINDOW_KEYUP       5
#define WINDOW_BUTTONDOWN  6
#define WINDOW_BUTTONUP    7
#define WINDOW_WHEEL       8
#define WINDOW_CUSTOM      0xFF

#define WINDOW_MOD_SHIFT      0x01
#define WINDOW_MOD_CTRL       0x02
#define WINDOW_MOD_LALT       0x04
#define WINDOW_MOD_RALT       0x08
#define WINDOW_MOD_RCTRL      0x10

#define WINDOW_KEY_UP         0x81
#define WINDOW_KEY_DOWN       0x82
#define WINDOW_KEY_LEFT       0x83
#define WINDOW_KEY_RIGHT      0x84
#define WINDOW_KEY_PGUP       0x85
#define WINDOW_KEY_PGDOWN     0x86
#define WINDOW_KEY_HOME       0x87
#define WINDOW_KEY_END        0x88
#define WINDOW_KEY_INS        0x89

#define WINDOW_KEY_F1         0x91
#define WINDOW_KEY_F2         0x92
#define WINDOW_KEY_F3         0x93
#define WINDOW_KEY_F4         0x94
#define WINDOW_KEY_F5         0x95
#define WINDOW_KEY_F6         0x96
#define WINDOW_KEY_F7         0x97
#define WINDOW_KEY_F8         0x98
#define WINDOW_KEY_F9         0x99
#define WINDOW_KEY_F10        0x9A
#define WINDOW_KEY_F11        0x9B
#define WINDOW_KEY_F12        0x9C

#define WINDOW_KEY_SHIFT      0xA1
#define WINDOW_KEY_CTRL       0xA2
#define WINDOW_KEY_LALT       0xA3
#define WINDOW_KEY_RALT       0xA4
#define WINDOW_KEY_RCTRL      0xA5

#define WINDOW_KEY_CUSTOM     0x100

typedef void *window_t;
typedef struct texture_t texture_t;

typedef struct {
  window_t *(*create)(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, void *data);
  int (*event)(window_t *window, int wait, int remove, int *key, int *mods, int *buttons);
  int (*destroy)(window_t *window);
  int (*erase)(window_t *window, uint32_t bg);
  int (*render)(window_t *window);
  int (*background)(window_t *window, uint32_t *raw, int width, int height);
  texture_t *(*create_texture)(window_t *window, int width, int height);
  int (*destroy_texture)(window_t *window, texture_t *texture);
  int (*update_texture)(window_t *window, texture_t *texture, uint8_t *raw);
  int (*draw_texture)(window_t *window, texture_t *texture, int x, int y);
  void (*status)(window_t *window, int *x, int *y, int *buttons);
  void (*title)(window_t *window, char *title);
  char *(*clipboard)(window_t *window, char *clipboard, int len);
  int (*event2)(window_t *window, int wait, int *arg1, int *arg2);
  int (*update)(window_t *window, int x, int y, int width, int height);
  int (*draw_texture_rect)(window_t *window, texture_t *texture, int tx, int ty, int w, int h, int x, int y);
  int (*update_texture_rect)(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h);
  int (*move)(window_t *window, int x, int y, int w, int h, int dx, int dy);
  int (*average)(window_t *window, int *x, int *y, int ms);
  void *data;
} window_provider_t;

#ifdef __cplusplus
}
#endif

#endif
