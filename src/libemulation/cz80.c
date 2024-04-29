#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "filter.h"
#include "ctelnet.h"
#include "modem.h"
#include "computer.h"
#include "disk.h"
#include "z80.h"
#include "cz80.h"
#include "pterm.h"
#include "debug.h"
#include "xalloc.h"

#define Z80_CLOCK     4000000
#define Z80_FPS       60
#define Z80_PERIOD    Z80_CLOCK/Z80_FPS
#define HOST_PERIOD   1000000/Z80_FPS

#define MEM_SIZE      64*1024

#define DEFAULT_SCREEN_WIDTH  640
#define DEFAULT_SCREEN_HEIGHT 200
#define DEFAULT_CHAR_WIDTH  8
#define DEFAULT_CHAR_HEIGHT 8

#define CHR_SIZE    256*DEFAULT_CHAR_HEIGHT

#define NUM_SPRITES   16
#define SPRITE_ACTIVE 0x80
#define SPRITE_DOUBLE 0x40

#define ADDR_PORT  0xFFFF
#define ADDR_DATA  0xFFFE

#define MAX_KBUF    256

typedef struct {
  z80_t *z;
  uint8_t ram[MEM_SIZE];
  disk_t *d;

  uint64_t key_t;
  uint8_t key;
  int64_t t0;
  int frame;
  int finish;
  int boot;

  uint64_t cs0;
  uint64_t dcs;

  vfs_session_t *session;
  surface_t *surface;
  int ptr;
  int font;
  int screen_width;
  int screen_height;
  int char_width;
  int char_height;
  uint32_t c[16];
  uint32_t x, y, x1, y1, scy1, scy2;
  int sprite_s, sprite_double, sprite_id, sprite_row, sprite_color;
  texture_t *sprite[NUM_SPRITES];
  uint32_t sprite_x[NUM_SPRITES], sprite_y[NUM_SPRITES];
  uint32_t sprite_src[DEFAULT_CHAR_WIDTH * DEFAULT_CHAR_HEIGHT];
  uint8_t sprite_flags[NUM_SPRITES];
  int custom_s, custom_code, custom_row;
  int cursor;
  int dirty;

  uint8_t kbuffer[MAX_KBUF];
  int kbuflen, pkbuf, gkbuf;

  pterm_callback_t cb;
  pterm_t *t;
  conn_filter_t *tcp;
  conn_filter_t *telnet;
  conn_filter_t *modem;
} cz80_data_t;

static uint8_t colors[] = {
   0x00, 0x00, 0x00,
   0xC0, 0x00, 0x00,
   0x00, 0xC0, 0x00,
   0xC0, 0xC0, 0x00,
   0x00, 0x00, 0xC0,
   0xC0, 0x00, 0xC0,
   0x00, 0xC0, 0xC0,
   0xC0, 0xC0, 0xC0,
   0x00, 0x00, 0x00,
   0xFF, 0x00, 0x00,
   0x00, 0xFF, 0x00,
   0xFF, 0xFF, 0x00,
   0x00, 0x00, 0xFF,
   0xFF, 0x00, 0xFF,
   0x00, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF
};

static uint8_t cz80_getb(computer_t *c, uint16_t addr) {
  cz80_data_t *cz80;

  cz80 = (cz80_data_t *)c->data;
  if (addr >= MEM_SIZE) return 0xFF;
  return cz80->ram[addr];
}

static void cz80_putb(computer_t *c, uint16_t addr, uint8_t b) {
  cz80_data_t *cz80;

  cz80 = (cz80_data_t *)c->data;
  if (addr < MEM_SIZE) {
    cz80->ram[addr] = b;
  }
}

static surface_t *lock_surface(cz80_data_t *cz80) {
  if (cz80->surface) return cz80->surface;
  if (cz80->ptr > 0) return ptr_lock(cz80->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(cz80_data_t *cz80) {
  if (cz80->ptr > 0) ptr_unlock(cz80->ptr, TAG_SURFACE);
}

static int term_draw(uint8_t col, uint8_t row, uint8_t code, uint32_t fg, uint32_t bg, uint8_t attr, void *data) {
  cz80_data_t *cz80 = (cz80_data_t *)data;
  surface_t *surface = lock_surface(cz80);
  uint32_t x, y;
  char s[2];

  x = col * cz80->char_width;
  y = row * cz80->char_height;
  if (attr & ATTR_BRIGHT) {
    fg += 8;
  }

  fg = cz80->c[fg];
  bg = cz80->c[bg];

  if (code == 0) code = ' ';
  s[0] = code;
  s[1] = 0;
  surface_print(surface, x, y, s, NULL, cz80->font, fg, bg);
  unlock_surface(cz80);
  cz80->dirty = 1;

  return 0;
}

static int term_erase(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, uint32_t bg, uint8_t attr, void *data) {
  cz80_data_t *cz80 = (cz80_data_t *)data;
  surface_t *surface = lock_surface(cz80);

  if (surface->encoding != SURFACE_ENCODING_PALETTE) {
    bg = cz80->c[bg];
  }
  surface_rectangle(surface, col1*cz80->char_width, row1*cz80->char_height, col2*cz80->char_width, row2*cz80->char_height, 1, bg);
  unlock_surface(cz80);
  cz80->dirty = 1;

  return 0;
}

static void sock_close(cz80_data_t *cz80) {
  modem_set(cz80->modem, NULL);

  if (cz80->telnet) {
    telnet_client_close(cz80->telnet);
    cz80->telnet = NULL;
  }

  if (cz80->tcp) {
    conn_close(cz80->tcp);
    cz80->tcp = NULL;
  }
}

static void serial_out(cz80_data_t *cz80, uint16_t port, uint8_t b) {
  switch (port) {
    case 0: // port A data
      //debug(1, "XXX", "term:%c modem:%c telnet:%c term: 0x%02X '%c'", pterm_getstate(cz80->t), modem_getstate(cz80->modem), telnet_client_state(cz80->telnet), b, b);
      pterm_send(cz80->t, &b, 1);
      break;
    case 1: // port A ctrl
      break;
    case 2: // port B data
      //debug(1, "XXX", "term:%c modem:%c telnet:%c serout: 0x%02X '%c'", pterm_getstate(cz80->t), modem_getstate(cz80->modem), telnet_client_state(cz80->telnet), b, b);
      if (cz80->modem->write(cz80->modem, &b, 1) != 1) {
        sock_close(cz80);
      }
      break;
    case 3: // port B ctrl
      break;
  }
}

static void clock_out(cz80_data_t *cz80, uint16_t port, uint8_t b) {
  switch (port) {
    case 0:
      cz80->dcs = (cz80->dcs & 0xFFFFFF00u) | b;
      break;
    case 1:
      cz80->dcs = (cz80->dcs & 0xFFFF00FFu) | (b << 8);
      break;
    case 2:
      cz80->dcs = (cz80->dcs & 0xFF00FFFFu) | (b << 16);
      break;
    case 3:
      cz80->dcs = (cz80->dcs & 0x00FFFFFFu) | (b << 24);
      cz80->cs0 = cz80->dcs;
      break;
  }
}

static void define_custom_char(cz80_data_t *cz80, uint8_t b) {
}

static void define_sprite(cz80_data_t *cz80, uint8_t b) {
}

static void video_out(cz80_data_t *cz80, uint16_t port, uint8_t b) {
  surface_t *surface;
  uint32_t fg, bg;

  switch (port) {
    case 0: // color
      fg = (b >> 4) & 0x07;
      bg = b & 0x07;
      pterm_setfg(cz80->t, fg);
      pterm_setbg(cz80->t, bg);
      break;
    case 1: // scroll
      //scroll(cz80, (int8_t)b);
      break;
    case 2: // x (msb)
      cz80->x = (cz80->x & 0x00FF) | (b << 8);
      if (cz80->x >= cz80->screen_width) cz80->x = cz80->screen_width-1;
      pterm_setx(cz80->t, cz80->x / cz80->char_width);
      break;
    case 3: // x (lsb)
      cz80->x = (cz80->x & 0xFF00) | b;
      if (cz80->x >= cz80->screen_width) cz80->x = cz80->screen_width-1;
      pterm_setx(cz80->t, cz80->x / cz80->char_width);
      break;
    case 4: // y
      cz80->y = b;
      if (cz80->y >= cz80->screen_height) cz80->y = cz80->screen_height-1;
      pterm_sety(cz80->t, cz80->y / cz80->char_height);
      break;
    case 5: // putchar
      term_draw(pterm_getx(cz80->t), pterm_gety(cz80->t), b, pterm_getfg(cz80->t), pterm_getbg(cz80->t), 0, cz80);
      break;
    case 6: // cursor
      pterm_cursor_enable(cz80->t, b ? 1 : 0);
      break;
    case 7: // cmd
      switch (b) {
        case 0: // cls
          pterm_home(cz80->t);
          pterm_cls(cz80->t);
          break;
        case 1: // set (x1,y1)
          cz80->x1 = cz80->x;
          cz80->y1 = cz80->y;
          break;
        case 2: // pixel
          surface = lock_surface(cz80);
          fg = pterm_getfg(cz80->t);
          if (surface->encoding != SURFACE_ENCODING_PALETTE) fg = cz80->c[fg];
          surface->setpixel(surface, cz80->x, cz80->y, fg);
          unlock_surface(cz80);
          cz80->dirty = 1;
          break;
        case 3: // line
          surface = lock_surface(cz80);
          fg = pterm_getfg(cz80->t);
          if (surface->encoding != SURFACE_ENCODING_PALETTE) fg = cz80->c[fg];
          surface_line(surface, cz80->x1, cz80->y1, cz80->x, cz80->y, fg);
          unlock_surface(cz80);
          cz80->dirty = 1;
          break;
        case 4: // rectangle
          surface = lock_surface(cz80);
          fg = pterm_getfg(cz80->t);
          if (surface->encoding != SURFACE_ENCODING_PALETTE) fg = cz80->c[fg];
          surface_rectangle(surface, cz80->x1, cz80->y1, cz80->x, cz80->y, 0, fg);
          unlock_surface(cz80);
          cz80->dirty = 1;
          break;
        case 5: // filled rectangle
          surface = lock_surface(cz80);
          bg = pterm_getbg(cz80->t);
          if (surface->encoding != SURFACE_ENCODING_PALETTE) bg = cz80->c[bg];
          surface_rectangle(surface, cz80->x1, cz80->y1, cz80->x, cz80->y, 1, bg);
          unlock_surface(cz80);
          cz80->dirty = 1;
          break;
        case 6: // ellipse
          // width = x, height = y
          surface = lock_surface(cz80);
          fg = pterm_getfg(cz80->t);
          if (surface->encoding != SURFACE_ENCODING_PALETTE) fg = cz80->c[fg];
          surface_ellipse(surface, cz80->x1, cz80->y1, cz80->x, cz80->y, 0, fg);
          unlock_surface(cz80);
          cz80->dirty = 1;
          break;
        case 7: // filled ellipse
          // width = x, height = y
          surface = lock_surface(cz80);
          bg = pterm_getbg(cz80->t);
          if (surface->encoding != SURFACE_ENCODING_PALETTE) bg = cz80->c[bg];
          surface_ellipse(surface, cz80->x1, cz80->y1, cz80->x, cz80->y, 0, bg);
          unlock_surface(cz80);
          cz80->dirty = 1;
          break;
        case 8: // set scroll region
          cz80->scy1 = cz80->y1;
          cz80->scy2 = cz80->y;
          break;
      }
      break;
    case 8: // define sprite
      define_sprite(cz80, b);
      break;
    case 9: // sprite active / position
      if (cz80->sprite[b & 0x0F]) {
        if (b & 0xF0) {
          cz80->sprite_x[b & 0x0F] = cz80->x;
          cz80->sprite_y[b & 0x0F] = cz80->y;
          cz80->sprite_flags[b & 0x0F] |= SPRITE_ACTIVE;
        } else {
          cz80->sprite_flags[b & 0x0F] &= ~SPRITE_ACTIVE;
        }
        cz80->dirty = 1;
      }
      break;
    case 10: // define custom char
      define_custom_char(cz80, b);
      break;
  }
}

static void cz80_io_out(cz80_data_t *cz80, uint16_t port, uint8_t b) {
  debug(DEBUG_TRACE, "CZ80", "out 0x%04X, 0x%02X", port, b);

  switch (port & 0xF0) {
    case 0x00: // video
      video_out(cz80, port & 0x1F, b);
      break;
    case 0x20: // serial
      serial_out(cz80, port & 0x03, b);
      break;
    case 0x40: // disk
      disk_out(cz80->d, port & 0x1F, b);
      break;
    case 0x60: // clock
      clock_out(cz80, port & 0x03, b);
      break;
    case 0x70:
      switch (port & 0x0F) {
        case 0x0C:
          debug(DEBUG_INFO, "CZ80", "debug '%c'", b);
          break;
        case 0x0D:
          debug(DEBUG_INFO, "CZ80", "debug %d", b);
          break;
        case 0x0E:
          debug(DEBUG_INFO, "CZ80", "debug 0x%02X", b);
          break;
        case 0x0F:
          // hack: out to this port exits emulation
          debug(DEBUG_INFO, "CZ80", "exit hack");
          z80_stop(cz80->z);
          break;
      }
      break;
  }
}

static void render(cz80_data_t *cz80) {
  surface_t *surface = lock_surface(cz80);
  surface_update(surface);
  unlock_surface(cz80);
}

static uint8_t video_in(cz80_data_t *cz80, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case 0:
      b = (pterm_getfg(cz80->t) << 4) | pterm_getbg(cz80->t);
      break;
    case 1:
      surface_t *surface = lock_surface(cz80);
      b = surface->getpixel(surface->data, cz80->x, cz80->y);
      unlock_surface(cz80);
      break;
  }

  return b;
}

static uint8_t serial_in(cz80_data_t *cz80, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case 0: // port A data
      if (cz80->kbuflen) {
        b = cz80->kbuffer[cz80->gkbuf];
        cz80->gkbuf = (cz80->gkbuf + 1) % MAX_KBUF;
        cz80->kbuflen--;
        //debug(1, "XXX", "term:%c modem:%c telnet:%c kbd: 0x%02X '%c'", pterm_getstate(cz80->t), modem_getstate(cz80->modem), telnet_client_state(cz80->telnet), b, b);
      }
      if (cz80->dirty) {
        render(cz80);
        cz80->dirty = 0;
      }
      break;
    case 1: // port A ctrl
      b = cz80->kbuflen ? 0xFF : 0x00;
      break;
    case 2: // port B data
      switch (cz80->modem->peek(cz80->modem, -1)) {
        case 1:
          if (cz80->modem->read(cz80->modem, &b) == 1) {
            //debug(1, "XXX", "term:%c modem:%c telnet:%c serin: 0x%02X '%c'", pterm_getstate(cz80->t), modem_getstate(cz80->modem), telnet_client_state(cz80->telnet), b, b);
            break;
          }
          // fall through
        case 0:
        case -1:
          sock_close(cz80);
          break;
      }
      break;
    case 3: // port B control
      switch (cz80->modem->peek(cz80->modem, 100)) {
        case 1:
          b = 0xFF;
          break;
        case -1:
          sock_close(cz80);
          break;
      }
    break;
  }

  return b;
}

static uint8_t clock_in(cz80_data_t *cz80, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case 0:
      cz80->dcs = sys_get_clock() - cz80->cs0;
      b = cz80->dcs & 0xFF;
      break;
    case 1:
      b = (cz80->dcs >> 8) & 0xFF;
      break;
    case 2:
      b = (cz80->dcs >> 16) & 0xFF;
      break;
    case 3:
      b = (cz80->dcs >> 24) & 0xFF;
      break;
  }

  return b;
}

static uint8_t cz80_io_in(cz80_data_t *cz80, uint16_t port) {
  uint8_t b;

  debug(DEBUG_TRACE, "CZ80", "in 0x%04X", port);

  switch (port & 0xF0) {
    case 0x00: // video
      b = video_in(cz80, port & 0x1F);
      break;
    case 0x20: // serial
      b = serial_in(cz80, port & 0x03);
      break;
    case 0x40: // disk controller
      b = disk_in(cz80->d, port & 0x1F);
      break;
    case 0x60: // clock
      b = clock_in(cz80, port & 0x03);
      break;
    default:
      b = 0;
  }

  return b;
}

static int cz80_option(computer_t *c, char *name, char *value) {
  cz80_data_t *cz80;

  cz80 = (cz80_data_t *)c->data;

  if (!sys_strcmp(name, "font")) {
    cz80->font = sys_atoi(value);
    cz80->char_width = surface_font_width(NULL, cz80->font);
    cz80->char_height = surface_font_height(NULL, cz80->font);
  }

  return 0;
}

static int cz80_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  cz80_data_t *cz80;
  int r = -1;

  cz80 = (cz80_data_t *)c->data;
  if (disk_validate(name, &skip, &tracks, &heads, &sectors, &sectorlen, &sector0, cz80->session) != -1) {
    r = disk_insert(cz80->d, drive, skip, tracks, heads, sectors, sectorlen, sector0, name);
  }

  return r;
}

static int cz80_run(computer_t *c, uint32_t us) {
  cz80_data_t *cz80;

  cz80 = (cz80_data_t *)c->data;

  if (cz80->boot) {
    disk_boot(cz80->d, 0x2000);
    z80_reset(cz80->z, 0x2000);
    cz80->boot = 0;
  }

  return z80_loop(cz80->z, (us * Z80_CLOCK) / 1000000);
}

static int cz80_close(computer_t *c) {
  cz80_data_t *cz80;
  int r = -1;

  cz80 = (cz80_data_t *)c->data;

  if (cz80) {
    disk_close(cz80->d);
    sock_close(cz80);
    xfree(cz80);
    r = 0;
  }
  xfree(c);

  return r;
}

static void cz80_callback(void *data, uint32_t cycles) {
  cz80_data_t *cz80;
  uint64_t dt, edt;
  uint8_t port, value;
  int ev = 0, arg1, arg2;

  cz80 = (cz80_data_t *)data;

  if (cycles) {
    surface_t *surface = lock_surface(cz80);
    ev = surface_event(surface, 0, &arg1, &arg2);
    unlock_surface(cz80);
  }

  switch (ev) {
    case WINDOW_KEYDOWN:
      if (arg1 < 128) {
        if (cz80->kbuflen < MAX_KBUF) {
          cz80->kbuffer[cz80->pkbuf] = arg1;
          cz80->pkbuf = (cz80->pkbuf + 1) % MAX_KBUF;
          cz80->kbuflen++;
        }
      }
      break;
    case WINDOW_KEYUP:
      break;
    case -1:
      z80_stop(cz80->z);
      return;
  }

  if (cycles == 0) {
    // halt
    port = cz80->ram[ADDR_PORT];

    if (port & 0x80) {
      port &= 0x7F;
      value = cz80_io_in(cz80, port);
      cz80->ram[ADDR_DATA] = value;
    } else {
      value = cz80->ram[ADDR_DATA];
      cz80_io_out(cz80, port, value);
    }
    z80_irq(cz80->z);
    return;
  }

  if (cz80->dirty) {
    render(cz80);
    cz80->dirty = 0;
  }

  cz80->frame++;
  dt = sys_get_clock() - cz80->t0;
  edt = cz80->frame * HOST_PERIOD;

  if (dt < edt) {
    debug(DEBUG_TRACE, "CZ80", "frame %d, sleep %llu", cz80->frame, edt - dt);
    //sys_usleep(edt - dt);
  }

  if (cz80->frame == Z80_FPS) {
    cz80->frame = 0;
    cz80->t0 = sys_get_clock();
  }

  if ((cz80->frame % 20) == 0) {
    cz80->cursor = !cz80->cursor;
    pterm_cursor(cz80->t, cz80->cursor);
  }
}

static void term_reply(char *buf, int n, void *data) {
  cz80_data_t *cz80 = (cz80_data_t *)data;

  debug(DEBUG_INFO, "TERM", "sending reply \"%.*s\"", n, buf);
  if (cz80->modem->write(cz80->modem, (uint8_t *)buf, n) != n) {
    sock_close(cz80);
  }
}

static int modem_dial(void *p, char *number) {
  cz80_data_t *cz80 = (cz80_data_t *)p;
  char address[64];
  int ip1, ip2, ip3, ip4, port;
  int i, sock, r = -1;

  if (number) {
    // dial
    if (number[0] == 'P' || number[0] == 'T') number++;

    if (sys_strchr(number, '.') == NULL && sys_strlen(number) == 17) {
      sys_sscanf(number, "%03d%03d%03d%03d%05d", &ip1, &ip2, &ip3, &ip4, &port);
      sys_snprintf(address, sizeof(address)-1, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    } else {
      sys_strncpy(address, number, sizeof(address)-1);
      port = 23;
      for (i = 0; address[i]; i++) {
        if (address[i] == ':') {
          address[i] = 0;
          port = 0;
          for (i++; address[i]; i++) {
            port = (port * 10) + (address[i] - '0');
          }
          break;
        }
      }
    }

    if ((sock = sys_socket_open_connect(address, port, IP_STREAM)) != -1) {
      cz80->tcp = conn_filter(sock);
      cz80->telnet = telnet_client_filter(cz80->tcp, "ANSI", cz80->screen_width / cz80->char_width, cz80->screen_height / cz80->char_height);
      modem_set(cz80->modem, cz80->telnet);
      debug(DEBUG_INFO, "MODEM", "online");
      r = 0;
    }

  } else {
    // hangup
    debug(DEBUG_INFO, "MODEM", "offline");
    sock_close(cz80);
    r = 0;
  }

  return r;
}

static int cz80_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  cz80_data_t *cz80;
  uint32_t i, j;

  cz80 = (cz80_data_t *)c->data;
  cz80->surface = surface;
  cz80->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface) {
    cz80->screen_width = surface->width;
    cz80->screen_height = surface->height;
    cz80->scy1 = 0;
    cz80->scy2 = surface->height-1;
    cz80->t = pterm_init(surface->width / cz80->char_width, surface->height / cz80->char_height, 0);
    pterm_callback(cz80->t, &cz80->cb);
    pterm_setfg(cz80->t, GREEN);
    pterm_setbg(cz80->t, BLACK);

    if (surface->encoding == SURFACE_ENCODING_PALETTE) {
      for (i = 0, j = 0; i < 16; i++) {
        surface_palette(surface, i, colors[j], colors[j+1], colors[j+2]);
        j += 3;
      }
    } else {
      for (i = 0, j = 0; i < 16; i++) {
        cz80->c[i] = surface_color_rgb(surface->encoding, NULL, 0, colors[j], colors[j+1], colors[j+2], 0xFF);
        j += 3;
      }
    }
  }

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

computer_t *cz80_init(vfs_session_t *session) {
  cz80_data_t *cz80;
  computer_t *c = NULL;

  if ((cz80 = xcalloc(1, sizeof(cz80_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      cz80->d = disk_init(c, MAX_DRIVES, 0, session);
      cz80->t0 = sys_get_clock();
      cz80->z = z80_open(Z80_PERIOD, 1, cz80_callback, cz80, c);
      cz80->boot = 1;
      cz80->dirty = 1;
      cz80->cs0 = sys_get_clock();
      cz80->session = session;
      cz80->kbuflen = 0;
      cz80->pkbuf = cz80->gkbuf = 0;
      cz80->modem = modem_filter(NULL, cz80, modem_dial);
      cz80->cb.data = cz80;
      cz80->cb.draw = term_draw;
      cz80->cb.erase = term_erase;
      cz80->cb.reply = term_reply;

      c->set_surface = cz80_set_surface;
      c->disk = cz80_disk;
      c->option = cz80_option;
      c->run = cz80_run;
      c->close = cz80_close;
      c->getb = cz80_getb;
      c->getop = cz80_getb;
      c->putb = cz80_putb;
      c->data = cz80;
    } else {
      xfree(cz80);
    }
  }

  return c;
}
