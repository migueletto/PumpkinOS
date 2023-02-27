#include "sys.h"
#include "pterm.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_ARGS 8
#define ESC 27

#define ATTR_INVERSE     0x80
#define ATTR_BRIGHT      0x40
#define ATTR_UNDERSCORE  0x20
#define ATTR_BLINK       0x10

#define BG(c) ((c) & 0x07)
#define FG(c) (((c) >> 4) & 0x07)

struct pterm_t {
  pterm_callback_t *cb;
  int vt100_state, vt100_firstdigit, vt100_num, vt100_arg[MAX_ARGS+1];
  int col, row;
  int saved_col, saved_row;
  int cols, rows, size;
  int row0, row1;  // scrolling region
  int keymode;
  int lastcol;
  uint8_t attr, saved_attr, color, saved_color;
  int cursor_state, cursor_enabled;
  uint8_t *char_buffer;
  uint8_t *attr_buffer;
  uint8_t *color_buffer;
  int rgb;
  uint32_t rgb_fg, rgb_bg, rgb_saved_fg, rgb_saved_bg;
  uint32_t *rgb_fg_buffer;
  uint32_t *rgb_bg_buffer;
};

static void term_decx(pterm_t *t);
static void term_decy(pterm_t *t);
static void term_incx(pterm_t *t, int d);
static void term_incy(pterm_t *t);
static void term_savecursor(pterm_t *t);
static void term_restorecursor(pterm_t *t);
static void term_sendc(pterm_t *t, uint8_t c);

static const uint32_t colors[8] = {
  0x000000,
  0xFF0000,
  0x00FF00,
  0xFFFF00,
  0x0000FF,
  0xFF00FF,
  0x00FFFF,
  0xFFFFFF
};

pterm_t *pterm_init(int cols, int rows, int rgb) {
  pterm_t *t;

  if ((t = xcalloc(1, sizeof(pterm_t))) != NULL) {
    t->rgb = rgb;
    t->cols = cols;
    t->rows = rows;
    t->size = t->cols * t->rows;
    t->char_buffer = xcalloc(t->size, 1);
    t->attr_buffer = xcalloc(t->size, 1);

    t->vt100_state = 0;
    t->lastcol = 0;
    t->keymode = 0;  // cursor mode

    t->col = t->row = 0;
    t->row0 = 0;
    t->row1 = t->rows;
    t->saved_col = t->saved_row = 0;

    t->attr = ATTR_BRIGHT;
    t->saved_attr = t->attr;

    t->cursor_enabled = 1;
    t->cursor_state = 0;

    if (rgb) {
      pterm_setfg(t, 0x000000);
      pterm_setbg(t, 0xFFFFFF);
      t->rgb_saved_fg = t->rgb_fg;
      t->rgb_saved_bg = t->rgb_bg;
      t->rgb_fg_buffer = xcalloc(t->size, sizeof(uint32_t));
      t->rgb_bg_buffer = xcalloc(t->size, sizeof(uint32_t));
    } else {
      pterm_setfg(t, BLACK);
      pterm_setbg(t, WHITE);
      t->saved_color = t->color;
      t->color_buffer = xcalloc(t->size, 1);
    }
  }

  return t;
}

void pterm_getsize(pterm_t *t, uint8_t *cols, uint8_t *rows) {
  if (t && cols && rows) {
    *cols = t->cols;
    *rows = t->rows;
  }
}

void pterm_close(pterm_t *t) {
  if (t) {
    if (t->char_buffer) xfree(t->char_buffer);
    if (t->attr_buffer) xfree(t->attr_buffer);
    if (t->color_buffer) xfree(t->color_buffer);
    if (t->rgb_fg_buffer) xfree(t->rgb_fg_buffer);
    if (t->rgb_bg_buffer) xfree(t->rgb_bg_buffer);
    xfree(t);
  }
}

void pterm_callback(pterm_t *t, pterm_callback_t *cb) {
  t->cb = cb;
}

void pterm_setfg(pterm_t *t, uint32_t c) {
  if (t->rgb) {
    t->rgb_fg = c;
    debug(DEBUG_TRACE, "TERM", "rgb fg 0x%06X", c);
  } else {
    c &= 0x07;
    t->color &= 0x0F;
    t->color |= (c << 4);
    debug(DEBUG_TRACE, "TERM", "fg color %d color 0x%02X", c, t->color);
  }
}

static void setfg(pterm_t *t, uint32_t fg) {
  if (t->rgb) {
    pterm_setfg(t, fg < 8 ? colors[fg] : 0);
  } else {
    pterm_setfg(t, fg);
  }
}

void pterm_setbg(pterm_t *t, uint32_t c) {
  if (t->rgb) {
    t->rgb_bg = c;
    debug(DEBUG_TRACE, "TERM", "rgb bg 0x%06X", c);
  } else {
    c &= 0x07;
    t->color &= 0xF0;
    t->color |= c;
    debug(DEBUG_TRACE, "TERM", "bg color %d color 0x%02X", c, t->color);
  }
}

static void setbg(pterm_t *t, uint32_t bg) {
  if (t->rgb) {
    pterm_setbg(t, bg < 8 ? colors[bg] : 0);
  } else {
    pterm_setbg(t, bg);
  }
}

uint32_t pterm_getfg(pterm_t *t) {
  return t->rgb ? t->rgb_fg : t->color >> 4;
}

uint32_t pterm_getbg(pterm_t *t) {
  return t->rgb ? t->rgb_bg : t->color & 0x0F;
}

void pterm_cursor(pterm_t *t, int show) {
  int i;

  if (t->cursor_enabled && t->cursor_state != show) {
    t->cursor_state = show;

    if (t->cb) {
      i = t->row * t->cols + t->col;
      if (t->cursor_state) {
        if (t->rgb) {
          t->cb->draw(t->col, t->row, ' ', t->rgb_fg, t->rgb_fg, 0, t->cb->data);
        } else {
          t->cb->draw(t->col, t->row, ' ', FG(t->color), FG(t->color), 0, t->cb->data);
        }
      } else {
        if (t->rgb) {
          t->cb->draw(t->col, t->row, t->char_buffer[i], t->rgb_fg_buffer[i], t->rgb_bg_buffer[i], t->attr_buffer[i], t->cb->data);
        } else {
          t->cb->draw(t->col, t->row, t->char_buffer[i], FG(t->color_buffer[i]), BG(t->color_buffer[i]), t->attr_buffer[i], t->cb->data);
        }
      }
    }
  }
}

void pterm_cursor_blink(pterm_t *t) {
  pterm_cursor(t, !t->cursor_state);
}

void pterm_cursor_enable(pterm_t *t, int enabled) {
  int i;

  t->cursor_enabled = enabled;

  if (!t->cursor_enabled && t->cursor_state) {
    if (t->cb) {
      i = t->row * t->cols + t->col;
      if (t->rgb) {
        t->cb->draw(t->col, t->row, t->char_buffer[i], t->rgb_fg_buffer[i], t->rgb_bg_buffer[i], t->attr_buffer[i], t->cb->data);
      } else {
        t->cb->draw(t->col, t->row, t->char_buffer[i], FG(t->color_buffer[i]), BG(t->color_buffer[i]), t->attr_buffer[i], t->cb->data);
      }
    }
    t->cursor_state = 0;
  }
}

static void term_delete_char(pterm_t *t, int col, int row) {
  int i, k;

  if (t->cb) {
    if (t->rgb) {
      t->cb->draw(col, row, ' ', t->rgb_fg, t->rgb_fg, 0, t->cb->data);
    } else {
      t->cb->draw(col, row, ' ', FG(t->color), FG(t->color), 0, t->cb->data);
    }
  }

  k = row * t->cols + col;
  for (i = col; i < t->cols; i++) {
    t->char_buffer[k] = t->char_buffer[k+1];
    t->attr_buffer[k] = t->attr_buffer[k+1];
    if (t->rgb) {
      t->rgb_fg_buffer[k] = t->rgb_fg_buffer[k+1];
      t->rgb_bg_buffer[k] = t->rgb_bg_buffer[k+1];
    } else {
      t->color_buffer[k] = t->color_buffer[k+1];
    }
    k++;
  }
  t->char_buffer[k] = ' ';
  t->attr_buffer[k] = t->attr;
  if (t->rgb) {
    t->rgb_fg_buffer[k] = t->rgb_fg;
    t->rgb_bg_buffer[k] = t->rgb_bg;
  } else {
    t->color_buffer[k] = t->color;
  }
}

static void term_clear_line(pterm_t *t, int row) {
  int i, k;

  xmemset(t->char_buffer + row * t->cols, ' ', t->cols);
  xmemset(t->attr_buffer + row * t->cols, t->attr, t->cols);

  if (t->rgb) {
    for (i = 0, k = row * t->cols; i < t->cols; i++, k++) {
      t->rgb_fg_buffer[k] = t->rgb_fg;
      t->rgb_bg_buffer[k] = t->rgb_bg;
    }
    if (t->cb) t->cb->erase(0, row, t->cols, row+1, t->rgb_bg, t->attr, t->cb->data);
  } else {
    xmemset(t->color_buffer + row * t->cols, t->color, t->cols);
    if (t->cb) t->cb->erase(0, row, t->cols, row+1, BG(t->color), t->attr, t->cb->data);
  }
}

static void term_clear_line_to_end(pterm_t *t, int col, int row) {
  int i, k;

  xmemset(t->char_buffer + row * t->cols + col, ' ', t->cols - col);
  xmemset(t->attr_buffer + row * t->cols + col, t->attr, t->cols - col);

  if (t->rgb) {
    for (i = 0, k = row * t->cols + col; i < t->cols - col; i++, k++) {
      t->rgb_fg_buffer[k] = t->rgb_fg;
      t->rgb_bg_buffer[k] = t->rgb_bg;
    }
    if (t->cb) t->cb->erase(col, row, t->cols, row+1, t->rgb_bg, t->attr, t->cb->data);
  } else {
    xmemset(t->color_buffer + row * t->cols + col, t->color, t->cols - col);
    if (t->cb) t->cb->erase(col, row, t->cols, row+1, BG(t->color), t->attr, t->cb->data);
  }
}

void pterm_clreol(pterm_t *t) {
  term_clear_line_to_end(t, t->col, t->row);
}

static void term_clear_line_from_begin(pterm_t *t, int col, int row) {
  int i, k;

  xmemset(t->char_buffer + row * t->cols, ' ', col);
  xmemset(t->attr_buffer + row * t->cols, t->attr, col);

  if (t->rgb) {
    for (i = 0, k = row * t->cols; i < col; i++, k++) {
      t->rgb_fg_buffer[k] = t->rgb_fg;
      t->rgb_bg_buffer[k] = t->rgb_bg;
    }
    if (t->cb) t->cb->erase(0, row, col, row+1, t->rgb_bg, t->attr, t->cb->data);
  } else {
    xmemset(t->color_buffer + row * t->cols, t->color, col);
    if (t->cb) t->cb->erase(0, row, col, row+1, BG(t->color), t->attr, t->cb->data);
  }
}

void pterm_cls(pterm_t *t) {
  int i;

  xmemset(t->char_buffer, ' ', t->size);
  xmemset(t->attr_buffer, t->attr, t->size);

  if (t->rgb) {
    for (i = 0; i < t->size; i++) {
      t->rgb_fg_buffer[i] = t->rgb_fg;
      t->rgb_bg_buffer[i] = t->rgb_bg;
    }
    if (t->cb) t->cb->erase(0, 0, t->cols, t->rows, t->rgb_bg, t->attr, t->cb->data);
  } else {
    xmemset(t->color_buffer, t->color, t->size);
    if (t->cb) t->cb->erase(0, 0, t->cols, t->rows, BG(t->color), t->attr, t->cb->data);
  }
}

static void pterm_clear_screen_to_end(pterm_t *t, int col, int row) {
  int i;

  xmemset(t->char_buffer + row * t->cols + col, ' ', t->cols - col);
  xmemset(t->attr_buffer + row * t->cols + col, t->attr, t->cols - col);
  xmemset(t->char_buffer + (row+1) * t->cols, ' ', (t->rows - row - 1) * t->cols);
  xmemset(t->attr_buffer + (row+1) * t->cols, t->attr, (t->rows - row - 1) * t->cols);

  if (t->rgb) {
    for (i = row * t->cols + col; i < t->size; i++) {
      t->rgb_fg_buffer[i] = t->rgb_fg;
      t->rgb_bg_buffer[i] = t->rgb_bg;
    }
    if (t->cb) {
      t->cb->erase(col, row, t->cols, row+1, t->rgb_bg, t->attr, t->cb->data);
      t->cb->erase(0, row+1, t->cols, t->rows, t->rgb_bg, t->attr, t->cb->data);
    }
  } else {
    xmemset(t->color_buffer + row * t->cols + col, t->color, t->cols - col);
    xmemset(t->color_buffer + (row+1) * t->cols, t->color, (t->rows - row - 1) * t->cols);
    if (t->cb) {
      t->cb->erase(col, row, t->cols, row+1, BG(t->color), t->attr, t->cb->data);
      t->cb->erase(0, row+1, t->cols, t->rows, BG(t->color), t->attr, t->cb->data);
    }
  }
}

static void pterm_clear_screen_from_begin(pterm_t *t, int col, int row) {
  int i;

  xmemset(t->char_buffer, ' ', (row - 1) * t->cols);
  xmemset(t->attr_buffer, t->attr, (row - 1) * t->cols);
  xmemset(t->char_buffer + row * t->cols, ' ', col);
  xmemset(t->attr_buffer + row * t->cols, t->attr, col);

  if (t->rgb) {
    for (i = 0; i < row * t->cols + col; i++) {
      t->rgb_fg_buffer[i] = t->rgb_fg;
      t->rgb_bg_buffer[i] = t->rgb_bg;
    }
    if (t->cb) {
      t->cb->erase(0, 0, t->cols, row, t->rgb_bg, t->attr, t->cb->data);
      t->cb->erase(0, row, col, row+1, t->rgb_bg, t->attr, t->cb->data);
    }
  } else {
    xmemset(t->color_buffer, t->color, (row - 1) * t->cols);
    xmemset(t->color_buffer + row * t->cols, t->color, col);
    if (t->cb) {
      t->cb->erase(0, 0, t->cols, row, BG(t->color), t->attr, t->cb->data);
      t->cb->erase(0, row, col, row+1, BG(t->color), t->attr, t->cb->data);
    }
  }
}

static void term_scroll_up(pterm_t *t, int row0, int row1) {
  int col, row, i, k1, k2;

  k1 = row0 * t->cols;
  k2 = (row1-1) * t->cols;
  for (i = k1; i < k2; i++) {
    t->char_buffer[i] = t->char_buffer[i+t->cols];
    t->attr_buffer[i] = t->attr_buffer[i+t->cols];
    if (t->rgb) {
      t->rgb_fg_buffer[i] = t->rgb_fg_buffer[i+t->cols];
      t->rgb_bg_buffer[i] = t->rgb_bg_buffer[i+t->cols];
    } else {
      t->color_buffer[i] = t->color_buffer[i+t->cols];
    }
  }
  k2 += t->cols;
  for (; i < k2; i++) {
    t->char_buffer[i] = ' ';
    t->attr_buffer[i] = t->attr;
    if (t->rgb) {
      t->rgb_fg_buffer[i] = t->rgb_fg;
      t->rgb_bg_buffer[i] = t->rgb_bg;
    } else {
      t->color_buffer[i] = t->color;
    }
  }

  if (t->cb) {
    for (row = row0, i = row0 * t->cols; row < row1-1; row++) {
      for (col = 0; col < t->cols; col++, i++) {
        if (t->rgb) {
          t->cb->draw(col, row, t->char_buffer[i], t->rgb_fg_buffer[i], t->rgb_bg_buffer[i], t->attr_buffer[i], t->cb->data);
        } else {
          t->cb->draw(col, row, t->char_buffer[i], FG(t->color_buffer[i]), BG(t->color_buffer[i]), t->attr_buffer[i], t->cb->data);
        }
      }
    }
    if (t->rgb) {
      t->cb->erase(0, row, t->cols, row+1, t->rgb_bg, t->attr, t->cb->data);
    } else {
      t->cb->erase(0, row, t->cols, row+1, BG(t->color), t->attr, t->cb->data);
    }
  }
}

static void term_scroll_down(pterm_t *t, int row0, int row1) {
  int col, row, i, k1, k2;

  k1 = (row0+1) * t->cols;
  k2 = row1 * t->cols;
  for (i = k1; i < k2; i++) {
    t->char_buffer[i] = t->char_buffer[i-t->cols];
    t->attr_buffer[i] = t->attr_buffer[i-t->cols];
    t->color_buffer[i] = t->color_buffer[i-t->cols];
  }
  k2 = k1;
  k1 -= t->cols;
  for (; i < k2; i++) {
    t->char_buffer[i] = ' ';
    t->attr_buffer[i] = t->attr;
    t->color_buffer[i] = t->color;
  }

  if (t->cb) {
    for (row = row0+1, i = (row0+1) * t->cols; row < row1; row++) {
      for (col = 0; col < t->cols; col++, i++) {
        t->cb->draw(col, row, t->char_buffer[i], FG(t->color_buffer[i]), BG(t->color_buffer[i]), t->attr_buffer[i], t->cb->data);
      }
    }
    t->cb->erase(0, row0, t->cols, row0+1, BG(t->color), t->attr, t->cb->data);
  }
}

// sbbs UTF8 probe sequence: 0xEF 0xBB 0xBF
// ESC[!  : query RIPscrip version number. Reply:
//   RIPSCRIPxxyyzz where xx is equal to the major version
//   number (zero padded), yy is equal to the minor version
//   number (zero padded), and zz equals the revision code
// ESC[0! : same as ESC[!
// ESC[1! : disables all RIPscrip processing
// ESC[2! : enables RIPscrip processing

void pterm_send(pterm_t *t, uint8_t *buf, int n) {
  int k, i, m;
  uint8_t c;
  char abuf[32];
  char dbuf[256];

  for (k = 0; k < n; k++) {
    c = buf[k];

    switch (t->vt100_state) {
      case 0:
        if (c == ESC) {
          t->vt100_num = 0;
          t->vt100_firstdigit = 1;
          t->vt100_arg[0] = 0;
          t->vt100_state = 1;
        } else {
          debug(DEBUG_TRACE, "TERM", "char 0x%02X '%c'", c, c);
          term_sendc(t, c);
        }
        break;

      case 1:
        t->vt100_state = 0;
        switch (c) {
          case '[':
            t->vt100_state = 2;
            break;
          case 'D':  // Scroll text up
            debug(DEBUG_TRACE, "TERM", "ESC D (scroll up)");
            term_scroll_up(t, t->row0, t->row1);
            break;
          case 'E':  // Newline (behaves like cr followed by do)
            debug(DEBUG_TRACE, "TERM", "ESC E (new line)");
            term_sendc(t, '\r');
            term_sendc(t, '\n');
            break;
          case 'M':  // Scroll text down
            debug(DEBUG_TRACE, "TERM", "ESC M (scroll down)");
            term_scroll_down(t, t->row0, t->row1);
            break;
          case '7':  // Save cursor position & attributes
            debug(DEBUG_TRACE, "TERM", "ESC 7 (save cursor)");
            term_savecursor(t);
            break;
          case '8':  // Restore cursor position & attributes
            debug(DEBUG_TRACE, "TERM", "ESC 8 (restore cursor)");
            term_restorecursor(t);
            break;
          case '=':
          case '>':
            debug(DEBUG_TRACE, "TERM", "ESC %c", c);
            break;
          case '(':
          case ')':  // Start/End alternate character set
          case '#':
            debug(DEBUG_TRACE, "TERM", "ESC %c (alternate charset)", c);
            t->vt100_state = 3;
            break;
        }
        break;

      case 2:
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
          sys_strcpy(dbuf, "ESC [");
          for (i = 0; i < t->vt100_num; i++) {
            if (i > 0) sys_strcat(dbuf, ";");
            sys_snprintf(&dbuf[sys_strlen(dbuf)], sizeof(dbuf)-sys_strlen(dbuf)-1, "%d", t->vt100_arg[i]);
          }
          i =sys_strlen(dbuf);
          dbuf[i++] = c;
          dbuf[i++] = 0;
          debug(DEBUG_TRACE, "TERM", "%s", dbuf);
        }
        t->vt100_state = 0;

        switch (c) {
          case 'A':  // Upline (cursor up)
            m = t->vt100_num ? t->vt100_arg[0] : 1;   
            for (i = 0; i < m; i++) term_decy(t);
            break;
          case 'B':  // Down one line
            m = t->vt100_num ? t->vt100_arg[0] : 1;   
            for (i = 0; i < m; i++) term_sendc(t, '\n');
            break;
          case 'C':
            // Non-destructive space (cursor right)
            m = t->vt100_num ? t->vt100_arg[0] : 1;   
            for (i = 0; i < m; i++) {
              if (t->col < t->cols-1) term_incx(t, 1);
            }
            break;
          case 'D':  // Move cursor left n positions
            m = t->vt100_num ? t->vt100_arg[0] : 1;   
            for (i = 0; i < m; i++) {
              term_decx(t);
            }
            t->lastcol = 0;
            break;
          case 'H':
          case 'f':
            if (t->vt100_num == 2) { // Screen-relative cursor motion
              pterm_sety(t, t->vt100_arg[0]-1);
              pterm_setx(t, t->vt100_arg[1]-1);
            } else if (t->vt100_num == 0) { // Home cursor
              pterm_home(t);
            }
            t->lastcol = 0;
            break;
          case 'J':  // Clear display
            if (!t->vt100_num) {
              t->vt100_arg[0] = 0;
            }
            switch (t->vt100_arg[0]) {
              case 0: pterm_clear_screen_to_end(t, t->col, t->row); break;
              case 1: pterm_clear_screen_from_begin(t, t->col, t->row); break;
              case 2: pterm_cls(t);
            }
            t->lastcol = 0;
            break;
          case 'K':  // Clear line
            if (!t->vt100_num) {
              t->vt100_arg[0] = 0;
            }
            switch (t->vt100_arg[0]) {
              case 0: term_clear_line_to_end(t, t->col, t->row); break;
              case 1: term_clear_line_from_begin(t, t->col, t->row); break;
              case 2: term_clear_line(t, t->row);
            }
            t->lastcol = 0;
            break;
          case 'h':
            if (t->vt100_num == 1) switch (t->vt100_arg[0]) {
              case 1:
                t->keymode = 1; // application
                break;
              case 25:
                pterm_cursor_enable(t, 1);
                break;
              case 38:
                pterm_cursor_enable(t, 0);
                break;
            }
            break;
          case 'l':
            if (t->vt100_num == 1) switch (t->vt100_arg[0]) {
              case 1:
                t->keymode = 0; // cursor
                break;
              case 25:
                pterm_cursor_enable(t, 0);
                break;
            }
            break;
          case 'm':
            if (t->vt100_num == 0) {
              // Turn off all attributes
              t->attr = 0;
              setfg(t, WHITE);
              setbg(t, BLACK);
            } else for (i = 0; i < t->vt100_num; i++) switch (t->vt100_arg[i]) {
              case 0:  // Turn off all attributes
                t->attr = 0;
                setfg(t, WHITE);
                setbg(t, BLACK);
                break;
              case 1:  // Turn on bold (extra bright) attribute
                t->attr |= ATTR_BRIGHT;
                debug(DEBUG_TRACE, "TERM", "bright attr 0x%02X", t->attr);
                break;
              case 4:  // Start underscore mode
                t->attr |= ATTR_UNDERSCORE;
                debug(DEBUG_TRACE, "TERM", "underscore attr 0x%02X", t->attr);
                break;
              case 5:  // Turn on blinking attribute
                t->attr |= ATTR_BLINK;
                debug(DEBUG_TRACE, "TERM", "blink attr 0x%02X", t->attr);
                break;
              case 7:  // Turn on inverse-video attribute
                t->attr |= ATTR_INVERSE;
                debug(DEBUG_TRACE, "TERM", "inverse attr 0x%02X", t->attr);
                break;
              case 30:
              case 31:
              case 32:
              case 33:
              case 34:
              case 35:
              case 36:
              case 37:
                setfg(t, t->vt100_arg[i] - 30);
                break;
              case 40:
              case 41:
              case 42:
              case 43:
              case 44:
              case 45:
              case 46:
              case 47:
                setbg(t, t->vt100_arg[i] - 40);
                break;
            }
            break;
          case 's':
            term_savecursor(t);
            break;
          case 'u':
            term_restorecursor(t);
            break;
          case 'c':
            if (t->vt100_num == 1 && t->vt100_arg[0] == 0) {
              // request CTerm version
            }
            break;
          case 'n':
            if (t->vt100_num == 1 && t->vt100_arg[0] == 6) {
              // Query cursor position
              sys_snprintf(abuf, sizeof(abuf)-1, "%c[%d;%dR", ESC, t->row + 1, t->col + 1);
              if (t->cb) t->cb->reply(abuf, sys_strlen(abuf), t->cb->data);
            }
            break;
          case 'r':  // Change scrolling region (VT100)
            if (t->vt100_num != 2) {
              t->vt100_arg[0] = 1;
              t->vt100_arg[1] = t->rows;
            } else if (t->vt100_arg[1] > t->rows) {
              t->vt100_arg[1] = t->rows;
            }
            t->row0 = t->vt100_arg[0]-1;
            t->row1 = t->vt100_arg[1]-1;
            t->rows = t->vt100_arg[1]; // XXX
            break;
          case 'L':
            // Insert line(s)
            if (!t->vt100_num) {
              t->vt100_arg[0] = 1;
            }
            for (i = 0; i < t->vt100_arg[0]; i++) {
              term_scroll_down(t, t->row, t->rows);
            }
            break;
          case 'M':
            // Delete line(s)
            if (!t->vt100_num) {
              t->vt100_arg[0] = 1;
            }
            for (i = 0; i < t->vt100_arg[0]; i++) {
              term_scroll_up(t, t->row, t->rows);
            }
            break;
          case 'P':
            // Delete char(s)
            if (!t->vt100_num) {
              t->vt100_arg[0] = 1;
            }
            for (i = 0; i < t->vt100_arg[0]; i++) {
              term_delete_char(t, t->col, t->row);
            }
            break;
          case '?':
            t->vt100_state = 2;
            break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if (t->vt100_firstdigit) {
              t->vt100_num++;
              t->vt100_arg[t->vt100_num] = 0;
              t->vt100_firstdigit = 0;
            }
            t->vt100_arg[t->vt100_num-1] *= 10;
            t->vt100_arg[t->vt100_num-1] += c - '0';
            t->vt100_state = 2;
            break;
          case ';':
            t->vt100_firstdigit = 1;
            t->vt100_state = 2;
            break;
        }
        break;

      case 3:
        t->vt100_state = 0;
        break;
    }
  }
}

static void term_draw_char_color(pterm_t *t, uint32_t c, uint8_t color, uint8_t attr, int col, int row) {
  uint32_t c1, c2;

  if (attr & ATTR_INVERSE) {
    c1 = BG(color);
    c2 = FG(color);
  } else {
    c1 = FG(color);
    c2 = BG(color);
  }

  //debug(DEBUG_TRACE, "TERM", "draw '%c' fg %d bg %d bright %d", c, c1, c2, attr & ATTR_BRIGHT ? 1 : 0);
  if (t->cb) t->cb->draw(col, row, c, c1, c2, attr, t->cb->data);

  t->char_buffer[row * t->cols + col] = c;
  t->attr_buffer[row * t->cols + col] = attr;
  t->color_buffer[row * t->cols + col] = color;
}

static void term_draw_char_rgb(pterm_t *t, uint32_t c, uint32_t fg, uint32_t bg, uint8_t attr, int col, int row) {
  uint32_t c1, c2;

  if (attr & ATTR_INVERSE) {
    c1 = bg;
    c2 = fg;
  } else {
    c1 = fg;
    c2 = bg;
  }

  if (t->cb) t->cb->draw(col, row, c, c1, c2, attr, t->cb->data);

  t->char_buffer[row * t->cols + col] = c;
  t->attr_buffer[row * t->cols + col] = attr;
  t->rgb_fg_buffer[row * t->cols + col] = fg;
  t->rgb_bg_buffer[row * t->cols + col] = bg;
}

static void term_sendc(pterm_t *t, uint8_t c) {
  int draw, lxt, lyt, n;

  draw = 0;

  switch (c) {
    case 0:
      break;
    case 7:
      // beep
      break;
    case 8:
      if (t->col) {
        if (t->lastcol) {
          t->lastcol = 0;
        } else {
          term_decx(t);
          if (t->rgb) {
            term_draw_char_rgb(t, ' ', t->rgb_fg, t->rgb_bg, t->attr, t->col, t->row);
          } else {
            term_draw_char_color(t, ' ', t->color, t->attr, t->col, t->row);
          }
        }
      }
      break;
    case 9:
      n = 8 - (t->col % 8);
      if (t->col+n == t->cols) {
        n--;
      }
      term_incx(t, n);
      break;
    case 10:
      term_incy(t);
      break;
    case 12:
      pterm_cls(t);
      break;
    case 13:
      pterm_setx(t, 0);
      t->lastcol = 0;
      break;
    default:
      draw = 1;
      lxt = t->col;
      lyt = t->row;
      if (lxt < (t->cols-1)) {
        term_incx(t, 1);
      } else {
        if (t->lastcol) {
          lxt = 0;
          pterm_setx(t, 1);
          term_incy(t);
          if (t->row >= t->rows) {
            term_scroll_up(t, t->row0, t->row1);
            term_decy(t);
          }
          lyt = t->row;
          t->lastcol = 0;
        } else {
          t->lastcol = 1;
        }
      }
      if (t->col >= t->cols) {
        pterm_setx(t, 0);
        term_incy(t);
      }
  }

  if (draw && lyt < t->rows) {
    if (t->rgb) {
      term_draw_char_rgb(t, c, t->rgb_fg, t->rgb_bg, t->attr, lxt, lyt);
    } else {
      term_draw_char_color(t, c, t->color, t->attr, lxt, lyt);
    }
  }

  if (t->row >= t->rows) {
    term_scroll_up(t, t->row0, t->row1);
    term_decy(t);
  }
}

static void term_restorecursor(pterm_t *t) {
  t->col = t->saved_col;
  t->row = t->saved_row;
  t->attr = t->saved_attr;
  t->color = t->saved_color;
}

static void term_savecursor(pterm_t *t) {
  t->saved_col = t->col;
  t->saved_row = t->row;
  t->saved_attr = t->attr;
  t->saved_color = t->color;
}

void pterm_setx(pterm_t *t, int col) {
  int e = t->cursor_enabled;
  pterm_cursor_enable(t, 0);
  if (col >= t->cols) {
    col = t->cols-1;
  }
  t->col = col;
  pterm_cursor_enable(t, e);
}

void pterm_sety(pterm_t *t, int row) {
  int e = t->cursor_enabled;
  pterm_cursor_enable(t, 0);
  if (row >= t->rows) {
    row = t->rows-1;
  }
  t->row = row;
  pterm_cursor_enable(t, e);
}

static void term_decx(pterm_t *t) {
  int e = t->cursor_enabled;
  pterm_cursor_enable(t, 0);
  if (t->col) t->col--;
  pterm_cursor_enable(t, e);
}

static void term_decy(pterm_t *t) {
  int e = t->cursor_enabled;
  pterm_cursor_enable(t, 0);
  if (t->row) t->row--;
  pterm_cursor_enable(t, e);
}

static void term_incx(pterm_t *t, int d) {
  int e = t->cursor_enabled;
  pterm_cursor_enable(t, 0);
  t->col += d;
  pterm_cursor_enable(t, e);
}

static void term_incy(pterm_t *t) {
  int e = t->cursor_enabled;
  pterm_cursor_enable(t, 0);
  t->row++;
  pterm_cursor_enable(t, e);
}

void pterm_home(pterm_t *t) {
  pterm_setx(t, 0);
  pterm_sety(t, 0);
}

void pterm_getchar(pterm_t *t, uint32_t index, uint8_t *code, uint32_t *fg, uint32_t *bg) {
  if (index < t->size) {
    *code = t->char_buffer[index];
    if (t->rgb) {
      *fg = (t->attr_buffer[index] & ATTR_INVERSE) ? t->rgb_bg_buffer[index] : t->rgb_fg_buffer[index];
      *bg = (t->attr_buffer[index] & ATTR_INVERSE) ? t->rgb_fg_buffer[index] : t->rgb_bg_buffer[index];
    } else {
      *fg = (t->attr_buffer[index] & ATTR_INVERSE) ? (t->color_buffer[index] & 0x07) : (t->color_buffer[index] >> 4) & 0x07;
      *bg = (t->attr_buffer[index] & ATTR_INVERSE) ? (t->color_buffer[index] >> 4) & 0x07 : (t->color_buffer[index] & 0x07);
    }
  }
}

int pterm_getcursor(pterm_t *t, uint8_t *col, uint8_t *row) {
  *col = t->col;
  *row = t->row;
  return t->cursor_state;
}

char pterm_getstate(pterm_t *t) {
  char c;

  switch (t->vt100_state) {
    case 0: c = 'a'; break;
    case 1: c = '['; break;
    case 2: c = 'C'; break;
    case 3: c = '#'; break;
  }

  return c;
}
