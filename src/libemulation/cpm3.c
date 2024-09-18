#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "script.h"
#include "media.h"
#include "vfs.h"
#include "sys.h"
#include "ptr.h"
#include "pwindow.h"
#include "pit_io.h"
#include "filter.h"
#include "login.h"
#include "telnet.h"
#include "rom.h"
#include "emulation.h"
#include "computer.h"
#include "disk.h"
#include "z80.h"
#include "cpm3.h"
#include "pterm.h"
#include "debug.h"
#include "xalloc.h"

#define Z80_CLOCK     4000000
#define Z80_FPS       60
#define Z80_PERIOD    Z80_CLOCK/Z80_FPS

#define MEM_SIZE    64*1024
#define NUM_BANKS   16

#define COLS        80
#define ROWS        25
#define VRAM_SIZE   COLS*ROWS

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

#define CHR_SIZE    256*CHAR_HEIGHT

#define SCREEN_WIDTH  (COLS * CHAR_WIDTH)
#define SCREEN_HEIGHT (ROWS * CHAR_HEIGHT)

typedef struct {
  z80_t *z;
  uint8_t num_banks, bank, wp_common;
  uint16_t segsize;
  uint8_t dsk_cmd;
  uint8_t banks[NUM_BANKS][MEM_SIZE];
  disk_t *d;
  uint8_t clkcmd;
  uint8_t clkfmt;
  int boot;

  uint64_t key_t;
  uint8_t key;
  uint32_t frame;
  int cursor;
  conn_filter_t *filter;

  vfs_session_t *session;
  window_provider_t *wp;
  uint32_t rgba[SCREEN_WIDTH * SCREEN_HEIGHT];
  uint8_t cidx[SCREEN_WIDTH * SCREEN_HEIGHT];
  uint8_t chr[CHR_SIZE];
  window_t *w;
  texture_t *screen;
  uint32_t c[16];
  int dirty;
  pterm_t *pterm;
} cpm3_data_t;

/*
        FFFF
        +--------+
   16KB | common |
        +--------+
        +--------+  +--------+  ..........  +--------+
        |        |  |        |              |        |
   48KB |        |  |        |  ..........  |        |
        | bank 0 |  | bank 1 |              | bank n |
        +--------+  +--------+  ..........  +--------+
        0000
*/

static uint8_t cpm3_getb(computer_t *c, uint16_t addr) {
  cpm3_data_t *cpm3;

  cpm3 = (cpm3_data_t *)c->data;

  if (cpm3->bank == 0) {
    return cpm3->banks[0][addr];
  }

  if (addr >= cpm3->segsize) {
    return cpm3->banks[0][addr];
  }

  return cpm3->banks[cpm3->bank][addr];
}

static void cpm3_putb(computer_t *c, uint16_t addr, uint8_t b) {
  cpm3_data_t *cpm3;

  cpm3 = (cpm3_data_t *)c->data;

  if ((addr >= cpm3->segsize) && (cpm3->wp_common != 0)) {
    return;
  }

  if (cpm3->bank == 0) {
    cpm3->banks[0][addr] = b;

  } else {
    if (addr >= cpm3->segsize) {
      cpm3->banks[0][addr] = b;
    } else {
      cpm3->banks[cpm3->bank][addr] = b;
    }
  }
}

static void cpm3_out(computer_t *c, uint16_t port, uint8_t b) {
  cpm3_data_t *cpm3;

  cpm3 = (cpm3_data_t *)c->data;
  //debug(DEBUG_INFO, "CPM3", "out %d, %d", port & 0xFF, b);

  switch (port & 0xFF) {
    case 1:   // console data port
      if (cpm3->filter) {
        cpm3->filter->write(cpm3->filter, &b, 1);
      } else {
        pterm_send(cpm3->pterm, &b, 1);
        cpm3->dirty = 1;
      }
      break;
    case 3:   // printer data port
    case 5:   // auxilary data port
      break;
    case 10:  // fdc-port: # of drive
      disk_out(cpm3->d, DSKDRV, b);
      break;
    case 11:  // fdc-port: # of track (0-255 apenas)
      disk_out(cpm3->d, DSKTKH, 0);
      disk_out(cpm3->d, DSKTKL, b);
      break;
    case 12:  // fdc-port: # of sector (low)
      disk_out(cpm3->d, DSKSECL, b);
      break;
    case 13:  // fdc-port: command (0=read, 1=write)
      disk_out(cpm3->d, DSKCMD, b);
      cpm3->dsk_cmd = b;
      break;
    case 15:  // dma-port: dma address low
      disk_out(cpm3->d, DSKDMAL, b);
      break;
    case 16:  // dma-port: dma address high
      disk_out(cpm3->d, DSKDMAH, b);
      break;
    case 17:  // fdc-port: # of sector high
      disk_out(cpm3->d, DSKSECH, b);
      break;
    case 20:  // out: initialize mmu (number of memory banks to initialize incl. bank 0), in: return number of initialised MMU banks
      if (b < NUM_BANKS) {
        cpm3->num_banks = b;
      }
      break;
    case 21:  // out: select mmu bank, in: return selected bank
      if (b < cpm3->num_banks) {
        cpm3->bank = b;
      }
      break;
    case 22:  // out: select MMU segment size (in pages a 256 bytes), in: return segment size
      cpm3->segsize = ((uint16_t)b) << 8;
      break;
    case 23:  // MMU write protect/unprotect common memory segment
      cpm3->wp_common = b;
      break;
    case 25:  // clock command
      if (b < 0xFF) {
        cpm3->clkcmd = b;
      } else {
        cpm3->clkfmt ^= 1;
      }
      break;
    default:
      break;
  }
}

static int to_bcd(int val) {
  int i = 0;

  while (val >= 10) {
    i += val / 10;
    i <<= 4;
    val %= 10;
  }
  i += val;
  return i;
}

/*
  Calculate number of days since 1.1.1978
  CP/M 3 and MP/M 2 are Y2K bug fixed and can handle the date,
  so the Y2K bug here is intentional.
*/
static uint16_t get_date(sys_tm_t *tm) {
  uint16_t i, val = 0;

  for (i = 1978; i < 1900 + tm->tm_year; i++) {
    val += 365;
    if (i % 4 == 0) val++;
  }
  val += tm->tm_yday + 1;

  return val;
}

static uint8_t cpm3_getclock(uint8_t cmd, uint8_t fmt) {
  sys_tm_t *tm, ttm;
  uint64_t t;
  uint8_t b = 0;

  // clkfmt: clock format, 0 = BCD, 1 = decimal

  t = sys_time();
  sys_localtime(&t, &ttm);
  tm = &ttm;

  switch (cmd) {
    case 0:  // seconds
      b = fmt ? tm->tm_sec : to_bcd(tm->tm_sec);
      break;
    case 1:  // minutes
      b = fmt ? tm->tm_min : to_bcd(tm->tm_min);
      break;
    case 2:  // hours
      b = fmt ? tm->tm_hour : to_bcd(tm->tm_hour);
      break;
    case 3:  // low byte number of days since 1.1.1978
      b = get_date(tm) & 255;
      break;
    case 4:  // high byte number of days since 1.1.1978
      b = get_date(tm) >> 8;
      break;
    case 5:  // day of month
      b = fmt ? tm->tm_mday : to_bcd(tm->tm_mday);
      break;
    case 6:  // month
      b = fmt ? tm->tm_mon : to_bcd(tm->tm_mon);
      break;
    case 7:  // year
      b = fmt ? tm->tm_year : to_bcd(tm->tm_year);
      break;

    default:
      // for every other clock command a 0 is returned
      b = 0;
      break;
  }

  return b;
}

static void render(cpm3_data_t *cpm3) {
  int addr, x, y, index;
  int caddr, i, j, cursor;
  uint32_t fg, bg;
  uint8_t col, row, code, mask;

  cursor = pterm_getcursor(cpm3->pterm, &col, &row);

  for (addr = 0; addr < VRAM_SIZE; addr++) {
    x = addr % COLS;
    y = addr / COLS;
    index = y * CHAR_HEIGHT * SCREEN_WIDTH + x * CHAR_WIDTH;

    pterm_getchar(cpm3->pterm, addr, &code, &fg, &bg);
    if (x == col && y == row) {
      if (cursor) {
        fg = WHITE;
        bg = BLACK;
      } else {
        fg = BLACK;
        bg = WHITE;
      }
    }
    if (code < 32) code = 32;
    caddr = code * CHAR_HEIGHT;

    for (i = 0; i < CHAR_HEIGHT; i++) {
      mask = cpm3->chr[caddr++];

      for (j = 0; j < CHAR_WIDTH; j++, mask <<= 1) {
        if ((mask & 0x80) != 0) {
          cpm3->cidx[index + j] = fg;
          cpm3->rgba[index + j] = cpm3->c[fg];
        } else {
          cpm3->cidx[index + j] = bg;
          cpm3->rgba[index + j] = cpm3->c[bg];
        }
      }
      index += SCREEN_WIDTH;
    }
  }

  if (cpm3->w) {
    cpm3->wp->update_texture(cpm3->w, cpm3->screen, (uint8_t *)cpm3->rgba);
    cpm3->wp->draw_texture(cpm3->w, cpm3->screen, 0, 0);
    cpm3->wp->render(cpm3->w);
  }
}

static uint8_t cpm3_in(computer_t *c, uint16_t port) {
  cpm3_data_t *cpm3;
  int key, mods, buttons;
  uint8_t b = 0;
  int n, r;

  cpm3 = (cpm3_data_t *)c->data;
  //debug(DEBUG_INFO, "CPM3", "in %d ...", port & 0xFF);

  switch (port & 0xFF) {
    case 0:   // console status port (console in status, return 0ffh if character ready, 00h if not)
      if (cpm3->filter) {
        n = cpm3->filter->peek(cpm3->filter, 0);
        b = n ? 0xFF : 0x00;
      } else {
        if (cpm3->w) {
          n = cpm3->wp->event(cpm3->w, 0, 0, &key, &mods, &buttons);
        }
        b = (n == WINDOW_KEY) ? 0xFF : 0x00;
      }
      break;
    case 1:   // console data port
      if (cpm3->filter) {
        n = cpm3->filter->read(cpm3->filter, &b);
        if (n <= 0) {
          z80_stop(cpm3->z);
          break;
        }
        if (b == 0x7F) b = 0x08;
      } else {
        for (;;) {
          if ((cpm3->frame % 20) == 0) {
            cpm3->cursor = !cpm3->cursor;
            pterm_cursor(cpm3->pterm, cpm3->cursor);
            cpm3->dirty = 1;
          }
          if (cpm3->dirty) {
            render(cpm3);
            cpm3->dirty = 0;
          }
          cpm3->frame++;

          if (cpm3->w) {
            r = cpm3->wp->event(cpm3->w, 20, 1, &key, &mods, &buttons);
          }

          if (r == -1) {
            z80_stop(cpm3->z);
            break;
          }
          if (r == WINDOW_KEY) {
            b = key;
            break;
          }
          if (r == 0) break;
        }
      }
      break;
    case 2:   // printer status port
      b = 0x00; // not ready
      break;
    case 4:   // auxilary status port (auxilary input status, 0ffh if ready, 00h if not)
      b = 0x00; // not ready
      break;
    case 5:   // auxilary data port
      b = 0x00;
      break;
    case 10:  // fdc-port: # of drive
      b = disk_in(cpm3->d, DSKDRV);
      break;
    case 11:  // fdc-port: # of track (0-255 apenas)
      b = disk_in(cpm3->d, DSKTKL);
      break;
    case 12:  // fdc-port: # of sector (low)
      b = disk_in(cpm3->d, DSKSECL);
      break;
    case 13:  // fdc-port: command (0=read, 1=write)
      b = cpm3->dsk_cmd;
      break;
    case 14:  // fdc-port: status (0=success)
      b = disk_in(cpm3->d, DSKCMD);
      break;
    case 15:  // dma-port: dma address low
      b = disk_in(cpm3->d, DSKDMAL);
      break;
    case 16:  // dma-port: dma address high
      b = disk_in(cpm3->d, DSKDMAH);
      break;
    case 17:  // fdc-port: # of sector high
      b = disk_in(cpm3->d, DSKSECH);
      break;
    case 20:  // out: initialize mmu (number of memory banks to initialize incl. bank 0), in: return number of initialised MMU banks
      b = cpm3->num_banks;
      break;
    case 21:  // out: select mmu bank, in: return selected bank
      b = cpm3->bank;
      break;
    case 22:  // out: select MMU segment size (in pages a 256 bytes), in: return segment size
      b = cpm3->segsize >> 8;
      break;
    case 23:  // MMU write protect/unprotect common memory segment
      b = cpm3->wp_common;
      break;
    case 25:  // clock command
      b = cpm3->clkcmd;
      break;
    case 26:  // clock data
      b =  cpm3_getclock(cpm3->clkcmd, cpm3->clkfmt);
      break;
  }

  //debug(DEBUG_INFO, "CPM3", "in %d = %d", port & 0xFF, b);

  return b;
}

static int cpm3_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  cpm3_data_t *cpm3;

  cpm3 = (cpm3_data_t *)c->data;
  return disk_insert(cpm3->d, drive, skip, tracks, heads, sectors, sectorlen, sector0, name);
}

static int cpm3_rom(computer_t *c, int num, uint32_t size, char *name) {
  cpm3_data_t *cpm3;
  int r = -1;

  cpm3 = (cpm3_data_t *)c->data;

  if (num == 0) {
    r = load_rom(cpm3->session, name, size, cpm3->chr) != -1 ? 0 : -1;
  }

  return r;
}

static int cpm3_run(computer_t *c) {
  cpm3_data_t *cpm3;

  cpm3 = (cpm3_data_t *)c->data;

  if (cpm3->boot) {
    disk_boot(cpm3->d, 0);
    cpm3->boot = 0;
  }

  return z80_loop(cpm3->z, Z80_PERIOD);
}

static int cpm3_close(computer_t *c) {
  cpm3_data_t *cpm3;
  int r = -1;

  cpm3 = (cpm3_data_t *)c->data;

  if (cpm3) {
    if (cpm3->pterm) pterm_close(cpm3->pterm);
    if (cpm3->w) {
      cpm3->wp->destroy_texture(cpm3->w, cpm3->screen);
      cpm3->wp->destroy(cpm3->w);
    }
    disk_close(cpm3->d);
    xfree(cpm3);
    r = 0;
  }
  xfree(c);

  return r;
}

static int cpm3_set_window(computer_t *c, window_provider_t *wp, int fullscreen) {
  cpm3_data_t *cpm3;
  int width, height;

  cpm3 = (cpm3_data_t *)c->data;
  cpm3->wp = wp;
  width = SCREEN_WIDTH;
  height = SCREEN_HEIGHT;
  cpm3->w = wp->create(ENC_RGBA, &width, &height, 2, 4, 0, fullscreen, 0, NULL, NULL);
  if (cpm3->w) {
    cpm3->screen = cpm3->wp->create_texture(cpm3->w, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!fullscreen) {
      wp->title(cpm3->w, "CP/M 3");
    }
  }

  return cpm3->w ? 0 : -1;
}

static int cpm3_set_filter(struct computer_t *c, conn_filter_t *filter) {
  cpm3_data_t *cpm3;

  cpm3 = (cpm3_data_t *)c->data;
  cpm3->filter = filter;

  return 0;
}

computer_t *cpm3_init(vfs_session_t *session) {
  cpm3_data_t *cpm3;
  computer_t *c = NULL;

  if ((cpm3 = xcalloc(1, sizeof(cpm3_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      cpm3->d = disk_init(c, 2, 1);
      cpm3->z = z80_open(Z80_PERIOD, 1, NULL, NULL, c);
      cpm3->segsize = 48*1024;
      cpm3->boot = 1;
      cpm3->session = session;
      cpm3->c[0]  = 0xFF000000;
      cpm3->c[1]  = 0xFFC00000;
      cpm3->c[2]  = 0xFF00C000;
      cpm3->c[3]  = 0xFFC0C000;
      cpm3->c[4]  = 0xFF0000C0;
      cpm3->c[5]  = 0xFFC000C0;
      cpm3->c[6]  = 0xFF00C0C0;
      cpm3->c[7]  = 0xFFC0C0C0;
      cpm3->c[8]  = 0xFF000000;
      cpm3->c[9]  = 0xFFFF0000;
      cpm3->c[10] = 0xFF00FF00;
      cpm3->c[11] = 0xFFFFFF00;
      cpm3->c[12] = 0xFF0000FF;
      cpm3->c[13] = 0xFFFF00FF;
      cpm3->c[14] = 0xFF00FFFF;
      cpm3->c[15] = 0xFFFFFFFF;
      cpm3->dirty = 1;
      cpm3->pterm = pterm_init(COLS, ROWS, 0);
      z80_halt_exits(cpm3->z);
      c->set_window = cpm3_set_window;
      c->set_filter = cpm3_set_filter;
      c->disk = cpm3_disk;
      c->rom = cpm3_rom;
      c->run = cpm3_run;
      c->close = cpm3_close;
      c->getb = cpm3_getb;
      c->getop = cpm3_getb;
      c->putb = cpm3_putb;
      c->out = cpm3_out;
      c->in = cpm3_in;
      c->data = cpm3;
    } else {
      xfree(cpm3);
    }
  }

  return c;
}
