#include "sys.h"
#include "vfs.h"
#include "filter.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "cas.h"
#include "zx81cas.h"
#include "rom.h"
#include "pwindow.h"
#include "computer.h"
#include "z80.h"
#include "zx81.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define Z80_CLOCK     3250000
#define Z80_FPS       50
#define Z80_PERIOD    Z80_CLOCK/Z80_FPS
#define HOST_PERIOD   1000000/Z80_FPS

#define SCANLINE_CYCLES  207
#define VIDEO_COLUMNS    256
#define VIDEO_LINES      192
#define SCREEN_WIDTH     VIDEO_COLUMNS
#define SCREEN_HEIGHT    VIDEO_LINES
#define VIDEO_ROM_ADDR   0x1E00
#define CHARSET_WIDTH    (16*8)
#define CHARSET_HEIGHT   (4*8)

#define ROM_SIZE      0x2800
#define RAM_SIZE      0x4000

#define K(m,bit) if (down) zx81->matrix[m] &= bit; else zx81->matrix[m] |= (bit^0xFF)
#define K_SSHIFT(m,bit) K(m,bit); K(0,0xFE)

#define K_KEY(m,k,bit) case k: K(m,bit); break
#define K_KEY_SSHIFT(m,k,bit) case k: K_SSHIFT(m,bit); break

#define K_LINE(m,a,b,c,d,e) \
  K_KEY(m,a,0xFE); \
  K_KEY(m,b,0xFD); \
  K_KEY(m,c,0xFB); \
  K_KEY(m,d,0xF7); \
  K_KEY(m,e,0xEF)

#define K_LINE_SSHIFT(m,a,b,c,d,e) \
  K_KEY_SSHIFT(m,a,0xFE); \
  K_KEY_SSHIFT(m,b,0xFD); \
  K_KEY_SSHIFT(m,c,0xFB); \
  K_KEY_SSHIFT(m,d,0xF7); \
  K_KEY_SSHIFT(m,e,0xEF)

#define MAX_KEYS 16

typedef struct {
  z80_t *z;
  uint8_t ram[RAM_SIZE];
  uint8_t rom[ROM_SIZE];
  uint32_t color[2];
  int lineCntrEnabled;
  int nmiEnabled;
  int lineCntr;
  int lineCntr2;
  int charLine;
  int currentCol;
  int inverseVideo;
  int finish;
  uint8_t matrix[8];
  int ikey, okey, nkey;
  int rkey[MAX_KEYS];
  int key[MAX_KEYS];
  int cas;
  tape_t tape;
  vfs_session_t *session;
  int ptr;
  surface_t *surface;
  uint8_t charset[CHARSET_WIDTH * CHARSET_HEIGHT];
  int64_t ut;
} zx81_data_t;

#if 0
  { 'z', 0xFE, 0xFD, 0 },
  { ':', 0xFE, 0xFD, WINDOW_MOD_SHIFT },
  { 'x', 0xFE, 0xFB, 0 },
  { ';', 0xFE, 0xFB, WINDOW_MOD_SHIFT },
  { 'c', 0xFE, 0xF7, 0 },
  { '?', 0xFE, 0xF7, WINDOW_MOD_SHIFT },
  { 'v', 0xFE, 0xEF, 0 },
  { '/', 0xFE, 0xEF, WINDOW_MOD_SHIFT },
  { 'a', 0xFD, 0xFE, 0 },
  { 's', 0xFD, 0xFD, 0 },
  { 'd', 0xFD, 0xFB, 0 },
  { 'f', 0xFD, 0xF7, 0 },
  { 'g', 0xFD, 0xEF, 0 },
  { 'q', 0xFB, 0xFE, 0 },
  { 'w', 0xFB, 0xFD, 0 },
  { 'e', 0xFB, 0xFB, 0 },
  { 'r', 0xFB, 0xF7, 0 },
  { 't', 0xFB, 0xEF, 0 },
  { '1', 0xF7, 0xFE, 0 },
  { 5,   0xF7, 0xFE, WINDOW_MOD_SHIFT },  // ctrl-e -> EDIT
  { '2', 0xF7, 0xFD, 0 },
  { '3', 0xF7, 0xFB, 0 },
  { '4', 0xF7, 0xF7, 0 },
  { '5', 0xF7, 0xEF, 0 },
  { WINDOW_KEY_LEFT, 0xF7, 0xEF, WINDOW_MOD_SHIFT },
  { '0', 0xEF, 0xFE, 0 },
  { 8,   0xEF, 0xFE, WINDOW_MOD_SHIFT },
  { '9', 0xEF, 0xFD, 0 },
  { 7,   0xEF, 0xFD, WINDOW_MOD_SHIFT }, // ctrl-g -> GRAPH
  { '8', 0xEF, 0xFB, 0 },
  { WINDOW_KEY_RIGHT, 0xEF, 0xFB, WINDOW_MOD_SHIFT },
  { '7', 0xEF, 0xF7, 0 },
  { WINDOW_KEY_UP, 0xEF, 0xF7, WINDOW_MOD_SHIFT },
  { '6', 0xEF, 0xEF, 0 },
  { WINDOW_KEY_DOWN, 0xEF, 0xEF, WINDOW_MOD_SHIFT },
  { 'p', 0xDF, 0xFE, 0 },
  { '"', 0xDF, 0xFE, WINDOW_MOD_SHIFT },
  { 'o', 0xDF, 0xFD, 0 },
  { ')', 0xDF, 0xFD, WINDOW_MOD_SHIFT },
  { 'i', 0xDF, 0xFB, 0 },
  { '(', 0xDF, 0xFB, WINDOW_MOD_SHIFT },
  { 'u', 0xDF, 0xF7, 0 },
  { '$', 0xDF, 0xF7, WINDOW_MOD_SHIFT },
  { 'y', 0xDF, 0xEF, 0 },
  { 13,  0xBF, 0xFE, 0 },
  { 6,   0xBF, 0xFE, WINDOW_MOD_SHIFT }, // ctrl-f -> FUNCTION
  { 'l', 0xBF, 0xFD, 0 },
  { '=', 0xBF, 0xFD, WINDOW_MOD_SHIFT },
  { 'k', 0xBF, 0xFB, 0 },
  { '+', 0xBF, 0xFB, WINDOW_MOD_SHIFT },
  { 'j', 0xBF, 0xF7, 0 },
  { '-', 0xBF, 0xF7, WINDOW_MOD_SHIFT },
  { 'h', 0xBF, 0xEF, 0 },
  { ' ', 0x7F, 0xFE, 0 },
  { '.', 0x7F, 0xFD, 0 },
  { ',', 0x7F, 0xFD, WINDOW_MOD_SHIFT },
  { 'm', 0x7F, 0xFB, 0 },
  { '>', 0x7F, 0xFB, WINDOW_MOD_SHIFT },
  { 'n', 0x7F, 0xF7, 0 },
  { '<', 0x7F, 0xF7, WINDOW_MOD_SHIFT },
  { 'b', 0x7F, 0xEF, 0 },
  { '*', 0x7F, 0xEF, WINDOW_MOD_SHIFT },
#endif

static surface_t *lock_surface(zx81_data_t *zx81) {
  if (zx81->surface) return zx81->surface;
  if (zx81->ptr > 0) return ptr_lock(zx81->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(zx81_data_t *zx81) {
  if (zx81->ptr > 0) ptr_unlock(zx81->ptr, TAG_SURFACE);
}

static uint8_t zx81_getb(computer_t *c, uint16_t addr) {
  zx81_data_t *zx81;
  uint8_t b;

  zx81 = (zx81_data_t *)c->data;

  if (addr < 0x4000) {
    b = addr < ROM_SIZE ? zx81->rom[addr] : 0x00;
  } else {
    addr -= 0x4000;
    b = addr < RAM_SIZE ? zx81->ram[addr] : 0x00;
  }

  return b;
}

static void getKey(zx81_data_t *zx81, surface_t *surface) {
  int ev, arg1, arg2;

  while ((ev = surface_event(surface, 0, &arg1, &arg2)) != 0) {
    switch (ev) {
      case WINDOW_KEYDOWN:
        if (arg1 == 12) {
          casInit(&zx81->tape);
          zx81_casRead(zx81->session, &zx81->tape, "/app_card/PALM/Programs/CZ80/Mazogs.p");
          zx81->cas = 1;
          break;
        }
      case WINDOW_KEYUP:
        if (arg1 == 12) break;
        if (zx81->nkey < MAX_KEYS) {
          zx81->key[zx81->ikey] = (ev == WINDOW_KEYDOWN ? 0x100 : 0) | arg1;
          zx81->rkey[zx81->ikey] = 32;
          zx81->ikey++;
          if (zx81->ikey == MAX_KEYS) zx81->ikey = 0;
          zx81->nkey++;
        }
        break;
      case -1:
        zx81->finish = 1;
        break;
    }
  }
}

// Video memory is addressed by the D_FILE pointer (400Ch) in ZX80/81 system area.
// The first byte in VRAM is a HALT opcode (76h), followed by the data (one byte per character) for each of the 24 lines,
// each line is terminated by a HALT opcode also. In case that a line contains less than 32 characters,
// the HALT opcode blanks (white) the rest of the line up to the right screen border.

// Character data in range 00h..3Fh displays the 64 characters, normally black on white.
// Characters may be inverted by setting Bit 7, ie. C0h..FFh represents the same as above displayed white on black.
// The fully expanded VRAM size is 793 bytes (32x24 + 25 HALTs).

// The character set is addressed by the I register multiplied by 100h. In the ZX81 this is 1Eh for 1E00h..1FFFh.
// Address of the character in ROM is 0x1E00 + char*8 + linecntr.

// A register called linecntr is used to address the vertical position (0..7) whithin a character line.
// This register is reset during vertical retrace, and then incremented once per scanline.
// The BIOS thus needs to 'execute' each character line eight times, before starting to 'execute' the next character line.

// Horizontal Scanline Timings:
// Horizontal Display    128 cycles (32 characters, 256 pixels)
// Horizontal Blanking    64 cycles (left and right screen border)
// Horizontal Retrace     15 cycles
// Total Scanline Time   207 cycles

static void endLine(zx81_data_t *zx81, uint16_t addr) {
  if (zx81->currentCol > 0) {
    int dt = (32 - zx81->currentCol) << 2;
    z80_irqn(zx81->z, dt);
    zx81->currentCol = 0;
    zx81->charLine = (zx81->charLine + 1) % 8;

    if (zx81->lineCntrEnabled) {
      zx81->lineCntr++;
      zx81->lineCntr2 += 2;
    }
  } else {
    zx81->charLine = 0;
    z80_irq(zx81->z);
  }
}

static void displayCode(zx81_data_t *zx81, uint8_t b) {
  int idxCharset, x, y, i;
  uint32_t c;

  if (zx81->lineCntr < VIDEO_LINES) {
    // interpret the "opcode" as a character
    uint8_t inverted = zx81->inverseVideo ? (b & 0x80) == 0 : (b & 0x80) != 0;
    uint8_t code = b & 0x3F;

    idxCharset = ((code & 0xF0) << 6) + (zx81->charLine << 7) + ((code & 0x0F) << 3);
    surface_t *surface = lock_surface(zx81);
    x = (surface->width - SCREEN_WIDTH*2) / 2 + (zx81->currentCol << 4);
    y = (surface->height - SCREEN_HEIGHT*2) / 2;

    for (i = 0; i < 8; i++, x += 2) {
      c = zx81->charset[idxCharset + i];
      if (inverted) c = c ^ 1;
      c = zx81->color[c];
      surface->setarea(surface->data, x, y + zx81->lineCntr2, x + 1, y + zx81->lineCntr2 + 1, c);
    }

    unlock_surface(zx81);
    zx81->currentCol++;
  }
}

static void zx81_putb(computer_t *c, uint16_t addr, uint8_t b) {
  zx81_data_t *zx81;

  zx81 = (zx81_data_t *)c->data;

  if (addr >= 0x4000) {
    addr -= 0x4000;
    if (addr < RAM_SIZE) {
      zx81->ram[addr] = b;
    }
  }
}

static void zx81_out(computer_t *c, uint16_t port, uint16_t b) {
  zx81_data_t *zx81;

  zx81 = (zx81_data_t *)c->data;
  debug(DEBUG_TRACE, "ZX81", "out 0x%04X, 0x%02X", port, b & 0xFF);

  // Writing any data to any port terminates the Vertical Retrace period, and restarts the LINECNTR counter.
  // The retrace signal is also output to the cassette (ie. the Cassette Output becomes High).

  //System.out.printf("HSYNC enabled\n");
  zx81->lineCntrEnabled = 1;

  switch (port & 0xFF) {
    case 0xFD:
      // Writing any data to this port disables the NMI generator.
      //System.out.printf("NMI disabled\n");
      zx81->nmiEnabled = 0;
      break;
    case 0xFE:
      // Writing any data to this port enables the NMI generator.
      // NMIs are used during SLOW mode vertical blanking periods to count the number of drawn blank scanlines.
      //System.out.printf("NMI enabled\n");
      zx81->nmiEnabled = 1;
      break;
    case 0xFF:
      //waveOutput(zx81, 255);
      break;
  }
}

static uint16_t zx81_in(computer_t *c, uint16_t port) {
  zx81_data_t *zx81;
  uint8_t down, b = 0;
  int key;

  zx81 = (zx81_data_t *)c->data;
  debug(DEBUG_TRACE, "ZX81", "in 0x%04X", port);

  if ((port & 0x01) == 0x00) {
    // Reading from this port initiates the Vertical Retrace period (and accordingly, Cassette Output becomes Low),
    // and resets the LINECNTR register to zero, LINECNTR remains stopped/zero until user terminates retrace.
    // In the ZX81, all of the above happens only if NMIs are disabled.

    // 0-4  Keyboard column bits (0=Pressed)
    // 5    Not used             (1)
    // 6    Display Refresh Rate (0=60Hz, 1=50Hz)
    // 7    Cassette input       (0=Normal, 1=Pulse)

    if (!zx81->nmiEnabled) {
      //System.out.printf("HSYNC disabled\n");
      zx81->lineCntr = 0;
      zx81->lineCntr2 = 0;
      zx81->lineCntrEnabled = 0;
    }

    b = 0x3F;
    surface_t *surface = lock_surface(zx81);
    getKey(zx81, surface);
    unlock_surface(zx81);

    if (zx81->nkey > 0) {
      key = zx81->key[zx81->okey];
      down = key & 0x100 ? 1 : 0;
      key &= 0xFF;

      zx81->rkey[zx81->okey]--;
      if (zx81->rkey[zx81->okey] == 0) {
        zx81->okey++;
        if (zx81->okey == MAX_KEYS) zx81->okey = 0;
        zx81->nkey--;
      }

      switch (key) {
        K_KEY(0, 'z', 0xFD);
        K_KEY(0, 'x', 0xFB);
        K_KEY(0, 'c', 0xF7);
        K_KEY(0, 'v', 0xEF);
        K_LINE(1, 'a', 's', 'd', 'f', 'g');
        K_LINE(2, 'q', 'w', 'e', 'r', 't');
        K_LINE(3, '1', '2', '3', '4', '5');
        K_LINE(4, '0', '9', '8', '7', '6');
        K_LINE(5, 'p', 'o', 'i', 'u', 'y');
        K_LINE(6, 13,  'l', 'k', 'j', 'h');
        K_KEY(7, ' ', 0xFE);
        K_KEY(7, WINDOW_KEY_LALT, 0xFD);
        K_KEY(7, 'm', 0xFB);
        K_KEY(7, 'n', 0xF7);
        K_KEY(7, 'b', 0xEF);

        K_KEY_SSHIFT(5, '"', 0xFE);
      }
    }

    switch (port >> 8) {
      case 0xFE: b &= zx81->matrix[0]; break;
      case 0xFD: b &= zx81->matrix[1]; break;
      case 0xFB: b &= zx81->matrix[2]; break;
      case 0xF7: b &= zx81->matrix[3]; break;
      case 0xEF: b &= zx81->matrix[4]; break;
      case 0xDF: b &= zx81->matrix[5]; break;
      case 0xBF: b &= zx81->matrix[6]; break;
      case 0x7F: b &= zx81->matrix[7]; break;
    }
    
    if (Z80_FPS == 50) b |= 0x40;

    if (!zx81->lineCntrEnabled) {
      if (zx81_casInput(&zx81->tape, z80_get_event_count(zx81->z), Z80_CLOCK)) {
        b |= 0x80;
      }
    }

    if (zx81->cas && casFinished(&zx81->tape)) {
      zx81->cas = 0;
    }

    //waveOutput(zx81, 0);
  }

  return b;
}

static int zx81_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  return -1;
}

static void decodeVideoRom(zx81_data_t *zx81) {
  // Address of the character in ROM is 0x1E00 + char*8 + linecntr.
  int idx, ch, p, i, j, k;
  uint8_t b, c;

  idx = 0;
  for (i = 0; i < 32; i++) {
    for (j = 0; j < 16; j++) {
      ch = ((i >> 3) << 4) + j;
      p = (ch << 3) + (i & 0x07);
      b = zx81->rom[VIDEO_ROM_ADDR + p];
      for (k = 0; k < 8; k++) {
        c = (b & 0x80) == 0 ? 1 : 0;
        zx81->charset[idx++] = c;
        b <<= 1;
      }
    }
  }
}

static int zx81_rom(computer_t *c, int num, uint32_t size, char *name) {
  zx81_data_t *zx81;
  int r = -1;

  zx81 = (zx81_data_t *)c->data;

  if (num == 0) {
    r = load_rom(zx81->session, name, size, zx81->rom) != -1 ? 0 : -1;
    decodeVideoRom(zx81);
  }

  return r;
}

static int zx81_run(computer_t *c, uint32_t us) {
  zx81_data_t *zx81;

  zx81 = (zx81_data_t *)c->data;
  if (zx81->finish) return -1;

  return z80_loop(zx81->z, (us * Z80_CLOCK) / 1000000);
}

static int zx81_delay(computer_t *c) {
  zx81_data_t *zx81;

  zx81 = (zx81_data_t *)c->data;

  return zx81->cas ? 0 : 1;
}

static int zx81_close(computer_t *c) {
  zx81_data_t *zx81;
  int r = -1;

  zx81 = (zx81_data_t *)c->data;

  if (zx81) {
    xfree(zx81);
    r = 0;
  }
  xfree(c);

  return r;
}

static void zx81_callback(void *data, uint32_t cycles) {
  zx81_data_t *zx81;
  surface_t *surface;
  uint64_t t;

  zx81 = (zx81_data_t *)data;

  if (zx81->nmiEnabled) {
    z80_nmi(zx81->z, 0);
  }

  surface = lock_surface(zx81);

  t = sys_get_clock();
  if ((t - zx81->ut) > HOST_PERIOD) {
    surface_update(surface, 0, surface->height);
    zx81->ut = t;
  }

  unlock_surface(zx81);
}

static uint8_t zx81_getop(computer_t *c, uint16_t addr) {
  zx81_data_t *zx81;
  uint8_t b;

  zx81 = (zx81_data_t *)c->data;

  if ((addr & 0x8000) != 0) {
    // map C000-FFFF to 4000-7FFF 
    addr &= 0x7FFF;

    // retrieve the "opcode"
    b = zx81_getb(c, addr);

    // if bit 6 is 0
    if ((b & 0x40) == 0) {
      // display the "opcode"
      displayCode(zx81, b);

      // set the "opcode" to 0x00 (nop)
      b = 0;

    } else {
      endLine(zx81, addr);
    }

  } else {
    b = zx81_getb(c, addr);
  }

  return b;
}

static int zx81_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  zx81_data_t *zx81;

  zx81 = (zx81_data_t *)c->data;
  zx81->surface = surface;
  zx81->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface->encoding == SURFACE_ENCODING_PALETTE) {
    surface_palette(surface, 0, 0x00, 0x00, 0x00);
    surface_palette(surface, 1, 0xFF, 0xFF, 0xFF);
    zx81->color[0] = 0;
    zx81->color[1] = 1;
  } else {
    zx81->color[0] = surface_color_rgb(surface->encoding, NULL, 0, 0x00, 0x00, 0x00, 0xFF);
    zx81->color[1] = surface_color_rgb(surface->encoding, NULL, 0, 0xFF, 0xFF, 0xFF, 0xFF);
  }
  surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, zx81->color[1]);

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

computer_t *zx81_init(vfs_session_t *session) {
  zx81_data_t *zx81;
  computer_t *c = NULL;
  int i;

  if ((zx81 = xcalloc(1, sizeof(zx81_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      zx81->z = z80_open(SCANLINE_CYCLES, 1, zx81_callback, zx81, c);
      zx81->ut = sys_get_clock();
      zx81->lineCntrEnabled = 1;
      zx81->nmiEnabled = 1;
      zx81->inverseVideo = 0;
      zx81->session = session;
      for (i = 0; i < 8; i++) zx81->matrix[i] = 0xFF;

      c->set_surface = zx81_set_surface;
      c->disk = zx81_disk;
      c->rom = zx81_rom;
      c->run = zx81_run;
      c->delay = zx81_delay;
      c->close = zx81_close;
      c->getb = zx81_getb;
      c->getop = zx81_getop;
      c->putb = zx81_putb;
      c->out = zx81_out;
      c->in = zx81_in;
      c->data = zx81;

    } else {
      xfree(zx81);
    }
  }

  return c;
}
