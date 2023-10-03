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
#include "m6809.h"
#include "bcoco.h"
#include "vdg.h"
#include "bmp.h"
#include "debug.h"
#include "xalloc.h"

#define COCO_CLOCK    1000000
#define COCO_FPS      60
#define COCO_PERIOD   COCO_CLOCK/COCO_FPS
#define HOST_PERIOD   1000000/COCO_FPS

#define IO_SIZE       256
#define ROM_SIZE      32768
#define RAM_SIZE      131072
#define VRAM_SIZE     8192

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 192

#define CHAR_WIDTH    8
#define CHAR_HEIGHT   12

#define CHR_SIZE      6*32*CHAR_HEIGHT

#define BMP_NUM_COLS  32
#define BMP_NUM_ROWS  6
#define BMP_SIZE      65536

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
  uint8_t ram[RAM_SIZE];
  uint8_t rom[ROM_SIZE];
  uint8_t vram[VRAM_SIZE];
  uint8_t io[IO_SIZE];
  uint8_t ms;
  int first, dirty;

  int64_t t0;
  int frame;

  vfs_session_t *session;
  window_provider_t *wp;
  window_t *w;
  texture_t *screen;
  uint32_t rgba[SCREEN_WIDTH * SCREEN_HEIGHT];
  uint32_t color[9];
  uint8_t vdg_mode;
  vdg_t vdg;

  uint8_t pia1Aisddr, pia1Bisddr;
  uint8_t pia1Addr, pia1Bddr;
  uint8_t pia2Aisddr, pia2Bisddr;
  uint8_t pia2Addr, pia2Bddr;
  keymap_t keymap[256];
  uint8_t kbd_col;
  uint8_t key;
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

static void vdg_char(void *p, uint8_t b, uint8_t fg, uint8_t bg, uint8_t col, uint8_t row) {
  coco_data_t *coco = (coco_data_t *)p;
  uint32_t x, y, caddr, index, i, j;
  uint8_t mask;

  if (b < LAST_CHAR) {
    x = col * CHAR_WIDTH;
    y =  row * CHAR_HEIGHT;
    caddr = b * CHAR_HEIGHT;
    index = y * SCREEN_WIDTH + x;
    for (i = 0; i < CHAR_HEIGHT && (y + i) < SCREEN_HEIGHT; i++) {
      mask = coco->chr[caddr++];
      for (j = 0; j < CHAR_WIDTH && (x + j) < SCREEN_WIDTH; j++, mask <<= 1) {
        if ((mask & 0x80) != 0) {
          coco->rgba[index + j] = coco->color[fg];
        } else {
          coco->rgba[index + j] = coco->color[bg];
        }
      }
      index += SCREEN_WIDTH;
    }
  }
}

static void vdg_clear(void *p, uint8_t c, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1) {
  coco_data_t *coco = (coco_data_t *)p;
  uint32_t i, j, i1, i2, d;

  i1 = y0 * SCREEN_WIDTH + x0;
  i2 = (y1-1) * SCREEN_WIDTH + x1;
  d = x1 - x0;
  for (i = i1; i < i2; i += SCREEN_WIDTH) {
    for (j = 0; j < d; j++) {
      coco->rgba[i + j] = coco->color[c];
    }
  }
}

static uint8_t pia1A_read(coco_data_t *coco) {
  uint8_t b = 0x7F;

  if (coco->key && coco->kbd_col != 0xFF) {
    if ((coco->kbd_col ^ 0xFF) & (coco->keymap[coco->key].column ^ 0xFF)) {
      b = coco->keymap[coco->key].line & 0x7F;
    }
    if (coco->kbd_col == SHIFT_COLUMN && coco->keymap[coco->key].shift) {
      b &= SHIFT_LINE;
    }
  }

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

static uint8_t pia_read(coco_data_t *coco, uint8_t addr) {
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

  //debug(1, "XXX", "pia_read  %02X %02X", addr, b);
  return b;
}

static uint8_t coco_getb(computer_t *c, uint16_t addr) {
  coco_data_t *coco;
  uint8_t b = 0xFF;

  coco = (coco_data_t *)c->data;

       if (addr <  0x8000) b = coco->ram[addr];
  else if (addr <  0xC000) b = coco->rom[addr & 0x7FFF];
  else if (addr <  0xF000) b = coco->ram[addr];
  else if (addr <  0xF800) b = coco->rom[addr & 0x7FFF];
  else if (addr <  0xFF00) b = coco->ram[addr];
  else if (addr <  0xFF40) b = pia_read(coco, addr & 0x23);
  else if (addr >= 0xFFF0) b = coco->rom[addr & 0x7FFF];

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
}

static void pia_write(coco_data_t *coco, uint8_t addr, uint8_t b) {
  //debug(1, "XXX", "pia_write %02X %02X", addr, b);

  switch (addr) {
    case 0x00:
    case 0x01:
    case 0x03:
    case 0x20:
    case 0x21:
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

static void coco_putb(computer_t *c, uint16_t addr, uint8_t b) {
  coco_data_t *coco;

  coco = (coco_data_t *)c->data;

  if (addr < 0x8000) {
    coco->ram[addr] = b;

  } else if (addr >= 0xC000 && addr < 0xDFFF) {
    if (!coco->ms) {
      coco->vram[addr & 0x1FFF] = b;
      coco->dirty = 1;
    } else {
      coco->ram[addr] = b;
    }

  } else if (addr >= 0xF800 && addr < 0xFF00) {
    coco->ram[addr] = b;

  } else if (addr >= 0xFF00 && addr < 0xFF40) {
    pia_write(coco, addr & 0x23, b);

  } else if (addr == 0xFFE0) {
    coco->ms = b & 0x01;
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
      r = load_rom(coco->session, name, size, coco->rom) != -1 ? 0 : -1;
      break;
  }

  return r;
}

static int coco_run(computer_t *c) {
  coco_data_t *coco;

  coco = (coco_data_t *)c->data;
  if (coco->first) {
    m6809_reset(coco->m6809);
    coco->first = 0;
  }

  m6809_execute(coco->m6809, COCO_PERIOD);

  return 0;
}

static int coco_close(computer_t *c) {
  coco_data_t *coco;
  int r = -1;

  coco = (coco_data_t *)c->data;

  if (coco) {
    if (coco->w) {
      if (coco->screen) {
        coco->wp->destroy_texture(coco->w, coco->screen);
      }
      coco->wp->destroy(coco->w);
    }
    m6809_close(coco->m6809);
    xfree(coco);
    r = 0;
  }
  xfree(c);

  return r;
}

static void render(coco_data_t *coco) {
  if (coco->w) {
    coco->wp->update_texture(coco->w, coco->screen, (uint8_t *)coco->rgba);
    coco->wp->draw_texture(coco->w, coco->screen, 0, 0);
    coco->wp->render(coco->w);
  }
}

static void coco_callback(void *data, uint32_t count) {
  coco_data_t *coco;
  uint64_t t, dt, edt;
  uint16_t addr;
  uint8_t key;
  int arg1, arg2;

  coco = (coco_data_t *)data;
  t = sys_get_clock();

  switch (coco->wp->event2(coco->w, 0, &arg1, &arg2)) {
    case WINDOW_KEYDOWN:
      key = arg1;
      if (key >= 'a' && key <= 'z') key &= 0xDF;
      coco->key = coco->keymap[key].key;
      break;
    case WINDOW_KEYUP:
      coco->key = 0xFF;
      break;
    case -1:
      return;
  }

  if (coco->dirty) {
    if (coco->vdg_mode & VDG_AG) {
      for (addr = 0; addr < 6144; addr++) {
        vdg_byte(&coco->vdg, coco->vdg_mode, addr, coco->vram[addr]);
      }
    } else {
      for (addr = 0; addr < 512; addr++) {
        vdg_byte(&coco->vdg, coco->vdg_mode, addr, coco->vram[addr]);
      }
    }
    render(coco);
    coco->dirty = 0;
  }

  coco->frame++;
  dt = t - coco->t0;
  edt = coco->frame * HOST_PERIOD;

  if (dt < edt) {
    debug(DEBUG_TRACE, "COCO", "frame %d, sleep %llu", coco->frame, edt - dt);
    sys_usleep(edt - dt);
  }

  if (coco->frame == COCO_FPS) {
    coco->frame = 0;
    coco->t0 = t;
  }
}

static int coco_set_window(computer_t *c, window_provider_t *wp, int fullscreen) {
  coco_data_t *coco;
  int width, height;

  coco = (coco_data_t *)c->data;

  coco->wp = wp;
  width = SCREEN_WIDTH;
  height = SCREEN_HEIGHT;

  coco->w = wp->create(ENC_RGBA, &width, &height, 2, 2, 0, fullscreen, 0, NULL);
  if (coco->w) {
    coco->screen = coco->wp->create_texture(coco->w, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!fullscreen) {
      wp->title(coco->w, "COCO");
    }
  }

  return coco->w ? 0 : -1;
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

computer_t *bcoco_init(vfs_session_t *session) {
  coco_data_t *coco;
  computer_t *c = NULL;

  if ((coco = xcalloc(1, sizeof(coco_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      coco->t0 = sys_get_clock();
      coco->m6809 = m6809_open(COCO_PERIOD, coco_callback, coco, c);

      coco->vdg.vdg_clear = vdg_clear;
      coco->vdg.vdg_char = vdg_char;
      coco->vdg.p = coco;
      coco->color[0]  = 0xFF000000;
      coco->color[1]  = 0xFF00FF00;
      coco->color[2]  = 0xFFFFFF00;
      coco->color[3]  = 0xFF0000FF;
      coco->color[4]  = 0xFFFF0000;
      coco->color[5]  = 0xFFFFFFFF;
      coco->color[6]  = 0xFF00FFFF;
      coco->color[7]  = 0xFFFF00FF;
      coco->color[8]  = 0xFFFF8000;
      coco->first = 1;
      coco->dirty = 1;
      coco->session = session;
      kbd_initmap(coco_keymap, coco->keymap);
      c->set_window = coco_set_window;
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
