#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "script.h"
#include "media.h"
#include "vfs.h"
#include "sys.h"
#include "pwindow.h"
#include "pit_io.h"
#include "filter.h"
#include "rom.h"
#include "emulation.h"
#include "computer.h"
#include "disk.h"
#include "z80.h"
#include "cz80m6847.h"
#include "vdg.h"
#include "debug.h"
#include "xalloc.h"

#define Z80_CLOCK     4000000
#define Z80_FPS       60
#define Z80_PERIOD    Z80_CLOCK/Z80_FPS
#define HOST_PERIOD   1000000/Z80_FPS

#define ROM_SIZE      8192
#define RAM_SIZE      65536
#define VRAM_SIZE     8192

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 192

#define CHAR_WIDTH    8
#define CHAR_HEIGHT   12

#define CHR_SIZE      6*32*CHAR_HEIGHT

#define BMP_NUM_COLS  32
#define BMP_NUM_ROWS  6
#define BMP_SIZE      65536

#define MAX_KBUF      256

typedef struct {
  z80_t *z;
  uint8_t rom[ROM_SIZE];
  uint8_t ram[RAM_SIZE];
  uint8_t vram[VRAM_SIZE];
  uint8_t chr[CHR_SIZE];
  uint8_t bmp[BMP_SIZE];
  disk_t *d;

  int64_t t0;
  int frame;
  uint32_t ticks;

  vfs_session_t *session;
  window_provider_t *wp;
  window_t *w;
  texture_t *screen;
  uint32_t rgba[SCREEN_WIDTH * SCREEN_HEIGHT];
  uint32_t color[9];
  uint32_t last_char;
  uint8_t vdg_mode;
  int dirty;
  vdg_t vdg;

  uint8_t sioa_reg;
  uint8_t siob_reg;
  uint8_t ms;
  uint32_t lba;

  uint8_t kbuffer[MAX_KBUF];
  int kbuflen, pkbuf, gkbuf;
} cz80m6847_data_t;

#pragma pack(2)
typedef struct {
  uint16_t type;
  uint32_t fileSize;
  uint32_t reserved;
  uint32_t dataOffset;
  uint32_t headerSize;
  uint32_t width;
  uint32_t height;
  uint16_t planes;
  uint16_t bpp;
  uint32_t compression;
  uint32_t dataSize;
  uint32_t hRes;
  uint32_t vRes;
  uint32_t colors;
  uint32_t iColors;
} bmp_t;
#pragma pack()

static uint8_t cz80m6847_getb(computer_t *c, uint16_t addr) {
  cz80m6847_data_t *cz80m6847;
  uint8_t b = 0;

  cz80m6847 = (cz80m6847_data_t *)c->data;

  switch (addr & 0xE000) {
    case 0x0000:
      b = cz80m6847->rom[addr & 0x1FFF];
      break;
    case 0x2000:
    case 0x4000:
    case 0x6000:
    case 0x8000:
    case 0xA000:
    case 0xC000:
    case 0xE000:
      b = cz80m6847->ram[addr];
      break;
  }

  return b;
}

static void cz80m6847_putb(computer_t *c, uint16_t addr, uint8_t b) {
  cz80m6847_data_t *cz80m6847;

  cz80m6847 = (cz80m6847_data_t *)c->data;

  switch (addr & 0xE000) {
    case 0x2000:
      //cz80m6847->vram[addr & 0x1FFF] = b;
      //cz80m6847->dirty = 1;
      //break;
    case 0x4000:
    case 0x6000:
    case 0x8000:
    case 0xA000:
    case 0xC000:
    //case 0xE000:
      cz80m6847->ram[addr] = b;
      break;
    case 0xE000:
      if (!cz80m6847->ms) {
        cz80m6847->vram[addr & 0x1FFF] = b;
        cz80m6847->dirty = 1;
      } else {
        cz80m6847->ram[addr] = b;
      }
      break;
  }
}

static void vdg_char(void *p, uint8_t b, uint8_t fg, uint8_t bg, uint8_t col, uint8_t row) {
  cz80m6847_data_t *cz80m6847 = (cz80m6847_data_t *)p;
  uint32_t x, y, caddr, index, i, j;
  uint8_t mask;

  if (b < cz80m6847->last_char) {
    x = col * CHAR_WIDTH;
    y =  row * CHAR_HEIGHT;
    caddr = b * CHAR_HEIGHT;
    index = y * SCREEN_WIDTH + x;
    for (i = 0; i < CHAR_HEIGHT && (y + i) < SCREEN_HEIGHT; i++) {
      mask = cz80m6847->chr[caddr++];
      for (j = 0; j < CHAR_WIDTH && (x + j) < SCREEN_WIDTH; j++, mask <<= 1) {
        if ((mask & 0x80) != 0) {
          cz80m6847->rgba[index + j] = cz80m6847->color[fg];
        } else {
          cz80m6847->rgba[index + j] = cz80m6847->color[bg];
        }
      }
      index += SCREEN_WIDTH;
    }
    cz80m6847->dirty = 1;
  }
}

static void vdg_clear(void *p, uint8_t c, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1) {
  cz80m6847_data_t *cz80m6847 = (cz80m6847_data_t *)p;
  uint32_t i, j, i1, i2, d;

  i1 = y0 * SCREEN_WIDTH + x0;
  i2 = (y1-1) * SCREEN_WIDTH + x1;
  d = x1 - x0;
  for (i = i1; i < i2; i += SCREEN_WIDTH) {
    for (j = 0; j < d; j++) {
      cz80m6847->rgba[i + j] = cz80m6847->color[c];
    }
  }
  cz80m6847->dirty = 1;
}

/*
static void custom_out(cz80m6847_data_t *cz80m6847, uint16_t port, uint8_t b) {
  switch (port) {
    case 0:
      debug(DEBUG_INFO, "CZ8047", "debug 0x%02X", b);
      break;
    case 1:
      // hack: out to this port exits emulation
      debug(DEBUG_INFO, "CZ8047", "exit hack");
      z80_stop(cz80m6847->z);
      break;
  }
}
*/

static void sio_out(cz80m6847_data_t *cz80m6847, uint16_t port, uint8_t b) {
  switch (port) {
    case 0:  // port A data
      cz80m6847->sioa_reg = 0;
      break;
    case 1:  // port A control
      cz80m6847->sioa_reg = b & 0x07;
      break;
    case 2:  // port B data
      cz80m6847->siob_reg = 0;
      break;
    case 3:  // port B control
      cz80m6847->siob_reg = b & 0x07;
      break;
  }
}

static void ide_out(cz80m6847_data_t *cz80m6847, uint16_t port, uint8_t b) {
  switch (port) {
    case 0: // data
      break;
    case 1: // feature
      break;
    case 2: // sector count
      break;
    case 3: // LBA 0
      cz80m6847->lba &= 0xFFFFFF00;
      cz80m6847->lba |= b;
      break;
    case 4: // LBA 1
      cz80m6847->lba &= 0xFFFF00FF;
      cz80m6847->lba |= (b << 8);
      break;
    case 5: // LBA 2
      cz80m6847->lba &= 0xFF00FFFF;
      cz80m6847->lba |= (b << 16);
      break;
    case 6: // LBA 3
      cz80m6847->lba &= 0x00FFFFFF;
      cz80m6847->lba |= ((b & 0x0F) << 24);
      break;
    case 7: // CMD
      switch (b) {
        case 0x020: // read
          debug(DEBUG_INFO, "CZ8047", "read sector %d", cz80m6847->lba);
          disk_read_lba(cz80m6847->d, cz80m6847->lba);
          break;
      }
      break;
  }
}

static int dbg_index = 0;
static uint8_t dbg_buf[256];

static void cz80m6847_out(computer_t *c, uint16_t port, uint8_t b) {
  cz80m6847_data_t *cz80m6847;

  cz80m6847 = (cz80m6847_data_t *)c->data;
  debug(DEBUG_TRACE, "CZ8047", "out 0x%04X, 0x%02X", port, b);

  switch (port & 0xE0) {
    case 0xE0: // latch
      switch (port) {
        case 0xFF:
          debug(DEBUG_INFO, "CZ8047", "value %3d 0x%02X '%c'", b, b, b);
          return;
        case 0xFE: // add to debug
          if (dbg_index < 255) dbg_buf[dbg_index++] = b;
          return;
        case 0xFD: // print debug
          if (b) {
            dbg_buf[dbg_index] = 0;
            debug(DEBUG_INFO, "CZ8047", (char *)dbg_buf);
          } else {
            debug_bytes(DEBUG_INFO, "CZ8047", dbg_buf, dbg_index);
          }
          dbg_index = 0;
          return;
      }
      cz80m6847->ms = b & 0x08;
      cz80m6847->vdg_mode = 0;
      // bit 0: GM0
      // bit 1: GM1
      // bit 2: GM2
      // bit 3: CSS
      // bit 4: INT/EXT
      // bit 5: A/G
      if (b & 0x01) cz80m6847->vdg_mode |= 0x08; // CSS
      if (b & 0x02) cz80m6847->vdg_mode |= 0x20; // A/G
      cz80m6847->vdg_mode |= (b & 0x70) >> 4;    // GMn
      break;
    case 0xC0: // SIO
      sio_out(cz80m6847, port & 0x03, b);
      break;
    case 0xA0: // IDE
      ide_out(cz80m6847, port & 0x07, b);
      break;
  }
}

static uint8_t sio_in(cz80m6847_data_t *cz80m6847, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case 0: // port A data
      if (cz80m6847->kbuflen) {
        b = cz80m6847->kbuffer[cz80m6847->gkbuf];
        cz80m6847->gkbuf = (cz80m6847->gkbuf + 1) % MAX_KBUF;
        cz80m6847->kbuflen--;
      }
      cz80m6847->sioa_reg = 0;
      break;
    case 1: // port A control
      if (cz80m6847->sioa_reg == 0) {
        b = cz80m6847->kbuflen ? 0x01 : 0x00;
      }
      cz80m6847->sioa_reg = 0;
      break;
    case 2: // port B data
      cz80m6847->siob_reg = 0;
      break;
    case 3: // port B control
      cz80m6847->siob_reg = 0;
      break;
  }

  return b;
}

static uint8_t ide_in(cz80m6847_data_t *cz80m6847, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case 0: // data
      b = disk_in(cz80m6847->d, DSKDAT);
      break;
    case 1: // error
      break;
    case 7: // status
      b = 0x48; // not busy, drive ready, data request ready
      break;
  }

  return b;
}

static uint8_t cz80m6847_in(computer_t *c, uint16_t port) {
  cz80m6847_data_t *cz80m6847;
  uint8_t b;

  cz80m6847 = (cz80m6847_data_t *)c->data;
  debug(DEBUG_TRACE, "CZ8047", "in 0x%04X", port);

  switch (port & 0xE0) {
    case 0xC0: // SIO
      b = sio_in(cz80m6847, port & 0x03);
      break;
    case 0xA0: // IDE
      b = ide_in(cz80m6847, port & 0x07);
      break;
    default:
      b = 0;
      break;
  }

  return b;
}

static int cz80m6847_rom(computer_t *c, int num, uint32_t size, char *name) {
  cz80m6847_data_t *cz80m6847;
  bmp_t *header;
  uint8_t *bmp, mask;
  int pitch, offset, i, j, k, col, row, x, y;
  int r = -1;

  cz80m6847 = (cz80m6847_data_t *)c->data;

  switch (num) {
    case 0:
      if (size > BMP_SIZE) size = BMP_SIZE;
      if (load_rom(cz80m6847->session, name, size, cz80m6847->bmp) != -1) {
        header = (bmp_t *)cz80m6847->bmp;
        if (header->type != 0x4D42) return -1;
        if (header->dataOffset != sizeof(bmp_t)) return -1;
        if (header->width != BMP_NUM_COLS * CHAR_WIDTH) return -1;
        if (header->height != BMP_NUM_ROWS * CHAR_HEIGHT) return -1;
        if (header->planes != 1) return -1;
        if (header->bpp != 24) return -1;
        if (header->compression != 0) return -1;

        cz80m6847->last_char = (BMP_NUM_ROWS + 1) * 32;
        bmp = &cz80m6847->bmp[sizeof(bmp_t)];
        mask = 0;
        j = 0;
        k = 0;

        // BMPs are stored upside down
        pitch = header->width * 3; // 3=RGB
        i = header->height * pitch;

        for (y = 0; y < header->height; y++) {
          i -= pitch;
          row = y / CHAR_HEIGHT;
          for (x = 0;;) {
            col = x / CHAR_WIDTH;
            offset = (row * BMP_NUM_COLS + col) * CHAR_HEIGHT;
            // if first channel is non zero then 1, otherwise 0
            mask = (mask << 1) | (bmp[i] ? 1 : 0);
            x++;
            j++;
            i += 3; // 3=RGB, skip other two channels
            if (j == CHAR_WIDTH) {
              cz80m6847->chr[offset + k] = mask;
              mask = 0;
              j = 0;
              offset += CHAR_HEIGHT;
            }
            if (x == header->width) break;
          }
          i -= pitch;
          k++;
          if (k == CHAR_HEIGHT) {
            k = 0;
          }
        }
/*
offset = 0;
char s[32];
for (i = 0; i < 192; i++) {
  debug(1, "XXX", "chr %d", i);
  for (j = 0; j < CHAR_HEIGHT; j++) {
    mask = cz80m6847->chr[offset++];
    for (k = 0; k < CHAR_WIDTH; k++) {
      s[k] = mask & 0x80 ? '1' : '0';
      mask = (mask << 1);
    }
    s[k] = 0;
    debug(1, "XXX", "%s", s);
  }
}
*/
        r = 0;
      }
      break;
    case 1:
      if (size > ROM_SIZE) size = ROM_SIZE;
      r = load_rom(cz80m6847->session, name, size, cz80m6847->rom) != -1 ? 0 : -1;
      break;
  }

  return r;
}

static int cz80m6847_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  cz80m6847_data_t *cz80m6847;

  cz80m6847 = (cz80m6847_data_t *)c->data;
  return disk_insert(cz80m6847->d, drive, skip, tracks, heads, sectors, sectorlen, sector0, name);
}

static int cz80m6847_run(computer_t *c) {
  cz80m6847_data_t *cz80m6847;

  cz80m6847 = (cz80m6847_data_t *)c->data;

  return z80_loop(cz80m6847->z, Z80_PERIOD);
}

static int cz80m6847_close(computer_t *c) {
  cz80m6847_data_t *cz80m6847;
  int r = -1;

  cz80m6847 = (cz80m6847_data_t *)c->data;

  if (cz80m6847) {
    if (cz80m6847->w) {
      if (cz80m6847->screen) {
        cz80m6847->wp->destroy_texture(cz80m6847->w, cz80m6847->screen);
      }
      cz80m6847->wp->destroy(cz80m6847->w);
    }
    disk_close(cz80m6847->d);
    z80_close(cz80m6847->z);
    xfree(cz80m6847);
    r = 0;
  }
  xfree(c);

  return r;
}

static void render(cz80m6847_data_t *cz80m6847) {
  if (cz80m6847->w) {
    cz80m6847->wp->update_texture(cz80m6847->w, cz80m6847->screen, (uint8_t *)cz80m6847->rgba);
    cz80m6847->wp->draw_texture(cz80m6847->w, cz80m6847->screen, 0, 0);
    cz80m6847->wp->render(cz80m6847->w);
  }
}

static void cz80m6847_callback(void *data, uint32_t cycles) {
  cz80m6847_data_t *cz80m6847;
  uint64_t dt, edt;
  uint16_t addr;
  int arg1, arg2;

  cz80m6847 = (cz80m6847_data_t *)data;

  switch (cz80m6847->wp->event2(cz80m6847->w, 0, &arg1, &arg2)) {
    case WINDOW_KEYDOWN:
      if (arg1 < 128) {
        if (cz80m6847->kbuflen < MAX_KBUF) {
          cz80m6847->kbuffer[cz80m6847->pkbuf] = arg1;
          cz80m6847->pkbuf = (cz80m6847->pkbuf + 1) % MAX_KBUF;
          cz80m6847->kbuflen++;
        }
      }
      break;
    case WINDOW_KEYUP:
      break;
    case -1:
      z80_stop(cz80m6847->z);
      return;
  }

  if (cz80m6847->dirty) {
    for (addr = 0; addr < 6144; addr++) {
      vdg_byte(&cz80m6847->vdg, cz80m6847->vdg_mode, addr, cz80m6847->vram[addr & 0x07FF]);
    }
    render(cz80m6847);
    cz80m6847->dirty = 0;
  }

  z80_irq(cz80m6847->z);

  cz80m6847->ticks++;
  cz80m6847->frame++;
  dt = sys_get_clock() - cz80m6847->t0;
  edt = cz80m6847->frame * HOST_PERIOD;

  if (dt < edt) {
    debug(DEBUG_TRACE, "CZ8047", "frame %d, sleep %llu", cz80m6847->frame, edt - dt);
    sys_usleep(edt - dt);
  }

  if (cz80m6847->frame == Z80_FPS) {
    cz80m6847->frame = 0;
    cz80m6847->t0 = sys_get_clock();
  }
}

static int cz80m6847_set_window(computer_t *c, window_provider_t *wp, int fullscreen) {
  cz80m6847_data_t *cz80m6847;
  int width, height;

  cz80m6847 = (cz80m6847_data_t *)c->data;

  cz80m6847->wp = wp;
  width = SCREEN_WIDTH;
  height = SCREEN_HEIGHT;

  cz80m6847->w = wp->create(ENC_RGBA, &width, &height, 2, 2, 0, fullscreen, 0, NULL, NULL);
  if (cz80m6847->w) {
    cz80m6847->screen = cz80m6847->wp->create_texture(cz80m6847->w, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!fullscreen) {
      wp->title(cz80m6847->w, "CZ80 M6847");
    }
  }

  return cz80m6847->w ? 0 : -1;
}

computer_t *cz80m6847_init(vfs_session_t *session) {
  cz80m6847_data_t *cz80m6847;
  computer_t *c = NULL;

  if ((cz80m6847 = xcalloc(1, sizeof(cz80m6847_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      cz80m6847->d = disk_init(c, 2, 0);
      cz80m6847->t0 = sys_get_clock();
      cz80m6847->z = z80_open(Z80_PERIOD, 1, cz80m6847_callback, cz80m6847, c);
      z80_reset(cz80m6847->z, 0x0000);
      //z80_debug(cz80m6847->z, 1);

      cz80m6847->vdg.vdg_clear = vdg_clear;
      cz80m6847->vdg.vdg_char = vdg_char;
      cz80m6847->color[0]  = 0xFF000000;
      cz80m6847->color[1]  = 0xFF00FF00;
      cz80m6847->color[2]  = 0xFFFFFF00;
      cz80m6847->color[3]  = 0xFF0000FF;
      cz80m6847->color[4]  = 0xFFFF0000;
      cz80m6847->color[5]  = 0xFFFFFFFF;
      cz80m6847->color[6]  = 0xFF00FFFF;
      cz80m6847->color[7]  = 0xFFFF00FF;
      cz80m6847->color[8]  = 0xFFFF8000;
      cz80m6847->dirty = 1;
      cz80m6847->ticks = 0;
      cz80m6847->session = session;
      cz80m6847->kbuflen = 0;
      cz80m6847->pkbuf = cz80m6847->gkbuf = 0;
      c->set_window = cz80m6847_set_window;
      c->disk = cz80m6847_disk;
      c->rom = cz80m6847_rom;
      c->run = cz80m6847_run;
      c->close = cz80m6847_close;
      c->getb = cz80m6847_getb;
      c->getop = cz80m6847_getb;
      c->putb = cz80m6847_putb;
      c->out = cz80m6847_out;
      c->in = cz80m6847_in;
      c->data = cz80m6847;
    } else {
      xfree(cz80m6847);
    }
  }

  return c;
}
