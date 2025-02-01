#ifndef COMPUTER_H
#define COMPUTER_H

#include "pfont.h"
#include "graphic.h"
#include "surface.h"

typedef struct computer_t {
  char *tag;

  int (*set_window)(struct computer_t *c, window_provider_t *wp, int fullscreen);
  int (*set_surface)(struct computer_t *c, int ptr, surface_t *surface);
  int (*set_color)(struct computer_t *c, int index, uint8_t red, uint8_t green, uint8_t blue);
  int (*set_display)(struct computer_t *c, int display);
  int (*set_filter)(struct computer_t *c, conn_filter_t *filter);

  int (*disk)(struct computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name);
  int (*rom)(struct computer_t *c, int num, uint32_t size, char *name);
  int (*option)(struct computer_t *c, char *name, char *value);
  int (*run)(struct computer_t *c, uint32_t us);
  int (*delay)(struct computer_t *c);
  int (*close)(struct computer_t *c);

  uint8_t (*getb)(struct computer_t *c, uint16_t addr);
  uint8_t (*getop)(struct computer_t *c, uint16_t addr);
  void (*putb)(struct computer_t *c, uint16_t addr, uint8_t b);

  uint8_t (*getb32)(struct computer_t *c, uint32_t addr);
  uint8_t (*getop32)(struct computer_t *c, uint32_t addr);
  void (*putb32)(struct computer_t *c, uint32_t addr, uint8_t b);

  void (*out)(struct computer_t *c, uint16_t port, uint16_t b);
  uint16_t (*in)(struct computer_t *c, uint16_t port);

  void *data;
  void *userdata;
} computer_t;

#endif
