#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "pwindow.h"
#include "filter.h"
#include "rom.h"
#include "computer.h"
#include "disk.h"
#include "pterm.h"
#include "i8086.h"
#include "c8086.h"
#include "debug.h"
#include "xalloc.h"

#define MAX_KBUF    256
#define NUM_COLORS  16

typedef struct {
  i8086_t *i8086;
  uint16_t ports[65536];
  disk_t *d;
  int frame;
  vfs_session_t *session;
  surface_t *surface;
  int ptr;
  uint32_t color[NUM_COLORS];
  int font, char_width, char_height;
  int dirty;
  pterm_callback_t cb;
  pterm_t *t;
  uint32_t lba;
  uint8_t kbuffer[MAX_KBUF];
  int kbuflen, pkbuf, gkbuf;
} c8086_data_t;

static const uint8_t colors[] = {
  0x00, 0x00, 0x00, // black 
  0x00, 0x00, 0xAA, // blue 
  0x00, 0xAA, 0x00, // green 
  0x00, 0xAA, 0xAA, // cyan 
  0xAA, 0x00, 0x00, // red 
  0xAA, 0x00, 0xAA, // magenta 
  0xAA, 0x55, 0x00, // brown 
  0xAA, 0xAA, 0xAA, // light gray 
  0x55, 0x55, 0x55, // dark gray 
  0x55, 0x55, 0xFF, // light blue
  0x55, 0xFF, 0x55, // light green
  0x55, 0xFF, 0xFF, // light cyan
  0xFF, 0x55, 0x55, // light red
  0xFF, 0x55, 0xFF, // light magenta
  0xFF, 0xFF, 0x55, // yellow
  0xFF, 0xFF, 0xFF, // white
};

static surface_t *lock_surface(c8086_data_t *c8086) {
  if (c8086->surface) return c8086->surface;
  if (c8086->ptr > 0) return ptr_lock(c8086->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(c8086_data_t *c8086) {
  if (c8086->ptr > 0) ptr_unlock(c8086->ptr, TAG_SURFACE);
}

#if 0
static void ide_out(c8086_data_t *c8086, uint16_t port, uint8_t b) {
  switch (port) {
    case 0: // data
      break;
    case 1: // feature
      break;
    case 2: // sector count
      break;
    case 3: // LBA 0
      c8086->lba &= 0xFFFFFF00;
      c8086->lba |= b;
      break;
    case 4: // LBA 1
      c8086->lba &= 0xFFFF00FF;
      c8086->lba |= (b << 8);
      break;
    case 5: // LBA 2
      c8086->lba &= 0xFF00FFFF;
      c8086->lba |= (b << 16);
      break;
    case 6: // LBA 3
      c8086->lba &= 0x00FFFFFF;
      c8086->lba |= ((b & 0x0F) << 24);
      break;
    case 7: // CMD
      switch (b) {
        case 0x020: // read
          debug(DEBUG_INFO, "C8086", "read sector %d", c8086->lba);
          disk_read_lba(c8086->d, c8086->lba);
          break;
      }
      break;
  }
}

static uint8_t ide_in(c8086_data_t *c8086, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case 0: // data
      b = disk_in(c8086->d, DSKDAT);
      break;
    case 1: // error
      break;
    case 7: // status
      b = 0x48; // not busy, drive ready, data request ready
      break;
  }

  return b;
}
#endif

static void c8086_out(computer_t *c, uint16_t port, uint16_t b) {
  c8086_data_t *c8086 = (c8086_data_t *)c->data;

  debug(DEBUG_INFO, "C8086", "out 0x%04X 0x%04X", port, b);
  c8086->ports[port] = b;
}

static uint16_t c8086_in(computer_t *c, uint16_t port) {
  c8086_data_t *c8086 = (c8086_data_t *)c->data;

  debug(DEBUG_INFO, "C8086", "in 0x%04X 0x%04X", port, c8086->ports[port]);
  return c8086->ports[port];
}

static int term_draw(uint8_t col, uint8_t row, uint8_t code, uint32_t fg, uint32_t bg, uint8_t attr, void *data) {
  c8086_data_t *c8086 = (c8086_data_t *)data;
  surface_t *surface = lock_surface(c8086);
  uint32_t x, y;
  char s[2];

  x = col * c8086->char_width;
  y = row * c8086->char_height;
  fg = c8086->color[fg];
  bg = c8086->color[bg];

  if (code == 0) code = ' ';
  s[0] = code;
  s[1] = 0;
  surface_print(surface, x, y, s, c8086->font, fg, bg);
  unlock_surface(c8086);
  c8086->dirty = 1;

  return 0;
}

static int term_erase(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, uint32_t bg, uint8_t attr, void *data) {
  c8086_data_t *c8086 = (c8086_data_t *)data;
  surface_t *surface = lock_surface(c8086);

  bg = c8086->color[bg];
  surface_rectangle(surface, col1*c8086->char_width, row1*c8086->char_height, col2*c8086->char_width, row2*c8086->char_height, 1, bg);
  unlock_surface(c8086);
  c8086->dirty = 1;

  return 0;
}

static int c8086_rom(computer_t *c, int num, uint32_t size, char *name) {
  c8086_data_t *c8086;
  uint8_t *mem;
  int r = -1;

  c8086 = (c8086_data_t *)c->data;

  switch (num) {
    case 0:
      mem = i8086_mem(c8086->i8086);
      // load BIOS at F000:0100, and set IP to 0100
      r = load_rom(c8086->session, name, size, mem) != -1 ? 0 : -1;
      break;
  }

  return r;
}

static int c8086_option(computer_t *c, char *name, char *value) {
  c8086_data_t *c8086;

  c8086 = (c8086_data_t *)c->data;

  if (!sys_strcmp(name, "font")) {
    c8086->font = sys_atoi(value);
    c8086->char_width = surface_font_width(c8086->font);
    c8086->char_height = surface_font_height(c8086->font);
  }

  return 0;
}

static int c8086_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  c8086_data_t *c8086;

  c8086 = (c8086_data_t *)c->data;
  return disk_insert(c8086->d, drive, skip, tracks, heads, sectors, sectorlen, sector0, name);
}

static int c8086_run(computer_t *c, uint32_t us) {
  c8086_data_t *c8086;

  c8086 = (c8086_data_t *)c->data;

  return i8086_loop(c8086->i8086);
}

static int c8086_close(computer_t *c) {
  c8086_data_t *c8086;
  int r = -1;

  c8086 = (c8086_data_t *)c->data;

  if (c8086) {
    disk_close(c8086->d);
    i8086_close(c8086->i8086);
    xfree(c8086);
    r = 0;
  }
  xfree(c);

  return r;
}

static void render(c8086_data_t *c8086) {
  surface_t *surface = lock_surface(c8086);
  surface_update(surface);
  unlock_surface(c8086);
}

static void c8086_callback(void *data, uint32_t count) {
  c8086_data_t *c8086 = (c8086_data_t *)data;

  if (c8086->dirty) {
    render(c8086);
    c8086->dirty = 0;
  }
}

static int c8086_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  c8086_data_t *c8086;
  int i, j;

  c8086 = (c8086_data_t *)c->data;
  c8086->surface = surface;
  c8086->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface->encoding == SURFACE_ENCODING_PALETTE) {
    for (i = 0, j = 0; i < NUM_COLORS; i++) {
      surface_palette(surface, i, colors[j], colors[j+1], colors[j+2]);
      c8086->color[i] = i;
      j += 3;
    }
  } else {
    for (i = 0, j = 0; i < NUM_COLORS; i++) {
      c8086->color[i] = surface_color_rgb(surface->encoding, NULL, 0, colors[j], colors[j+1], colors[j+2], 0xFF);
      j += 3;
    }
  }
  surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, c8086->color[0]);

  c8086->t = pterm_init(surface->width / c8086->char_width, surface->height / c8086->char_height, 0);
  pterm_callback(c8086->t, &c8086->cb);
  pterm_setfg(c8086->t, 7);
  pterm_setbg(c8086->t, 0);

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

computer_t *c8086_init(vfs_session_t *session) {
  c8086_data_t *c8086;
  computer_t *c = NULL;

  if ((c8086 = xcalloc(1, sizeof(c8086_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      c8086->d = disk_init(c, 2, 0, session);
      c8086->i8086 = i8086_open(0, c8086_callback, c8086, c);
      c8086->dirty = 1;
      c8086->session = session;
      c8086->kbuflen = 0;
      c8086->pkbuf = c8086->gkbuf = 0;
      c8086->cb.data = c8086;
      c8086->cb.draw = term_draw;
      c8086->cb.erase = term_erase;
      c->set_surface = c8086_set_surface;
      c->option = c8086_option;
      c->disk = c8086_disk;
      c->rom = c8086_rom;
      c->run = c8086_run;
      c->close = c8086_close;
      c->out = c8086_out;
      c->in = c8086_in;
      c->data = c8086;
    } else {
      xfree(c8086);
    }
  }

  return c;
}

void i8086_direct_video(computer_t *computer, uint32_t addr, uint16_t data, int w) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;
  uint8_t code, attr, dummy_code;
  uint32_t index, fg, bg, dummy_color;

  // video memory: even byte is the character code, odd byte is the attribute
  // attribute byte:
  // bit 0: blue foreground
  // bit 1: green foreground
  // bit 2: red foreground
  // bit 3: bright foreground
  // bit 4: blue background
  // bit 5: green background
  // bit 6: red background
  // bit 7: bright background; or blinking text
  // ** black+bright shows as dark grey, and yellow without bright shows as brown

  index = addr >> 1;

  if (w) {
    if (addr & 1) {
      debug(DEBUG_TRACE, "C8086", "odd video write 0x%05X = 0x%04X", addr, data);
      attr = data & 0xFF;
      fg = attr & 0x0F;
      bg = attr >> 4;
      pterm_getchar(c8086->t, index, &code, &dummy_color, &dummy_color);
      pterm_putchar(c8086->t, index, code, fg, bg);
      code = data >> 8;
      pterm_getchar(c8086->t, index+1, &dummy_code, &fg, &bg);
      pterm_putchar(c8086->t, index+1, code, fg, bg);

    } else {
      debug(DEBUG_TRACE, "C8086", "even video write 0x%05X = 0x%04X", addr, data);
      code = data & 0xFF;
      attr = data >> 8;
      fg = attr & 0x0F;
      bg = attr >> 4;
      pterm_putchar(c8086->t, index, code, fg, bg);
    }

  } else {
    if (addr & 1) {
      debug(DEBUG_TRACE, "C8086", "odd video write 0x%05X = 0x%02X", addr, data);
      attr = data & 0xFF;
      fg = attr & 0x0F;
      bg = attr >> 4;
      pterm_getchar(c8086->t, index, &code, &dummy_color, &dummy_color);
      pterm_putchar(c8086->t, index, code, fg, bg);
    } else {
      debug(DEBUG_TRACE, "C8086", "even video write 0x%05X = 0x%02X", addr, data);
      code = data & 0xFF;
      pterm_getchar(c8086->t, index, &dummy_code, &fg, &bg);
      pterm_putchar(c8086->t, index, code, fg, bg);
    }
  }
}

void i8086_cls(computer_t *computer) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;

  pterm_cls(c8086->t);
}

void i8086_set_cursor(computer_t *computer, uint8_t row, uint8_t col) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;

  pterm_sety(c8086->t, row);
  pterm_setx(c8086->t, col);
}

void i8086_get_cursor(computer_t *computer, uint8_t *row, uint8_t *col) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;

  pterm_getcursor(c8086->t, col, row);
}

void i8086_char_cursor(computer_t *computer, uint8_t *code, uint8_t *color) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;
  uint32_t fg, bg;
  uint8_t col, row;

  pterm_getcursor(c8086->t, &col, &row);
  pterm_getchar(c8086->t, row*80 + col, code, &fg, &bg);
  *color = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

void i8086_putc(computer_t *computer, uint8_t b) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;

  pterm_send(c8086->t, &b, 1);
}

static void check_kbd(c8086_data_t *c8086) {
  surface_t *surface = lock_surface(c8086);
  int arg1, arg2;

  switch (surface_event(surface, 0, &arg1, &arg2)) {
    case WINDOW_KEYDOWN:
      break;
    case WINDOW_KEYUP:
      if (arg1 < 128) {
        if (c8086->kbuflen < MAX_KBUF) {
          c8086->kbuffer[c8086->pkbuf] = arg1;
          c8086->pkbuf = (c8086->pkbuf + 1) % MAX_KBUF;
          c8086->kbuflen++;
        }
      }
      break;
    case -1:
      i8086_stop(c8086->i8086);
      break;
  }
}

uint8_t i8086_kbhit(computer_t *computer) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;
  uint8_t b = 0;

  check_kbd(c8086);

  if (c8086->kbuflen > 0) {
    b = c8086->kbuffer[c8086->gkbuf];
  }

  return b;
}

uint8_t i8086_getc(computer_t *computer) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;
  uint8_t b = 0;

  check_kbd(c8086);

  if (c8086->kbuflen > 0) {
    b = c8086->kbuffer[c8086->gkbuf];
    c8086->gkbuf = (c8086->gkbuf + 1) % MAX_KBUF;
    c8086->kbuflen--;
  }

  return b;
}

uint32_t i8086_seek(computer_t *computer, uint8_t d, uint32_t lba) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;

  //debug(DEBUG_INFO, "DISK", "seek 0x%02X %u", d, lba);
  c8086->lba = lba + 1;
  
  return 0;
}

uint32_t i8086_read(computer_t *computer, uint8_t d, void *p, uint32_t len) {
  c8086_data_t *c8086 = (c8086_data_t *)computer->data;

  return disk_read_lba2(c8086->d, c8086->lba, p, len);
}

uint32_t i8086_write(computer_t *computer, uint8_t d, void *p, uint32_t len) {
  //debug(DEBUG_INFO, "C8086", "write 0x%02X 0x%04X", d, len);
  return 0;
}
