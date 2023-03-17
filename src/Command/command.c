#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "script.h"
#include "pfont.h"
#include "graphic.h"
#include "ptr.h"
#include "vfs.h"
#include "util.h"
#include "findargs.h"
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
#include "file.h"
#include "deploy.h"
#include "resource.h"

#define AppID    'CmdP'

#define TAG_DB   "cmd_db"
#define TAG_RSRC "cmd_rsrc"

#define MAXCMD 512

#define SCREEN_PEN_DOWN 1
#define SCREEN_PEN_MOVE 2
#define SCREEN_PEN_UP   3

#define COLS 80
#define ROWS 25

typedef struct {
  char *name;
  int (*function)(int pe);
} command_builtin_t;

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
  UInt16 font;
  RGBColorType foreground;
  RGBColorType background;
  RGBColorType highlight;
} command_prefs_t;

typedef struct {
  int pe;
  Int32 wait;
  UInt16 blink;
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
  command_prefs_t prefs;
} command_data_t;

static const RGBColorType defaultForeground = { 0, 0xFF, 0xFF, 0xFF };
static const RGBColorType defaultBackground = { 0, 0x13, 0x32, 0x65 };
static const RGBColorType defaultHighlight  = { 0, 0xFF, 0xFF, 0x80 };

static int command_script_cls(int pe);
static int command_script_print(int pe);
static int command_script_cd(int pe);
static int command_script_ls(int pe);
static int command_script_lsdb(int pe);
static int command_script_launch(int pe);
static int command_script_ps(int pe);
static int command_script_kill(int pe);
static int command_script_cat(int pe);
static int command_script_run(int pe);
static int command_script_deploy(int pe);
static int command_script_edit(int pe);
static int command_script_telnet(int pe);
static int command_script_exit(int pe);
static int command_script_pit(int pe);
static int command_script_crash(int pe);

static const command_builtin_t builtinCommands[] = {
  { "cls",    command_script_cls    },
  { "print",  command_script_print  },
  { "cd",     command_script_cd     },
  { "ls",     command_script_ls     },
  { "lsdb",   command_script_lsdb   },
  { "launch", command_script_launch },
  { "ps",     command_script_ps     },
  { "kill",   command_script_kill   },
  { "cat",    command_script_cat    },
  { "run",    command_script_run    },
  { "deploy", command_script_deploy },
  { "edit",   command_script_edit   },
  { "telnet", command_script_telnet },
  { "exit",   command_script_exit   },
  { "pit",    command_script_pit    },
  { "crash",  command_script_crash  },
  { NULL, NULL }
};

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

static void command_script(command_data_t *data, char *s) {
  char *argv[8], *val;
  int argc, i, j, index;

  if (s && s[0]) {
    // split the command line into argv, argv[0] is the first word
    argc = pit_findargs(s, argv, 8, NULL, NULL);

    if (argc > 0) {
      // check if argv[0] is a builtin command
      for (i = 0; builtinCommands[i].name; i++) {
        if (!StrCompare(argv[0], builtinCommands[i].name)) break;
      }

      if (builtinCommands[i].function) {
        // yes, run the builtin command
        index = script_get_stack(data->pe);
        script_set_stack(data->pe, 0);
        for (j = 1; j < argc; j++) {
          // arguments are always pushed as strings
          script_push_string(data->pe, argv[j]);
        }
        builtinCommands[i].function(data->pe);
        script_set_stack(data->pe, index);

      } else {
        // argv[0] is not a builtin command, evaluate the whole expression
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
      for (i = 0; i < argc; i++) {
        if (argv[i]) xfree(argv[i]);
      }
    }
  }
}

static void command_prompt(command_data_t *data) {
  command_puts(data, data->cwd);
  command_putc(data, '>');
}

static void command_expand(command_data_t *data) {
  FileInfoType info;
  FileRef f;
  UInt32 op;
  char buf[MAXCMD];
  char absolute[MAXCMD];
  int i, j, first, last, len;

  if (data->cmdIndex > 0 && data->cmd[data->cmdIndex-1] != '/') {
    data->cmd[data->cmdIndex] = 0;

    // find the previoous '"' int he command buffer
    for (i = data->cmdIndex-1; i >= 0; i--) {
      if (data->cmd[i] == '"') break;
    }

    if (i >= 0 && data->cmd[i] == '"') {
      // found '"', advance one position
      i++;

      if (data->cmd[i] == '/') {
        // it is an absolute path, just copy it
        StrNCopy(absolute, &data->cmd[i], MAXCMD - 1);
      } else {
        // it is a relative path, copy the current directoy
        StrNCopy(absolute, data->cwd, MAXCMD - 1);
        // and append the command
        StrNCat(absolute, &data->cmd[i], MAXCMD - StrLen(absolute) - 1);
      }

      // find the last '/' in the absolute path
      first = last = 0;
      for (j = 1; absolute[j]; j++) {
        if (absolute[j] == '/') last = j;
      }

      // set the name of the directory in buf
      MemSet(buf, sizeof(buf), 0);
      if (last == first) {
        StrCopy(buf, "/");
      } else {
        StrNCopy(buf, &absolute[first], last - first);
      }

      // open the directory
      if (VFSFileOpen(1, buf, vfsModeRead, &f) == errNone) {
        for (op = vfsIteratorStart;;) {
          // read the next entry in the directory
          MemSet(buf, sizeof(buf), 0);
          info.nameP = buf;
          info.nameBufLen = sizeof(buf)-1;
          if (VFSDirEntryEnumerate(f, &op, &info) != errNone) break;

          // if the typed prefix matches the begining of the entry name
          len = StrLen(&absolute[last+1]);
          if (StrNCompare(&absolute[last+1], info.nameP, len) == 0) {
            // fills in the rest of the name
            data->cmdIndex = StrLen(data->cmd);
            for (i = len; info.nameP[i]; i++) {
              if (data->cmdIndex < MAXCMD-1) {
                command_putc(data, info.nameP[i]);
                data->cmd[data->cmdIndex++] = info.nameP[i];
              }
            }
            // if the entry is a directory, append '/'
            if (data->cmdIndex < MAXCMD-1 && info.attributes & vfsFileAttrDirectory) {
              command_putc(data, '/');
              data->cmd[data->cmdIndex++] = '/';
            }
            data->cmd[data->cmdIndex] = 0;
            // exit
            break;
          }
        }
        VFSFileClose(f);
      }
    }
  }
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
      case '\t':
        command_expand(data);
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

static Boolean ctlSelected(FormType *formP, UInt16 id) {
  ControlType *ctl;
  UInt16 index;

  index = FrmGetObjectIndex(formP, id);
  ctl = (ControlType *)FrmGetObjectPtr(formP, index);

  return CtlGetValue(ctl) != 0;
}

static Boolean MenuEvent(UInt16 id) {
  command_data_t *data = pumpkin_get_data();
  MemHandle h;
  UInt16 i, length;
  Coord pos, col, row;
  uint32_t fg, bg;
  uint8_t c;
  char *s, buf[MAXCMD];
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
      if (data->selected) {
        i = 0;
        pos = data->row0 * data->ncols + data->col0;

        if (data->row0 == data->row1) {
          for (col = data->col0; col <= data->col1; col++, pos++) {
            pterm_getchar(data->t, pos, &c, &fg, &bg);
            if (i < MAXCMD) buf[i++] = c;
          }
          // remove trailing spaces
          for (; i > 0 && buf[i-1] == ' '; i--);

        } else {
          for (col = data->col0; col < data->ncols; col++, pos++) {
            pterm_getchar(data->t, pos, &c, &fg, &bg);
            if (i < MAXCMD) buf[i++] = c;
          }
          // remove trailing spaces
          for (; i > 0 && buf[i-1] == ' '; i--);
          // add newline
          if (i < MAXCMD) buf[i++] = '\n';

          for (row = data->row0+1; row < data->row1; row++) {
            for (col = 0; col < data->ncols; col++, pos++) {
              pterm_getchar(data->t, pos, &c, &fg, &bg);
              if (i < MAXCMD) buf[i++] = c;
            }
            // remove trailing spaces
            for (; i > 0 && buf[i-1] == ' '; i--);
            // add newline
            if (i < MAXCMD) buf[i++] = '\n';
          }

          for (col = 0; col <= data->col1; col++, pos++) {
            pterm_getchar(data->t, pos, &c, &fg, &bg);
            if (i < MAXCMD) buf[i++] = c;
          }
          // remove trailing spaces
          for (; i > 0 && buf[i-1] == ' '; i--);
        }

        if (i > 0) {
          debug(DEBUG_INFO, "Command", "copying to clipboard \"%.*s\"", i, buf);
          ClipboardAddItem(clipboardText, buf, i);
        }
      }
      handled = true;
      break;

    case prefsCmd:
      FrmPopupForm(PrefsForm);
      handled = true;
      break;

    case forkCmd:
      pumpkin_fork();
      handled = true;
      break;

  }

  return handled;
}

static void command_update_line(command_data_t *data, Int16 row, Int16 col1, Int16 col2, Boolean selected) {
  RectangleType rect;
  RGBColorType fg_rgb, bg_rgb;
  Int16 pos, col, x, y;
  uint32_t fg, bg;
  uint8_t c;

  y = row * data->fheight;
  x = col1 * data->fwidth;
  pos = row * data->ncols + col1;

  for (col = col1; col <= col2; col++, pos++) {
    pterm_getchar(data->t, pos, &c, &fg, &bg);
    if (selected) {
      bg_rgb = data->prefs.highlight;
      fg_rgb.r = 255 - bg_rgb.r;
      fg_rgb.g = 255 - bg_rgb.g;
      fg_rgb.b = 255 - bg_rgb.b;
    } else {
      LongToRGB(bg, &bg_rgb);
      LongToRGB(fg, &fg_rgb);
    }
    WinSetBackColorRGB(&bg_rgb, NULL);
    WinSetTextColorRGB(&fg_rgb, NULL);

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
  FormType *formP;
  RectangleType rect;
  RGBColorType old;
  UInt32 swidth, sheight;
  Coord x, y;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      data->cmdIndex = 0;

      WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
      formP = FrmGetActiveForm();
      wh = FrmGetWindowHandle(formP);
      RctSetRectangle(&rect, 0, 0, swidth, sheight);
      WinSetBounds(wh, &rect);
      WinSetBackColorRGB(&data->prefs.background, &old);
      WinEraseRectangle(&rect, 0);
      WinSetBackColorRGB(&old, NULL);

      pumpkin_script_init(data->pe, pumpkin_script_engine_id(), 1);
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
      handled = MenuEvent(event->data.menu.itemID);
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

static void DrawColor(RGBColorType *rgb, RectangleType *rect) {
  RGBColorType old, aux;
  UInt16 x, y, dx, dy;

  x = rect->topLeft.x;
  y = rect->topLeft.y;
  dx = rect->extent.x;
  dy = rect->extent.y;

  WinSetBackColorRGB(rgb, &old);
  WinEraseRectangle(rect, 0);
  WinSetBackColorRGB(&old, NULL);

  aux.r = 0;
  aux.g = 0;
  aux.b = 0;
  WinSetForeColorRGB(&aux, &old);
  WinDrawLine(x, y, x+dx-1, y);
  WinDrawLine(x+dx-1, y, x+dx-1, y+dy-1);
  WinDrawLine(x+dx-1, y+dy-1, x, y+dy-1);
  WinDrawLine(x, y+dy-1, x, y);
  WinSetForeColorRGB(&old, NULL);
}

static void DrawColorGadget(FormType *formP, UInt16 id, RGBColorType *rgb) {
  FormGadgetTypeInCallback *gad;
  UInt16 index;

  index = FrmGetObjectIndex(formP, id);
  gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(formP, index);
  DrawColor(rgb, &gad->rect);
}

static Boolean ColorGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  command_data_t *data = pumpkin_get_data();
  RGBColorType rgb;
  EventType *event;
  Boolean ok;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  switch (cmd) {
    case formGadgetDrawCmd:
      switch (gad->id) {
        case fgCtl: DrawColor(&data->prefs.foreground, &gad->rect); break;
        case bgCtl: DrawColor(&data->prefs.background, &gad->rect); break;
        case hlCtl: DrawColor(&data->prefs.highlight, &gad->rect); break;
        default: return true;
      }
      break;
    case formGadgetEraseCmd:
      WinEraseRectangle(&gad->rect, 0);
      break;
    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        switch (gad->id) {
          case fgCtl: rgb = data->prefs.foreground; break;
          case bgCtl: rgb = data->prefs.background; break;
          case hlCtl: rgb = data->prefs.highlight; break;
          default: return true;
        }
        ok = false;
        if (UIPickColor(NULL, &rgb, UIPickColorStartRGB, NULL, NULL)) {
          DrawColor(&rgb, &gad->rect);
          ok = true;
        }
        if (ok) {
          switch (gad->id) {
            case fgCtl: data->prefs.foreground = rgb; break;
            case bgCtl: data->prefs.background = rgb; break;
            case hlCtl: data->prefs.highlight = rgb; break;
          }
        }
      }
      break;
  }

  return true;
}

static Boolean PrefsFormHandleEvent(EventType *event) {
  command_data_t *data = pumpkin_get_data();
  FormType *formP;
  ControlType *ctl;
  UInt32 color;
  UInt16 index, sel, width, height;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      formP = FrmGetActiveForm();
      switch (data->prefs.font) {
        case font6x10Id:  sel = sel6x10;  break;
        case font8x14Id:  sel = sel8x14;  break;
        case font8x16Id:  sel = sel8x16;  break;
        default: sel = sel8x14; break;
      }
      index = FrmGetObjectIndex(formP, sel);
      ctl = (ControlType *)FrmGetObjectPtr(formP, index);
      CtlSetValue(ctl, 1);
      FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, fgCtl), ColorGadgetCallback);
      FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, bgCtl), ColorGadgetCallback);
      FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, hlCtl), ColorGadgetCallback);
      FrmDrawForm(formP);
      handled = true;
      break;
    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case dflBtn:
          data->prefs.font = font8x14Id;
          data->prefs.foreground = defaultForeground;
          data->prefs.background = defaultBackground;
          data->prefs.highlight  = defaultHighlight;
          formP = FrmGetActiveForm();
          index = FrmGetObjectIndex(formP, sel8x14);
          ctl = (ControlType *)FrmGetObjectPtr(formP, index);
          CtlSetValue(ctl, 1);
          DrawColorGadget(formP, fgCtl, &data->prefs.foreground);
          DrawColorGadget(formP, bgCtl, &data->prefs.background);
          DrawColorGadget(formP, hlCtl, &data->prefs.highlight);
          break;
        case okBtn:
          formP = FrmGetActiveForm();
          if (ctlSelected(formP, sel6x10)) {
            data->prefs.font = font6x10Id;
            width = 6;
            height = 10;
          } else if (ctlSelected(formP, sel8x14)) {
            data->prefs.font = font8x14Id;
            width = 8;
            height = 14;
          } else if (ctlSelected(formP, sel8x16)) {
            data->prefs.font = font8x16Id;
            width = 8;
            height = 16;
          }
          PrefSetAppPreferences(AppID, 1, 1, &data->prefs, sizeof(command_prefs_t), true);
          pumpkin_set_size(AppID, COLS * width, ROWS * height);
          color = RGBToLong(&data->prefs.foreground);
          pterm_setfg(data->t, color);
          color = RGBToLong(&data->prefs.background);
          pterm_setbg(data->t, color);
          FrmReturnToForm(MainForm);
          break;
        case cancelBtn:
          FrmReturnToForm(MainForm);
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormType *formP;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      formP = FrmInitForm(form);
      FrmSetActiveForm(formP);

      switch (form) {
        case MainForm:
          FrmSetEventHandler(formP, MainFormHandleEvent);
          break;
        case PrefsForm:
          FrmSetEventHandler(formP, PrefsFormHandleEvent);
          break;
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

Int32 read_file(char *name, char *b, Int32 max) {
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
  Int32 n, max = 65536;
  char *buf, *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    if ((buf = MemPtrNew(max)) != NULL) {
      MemSet(buf, max, 0);

      if ((n = read_file(name, buf, max-1)) == -1) {
        command_puts(data, "Error reading file\r\n");

      } else if (n > 0) {
        if (run) {
          if (pumpkin_script_run_string(pe, buf) != 0) {
            command_puts(data, "Error loading file\r\n");
          }
        } else {
          command_puts(data, buf);
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

static int command_script_run(int pe) {
  return command_script_file(pe, 1);
}

static int command_script_deploy(int pe) {
  command_data_t *data = pumpkin_get_data();
  UInt32 creator;
  char *name = NULL;
  char *screator = NULL;
  char *script = NULL;

  if (script_get_string(pe, 0, &name) == 0 &&
      script_get_string(pe, 1, &screator) == 0 &&
      script_get_string(pe, 2, &script) == 0) {

    pumpkin_s2id(&creator, screator);
    if (command_app_deploy(name, creator, script) == errNone) {
      command_puts(data, "Script deployed\r\n");
    } else {
      command_puts(data, "Error deploying script\r\n");
    }
  }

  if (name) xfree(name);
  if (screator) xfree(screator);
  if (script) xfree(script);

  return 0;
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
  char *dir = NULL;

  if (script_get_string(pe, 0, &dir) != 0) {
    dir = xstrdup("/");
  }

  if (VFSChangeDir(1, dir) == errNone) {
    VFSCurrentDir(1, data->cwd, MAXCMD);
  } else {
    command_puts(data, "Invalid directory\r\n");
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
  command_data_t *data = pumpkin_get_data();

  data->blink++;
  if (data->blink == 50) {
    pterm_cursor_blink(data->t);
    data->blink = 0;
  }

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

  return n;
}

static int command_script_pit(int pe) {
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
  UInt16 prefsSize;
  FontID old;
  uint32_t color;
  int i;

  data = xcalloc(1, sizeof(command_data_t));
  pumpkin_set_data(data);

  prefsSize = sizeof(command_prefs_t);
  if (PrefGetAppPreferences(AppID, 1, &data->prefs, &prefsSize, true) == noPreferenceFound) {
    data->prefs.font = font8x14Id;
    data->prefs.foreground = defaultForeground;
    data->prefs.background = defaultBackground;
    data->prefs.highlight  = defaultHighlight;
    PrefSetAppPreferences(AppID, 1, 1, &data->prefs, sizeof(command_prefs_t), true);
  }

  data->wait = SysTicksPerSecond() / 2;
  VFSChangeDir(1, "/");
  VFSCurrentDir(1, data->cwd, MAXCMD);

  if ((data->fh = DmGetResource(fontExtRscType, data->prefs.font)) != NULL) {
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

  color = RGBToLong(&data->prefs.foreground);
  pterm_setfg(data->t, color);
  color = RGBToLong(&data->prefs.background);
  pterm_setbg(data->t, color);

  if ((data->pe = pumpkin_script_create()) > 0) {
    script_loadlib(data->pe, "libshell");
    script_loadlib(data->pe, "liboshell");

    for (i = 0; builtinCommands[i].name; i++) {
      pumpkin_script_global_function(data->pe, builtinCommands[i].name, builtinCommands[i].function);
    }
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

  PrefSetAppPreferences(AppID, 1, 1, &data->prefs, sizeof(command_prefs_t), true);

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
