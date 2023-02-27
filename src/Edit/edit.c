#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "editor.h"
#include "edit.h"
#include "syntax.h"
#include "color.h"
#include "util.h"
#include "debug.h"
#include "xalloc.h"

#define LINE_SIZE 1024

#define editPluginId 'edit'

typedef enum {
  state_initial,
  state_command,
  state_d,
  state_insert,
  state_exit
} state_t;

typedef struct edit_line_t {
  char *buf;
  struct edit_line_t *prev;
  struct edit_line_t *next;
  int span;
} edit_line_t;

typedef struct {
  edit_line_t *first;
  edit_line_t *last;
  edit_line_t *current;
  edit_line_t *page;
  state_t state;
  char *filename;
  int nlines;
  int ncols, nrows;
  int col, row, line, iline, len, shown;
  int old_line, old_iline;
  int saved_col, saved_row;
  int changed;
  int ncmd;
  char *cmd;
  syntax_highlight_t *shigh;
  syntax_plugin_t *syntax;
  uint32_t foregroundColor, backgroundColor, emptyColor, hiddenColor;
} edit_t;

static const char *UNSAVED = "Unsaved changes";
static const char *INSERT  = "Insert";

static const RGBColorType foregroundColorRGB = { 0, 0xff, 0xff, 0xff };
static const RGBColorType backgroundColorRGB = { 0, 0x13, 0x32, 0x65 };
static const RGBColorType hiddenColorRGB = { 0, 0x40, 0x20, 0x20 };
static const RGBColorType emptyColorRGB  = { 0, 0x20, 0x20, 0x20 };

static void process_lines(edit_t *data, char *buf, int size) {
  edit_line_t *line = NULL;
  int i, j, k, len;
  char c;

  for (i = 0, k = 0; i < size; i++) {
    c = buf[i];
    debug(DEBUG_TRACE, "EDIT", "char %d: %c (0x%02X)", i, c, c);

    switch (c) {
      case '\r':
        break;

      case '\n':
        if (line == NULL) {
          line = xcalloc(1, sizeof(edit_line_t));
          line->buf = xcalloc(1, 1);
        } else {
          len = sys_strlen(line->buf);
          line->buf = xrealloc(line->buf, len+1);
        }
        debug(DEBUG_TRACE, "EDIT", "line %d: \"%s\"", k++, line->buf);
        if (data->first == NULL) {
          data->first = line;
          data->last = line;
        } else {
          data->last->next = line;
          line->prev = data->last;
          data->last = line;
        }
        line = NULL;
        break;

      default:
        if (line == NULL) {
          line = xcalloc(1, sizeof(edit_line_t));
          line->buf = xcalloc(1, LINE_SIZE);
          j = 0;
        }
        if (j < LINE_SIZE-1) {
          line->buf[j++] = c;
        }
        break;
    }
  }

  if (line) {
    if (line->buf) {
      len = sys_strlen(line->buf);
      line->buf = xrealloc(line->buf, len+1);
      debug(DEBUG_TRACE, "EDIT", "line %d: \"%s\"", k++, line->buf);
      if (data->first == NULL) {
        data->first = line;
        data->last = line;
      } else {
        data->last->next = line;
        line->prev = data->last;
        data->last = line;
      }
    }
  }
  data->nlines = k;
}

static void save_file(editor_t *e, edit_t *data) {
  edit_line_t *line;
  char lf = '\n';
  void *f;

  if ((f = e->fopen(e->data, data->filename, 1)) != NULL) {
    for (line = data->first; line; line = line->next) {
      e->fwrite(e->data, f, line->buf, sys_strlen(line->buf));
      e->fwrite(e->data, f, &lf, 1);
      e->fclose(e->data, f);
    }
  }
}

static void edit_print(void *data, uint32_t fg, uint32_t bg, char *s, int n) {
  editor_t *e = (editor_t *)data;

  e->color(e->data, fg, bg);
  if (s && n) e->write(e->data, s, n);
}

static void draw_line(editor_t *e, edit_t *data, char *line, int len) {
  int i;

  if (data->syntax) {
    data->syntax->syntax_begin_line(data->shigh, edit_print, e);
    for (i = 0; i < len; i++) {
      data->syntax->syntax_char(data->shigh, line[i]);
    }
    data->syntax->syntax_end_line(data->shigh);
  } else {
    edit_print(e, data->foregroundColor, data->backgroundColor, line, len);
  }
}

static void draw_page(editor_t *e, edit_t *data, int from_current, int all) {
  edit_line_t *line;
  int len, oldspan, color, draw, i;

  data->shown = 0;
  color = data->emptyColor;
  draw = all;

  for (i = 0, line = data->page; i < data->nrows-1 && line; line = line->next) {
    oldspan = line->span;
    len = sys_strlen(line->buf);
    line->span = len ? (len + data->ncols - 1) / data->ncols : 1;
    if (i+line->span > data->nrows-1) {
      debug(DEBUG_TRACE, "EDIT", "row %d: \"%s\" (span %d) does not fit", i, line->buf, line->span);
      color = data->hiddenColor;
      break;
    }
    if (line->span != oldspan) draw = 1;
    if (from_current && line == data->current) draw = 1;
    debug(DEBUG_TRACE, "EDIT", "row %d: \"%s\" (span %d)", i, line->buf, line->span);
    if (draw) {
      e->cursor(e->data, 0, i);
      draw_line(e, data, line->buf, len);
      e->clreol(e->data);
    }
    data->shown++;
    i += line->span;
  }

  // draw empty lines to the end
  for (; i < data->nrows-1; i++) {
    debug(DEBUG_TRACE, "EDIT", "row %d empty", i);
    e->cursor(e->data, 0, i);
    e->color(e->data, data->foregroundColor, color);
    e->clreol(e->data);
  }

  e->cursor(e->data, 0, i);
  e->color(e->data, data->foregroundColor, data->emptyColor);
  e->clreol(e->data);
}

static void draw_status(editor_t *e, edit_t *data, char *status, int inverse) {
  int col, row, len;

  col = data->col;
  row = data->row;
  e->cursor(e->data, 0, data->nrows-1);

  if (status != NULL) {
    if (inverse) {
      e->color(e->data, data->emptyColor, data->foregroundColor);
    } else {
      e->color(e->data, data->foregroundColor, data->emptyColor);
    }
    len = sys_strlen(status);
    if (len >= data->ncols) len = data->ncols-1;
    e->write(e->data, status, len);
  }
  e->color(e->data, data->foregroundColor, data->emptyColor);
  e->clreol(e->data);

  data->col = col;
  data->row = row;
  e->cursor(e->data, data->col, data->row);
}

static void draw_pos(editor_t *e, edit_t *data) {
  char buf[256];

  sys_snprintf(buf, sizeof(buf)-1, "\"%s\"  %d,%d of %d", data->filename, data->line+1, data->iline+1, data->nlines);
  draw_status(e, data, buf, 0);
}

static void adjust_cursor(editor_t *e, edit_t *data) {
  edit_line_t *line;
  int row, row0, start, end, i;

  for (line = data->current, start = data->old_line; line && line != data->page; line = line->prev, start--);

debug(1, "XXX", "adjust row=%d line=%d start=%d", data->row, data->line, start);
  row0 = 0;
  for (line = data->page, end = start, row = 0; line && row+line->span < data->nrows-1; row += line->span, line = line->next, end++) {
debug(1, "XXX", "adjust old=%d test=%d", data->old_line, end);
    if (data->old_line == end) {
      row0 = row;
debug(1, "XXX", "adjust old=%d test=%d equal, row0=%d", data->old_line, end, row);
    }
  }
  if (data->old_line == end) {
    row0 = row;
debug(1, "XXX", "adjust old=%d test=%d equal out, row0=%d", data->old_line, end, row);
  }
debug(1, "XXX", "adjust row=%d line=%d start=%d end=%d row0=%d", data->row, data->line, start, end, row0);

  if (data->line == data->old_line) {
    data->col = data->iline % data->ncols;
    data->row = row0 + data->iline / data->ncols;
debug(1, "XXX", "adjust line == old_line new row=%d line=%d start=%d end=%d row0=%d", data->row, data->line, start, end, row0);

  } else if (data->line < data->old_line) {
    if (data->line < start) {
      for (i = 0; i < data->old_line - data->line; i++) {
        data->current = data->current->prev;
        if (data->page->prev) data->page = data->page->prev;
      }
      data->len = sys_strlen(data->current->buf);
      data->col = 0;
      data->row = 0;
      for (line = data->page; line != data->current; line = line->next) {
        data->row += line->span;
      }
      draw_page(e, data, 0, 1);
    } else {
      data->row = row0;
      for (i = 0; i < data->old_line - data->line; i++) {
        data->current = data->current->prev;
        data->row -= data->current->span;
      }
      data->len = sys_strlen(data->current->buf);
      if (data->iline >= data->len) {
        data->iline = data->len ? data->len-1 : 0;
      }
      data->col = data->iline % data->ncols;
      data->row += data->iline / data->ncols;
    }

  } else {
debug(1, "XXX", "adjust line > old_line");
    if (data->line > end) {
debug(1, "XXX", "adjust line > end (%d)", data->line - data->old_line);
      for (i = 0; i < data->line - data->old_line; i++) {
        if (data->current->next == NULL) break;
        data->current = data->current->next;
        if (data->page->next) data->page = data->page->next;
      }
      data->len = sys_strlen(data->current->buf);
      data->col = 0;
      data->row = 0;
      for (line = data->page; line != data->current; line = line->next) {
debug(1, "XXX", "adjust row = %d + %d = %d", data->row, line->span, data->row + line->span);
        data->row += line->span;
      }
      draw_page(e, data, 0, 1);
    } else {
debug(1, "XXX", "adjust line <= end (%d)", data->line - data->old_line);
      data->row = row0;
      for (i = 0; i < data->line - data->old_line; i++) {
debug(1, "XXX", "adjust row = %d + %d = %d", data->row, data->current->span, data->row + data->current->span);
        data->row += data->current->span;
        data->current = data->current->next;
      }
      data->len = sys_strlen(data->current->buf);
    }
    if (data->iline >= data->len) {
      data->iline = data->len ? data->len-1 : 0;
    }
    data->col = data->iline % data->ncols;
    data->row += data->iline / data->ncols;
  }

debug(1, "XXX", "adjust final col=%d row=%d", data->col, data->row);
  e->cursor(e->data, data->col, data->row);
  draw_pos(e, data);
}

static void edit_command(editor_t *e, edit_t *data, char *cmd) {
  int i, num;
  edit_line_t *line;

  if (cmd[0] == 0) {
    data->state = state_initial;
    data->col = data->saved_col;
    data->row = data->saved_row;
    e->cursor(e->data, data->col, data->row);

  } else if (!sys_strcmp(cmd, "q") || !sys_strcmp(cmd, "q!")) {
    if (data->changed && cmd[1] == 0) {
      draw_status(e, data, (char *)UNSAVED, 1);
      data->state = state_initial;
      data->col = data->saved_col;
      data->row = data->saved_row;
      e->cursor(e->data, data->col, data->row);

    } else {
      e->color(e->data, data->foregroundColor, data->backgroundColor);
      data->state = state_exit;
    }

  } else if (!sys_strcmp(cmd, "w") || !sys_strcmp(cmd, "wq")) {
    if (data->filename) {
      save_file(e, data);
    }

    if (cmd[1] == 'q') {
      e->color(e->data, data->foregroundColor, data->backgroundColor);
      data->state = state_exit;
    }

  } else {
    data->state = state_initial;
    data->col = data->saved_col;
    data->row = data->saved_row;

    for (i = 0; cmd[i]; i++) {
      if (cmd[i] < '0' || cmd[i] > '9') break;
    }
    if (cmd[i] == 0 && data->nlines > 0) {
      num = sys_atoi(cmd);
      if (num > 0) num--;
      if (num >= data->nlines) num = data->nlines-1;
      for (i = 0, line = data->first; line; i++, line = line->next) {
        if (i == num) {
          data->old_line = data->line;
          data->old_iline = data->iline;
          data->line = i;
          data->iline = 0;
debug(1, "XXX", "edit_command line=%d old_line=%d old_iline=%d", data->line, data->old_line, data->old_iline);
          adjust_cursor(e, data);
          break;
        }
      }
    }
  }
}

static int getnum(edit_t *data) {
  int i, num = 1;

  if (data->ncmd > 0) {
    num = 0;
    for (i = 0; data->cmd[i]; i++) {
      if (data->cmd[i] < '0' || data->cmd[i] > '9') break;
    }
    if (data->cmd[i] == 0) {
      num = sys_atoi(data->cmd);
    }
    data->ncmd = 0;
  }

  return num;
}
 
static void doit(editor_t *e, edit_t *data) {
  edit_line_t *line, *last;
  int i, n, found, r;
  uint16_t key;
  char c;

  data->current = data->first;
  data->len = sys_strlen(data->current->buf);
  data->iline = 0;
  data->col = data->row = 0;
  data->page = data->first;
  data->line = 0;
  draw_page(e, data, 0, 1);
  e->cursor(e->data, 0, 0);
  draw_pos(e, data);

  for (data->state = state_initial; data->state != state_exit && !pumpkin_must_finish();) {
    e->color(e->data, data->foregroundColor, data->backgroundColor);
    r = e->read(e->data, &key);
    if (r == 0) continue;
    if (r == -1) break;
    switch (data->state) {
        case state_exit:
          break;
        case state_initial:
          switch (key) {
            case 6: // page down (ctrl-f)
              n = getnum(data);
              for (i = 0; i < n && data->line < data->nlines-1; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->line += data->nrows-1;
                data->iline = 0;
                if (data->line >= data->nlines) data->line = data->nlines-1;
                adjust_cursor(e, data);
              }
              break;
            case 2: // page up (ctrl-b)
              n = getnum(data);
              for (i = 0; i < n && data->line > 0; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->line -= data->nrows-1;
                data->iline = 0;
                if (data->line < 0) data->line = 0;
                adjust_cursor(e, data);
              }
              break;
            case 25: // scroll down (ctrl-y)
              if (data->page->prev) {
                data->page = data->page->prev;
                data->old_line = data->line;
                data->old_iline = data->iline;
                draw_page(e, data, 0, 1);
                for (line = data->page, i = 0, found = 0; i < data->shown && !found; line = line->next, i++) {
                  if (line == data->current) {
                    found = 1;
debug(1, "XXX", "current line %d in sight", data->line);
                  }
                }
                if (!found) {
debug(1, "XXX", "current line %d not in sight", data->line);
                  last = line;
                  for (i = 1; line != data->current; line = line->next, i++);
                  data->line -= i;
debug(1, "XXX", "current line back %d to %d", i, data->line);
                  data->current = last;
                }
                adjust_cursor(e, data);
              }
              break;
            case 'G': // last line
              if (data->line < data->nlines-1) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->line = data->nlines-1;
                data->iline = 0;
                adjust_cursor(e, data);
              }
              break;
            case 'H': // upper left corner
              data->iline = 0;
              data->col = 0;
              data->row = 0;
              for (line = data->current; line != data->page; line = line->prev, data->line--);
              data->current = data->page;
              data->len = sys_strlen(data->current->buf);
              e->cursor(e->data, data->col, data->row);
              break;
            case 'M': // middle row
              data->iline = 0;
              data->col = 0;
              data->row = 0;
              for (line = data->current; line != data->page; line = line->prev, data->line--);
              for (i = 0; i < data->shown/2; line = line->next, data->row += line->span, data->line++, i++);
              data->current = line;
              data->len = sys_strlen(data->current->buf);
              e->cursor(e->data, data->col, data->row);
              break;
            case 'L': // lower left corner
              data->iline = 0;
              data->col = 0;
              data->row = 0;
              for (line = data->page; line != data->current; line = line->next, data->row += line->span);
              for (; data->row+line->span < data->nrows-1; line = line->next, data->line++, data->row += line->span);
              data->current = line;
              data->len = sys_strlen(data->current->buf);
              e->cursor(e->data, data->col, data->row);
              break;
            case vchrPageUp:
            case 'k': // cursor up
              n = getnum(data);
              for (i = 0; i < n && data->line > 0; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->line--;
                adjust_cursor(e, data);
              }
              break;
            case vchrPageDown:
            case 'j': // cursor down
              n = getnum(data);
debug(1, "XXX", "cursor down n=%d", n);
              for (i = 0; i < n && data->line < data->nlines-1; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->line++;
debug(1, "XXX", "cursor down to line %d", data->line);
                adjust_cursor(e, data);
              }
              break;
            case vchrRockerLeft:
            case 'h': // cursor left
              n = getnum(data);
              for (i = 0; i < n && data->iline > 0; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline--;
                adjust_cursor(e, data);
              }
              break;
            case vchrRockerRight:
            case ' ':
            case 'l': // cursor right
              n = getnum(data);
              for (i = 0; i < n && data->iline < data->len-1; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline++;
                adjust_cursor(e, data);
              }
              break;
            case 'w':
              n = getnum(data);
              for (i = 0; i < n && data->iline < data->len-1; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                for (; data->iline < data->len; data->iline++) {
                  if (data->current->buf[data->iline] == ' ') break;
                }
                for (; data->iline < data->len; data->iline++) {
                  if (data->current->buf[data->iline] != ' ') break;
                }
                if (data->iline < data->len) {
                  adjust_cursor(e, data);
                } else {
                  data->iline = data->old_iline;
                }
              }
              break;
            case 'b':
              n = getnum(data);
              for (i = 0; i < n && data->iline > 0; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline--;
                if (data->current->buf[data->iline] == ' ') {
                  for (; data->iline > 0; data->iline--) {
                    if (data->current->buf[data->iline] != ' ') break;
                  }
                }
                for (; data->iline > 0; data->iline--) {
                  if (data->current->buf[data->iline] == ' ') break;
                }
                if (data->current->buf[data->iline] == ' ') data->iline++;
                adjust_cursor(e, data);
              }
              break;
            case 'e':
              n = getnum(data);
              for (i = 0; i < n && data->iline < data->len-1; i++) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline++;
                if (data->current->buf[data->iline] == ' ') {
                  for (; data->iline < data->len-1; data->iline++) {
                    if (data->current->buf[data->iline] != ' ') break;
                  }
                }
                for (; data->iline < data->len-1; data->iline++) {
                  if (data->current->buf[data->iline] == ' ') break;
                }
                if (data->current->buf[data->iline] == ' ') data->iline--;
                adjust_cursor(e, data);
              }
              break;
            case '^':
              if (data->iline > 0) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline = 0;
                for (data->iline = 0; data->iline < data->len; data->iline++) {
                  if (data->current->buf[data->iline] != ' ') break;
                }
                adjust_cursor(e, data);
              }
              break;
            case '$':
              if (data->iline < data->len-1) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline = data->len-1;
                adjust_cursor(e, data);
              }
              break;
            case 'J': // join lines
              if (data->line < data->nlines-1) {
                data->current->buf = xrealloc(data->current->buf, data->len + 1 + sys_strlen(data->current->next->buf) + 1);
                sys_strcat(data->current->buf, " ");
                sys_strcat(data->current->buf, data->current->next->buf);
                data->len = sys_strlen(data->current->buf);
                data->nlines--;
                xfree(data->current->next->buf);
                data->current->next = data->current->next->next;
                xfree(data->current->next->prev);
                data->current->next->prev = data->current;
                data->old_line = data->line;
                data->old_iline = data->iline;
                draw_page(e, data, 1, 0);
                adjust_cursor(e, data);
                data->changed = 1;
              }
              break;
            case 'O':
              if (data->nlines > 0 && (line = xcalloc(1, sizeof(edit_line_t))) != NULL) {
                line->buf = xcalloc(1, LINE_SIZE);
                data->len = 0;
                line->next = data->current;
                line->prev = data->current->prev;
                if (line->prev) line->prev->next = line;
                data->current->prev = line;
                data->current = line;
                data->nlines++;
                data->line--;
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline = 0;
                draw_page(e, data, 1, 0);
                adjust_cursor(e, data);
                data->changed = 1;
                data->state = state_insert;
                draw_status(e, data, (char *)INSERT, 1);
              }
              break;
            case 'o':
              if (data->nlines > 0 && (line = xcalloc(1, sizeof(edit_line_t))) != NULL) {
                line->buf = xcalloc(1, LINE_SIZE);
                data->len = 0;
                line->next = data->current->next;
                if (line->next) line->next->prev = line; 
                line->prev = data->current;
                data->current->next = line;
                data->current = line;
                data->nlines++;
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->line++;
                data->iline = 0;
                draw_page(e, data, 1, 0);
                adjust_cursor(e, data);
                data->changed = 1;
                data->state = state_insert;
                draw_status(e, data, (char *)INSERT, 1);
              }
              break;
            case 'I':
              if (data->len < LINE_SIZE-1) {
                data->current->buf = xrealloc(data->current->buf, LINE_SIZE-1);
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline = 0;
                adjust_cursor(e, data);
                data->changed = 1;
                data->state = state_insert;
                draw_status(e, data, (char *)INSERT, 1);
              }
              break;
            case 'i':
              if (data->len < LINE_SIZE-1) {
                data->current->buf = xrealloc(data->current->buf, LINE_SIZE-1);
                data->changed = 1;
                data->state = state_insert;
                draw_status(e, data, (char *)INSERT, 1);
              }
              break;
            case 'A':
              if (data->len < LINE_SIZE-1) {
                data->current->buf = xrealloc(data->current->buf, LINE_SIZE-1);
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline = data->len;
                adjust_cursor(e, data);
                data->changed = 1;
                data->state = state_insert;
                draw_status(e, data, (char *)INSERT, 1);
              }
              break;
            case 'a':
              if (data->len < LINE_SIZE-1) {
                data->current->buf = xrealloc(data->current->buf, LINE_SIZE-1);
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline++;
                adjust_cursor(e, data);
                data->changed = 1;
                data->state = state_insert;
                draw_status(e, data, (char *)INSERT, 1);
              }
              break;
            case 'x':
              if (data->len > 0) {
                n = data->len - data->iline;
                for (i = 0; i < n; i++) {
                  data->current->buf[data->iline+i] = data->current->buf[data->iline+i+1];
                }
                data->current->buf[data->len] = 0;
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->len--;
                if (data->iline == data->len) data->iline--;
                data->changed = 1;
                draw_page(e, data, 1, 0);
                adjust_cursor(e, data);
              }
              break;
            case 'd':
              data->state = state_d;
              break;
            case 27:
              data->ncmd = 0;
              break;
            case ':':
              data->saved_col = data->col;
              data->saved_row = data->row;
              e->color(e->data, data->foregroundColor, data->emptyColor);
              e->cursor(e->data, 0, data->nrows-1);
              c = key;
              e->write(e->data, &c, 1);
              e->clreol(e->data);
              data->col = 1;
              data->row = data->nrows-1;
              data->ncmd = 0;
              data->state = state_command;
              break;
            default:
              if (c >= '0' && c <= '9') {
                if (data->ncmd < data->ncols-1) {
                  data->cmd[data->ncmd++] = c;
                }
              }
              break;
          }
          break;
        case state_command:
          switch (key) {
            case 27:
              data->ncmd = 0;
              // fall-through
            case '\n':
              draw_status(e, data, NULL, 0);
              data->cmd[data->ncmd] = 0;
              edit_command(e, data, data->cmd);
              data->ncmd = 0;
              break;
            case '\b':
              if (data->ncmd > 0) {
                data->ncmd--;
                c = ' ';
                data->col--;
                e->color(e->data, data->foregroundColor, data->emptyColor);
                e->cursor(e->data, data->col, data->nrows-1);
                e->write(e->data, &c, 1);
                e->cursor(e->data, data->col, data->nrows-1);
              } else {
                draw_status(e, data, NULL, 0);
                data->cmd[data->ncmd] = 0;
                edit_command(e, data, data->cmd);
                data->ncmd = 0;
              }
              break;
            default:
              if (data->ncmd < data->ncols-1 && key >= 32 && key < 256) {
                data->cmd[data->ncmd++] = key;
                e->color(e->data, data->foregroundColor, data->emptyColor);
                c = key;
                e->write(e->data, &c, 1);
                data->col++;
              }
              break;
          }
          break;
        case state_d: // d
          switch (key) {
            case 27:
              break;
            case 'd': // delete line
              n = getnum(data);
              for (i = 0; i < n && data->line < data->nlines; i++) {
                line = data->current;
                if (line->prev) line->prev->next = line->next;
                if (line->next) line->next->prev = line->prev;
                if (data->first == line) data->first = line;
                xfree(line->buf);
                data->old_line = data->line;
                data->old_iline = data->iline;
                if (line->next) {
                  data->current = line->next;
                  data->len = sys_strlen(data->current->buf);
                } else if (line->prev) {
                  data->line--;
                  data->current = line->prev;
                  data->len = sys_strlen(data->current->buf);
                } else {
                  data->line = 0;
                  data->current = NULL;
                  data->len = 0;
                }
                data->nlines--;
                xfree(line);
                draw_page(e, data, 1, 0);
                adjust_cursor(e, data);
                data->changed = 1;
              }
              break;
          }
          data->state = state_initial;
          break;
        case state_insert:
          switch (key) {
            case 27:
              draw_status(e, data, NULL, 0);
              data->len = sys_strlen(data->current->buf);
              data->current->buf = xrealloc(data->current->buf, data->len+1);
              data->old_line = data->line;
              data->old_iline = data->iline;
              if (data->iline) data->iline--;
              adjust_cursor(e, data);
              data->state = state_initial;
              break;
            case '\b':
              if (data->iline > 0) {
                data->old_line = data->line;
                data->old_iline = data->iline;
                data->iline--;
                n = data->len - data->iline;
                for (i = 0; i < n; i++) {
                  data->current->buf[data->iline-i] = data->current->buf[data->iline+1-i];
                }
                data->len--;
                draw_page(e, data, 1, 0);
                adjust_cursor(e, data);
              }
              break;
            default:
              if (data->len < LINE_SIZE-1) {
                if (key >= 32 && key < 256) {
                  data->old_line = data->line;
                  data->old_iline = data->iline;
                  if (data->iline < data->len) {
                    n = data->len - data->iline;
                    for (i = 0; i < n; i++) {
                      data->current->buf[data->len-i] = data->current->buf[data->len-i-1];
                    }
                  }
                  data->current->buf[data->iline] = key;
                  data->iline++;
                  data->len++;
                  data->current->buf[data->len] = 0;
                  draw_page(e, data, 1, 0);
                  adjust_cursor(e, data);
                }
              }
              break;
          }
          break;
    }
  }
}

static int edit_edit(editor_t *e, char *filename) {
  edit_t data;
  edit_line_t *line, *next;
  void *f;
  char *ext, *buf = NULL;
  int size, n, r = -1;

  if (e) {
    xmemset(&data, 0, sizeof(data));
    if (filename) {
      data.filename = xstrdup(filename);
      if ((f = e->fopen(e->data, filename, 0)) != NULL) {
        if ((size = e->fsize(e->data, f)) > 0) {
          if ((buf = xcalloc(1, size)) != NULL) {
            n = e->fread(e->data, f, buf, size);
            debug(DEBUG_TRACE, "EDIT", "read %d bytes of %d", n, size);
          }
        }
        e->fclose(e->data, f);
      }
    }

    if (buf) {
      e->window(e->data, &data.ncols, &data.nrows);
      process_lines(&data, buf, size);
      xfree(buf);
      data.foregroundColor = RGBToLong((RGBColorType *)&foregroundColorRGB);
      data.backgroundColor = RGBToLong((RGBColorType *)&backgroundColorRGB);
      data.emptyColor = RGBToLong((RGBColorType *)&emptyColorRGB);
      data.hiddenColor = RGBToLong((RGBColorType *)&hiddenColorRGB);
      data.cmd = xcalloc(1, data.ncols);
      if ((ext = getext(filename)) != NULL) {
        if ((data.syntax = syntax_get_plugin(ext)) != NULL) {
          data.shigh = data.syntax->syntax_create(RGBToLong((RGBColorType *)&backgroundColorRGB));
        }
      }
      e->color(e->data, data.foregroundColor, data.emptyColor);
      e->cursor(e->data, 0, data.nrows-1);
      e->clreol(e->data);
      doit(e, &data);
      for (line = data.first; line;) {
        if (line->buf) xfree(line->buf);
        next = line->next;
        xfree(line);
        line = next;
      }
      xfree(data.cmd);
      if (data.syntax) data.syntax->syntax_destroy(data.shigh);
      if (data.filename) xfree(data.filename);
    }
  }

  return r;
}

static void edit_destroy(editor_t *e) {
  if (e) {
    e->edit = NULL;
    e->destroy = NULL;
  }
}

static void *PluginMain(void *p) {
  editor_t *e = (editor_t *)p;

  if (e) {
    e->edit = edit_edit;
    e->destroy = edit_destroy;
  }

  return e;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = editPluginType;
  *id = editPluginId;

  return PluginMain;
}
