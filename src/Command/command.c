#include <PalmOS.h>
#include <VFSMgr.h>
#include <time.h>

#include "script.h"
#include "pfont.h"
#include "graphic.h"
#include "ptr.h"
#include "vfs.h"
#include "sys.h"
#include "filter.h"
#include "shell.h"
#include "telnet.h"
#include "pterm.h"
#include "editor.h"
#include "peditor.h"
#include "edit.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"
#include "color.h"
#include "fill.h"
#include "resource.h"

#define TAG_DB   "cmd_db"
#define TAG_RSRC "cmd_rsrc"

#define MAXCMD 512

#define SCREEN_PEN_DOWN 1
#define SCREEN_PEN_MOVE 2
#define SCREEN_PEN_UP   3

//#define FONT font16x16Id
#define FONT font8x14Id

typedef struct {
  char *tag;
  DmOpenRef dbRef;
} command_db_t;

typedef struct {
  char *tag;
  UInt32 type;
  UInt16 id;
  MemHandle h;
} command_resource_t;

typedef struct {
  int pe;
  Int32 wait;
  char cmd[MAXCMD];
  UInt16 cmdIndex;
  FontID font;
  UInt16 fwidth, fheight;
  UInt16 ncols, nrows;
  pterm_t *t;
  pterm_callback_t cb;
  char cwd[MAXCMD];
  MemHandle fh;
  FontType *f;
  conn_filter_t *telnet;
  Coord col0, col1, row0, row1;
  Boolean moved, selected, down;
} command_data_t;

static const RGBColorType foreground = { 0, 0xFF, 0xFF, 0xFF };
static const RGBColorType background = { 0, 0x13, 0x32, 0x65 };
static const RGBColorType highlight  = { 0, 0xFF, 0xFF, 0x80 };

static void command_putc(command_data_t *data, char c) {
  uint8_t b = c;

  pterm_cursor(data->t, 0);
  pterm_send(data->t, &b, 1);
}

static void command_puts(command_data_t *data, char *s) {
  int i, n;
  char prev;

  if (s) {
    n = StrLen(s);
    for (i = 0, prev = 0; i < n; prev = s[i], i++) {
      if (s[i] == '\n' && prev != '\r') {
        command_putc(data, '\r');
      }
      command_putc(data, s[i]);
    }
  }
}

static void command_script(command_data_t *data, char *s) {
  char *val;

  if (s && s[0]) {
    val = pumpkin_script_call(data->pe, "command_eval", s);

    if (val) {
      if (val[0]) {
        command_puts(data, val);
        command_putc(data, '\r');
        command_putc(data, '\n');
      }
      xfree(val);
    }
  }
}

static void command_prompt(command_data_t *data) {
  command_puts(data, data->cwd);
  command_putc(data, '>');
}

static void command_key(command_data_t *data, UInt8 c) {
  conn_filter_t *conn, *telnet;

  telnet = data->telnet;

  if (telnet) {
    if (telnet->write(telnet, &c, 1) == -1) {
      data->telnet = NULL;
      conn = telnet->next;
      telnet_close(telnet);
      conn_close(conn);
      data->wait = SysTicksPerSecond() / 2;
    }

  } else {
    switch (c) {
      case '\b':
        if (data->cmdIndex > 0) {
          command_putc(data, '\b');
          data->cmdIndex--;
        }
        break;
      case '\n':
        command_putc(data, '\r');
        command_putc(data, '\n');
        data->cmd[data->cmdIndex] = 0;
        data->cmdIndex = 0;
        if (data->cmd[0]) {
          command_script(data, data->cmd);
        }
        command_prompt(data);
        break;
      default:
        if (data->cmdIndex < MAXCMD-1) {
          command_putc(data, c);
          data->cmd[data->cmdIndex++] = c;
        }
        break;
    }
  }
}

static Boolean MenuEvent(FormType *frm, UInt16 id) {
  command_data_t *data = pumpkin_get_data();
  MemHandle h;
  UInt16 i, length;
  //char buf[256];
  char *s;
  Boolean handled = false;

  switch (id) {
    case aboutCmd:
      AbtShowAbout(AppID);
      handled = true;
      break;

    case pasteCmd:
      if ((h = ClipboardGetItem(clipboardText, &length)) != NULL) {
        if (length > 0 && (s = MemHandleLock(h)) != NULL) {
          for (i = 0; i < length; i++) {
            command_key(data, s[i]);
          }
          MemHandleUnlock(h);
        }
      }
      handled = true;
      break;

    case copyCmd:
/*
      length = sizeof(buf);
      if (screen_selection(t, buf, &length) == 0 && length > 0) {
        ClipboardAddItem(clipboardText, buf, length);
      }
*/
      handled = true;
      break;
  }

  return handled;
}

static void command_update_line(command_data_t *data, Int16 row, Int16 col1, Int16 col2, Boolean selected) {
  RectangleType rect;
  RGBColorType rgb, sel;
  Int16 pos, col, x, y;
  uint32_t fg, bg;
  uint8_t c;

  y = row * data->fheight;
  x = col1 * data->fwidth;
  pos = row * data->ncols + col1;

  for (col = col1; col <= col2; col++, pos++) {
    pterm_getchar(data->t, pos, &c, &fg, &bg);
    if (selected) {
      sel = highlight;
    } else {
      LongToRGB(bg, &sel);
    }
    LongToRGB(fg, &rgb);
    WinSetBackColorRGB(&sel, NULL);
    WinSetTextColorRGB(&rgb, NULL);

    RctSetRectangle(&rect, x, y, data->fwidth, data->fheight);
    WinEraseRectangle(&rect, 0);
    WinPaintChar(c, x, y);
    x += data->fwidth;
  }
}

static void command_draw(command_data_t *data) {
  RGBColorType oldt, oldb;
  FontID old;
  Int16 row;

  old = FntSetFont(data->font);
  WinSetBackColorRGB(NULL, &oldb);
  WinSetTextColorRGB(NULL, &oldt);

  for (row = 0; row < data->nrows; row++) {
    command_update_line(data, row, 0, data->ncols-1, false);
  }

  WinSetBackColorRGB(&oldb, NULL);
  WinSetTextColorRGB(&oldt, NULL);
  FntSetFont(old);
}

static void command_update(command_data_t *data, Int16 row1, Int16 col1, Int16 row2, Int16 col2, Boolean selected) {
  RGBColorType oldt, oldb;
  FontID old;
  Int16 row;

  old = FntSetFont(data->font);
  WinSetBackColorRGB(NULL, &oldb);
  WinSetTextColorRGB(NULL, &oldt);

  if (row1 == row2) {
    command_update_line(data, row1, col1, col2, selected);
  } else if (row1 < row2) {
    command_update_line(data, row1, col1, data->ncols-1, selected);
    for (row = row1 + 1; row < row2; row++) {
      command_update_line(data, row, 0, data->ncols-1, selected);
    }
    command_update_line(data, row2, 0, col2, selected);
  }

  WinSetBackColorRGB(&oldb, NULL);
  WinSetTextColorRGB(&oldt, NULL);
  FntSetFont(old);
}

void command_drag(command_data_t *data, Int16 x, Int16 y, Int16 type) {
  UInt32 swidth, sheight;
  Int16 col, row;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);

  if (x >= 0 && x < swidth && y >= 0 && y < sheight) {
    pterm_cursor(data->t, 0);
    col = x / data->fwidth;
    row = y / data->fheight;

    switch (type) {
      case SCREEN_PEN_DOWN:
        data->moved = false;
        if (data->selected) {
          data->selected = false;
          command_draw(data);
        }
        data->col0 = col;
        data->row0 = row;
        data->col1 = col;
        data->row1 = row;
        data->down = true;
        break;
      case SCREEN_PEN_MOVE:
        if (data->down) {
          if (row != data->row1 || col != data->col1) {
            if (row > data->row1 || (row == data->row1 && col > data->col1)) {
              if (data->row0 > data->row1 || (data->row0 == data->row1 && data->col0 > data->col1)) {
                command_update(data, data->row1, data->col1, row, col, false);
              } else {
                command_update(data, data->row0, data->col0, row, col, true);
              }
            } else if (row < data->row1 || (row == data->row1 && col < data->col1)) {
              if (data->row0 > data->row1 || (data->row0 == data->row1 && data->col0 > data->col1)) {
                command_update(data, row, col, data->row0, data->col0, true);
              } else {
                command_update(data, row, col, data->row1, data->col1, false);
              }
            }
            data->col1 = col;
            data->row1 = row;
            data->moved = true;
          }
        }
        break;
      case SCREEN_PEN_UP:
        data->selected = data->moved;
        data->moved = false;
        data->down = false;
        break;
    }
  }
}

static Boolean MainFormHandleEvent(EventType *event) {
  command_data_t *data = pumpkin_get_data();
  WinHandle wh;
  FormType *frm;
  RectangleType rect;
  RGBColorType old;
  UInt32 swidth, sheight;
  Coord x, y;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      data->cmdIndex = 0;

      WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
      frm = FrmGetActiveForm();
      wh = FrmGetWindowHandle(frm);
      RctSetRectangle(&rect, 0, 0, swidth, sheight);
      WinSetBounds(wh, &rect);
      WinSetBackColorRGB(&background, &old);
      WinEraseRectangle(&rect, 0);
      WinSetBackColorRGB(&old, NULL);

      pumpkin_script_init(data->pe, sysRsrcTypeScript, 1);
      pterm_cls(data->t);
      command_puts(data, "PumpkinOS shell\r\n");
      command_prompt(data);
      handled = true;
      break;

    case frmUpdateEvent:
      handled = true;
      break;

    case nilEvent:
      pterm_cursor_blink(data->t);
      handled = true;
      break;

    case keyDownEvent:
      if (!(event->data.keyDown.modifiers & commandKeyMask)) {
        command_key(data, event->data.keyDown.chr);
        handled = true;
      }
      break;

    case menuEvent:
      frm = FrmGetActiveForm();
      handled = MenuEvent(frm, event->data.menu.itemID);
      break;

    case penDownEvent:
      x = event->screenX;
      y = event->screenY;
      command_drag(data, x, y, SCREEN_PEN_DOWN);
      break;

    case penMoveEvent:
      x = event->screenX;
      y = event->screenY;
      command_drag(data, x, y, SCREEN_PEN_MOVE);
      break;

    case penUpEvent:
      x = event->screenX;
      y = event->screenY;
      command_drag(data, x, y, SCREEN_PEN_UP);
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormType *frm;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);

      switch (form) {
        case MainForm:
          FrmSetEventHandler(frm, MainFormHandleEvent);
          break;
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static void check_prefix(char *path, char *buf, int n) {
  command_data_t *data = pumpkin_get_data();

  if (path) {
    if (path[0] == '/') {
      StrNCopy(buf, path, n-1);
    } else {
      StrNCopy(buf, data->cwd, n-1);
      StrNCat(buf, path, n-1-StrLen(buf));
    }
  } else {
    StrNCopy(buf, data->cwd, n-1);
  }
}

static Int32 read_file(char *name, char *b, Int32 max) {
  UInt32 iterator, nread;
  UInt16 volume;
  FileRef fr;
  char buf[MAXCMD];
  Int32 n = -1;

  check_prefix(name, buf, MAXCMD);

  iterator = vfsIteratorStart;
  if (VFSVolumeEnumerate(&volume, &iterator) == errNone) {
    if (VFSFileOpen(volume, buf, vfsModeRead, &fr) == errNone) {
      if (VFSFileRead(fr, max, b, &nread) == errNone) {
        n = nread;
      }
      VFSFileClose(fr);
    }
  }

  return n;
}

static int command_script_file(int pe, int run) {
  command_data_t *data = pumpkin_get_data();
  Int32 max = 65536;
  Int32 n;
  char *buf, *val, *name = NULL;
  int r;

  if (script_get_string(pe, 0, &name) == 0) {
    if ((buf = MemPtrNew(max)) != NULL) {
      MemSet(buf, max, 0);
      StrCopy(buf, "--\n");

    if ((n = read_file(name, &buf[3], max-4)) == -1) {
        command_puts(data, "Error reading file\r\n");

      } else if (n > 0) {
        if (run) {
          val = pumpkin_script_run(pe, buf, &r);
          if (r == 0) {
            if (val) {
              if (val[0]) {
                command_puts(data, val);
                command_putc(data, '\r');
                command_putc(data, '\n');
              }
              xfree(val);
            }
          } else {
            command_puts(data, "Error loading file\r\n");
          }
        } else {
          command_puts(data, &buf[3]);
          command_putc(data, '\r');
          command_putc(data, '\n');
        }
      }
      MemPtrFree(buf);
    }
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_cat(int pe) {
  return command_script_file(pe, 0);
}

static int command_script_load(int pe) {
  return command_script_file(pe, 1);
}

static int command_script_screen_rgb(int pe) {
  script_int_t red, green, blue;
  RGBColorType rgb;
  UInt32 c;
  int r = -1;

  if (script_get_integer(pe, 0, &red) == 0 &&
      script_get_integer(pe, 1, &green) == 0 &&
      script_get_integer(pe, 2, &blue) == 0) {

    rgb.r = red;
    rgb.g = green;
    rgb.b = blue;
    c = RGBToLong(&rgb);
    r = script_push_integer(pe, c);
  }

  return r;
}

static int command_script_screen_width(int pe) {
  UInt32 width;

  WinScreenMode(winScreenModeGet, &width, NULL, NULL, NULL);

  return script_push_integer(pe, width);
}

static int command_script_screen_height(int pe) {
  UInt32 height;

  WinScreenMode(winScreenModeGet, NULL, &height, NULL, NULL);

  return script_push_integer(pe, height);
}

static int command_script_setpixel(int pe) {
  script_int_t x, y, c;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    WinPaintPixel(x, y);
    WinSetForeColorRGB(&old, NULL);
    r = 0;
  }

  return r;
}

static int command_script_clear(int pe) {
  script_int_t c;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &c) == 0) {
    LongToRGB(c, &rgb);
    WinSetBackColorRGB(&rgb, &old);
    WinEraseWindow();
    WinSetBackColorRGB(&old, NULL);
    r =  0;
  }

  return r;
}

static int command_script_draw_text(int pe) {
  script_int_t x, y, fg, bg;
  char *s = NULL;
  RGBColorType rgb, oldt, oldb;
  int r = -1;

  if (script_get_string(pe, 0, &s) == 0 &&
      script_get_integer(pe, 1, &x) == 0 &&
      script_get_integer(pe, 2, &y) == 0 &&
      script_get_integer(pe, 3, &fg) == 0 &&
      script_get_integer(pe, 4, &bg) == 0) {

    LongToRGB(fg, &rgb);
    WinSetTextColorRGB(&rgb, &oldt);
    LongToRGB(bg, &rgb);
    WinSetBackColorRGB(&rgb, &oldb);
    WinPaintChars(s, StrLen(s), x, y);
    WinSetBackColorRGB(&oldb, NULL);
    WinSetTextColorRGB(&oldt, NULL);

    r = 0;
  }

  if (s) xfree(s);

  return r;
}

static int command_script_draw_line(int pe) {
  script_int_t x1, y1, x2, y2, c;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &x1) == 0 &&
      script_get_integer(pe, 1, &y1) == 0 &&
      script_get_integer(pe, 2, &x2) == 0 &&
      script_get_integer(pe, 3, &y2) == 0 &&
      script_get_integer(pe, 4, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    WinPaintLine(x1, y1, x2, y2);
    WinSetForeColorRGB(&old, NULL);
    r = 0;
  }

  return r;
}

static int command_script_draw_rect(int pe) {
  script_int_t x, y, w, h, c;
  RectangleType rect;
  RGBColorType rgb, old;
  int filled, r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &w) == 0 &&
      script_get_integer(pe, 3, &h) == 0 &&
      script_get_boolean(pe, 4, &filled) == 0 &&
      script_get_integer(pe, 5, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    if (filled) {
      RctSetRectangle(&rect, x, y, w, h);
      WinPaintRectangle(&rect, 0);
    } else {
      WinPaintLine(x, y, x+w-1, y);
      WinPaintLine(x+w-1, y, x+w-1, y+h-1);
      WinPaintLine(x+w-1, y+h-1, x, y+h-1);
      WinPaintLine(x, y+h-1, x, y);
    }
    WinSetForeColorRGB(&old, NULL);
    r = 0;
  }

  return r;
}

static void command_graphic_setpixel(void *data, int x, int y, uint32_t color) {
  RGBColorType rgb, old;

  LongToRGB(color, &rgb);
  WinSetForeColorRGB(&rgb, &old);
  WinPaintPixel(x, y);
  WinSetForeColorRGB(&old, NULL);
}

static int command_script_draw_circle(int pe) {
  script_int_t x, y, rx, ry, c;
  int filled, r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &rx) == 0 &&
      script_get_integer(pe, 3, &ry) == 0 &&
      script_get_boolean(pe, 4, &filled) == 0 &&
      script_get_integer(pe, 5, &c) == 0) {

    graphic_ellipse(NULL, x, y, rx, ry, filled, c, command_graphic_setpixel, NULL);
    r = 0;
  }

  return r;
}

static int command_script_fill(int pe) {
  script_int_t x, y, c;
  RectangleType rect;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    RctSetRectangle(&rect, 0, 0, 0, 0);
    SeedFill(x, y, &rect, &rgb);
    WinSetForeColorRGB(NULL, &old);
    r = 0;
  }

  return r;
}

static void db_destructor(void *p) {
  command_db_t *db;

  if (p) {
    db = (command_db_t *)p;
    if (db->dbRef) DmCloseDatabase(db->dbRef);
    xfree(db);
  }
}

static int command_script_open_db(int pe) {
  command_db_t *db;
  LocalID dbID;
  char *dbname = NULL;
  int ptr, r = -1;

  if (script_get_string(pe, 0, &dbname) == 0) {
    if ((db = xcalloc(1, sizeof(command_db_t))) != NULL) {
      if ((dbID = DmFindDatabase(0, dbname)) != 0) {
        if ((db->dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
          db->tag = TAG_DB;
          ptr = ptr_new(db, db_destructor);
          r = script_push_integer(pe, ptr);
        } else {
          xfree(db);
        }
      } else {
        xfree(db);
      }
    }
  }

  if (dbname) xfree(dbname);

  return r;
}

static int command_script_close_db(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    r = ptr_free(ptr, TAG_DB);
  }

  return r;
}

static void rsrc_destructor(void *p) {
  command_resource_t *rsrc;

  if (p) {
    rsrc = (command_resource_t *)p;
    if (rsrc->h) DmReleaseResource(rsrc->h);
    xfree(rsrc);
  }
}

static int command_script_get_rsrc(int pe, Boolean one) {
  UInt32 ptype;
  script_int_t id;
  command_resource_t *rsrc;
  char *type = NULL;
  int ptr, r = -1;

  if (script_get_string(pe, 0, &type) == 0 &&
      script_get_integer(pe, 1, &id) == 0) {

    if ((rsrc = xcalloc(1, sizeof(command_resource_t))) != NULL) {
      pumpkin_s2id(&ptype, type);
      rsrc->h = one ? DmGet1Resource(ptype, id) : DmGetResource(ptype, id);
      if (rsrc->h != NULL) {
        rsrc->tag = TAG_RSRC;
        rsrc->type = ptype;
        rsrc->id = id;
        ptr = ptr_new(rsrc, rsrc_destructor);
        r = script_push_integer(pe, ptr);
      } else {
        xfree(rsrc);
      }
    }
  }

  if (type) xfree(type);

  return r;
}

static int command_script_get_resource(int pe) {
  return command_script_get_rsrc(pe, false);
}

static int command_script_get1_resource(int pe) {
  return command_script_get_rsrc(pe, true);
}

static int command_script_release_resource(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    r = ptr_free(ptr, TAG_RSRC);
  }

  return r;
}

static int command_script_bitmap_draw(int pe) {
  BitmapType *bmp;
  command_resource_t *rsrc;
  script_int_t ptr, x, y;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &x) == 0 &&
      script_get_integer(pe, 2, &y) == 0) {

    if ((rsrc = ptr_lock(ptr, TAG_RSRC)) != NULL) {
      if (rsrc->type == bitmapRsc || rsrc->type == iconType) {
        if (rsrc->h && (bmp = MemHandleLock(rsrc->h)) != NULL) {
          WinPaintBitmap(bmp, x, y);
          MemHandleUnlock(rsrc->h);
          r = 0;
        }
      }
      ptr_unlock(ptr, TAG_RSRC);
    }
  }

  return r;
}

static int command_script_bitmap_dimensions(int pe, Coord *width, Coord *height) {
  BitmapType *bmp;
  command_resource_t *rsrc;
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if ((rsrc = ptr_lock(ptr, TAG_RSRC)) != NULL) {
      if (rsrc->type == bitmapRsc || rsrc->type == iconType) {
        if (rsrc->h && (bmp = MemHandleLock(rsrc->h)) != NULL) {
          BmpGetDimensions(bmp, width, height, NULL);
          MemHandleUnlock(rsrc->h);
          r = 0;
        }
      }
      ptr_unlock(ptr, TAG_RSRC);
    }
  }

  return r;
}

static int command_script_bitmap_width(int pe) {
  Coord width, height;
  int r;

  if ((r = command_script_bitmap_dimensions(pe, &width, &height)) == 0) {
    r = script_push_integer(pe, width);
  }

  return r;
}

static int command_script_bitmap_height(int pe) {
  Coord width, height;
  int r;

  if ((r = command_script_bitmap_dimensions(pe, &width, &height)) == 0) {
    r = script_push_integer(pe, height);
  }

  return r;
}

static int command_script_kill(int pe) {
  char *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    pumpkin_kill(name);
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_cls(int pe) {
  command_data_t *data = pumpkin_get_data();
  pterm_cls(data->t);
  pterm_home(data->t);

  return 0;
}

static char *value_tostring(int pe, script_arg_t *arg) {
  char buf[64], *val;

  switch (arg->type) {
    case SCRIPT_ARG_INTEGER:
      StrPrintF(buf, "%d", arg->value.i);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_REAL:
      StrPrintF(buf, "%f", arg->value.d);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_BOOLEAN:
      StrCopy(buf, arg->value.i ? "true" : "false");
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_STRING:
      val = arg->value.s;
      break;
    case SCRIPT_ARG_LSTRING:
      val = xcalloc(1, arg->value.l.n + 1);
      if (val) {
        MemMove(val, arg->value.l.s, arg->value.l.n);
      }
      xfree(arg->value.l.s);
      break;
    case SCRIPT_ARG_OBJECT:
      StrCopy(buf, "<object>");
      val = xstrdup(buf);
      script_remove_ref(pe, arg->value.r);
      break;
    case SCRIPT_ARG_FUNCTION:
      StrCopy(buf, "<function>");
      val = xstrdup(buf);
      script_remove_ref(pe, arg->value.r);
      break;
    case SCRIPT_ARG_FILE:
      StrCopy(buf, "<file>");
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_NULL:
      StrCopy(buf, "nil");
      val = xstrdup(buf);
      break;
    default:
      StrPrintF(buf, "type(%c)", arg->type);
      val = xstrdup(buf);
      break;
  }

  return val;
}

static int command_script_print(int pe) {
  command_data_t *data = pumpkin_get_data();
  script_arg_t arg;
  Int16 i;
  Boolean crlf;
  char *s = NULL;

  crlf = false;

  for (i = 0;; i++) {
    if (script_get_value(pe, i, SCRIPT_ARG_ANY, &arg) != 0) break;
    s = value_tostring(pe, &arg);
    if (s) {
      command_puts(data, s);
      command_putc(data, '\t');
      xfree(s);
      crlf = true;
    }
  }

  if (crlf) {
    command_putc(data, '\r');
    command_putc(data, '\n');
  }

  return 0;
}

static int command_script_event(int pe) {
  script_int_t wait;
  EventType event;
  Err err;

  event.eType = nilEvent;

  if (script_get_integer(pe, 0, &wait) == 0) {
    EvtGetEvent(&event, wait);
    if (!SysHandleEvent(&event)) {
      if (!MenuHandleEvent(NULL, &event, &err)) {
        FrmDispatchEvent(&event);
      }
    }
  }

  return script_push_integer(pe, event.eType);
}

static UInt32 ScriptPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  command_data_t *data;
  LocalID dbID;
  DmOpenRef dbRef;
  int pe, obj, max, n;
  int32_t r;
  char *name, *buf;

  if ((pe = pumpkin_script_create()) > 0) {
    pumpkin_script_global_function(pe, "event", command_script_event);
    pumpkin_script_global_iconst(pe, "keyDown", keyDownEvent);
    pumpkin_script_global_iconst(pe, "appStop", appStopEvent);

    if ((obj = pumpkin_script_create_obj(pe, "screen")) != -1) {
      pumpkin_script_obj_function(pe, obj, "width",    command_script_screen_width);
      pumpkin_script_obj_function(pe, obj, "height",   command_script_screen_height);
      pumpkin_script_obj_function(pe, obj, "rgb",      command_script_screen_rgb);
      pumpkin_script_obj_function(pe, obj, "setpixel", command_script_setpixel);
      pumpkin_script_obj_function(pe, obj, "clear",    command_script_clear);
      pumpkin_script_obj_function(pe, obj, "draw",     command_script_draw_text);
      pumpkin_script_obj_function(pe, obj, "line",     command_script_draw_line);
      pumpkin_script_obj_function(pe, obj, "rect",     command_script_draw_rect);
      pumpkin_script_obj_function(pe, obj, "circle",   command_script_draw_circle);
      pumpkin_script_obj_function(pe, obj, "fill",     command_script_fill);
    }

    if ((obj = pumpkin_script_create_obj(pe, "db")) != -1) {
      pumpkin_script_obj_function(pe, obj, "open",     command_script_open_db);
      pumpkin_script_obj_function(pe, obj, "close",    command_script_close_db);
    }

    if ((obj = pumpkin_script_create_obj(pe, "rsrc")) != -1) {
      pumpkin_script_obj_function(pe, obj, "get",      command_script_get_resource);
      pumpkin_script_obj_function(pe, obj, "get1",     command_script_get1_resource);
      pumpkin_script_obj_function(pe, obj, "release",  command_script_release_resource);
    }

    if ((obj = pumpkin_script_create_obj(pe, "bitmap")) != -1) {
      pumpkin_script_obj_function(pe, obj, "draw",     command_script_bitmap_draw);
      pumpkin_script_obj_function(pe, obj, "width",    command_script_bitmap_width);
      pumpkin_script_obj_function(pe, obj, "height",   command_script_bitmap_height);
    }

    data = xcalloc(1, sizeof(command_data_t));
    StrCopy(data->cwd, "/");
    pumpkin_set_data(data);
    name = (char *)cmdPBP;

    dbRef = NULL;
    if ((dbID = DmFindDatabase(0, "Command")) != 0) {
      dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly);
    }

    max = 65536;
    if ((buf = xcalloc(1, max)) != NULL) {
      StrCopy(buf, "--\n");
      if ((n = read_file(name, &buf[3], max-4)) > 0) {
        pumpkin_script_run(pe, buf, &r);
      }
      xfree(buf);
    }

    DmCloseDatabase(dbRef);
    xfree(name);
    pumpkin_set_data(NULL);
    xfree(data);
    pumpkin_script_destroy(pe);
  }

  return 0;
}

static int command_script_launch_script(int pe) {
  char *path = NULL;
  char *param, *name;
  int i;

  if (script_get_string(pe, 0, &path) == 0) {
    param = xstrdup(path);
    name = path;
    for (i = StrLen(path)-1; i > 0; i--) {
      if (path[i] == '/') {
        path[i] = 0;
        name = &path[i+1];
        break;
      }
    }
    pumpkin_launch_request(name, sysAppLaunchCmdNormalLaunch, (uint8_t *)param, 0, ScriptPilotMain, 0);
  }

  if (path) xfree(path);

  return 0;
}

static int command_script_launch(int pe) {
  UInt32 result;
  char *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    LocalID dbID = DmFindDatabase(0, name);
    if (dbID) {
      SysAppLaunch(0, dbID, 0, sysAppLaunchCmdNormalLaunch, NULL, &result);
    }
  }

  if (name) xfree(name);

  return 0;
}

static Int16 command_print_item(command_data_t *data, UInt16 cardNo, LocalID dbID, char *pname, char *pattern, UInt32 ptype, UInt32 pcreator) {
  char name[dmDBNameLength], stype[8], screator[8];
  UInt32 type, creator;
  Boolean match;
  Err err;
  Int16 n, m, r = -1;

  if (!dbID && pname) {
    StrNCopy(name, pname, dmDBNameLength-1);
    type = sysFileTApplication;
    creator = AppID;
    err = errNone;
  } else {
    err = DmDatabaseInfo(cardNo, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator);
  }

  if (err == errNone) {
    if (pattern) {
      n = StrLen(pattern);
      m = StrLen(name);
      match = false;

      if (n > 0) {
        if (n == 1 && pattern[0] == '*') {
          match = true;
        } else if (pattern[0] == '*') {
          if (m >= n-1) {
            match = StrCompare(&name[m-(n-1)], &pattern[1]) == 0;
          }
        } else if (pattern[n-1] == '*') {
          if (m >= n) {
            match = StrNCompare(name, pattern, n-1) == 0;
          }
        } else {
          match = StrCompare(name, pattern) == 0;
        }
      }
    } else {
      match = true;
    }

    if (match && ptype && type != ptype) {
      match = false;
    }
    if (match && pcreator && creator != pcreator) {
      match = false;
    }

    if (match) {
      pumpkin_id2s(type, stype);
      command_puts(data, stype);
      command_putc(data, ' ');
      pumpkin_id2s(creator, screator);
      command_puts(data, screator);
      command_putc(data, ' ');
      command_puts(data, name);
      command_putc(data, '\r');
      command_putc(data, '\n');
    }
    r = 0;
  }

  return r;
}

static int ps_callback(int i, char *name, int m68k, void *data) {
  LocalID dbID;

  dbID = DmFindDatabase(0, name);

  return command_print_item((command_data_t *)data, 0, dbID, name, NULL, 0, 0);
}

static int command_script_ps(int pe) {
  command_data_t *data = pumpkin_get_data();

  pumpkin_ps(ps_callback, data);

  return 0;
}

static int command_script_cd(int pe) {
  command_data_t *data = pumpkin_get_data();
  FileInfoType info;
  FileRef f;
  UInt32 op;
  char buf[MAXCMD];
  char *dir = NULL;

  if (script_get_string(pe, 0, &dir) == 0) {
    if (StrCompare(dir, "..") == 0) {
    } else {
      if (VFSFileOpen(1, data->cwd, vfsModeRead, &f) == errNone) {
        for (op = vfsIteratorStart;;) {
          MemSet(buf, sizeof(buf), 0);
          info.nameP = buf;
          info.nameBufLen = sizeof(buf)-1;   
          if (VFSDirEntryEnumerate(f, &op, &info) != errNone) break;
          if (StrCompare(dir, info.nameP) == 0) {
            if (info.attributes & vfsFileAttrDirectory) {
              StrNCat(data->cwd, info.nameP, MAXCMD-1-StrLen(data->cwd));
              StrNCat(data->cwd, "/", MAXCMD-1-StrLen(data->cwd));
            } else {
              command_puts(data, "Not a directory\r\n");
            }
            break;
          }
        }
        VFSFileClose(f);
      }
    }
  }

  if (dir) xfree(dir);

  return 0;
}

static int command_script_ls(int pe) {
  command_data_t *data = pumpkin_get_data();
  FileInfoType info;
  FileRef f;
  UInt32 op;
  char buf[MAXCMD];
  char *dir = NULL;

  script_opt_string(pe, 0, &dir);
  check_prefix(dir, buf, MAXCMD);

  if (VFSFileOpen(1, buf, vfsModeRead, &f) == errNone) {
    for (op = vfsIteratorStart;;) {
      MemSet(buf, sizeof(buf), 0);
      info.nameP = buf;
      info.nameBufLen = sizeof(buf)-1;   
      if (VFSDirEntryEnumerate(f, &op, &info) != errNone) break;
      command_putc(data, info.attributes & vfsFileAttrDirectory ? 'd' : ' ');
      command_putc(data, ' ');
      command_puts(data, info.nameP);
      command_putc(data, '\r');
      command_putc(data, '\n');
    }
    VFSFileClose(f);
  }

  if (dir) xfree(dir);

  return 0;
}

static int command_script_lsdb(int pe) {
  command_data_t *data = pumpkin_get_data();
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  Boolean first;
  UInt32 ptype, pcreator;
  char *pattern = NULL;
  char *type = NULL;
  char *creator = NULL;

  script_opt_string(pe, 0, &pattern);
  script_opt_string(pe, 1, &type);
  script_opt_string(pe, 2, &creator);

  if (type && type[0]) {
    pumpkin_s2id(&ptype, type);
  } else {
    ptype = 0;
  }

  if (creator && creator[0]) {
    pumpkin_s2id(&pcreator, creator);
  } else {
    pcreator = 0;
  }

  for (first = true;; first = false) {
    if (DmGetNextDatabaseByTypeCreator(first, &stateInfo, 0, 0, false, &cardNo, &dbID) != errNone) break;
    command_print_item(data, cardNo, dbID, NULL, pattern && pattern[0] ? pattern : NULL, ptype, pcreator);
  }

  if (pattern) xfree(pattern);
  if (type) xfree(type);
  if (creator) xfree(creator);

  return 0;
}

static int command_script_edit(int pe) {
  command_data_t *data = pumpkin_get_data();
  editor_t e;
  char *name = NULL;

  script_opt_string(pe, 0, &name);

  xmemset(&e, 0, sizeof(editor_t));
  pumpkin_editor_init(&e, data->t);

  if (editor_get_plugin(&e, sysAnyPluginId) == 0 && e.edit) {
    e.edit(&e, name);
    pterm_cls(data->t);
    pterm_home(data->t);
    if (e.destroy) e.destroy(&e);
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_telnet(int pe) {
  command_data_t *data = pumpkin_get_data();
  char *host = NULL;
  script_int_t port;
  conn_filter_t *conn, *telnet;
  int fd;

  if (script_get_string(pe, 0, &host) == 0) {
     if (script_opt_integer(pe, 1, &port) != 0) {
       port = 23;
     }

     if ((fd = sys_socket_open_connect(host, port, IP_STREAM)) != -1) {
       if ((conn = conn_filter(fd)) != NULL) {
         if ((telnet = telnet_filter(conn)) != NULL) {
           data->telnet = telnet;
           pterm_cls(data->t);
           data->wait = SysTicksPerSecond() / 10;
         }
       }
     }
  }

  if (host) xfree(host);

  return 0;
}

static int command_script_exit(int pe) {
  EventType event;

  event.eType = appStopEvent;
  EvtAddEventToQueue(&event);
  
  return 0;
}

static int command_shell_filter_peek(conn_filter_t *filter, uint32_t us) {
  return EvtSysEventAvail(true) ? 1 : EvtPumpEvents(us);
}

static int command_shell_filter_read(conn_filter_t *filter, uint8_t *b) {
  command_data_t *data = pumpkin_get_data();
  EventType event;
  Err err;
  int stop = 0, r = -1;

  do {
    EvtGetEvent(&event, data->wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;

    switch (event.eType) {
      case nilEvent:
        pterm_cursor_blink(data->t);
        break;
      case keyDownEvent:
        if (!(event.data.keyDown.modifiers & commandKeyMask)) {
          *b = event.data.keyDown.chr;
          if (*b == '\n') *b = '\r';
          stop = 1;
          r = 1;
        }
        break;
      case appStopEvent:
        stop = 1;
        break;
      default:
        FrmDispatchEvent(&event);
        break;
    }
  } while (!stop);

  return r;
}

static int command_shell_filter_write(conn_filter_t *filter, uint8_t *b, int n) {
  command_data_t *data = pumpkin_get_data();
  int i;

  for (i = 0; i < n; i++) {
    command_putc(data, (char)b[i]);
  }

  return 0;
}

static int command_script_shell(int pe) {
  shell_provider_t *p;
  conn_filter_t filter;
  int r = -1;

  if ((p = script_get_pointer(pe, SHELL_PROVIDER)) != NULL) {
    xmemset(&filter, 0, sizeof(conn_filter_t));
    filter.peek = command_shell_filter_peek;
    filter.read = command_shell_filter_read;
    filter.write = command_shell_filter_write;
    r = p->run(&filter, pe, 0);
  }

  return r;
}

static int command_script_crash(int pe) {
  script_int_t fatal;

  if (script_get_integer(pe, 0, &fatal) == 0) {
    pumpkin_test_exception(fatal);
  }

  return 0;
}

static int command_pterm_draw(uint8_t col, uint8_t row, uint8_t code, uint32_t fg, uint32_t bg, uint8_t attr, void *_data) {
  command_data_t *data = (command_data_t *)_data;
  RGBColorType text, back, oldt, oldf, oldb;
  uint32_t aux, x, y;
  FontID old;

  if (col < data->ncols && row < data->nrows) {
    x = col * data->fwidth;
    y = row * data->fheight;
    old = FntSetFont(data->font);
    if (attr & ATTR_INVERSE) {
      aux = fg;
      fg = bg;
      bg = aux;
    }
    LongToRGB(fg, &text);
    LongToRGB(bg, &back);
    WinSetTextColorRGB(&text, &oldt);
    WinSetBackColorRGB(&back, &oldb);
    WinPaintChar(code, x, y);
    if (attr & ATTR_UNDERSCORE) {
      WinSetForeColorRGB(&text, &oldf);
      WinPaintLine(x, y + data->fheight - 1, x + data->fwidth - 1, y + data->fheight - 1);
      WinSetForeColorRGB(&oldf, NULL);
    }
    WinSetTextColorRGB(&oldt, NULL);
    WinSetBackColorRGB(&oldb, NULL);
    FntSetFont(old);
  }

  return 0;
}

static int command_pterm_erase(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, uint32_t bg, uint8_t attr, void *_data) {
  command_data_t *data = (command_data_t *)_data;
  RGBColorType back, oldb;
  RectangleType rect;

  if (col1 < data->ncols && col2 <= data->ncols && row1 < data->nrows && row2 <= data->nrows && col2 >= col1 && row2 >= row1) {
    //if (attr & ATTR_BRIGHT) bg += 8;
    LongToRGB(bg, &back);
    WinSetBackColorRGB(&back, &oldb);
    RctSetRectangle(&rect, col1 * data->fwidth, row1 * data->fheight, (col2 - col1) * data->fwidth, (row2 - row1) * data->fheight);
    WinEraseRectangle(&rect, 0);
    WinSetBackColorRGB(&oldb, NULL);
  }

  return 0;
}

static Err StartApplication(void *param) {
  command_data_t *data;
  UInt32 swidth, sheight;
  FontID old;
  uint32_t color;

  data = xcalloc(1, sizeof(command_data_t));
  pumpkin_set_data(data);

  data->wait = SysTicksPerSecond() / 2;
  StrCopy(data->cwd, "/");

  if ((data->fh = DmGetResource(fontExtRscType, FONT)) != NULL) {
    data->f = MemHandleLock(data->fh);
    FntDefineFont(128, data->f);
  }

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  old = FntSetFont(128);
  data->font = 128;
  data->fwidth = FntCharWidth('A');
  data->fheight = FntCharHeight();
  FntSetFont(old);
  data->ncols = swidth  / data->fwidth;
  data->nrows = sheight / data->fheight;
  data->t = pterm_init(data->ncols, data->nrows, 1);
  data->cb.draw = command_pterm_draw;
  data->cb.erase = command_pterm_erase;
  data->cb.data = data;
  pterm_callback(data->t, &data->cb);

  color = RGBToLong((RGBColorType *)&foreground);
  pterm_setfg(data->t, color);
  color = RGBToLong((RGBColorType *)&background);
  pterm_setbg(data->t, color);

  if ((data->pe = pumpkin_script_create()) > 0) {
    script_loadlib(data->pe, "libshell");

    pumpkin_script_global_function(data->pe, "cls",    command_script_cls);
    pumpkin_script_global_function(data->pe, "print",  command_script_print);
    pumpkin_script_global_function(data->pe, "cd",     command_script_cd);
    pumpkin_script_global_function(data->pe, "ls",     command_script_ls);
    pumpkin_script_global_function(data->pe, "lsdb",   command_script_lsdb);
    pumpkin_script_global_function(data->pe, "launch", command_script_launch);
    pumpkin_script_global_function(data->pe, "run",    command_script_launch_script);
    pumpkin_script_global_function(data->pe, "ps",     command_script_ps);
    pumpkin_script_global_function(data->pe, "kill",   command_script_kill);
    pumpkin_script_global_function(data->pe, "cat",    command_script_cat);
    pumpkin_script_global_function(data->pe, "read",   command_script_load);
    pumpkin_script_global_function(data->pe, "edit",   command_script_edit);
    pumpkin_script_global_function(data->pe, "telnet", command_script_telnet);
    pumpkin_script_global_function(data->pe, "exit",   command_script_exit);
    pumpkin_script_global_function(data->pe, "shell",  command_script_shell);
    pumpkin_script_global_function(data->pe, "crash",  command_script_crash);
  }

  FrmGotoForm(MainForm);

  return errNone;
}

static void check_telnet(void) {
  command_data_t *data = pumpkin_get_data();
  conn_filter_t *conn, *telnet;
  uint8_t b;
  int r;

  if (data->telnet != NULL) {
    telnet = data->telnet;
    for (;;) {
      r = telnet->peek(telnet, 0);
      if (r == 0) break;
      if (r == -1 || telnet->read(telnet, &b) == -1) {
        data->telnet = NULL;
        conn = telnet->next;
        telnet_close(telnet);
        conn_close(conn);
        data->wait = SysTicksPerSecond() / 2;
        break;
      }
      command_putc(data, b);
    }
  }
}

static void EventLoop() {
  command_data_t *data;
  EventType event;
  Err err;
 
  data = pumpkin_get_data();

  do {
    check_telnet();
    EvtGetEvent(&event, data->wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);

  } while (event.eType != appStopEvent);
}

static void StopApplication(void) {
  command_data_t *data;

  FrmCloseAllForms();
  data = pumpkin_get_data();

  if (data->fh) {
    if (data->f) MemHandleUnlock(data->fh);
    DmReleaseResource(data->fh);
  }

  pumpkin_script_destroy(data->pe);
  xfree(data);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    default:
      debug(DEBUG_INFO, "Command", "launch code %d ignored", cmd);
      return 0;
  }

  if (StartApplication((void *)cmdPBP) == errNone) {
    EventLoop();
    StopApplication();
  }

  return 0;
}
