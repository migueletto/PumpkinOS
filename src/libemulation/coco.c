#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "filter.h"
#include "rom.h"
#include "computer.h"
#include "disk.h"
#include "m6809.h"
#include "coco.h"
#include "vdg.h"
#include "bmp.h"
#include "debug.h"
#include "xalloc.h"

#define COCO_CLOCK    1000000
#define COCO_FPS      60
#define COCO_PERIOD   COCO_CLOCK/COCO_FPS

#define IO_SIZE       256
#define ROM_SIZE      8192
#define RAM_SIZE      65536

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 192

#define CHAR_WIDTH    8
#define CHAR_HEIGHT   12

#define BMP_NUM_COLS  32
#define BMP_NUM_ROWS  6
#define BMP_SIZE      65536

#define CHR_SIZE      BMP_NUM_ROWS*BMP_NUM_COLS*CHAR_HEIGHT
#define LAST_CHAR     ((BMP_NUM_ROWS + 1) * BMP_NUM_COLS)

#define MAX_KBUF      256

#define SHIFT_COLUMN  0x7F
#define SHIFT_LINE    0xBF

typedef struct {
  uint8_t key;
  uint8_t line, column;
  uint8_t shift, ctrl;
} keymap_t;

typedef struct {
  m6809_t *m6809;
  uint8_t chr[CHR_SIZE];
  uint8_t bmp[BMP_SIZE];
  uint8_t cbasic[ROM_SIZE];
  uint8_t ebasic[ROM_SIZE];
  uint8_t ram[RAM_SIZE];
  uint8_t io[IO_SIZE];
  uint8_t pia1Aisddr, pia1Bisddr;
  uint8_t pia1Addr, pia1Bddr;
  uint8_t pia2Aisddr, pia2Bisddr;
  uint8_t pia2Addr, pia2Bddr;
  disk_t *d;
  int first;

  vfs_session_t *session;
  int ptr;
  surface_t *surface;
  uint32_t color[9];
  uint8_t vdg_mode;
  uint8_t sam_reg;
  vdg_t vdg;

  keymap_t keymap[256];
  uint8_t kbd_col;
  uint8_t key;
  uint8_t joystick;
  uint8_t button;
} coco_data_t;

#define KEY_BACK     8
#define KEY_ENTER   13
#define KEY_BREAK   27
#define KEY_CLEAR   WINDOW_KEY_HOME
#define KEY_INVERSE WINDOW_KEY_END

static keymap_t coco_keymap[70] = {
  {'@', 0xFE, 0xFE, 0, 0},
  {'A', 0xFE, 0xFD, 0, 0},
  {'B', 0xFE, 0xFB, 0, 0},
  {'C', 0xFE, 0xF7, 0, 0},
  {'D', 0xFE, 0xEF, 0, 0},
  {'E', 0xFE, 0xDF, 0, 0},
  {'F', 0xFE, 0xBF, 0, 0},
  {'G', 0xFE, 0x7F, 0, 0},

  {'H', 0xFD, 0xFE, 0, 0},
  {'I', 0xFD, 0xFD, 0, 0},
  {'J', 0xFD, 0xFB, 0, 0},
  {'K', 0xFD, 0xF7, 0, 0},
  {'L', 0xFD, 0xEF, 0, 0},
  {'M', 0xFD, 0xDF, 0, 0},
  {'N', 0xFD, 0xBF, 0, 0},
  {'O', 0xFD, 0x7F, 0, 0},

  {'P', 0xFB, 0xFE, 0, 0},
  {'Q', 0xFB, 0xFD, 0, 0},
  {'R', 0xFB, 0xFB, 0, 0},
  {'S', 0xFB, 0xF7, 0, 0},
  {'T', 0xFB, 0xEF, 0, 0},
  {'U', 0xFB, 0xDF, 0, 0},
  {'V', 0xFB, 0xBF, 0, 0},
  {'W', 0xFB, 0x7F, 0, 0},

  {'X', 0xF7, 0xFE, 0, 0},
  {'Y', 0xF7, 0xFD, 0, 0},
  {'Z', 0xF7, 0xFB, 0, 0},
  {WINDOW_KEY_UP, 0xF7, 0xF7, 0, 0},
  {WINDOW_KEY_DOWN, 0xF7, 0xEF, 0, 0},
  {KEY_BACK, 0xF7, 0xDF, 0, 0},
  {WINDOW_KEY_RIGHT, 0xF7, 0xBF, 0, 0},
  {' ', 0xF7, 0x7F, 0, 0},

  {'[', 0xF7, 0xEF, 1, 0},
  {']', 0xF7, 0xBF, 1, 0},

  {'0', 0xEF, 0xFE, 0, 0},
  {'1', 0xEF, 0xFD, 0, 0},
  {'2', 0xEF, 0xFB, 0, 0},
  {'3', 0xEF, 0xF7, 0, 0},
  {'4', 0xEF, 0xEF, 0, 0},
  {'5', 0xEF, 0xDF, 0, 0},
  {'6', 0xEF, 0xBF, 0, 0},
  {'7', 0xEF, 0x7F, 0, 0},

  {KEY_INVERSE, 0xEF, 0xFE, 1, 0},
  {'!', 0xEF, 0xFD, 1, 0},
  {'"', 0xEF, 0xFB, 1, 0},
  {'#', 0xEF, 0xF7, 1, 0},
  {'$', 0xEF, 0xEF, 1, 0},
  {'%', 0xEF, 0xDF, 1, 0},
  {'&', 0xEF, 0xBF, 1, 0},
  {'\'', 0xEF, 0x7F, 1, 0},  // nao funciona

  {'8', 0xDF, 0xFE, 0, 0},
  {'9', 0xDF, 0xFD, 0, 0},
  {':', 0xDF, 0xFB, 0, 0},
  {';', 0xDF, 0xF7, 0, 0},
  {',', 0xDF, 0xEF, 0, 0},
  {'-', 0xDF, 0xDF, 0, 0},
  {'.', 0xDF, 0xBF, 0, 0},
  {'/', 0xDF, 0x7F, 0, 0},

  {'(', 0xDF, 0xFE, 1, 0},
  {')', 0xDF, 0xFD, 1, 0},
  {'*', 0xDF, 0xFB, 1, 0},
  {'+', 0xDF, 0xF7, 1, 0},
  {'<', 0xDF, 0xEF, 1, 0},
  {'=', 0xDF, 0xDF, 1, 0},
  {'>', 0xDF, 0xBF, 1, 0},
  {'?', 0xDF, 0x7F, 1, 0},   // nao funciona

  {KEY_ENTER, 0xBF, 0xFE, 0, 0},
  {KEY_CLEAR, 0xBF, 0xFD, 0, 0},
  {KEY_BREAK, 0xBF, 0xFB, 0, 0},

  {0, 0, 0, 0, 0}
};

static uint8_t colors[] = {
  0x00, 0x00, 0x00,
  0x00, 0xFF, 0x00,
  0xFF, 0xFF, 0x00,
  0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00,
  0xFF, 0xFF, 0xFF,
  0x00, 0xFF, 0xFF,
  0xFF, 0x00, 0xFF,
  0xFF, 0x80, 0x00
};

static surface_t *lock_surface(coco_data_t *coco) {
  if (coco->surface) return coco->surface;
  if (coco->ptr > 0) return ptr_lock(coco->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(coco_data_t *coco) {
  if (coco->ptr > 0) ptr_unlock(coco->ptr, TAG_SURFACE);
}

static void vdg_char(void *p, uint8_t b, uint8_t fg, uint8_t bg, uint8_t col, uint8_t row) {
  coco_data_t *coco = (coco_data_t *)p;
  uint32_t x, y, caddr, i, j, cfg, cbg, color;
  uint8_t mask;

  if (b < LAST_CHAR) {
    surface_t *surface = lock_surface(coco);
    x = (surface->width - SCREEN_WIDTH*2) / 2 + col * CHAR_WIDTH*2;
    y = (surface->height - SCREEN_HEIGHT*2) / 2 + row * CHAR_HEIGHT*2;
    caddr = b * CHAR_HEIGHT;
    cfg = coco->color[fg];
    cbg = coco->color[bg];

    for (i = 0; i < CHAR_HEIGHT*2; i++) {
      mask = coco->chr[caddr];
      if (i & 1) caddr++;
      for (j = 0; j < CHAR_WIDTH*2; j++) {
        color = (mask & 0x80) ? cfg : cbg;
        surface->setpixel(surface->data, x+j, y+i, color);
        if (j & 1) mask <<= 1;
      }
    }
    unlock_surface(coco);
  }
}

static void vdg_clear(void *p, uint8_t c, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1) {
  coco_data_t *coco = (coco_data_t *)p;
  surface_t *surface = lock_surface(coco);
  uint32_t x, y, color;

  x0 += (surface->width - SCREEN_WIDTH) / 2;
  y0 += (surface->height - SCREEN_HEIGHT) / 2;
  color = coco->color[c];

  for (y = y0; y < y1; y++) {
    for (x = x0; x < x1; x++) {
      surface->setpixel(surface->data, x, y, color);
    }
  }
  unlock_surface(coco);
}

static uint8_t pia1A_read(coco_data_t *coco) {
  uint8_t b = 0x7F;

  if (coco->button) {
    b = (coco->joystick == 1) ? 0x7E : 0x7D;

  } else if (coco->key && coco->kbd_col != 0xFF) {
    if ((coco->kbd_col ^ 0xFF) & (coco->keymap[coco->key].column ^ 0xFF)) {
      b = coco->keymap[coco->key].line & 0x7F;
    }
    if (coco->kbd_col == SHIFT_COLUMN && coco->keymap[coco->key].shift) {
      b &= SHIFT_LINE;
    }
  }

  //if (coco->joystick == joy_control && joy_coord[joy_axis] > da_value)
    //b |= 0x80;

  return b;
}

static uint8_t pia1B_read(coco_data_t *coco) {
  return coco->io[0x02];
}

static uint8_t pia2A_read(coco_data_t *coco) {
  return coco->io[0x20] & 0xFE; // bit 0: cas input
}

static uint8_t pia2B_read(coco_data_t *coco) {
  return (coco->io[0x22] & 0xFA) | 0x04;
}

static uint8_t io_read(coco_data_t *coco, uint8_t addr) {
  uint8_t b;

  switch (addr) {
    case 0x00:
      b = coco->pia1Aisddr ? coco->pia1Addr : pia1A_read(coco);
      break;
    case 0x02:
      b = coco->pia1Bisddr ? coco->pia1Bddr : pia1B_read(coco);
      break;
    case 0x20:
      b = coco->pia2Aisddr ? coco->pia2Addr : pia2A_read(coco);
      break;
    case 0x22:
      b = coco->pia2Bisddr ? coco->pia2Bddr : pia2B_read(coco);
      break;
    case 0x01:
    case 0x03:
    case 0x21:
    case 0x23:
      b = coco->io[addr];
      coco->io[addr] &= 0x7F;
      break;
    default:
      b = coco->io[addr];
      break;
  }

  //debug(1, "XXX", "io_read  %02X %02X", addr, b);
  return b;
}

static uint8_t coco_getb(computer_t *c, uint16_t addr) {
  coco_data_t *coco;
  uint8_t b = 0xFF;

  coco = (coco_data_t *)c->data;

       if (addr <  0x8000) b = coco->ram[addr];
  else if (addr <  0xA000) b = coco->ebasic[addr & 0x1FFF];
  else if (addr <  0xC000) b = coco->cbasic[addr & 0x1FFF];
  else if (addr <  0xFF00) b = 0xFF;
  else if (addr <  0xFF40) b = io_read(coco, addr & 0x23);
  else if (addr >= 0xFFF0) b = coco->cbasic[addr & 0x1FFF];

  return b;
}

/*
  bit 0: GM0
  bit 1: GM1
  bit 2: GM2
  bit 3: CSS
  bit 4: INT/EXT
  bit 5: A/G
*/

static void pia2B_write(coco_data_t *coco, uint8_t b) {
  coco->vdg_mode = 0;
  if (b & 0x80) coco->vdg_mode |= VDG_AG;
  if (b & 0x08) coco->vdg_mode |= VDG_CSS;
  coco->vdg_mode |= (b >> 4) & 0x07;        // GM0 GM1 GM2
  //sound1bit(b & 0x02);
}

static void io_write(coco_data_t *coco, uint8_t addr, uint8_t b) {
  //debug(1, "XXX", "io_write %02X %02X", addr, b);

  switch (addr) {
    case 0x00:
    case 0x01:
    case 0x03:
    case 0x20:
    case 0x21: // bit 3: cas motor
    case 0x23:
      coco->io[addr] = b;
      break;
    case 0x02:
      coco->io[addr] = b;
      coco->kbd_col = b;
      break;
    case 0x22:
      coco->io[addr] = b;
      pia2B_write(coco, b);
      break;
  }
}

// SAM register (16 bits)
// 0123 4567 89AB CDEF
// vvva aaaa aapr rssm

// vvv: video mode (0=text, 1=64x64x4/128x64x2, 2=128x64x4, 3=pmode0, 4=pmode1, 5=pmode2, 6=pmode3/pmode4, 7=???
// aaaaaaa: video address 7 most significant bits
// p: ram page, only for Dragon64 (0=0x0000-0xFFFF is mapped to lower 32K RAM, 1=0x0000-0xFFFF is mapped to upper 32K RAM)
// rr: cpu rate (00=0.89MHz, 01=1.78MHz for ROM access, 10=1.78MHz with video loss, 11=undefined)
// ss: memory size (00=4K, 01=16K, 10=64K, 11=???)
// m: all ram mode flag (0=32K RAM + 32K ROM, 1=64K RAM)

static void sam_write(coco_data_t *coco, uint8_t reg) {
  if (reg & 1) {
    coco->sam_reg |= (1 << (reg >> 1));
  } else {
    coco->sam_reg &= ~(1 << (reg >> 1));
  }
}

static void coco_putb(computer_t *c, uint16_t addr, uint8_t b) {
  coco_data_t *coco;

  coco = (coco_data_t *)c->data;

  if (addr < 0x8000) {
    coco->ram[addr] = b;

  } else if (addr >= 0xFF00 && addr < 0xFF40) {
    io_write(coco, addr & 0x23, b);

  } else if (addr >= 0xFFC0 && addr < 0xFFE0) {
    sam_write(coco, addr - 0xFFC0);
  }
}

static int coco_rom(computer_t *c, int num, uint32_t size, char *name) {
  coco_data_t *coco;
  int r = -1;

  coco = (coco_data_t *)c->data;

  switch (num) {
    case 0:
      if (size > BMP_SIZE) size = BMP_SIZE;
      if (load_rom(coco->session, name, size, coco->bmp) != -1) {
        r = bmp_decode(coco->bmp, coco->chr, BMP_NUM_COLS, BMP_NUM_ROWS, CHAR_WIDTH, CHAR_HEIGHT);
      }
      break;
    case 1:
      if (size > ROM_SIZE) size = ROM_SIZE;
      r = load_rom(coco->session, name, size, coco->cbasic) != -1 ? 0 : -1;
      break;
    case 2:
      if (size > ROM_SIZE) size = ROM_SIZE;
      r = load_rom(coco->session, name, size, coco->ebasic) != -1 ? 0 : -1;
      break;
  }

  return r;
}

static int coco_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  coco_data_t *coco;

  coco = (coco_data_t *)c->data;
  return disk_insert(coco->d, drive, skip, tracks, heads, sectors, sectorlen, sector0, name);
}

static int coco_run(computer_t *c, uint32_t us) {
  coco_data_t *coco;

  coco = (coco_data_t *)c->data;
  if (coco->first) {
    m6809_reset(coco->m6809);
    coco->first = 0;
  }

  m6809_execute(coco->m6809, (us * COCO_CLOCK) / 1000000);

  return 0;
}

static int coco_close(computer_t *c) {
  coco_data_t *coco;
  int r = -1;

  coco = (coco_data_t *)c->data;

  if (coco) {
    disk_close(coco->d);
    m6809_close(coco->m6809);
    xfree(coco);
    r = 0;
  }
  xfree(c);

  return r;
}

static void coco_callback(void *data, uint32_t count) {
  coco_data_t *coco;
  uint16_t addr;
  int ev, arg1, arg2;

  coco = (coco_data_t *)data;

  if (coco->vdg_mode & VDG_AG) {
    for (addr = 0; addr < 6144; addr++) {
      vdg_byte(&coco->vdg, coco->vdg_mode, addr, coco->ram[0x600 + addr]);
    }
  } else {
    for (addr = 0; addr < 512; addr++) {
      vdg_byte(&coco->vdg, coco->vdg_mode, addr, coco->ram[0x400 + addr]);
    }
  }

  surface_t *surface = lock_surface(coco);
  ev = surface_event(surface, 0, &arg1, &arg2);
  surface_update(surface, 0, surface->height);
  unlock_surface(coco);

  switch (ev) {
    case WINDOW_KEYDOWN:
      coco->key = arg1;
      if (coco->key >= 'a' && coco->key <= 'z') coco->key &= 0xDF;
      break;
    case WINDOW_KEYUP:
      coco->key = 0;
      break;
    case -1:
      return;
  }
}

static int coco_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  coco_data_t *coco;
  int i, j;

  coco = (coco_data_t *)c->data;
  coco->surface = surface;
  coco->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface->encoding == SURFACE_ENCODING_PALETTE) {
    for (i = 0, j = 0; i < 9; i++) {
      surface_palette(surface, i, colors[j], colors[j+1], colors[j+2]);
      coco->color[i] = i;
      j += 3;
    }
  } else {
    for (i = 0, j = 0; i < 9; i++) {
      coco->color[i] = surface_color_rgb(surface->encoding, NULL, 0, colors[j], colors[j+1], colors[j+2], 0xFF);
      j += 3;
    }
  }
  surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, coco->color[0]);

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

static void kbd_initmap(keymap_t *in, keymap_t *out) {
  int i;

  for (i = 0; i < 256; i++) {
    out[i].key = 0;
    out[i].line = 0xFF;
    out[i].column = 0xFF;
    out[i].shift = 0;
    out[i].ctrl = 0;
  }

  for (i = 0; in[i].key; i++) {
    out[in[i].key].key = in[i].key;
    out[in[i].key].line = in[i].line;
    out[in[i].key].column = in[i].column;
    out[in[i].key].shift = in[i].shift;
    out[in[i].key].ctrl = in[i].ctrl;
  }
}

computer_t *coco_init(vfs_session_t *session) {
  coco_data_t *coco;
  computer_t *c = NULL;

  if ((coco = xcalloc(1, sizeof(coco_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      coco->d = disk_init(c, 2, 0, session);
      coco->m6809 = m6809_open(COCO_PERIOD, coco_callback, coco, c);

      coco->sam_reg = (0x0400 >> 9) << 3;
      coco->vdg.vdg_clear = vdg_clear;
      coco->vdg.vdg_char = vdg_char;
      coco->vdg.p = coco;
      coco->first = 1;
      coco->session = session;
      kbd_initmap(coco_keymap, coco->keymap);
      c->set_surface = coco_set_surface;
      c->disk = coco_disk;
      c->rom = coco_rom;
      c->run = coco_run;
      c->close = coco_close;
      c->getb = coco_getb;
      c->putb = coco_putb;
      c->data = coco;
    } else {
      xfree(coco);
    }
  }

  return c;
}
