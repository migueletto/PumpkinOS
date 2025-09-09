#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "resource.h"
#include "pumpkin.h"
#include "pterm.h"
#include "editor.h"
#include "peditor.h"
#include "edit.h"
#include "color.h"
#include "VFSExplorer.h"
#include "debug.h"

#define FILE_ROOT "/PALM/Programs/forc/"
#define MAX_PATH 256

#define Y0 16

typedef struct {
  VFSExplorer *vfse;
  RectangleType rect;
  UInt16 numRows, numCols;
  UInt16 row, col;
  FontID font;
  UInt16 fontWidth, fontHeight;
  Boolean enableCursor;
  Boolean saveCursor;
  uint32_t fg, bg;
  uint8_t *buffer;
} app_data_t;

static void InitData(void) {
  app_data_t *data;
  FileRef f;

  data = sys_calloc(1, sizeof(app_data_t));
  pumpkin_set_data(data);

  if (VFSFileOpen(1, FILE_ROOT, vfsModeRead, &f) == errNone) {
    VFSFileClose(f);
  } else {
    VFSDirCreate(1, FILE_ROOT);
  }
}

static void FinishData(void) {
  app_data_t *data = pumpkin_get_data();

  pumpkin_set_data(NULL);
  sys_free(data);
}

static void draw_chars(app_data_t *data, char *buf, int n, int col, int row, uint32_t fg, uint32_t bg) {
  RGBColorType text, back, oldt, oldb;
  FontID oldFont;

  oldFont = FntSetFont(data->font);
  LongToRGB(fg, &text);
  LongToRGB(bg, &back);
  WinSetTextColorRGB(&text, &oldt);
  WinSetBackColorRGB(&back, &oldb);

  WinPaintChars(buf, n, data->rect.topLeft.x + col * data->fontWidth, data->rect.topLeft.y + row * data->fontHeight);

  WinSetTextColorRGB(&oldt, NULL);
  WinSetBackColorRGB(&oldb, NULL);
  FntSetFont(oldFont);
}

static int forc_editor_cursor(void *data_, int col, int row) {
  app_data_t *data = (app_data_t *)data_;

  if (col < 0) col = 0;
  else if (col >= data->numCols) col = data->numCols - 1;
  if (row < 0) row = 0;
  else if (row >= data->numRows) row = data->numRows - 1;

  if (col != data->col || row != data->row) {
    debug(DEBUG_INFO, APPNAME, "update cursor %2d,%2d -> %2d,%2d", data->col, data->row, col, row);
    draw_chars(data, (char *)&data->buffer[data->row * data->numCols + data->col], 1, data->col, data->row, data->fg, data->bg);
    data->col = col;
    data->row = row;
    draw_chars(data, (char *)&data->buffer[data->row * data->numCols + data->col], 1, data->col, data->row, data->bg, data->fg);
  }

  return 0;
}

static int forc_editor_cls(void *data_, uint32_t bg) {
  app_data_t *data = (app_data_t *)data_;
  RGBColorType back, oldb;

  LongToRGB(bg, &back);
  WinSetBackColorRGB(&back, &oldb);
  WinEraseRectangle(&data->rect, 0);
  WinSetBackColorRGB(&oldb, NULL);

  sys_memset(data->buffer, ' ', data->numRows * data->numCols);

  return 0;
}

static int forc_editor_clreol(void *data_, uint32_t bg) {
  app_data_t *data = (app_data_t *)data_;
  RGBColorType back, oldb;
  RectangleType rect;
  UInt16 i, p;
  Coord x;

  LongToRGB(bg, &back);
  WinSetBackColorRGB(&back, &oldb);
  x = data->col * data->fontWidth;
  RctSetRectangle(&rect, data->rect.topLeft.x + x, data->rect.topLeft.y + data->row * data->fontHeight, data->rect.extent.x - x, data->fontHeight);
  WinEraseRectangle(&rect, 0);
  WinSetBackColorRGB(&oldb, NULL);

  p = data->row * data->numCols;
  for (i = data->col; i < data->numCols; i++) {
    data->buffer[p + i] = ' ';
  }

  return 0;
}

static int forc_editor_peek(void *data_, int ms) {
  return EvtPumpEvents(ms < 0 ? evtWaitForever : ms*1000) == 0 ? 0 : 1;
}

static int forc_editor_read(void *data_, uint16_t *c) {
  EventType event;
  int r = 0;

  EvtGetEvent(&event, SysTicksPerSecond() / 2);

  if (!SysHandleEvent(&event)) {
    switch (event.eType) {
      case keyDownEvent:
        *c = event.data.keyDown.chr;
        r = 1;
        break;
      case appStopEvent:
        r = -1;
        break;
      default:
        break;
    }
  }

  return r;
}

static int forc_editor_write(void *data_, char *buf, int len) {
  app_data_t *data = (app_data_t *)data_;
  UInt16 i, p;

  if (buf && len > 0) {
    debug(DEBUG_INFO, APPNAME, "write %2d,%2d [%.*s]", data->col, data->row, len, buf);
    draw_chars(data, buf, len, data->col, data->row, data->fg, data->bg);

    p = data->row * data->numCols + data->col;
    for (i = 0; i < len; i++) {
      data->buffer[p + i] = buf[i];
      data->col++;
      if (data->col == data->numCols) {
        data->col = 0;
        if (data->row < data->numRows - 1) {
          data->row++;
        }
      }
    }
  }

  return 0;
}

static int forc_editor_color(void *data_, uint32_t fg, uint32_t bg) {
  app_data_t *data = (app_data_t *)data_;

  data->fg = fg;
  data->bg = bg;

  return 0;
}

static int forc_editor_window(void *data_, int *ncols, int *nrows) {
  app_data_t *data = (app_data_t *)data_;

  if (ncols) *ncols = data->numCols;
  if (nrows) *nrows = data->numRows;

  return 0;
}

static void resize(FormType *frm, app_data_t *data) {
  WinHandle wh;
  RectangleType rect;
  UInt32 swidth, sheight;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  wh = FrmGetWindowHandle(frm);
  RctSetRectangle(&rect, 0, 0, swidth, sheight);
  WinSetBounds(wh, &rect);
  WinSetClipingBounds(&frm->window, &rect);
}

static Boolean EditFormHandleEvent(EventType *event) {
  app_data_t *data = pumpkin_get_data();
  editor_t e;
  FormType *frm;
  Coord width, height;
  UInt32 density;
  FontID oldFont;
  char *filename;
  char path[MAX_PATH];
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      debug(DEBUG_INFO, APPNAME, "EditForm frmOpenEvent");
      frm = FrmGetActiveForm();
      resize(frm, data);
      FrmDrawForm(frm);

      if ((filename = VFSExplorerSelectedItem(data->vfse)) != NULL) {
        debug(DEBUG_INFO, APPNAME, "editing file [%s]", filename);
        sys_memset(&e, 0, sizeof(editor_t));
        e.data = data;
        WinGetWindowExtent(&width, &height);
        RctSetRectangle(&data->rect, 0, Y0, width, height - Y0);

        WinScreenGetAttribute(winScreenDensity, &density);
        data->font = mono8x14Font;
        oldFont = FntSetFont(data->font);
        data->fontHeight = FntCharHeight();
        data->fontWidth =  FntCharWidth('A');
        data->numRows = data->rect.extent.y / data->fontHeight;
        data->numCols = data->rect.extent.x / data->fontWidth;
        FntSetFont(oldFont);

        if (data->buffer == NULL) {
          data->buffer = sys_calloc(data->numRows, data->numCols);
          sys_memset(data->buffer, ' ', data->numRows * data->numCols);
        }

        e.cursor = forc_editor_cursor;
        e.cls = forc_editor_cls;
        e.clreol = forc_editor_clreol;
        e.peek = forc_editor_peek;
        e.read = forc_editor_read;
        e.write = forc_editor_write;
        e.color = forc_editor_color;
        e.window = forc_editor_window;
        pumpkin_editor_init_io(&e);

        if (editor_get_plugin(&e, sysAnyPluginId) == 0 && e.edit) {
          sys_snprintf(path, sizeof(path)-1, "%s%s", VFSExplorerCurrentPath(data->vfse), filename);
          debug(DEBUG_INFO, APPNAME, "edit begin [%s]", path);
          e.edit(&e, path);
          debug(DEBUG_INFO, APPNAME, "edit end");
          if (e.destroy) e.destroy(&e);
        }
        debug(DEBUG_INFO, APPNAME, "returnig to MainForm");
        FrmGotoForm(MainForm);
      }
      handled = true;
      break;

    case winDisplayChangedEvent:
      frm = FrmGetActiveForm();
      resize(frm, data);
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static Boolean MainFormHandleEvent(EventType *event) {
  app_data_t *data = pumpkin_get_data();
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resize(frm, data);
      if (data->vfse == NULL) {
        debug(DEBUG_INFO, APPNAME, "creating vfse");
        data->vfse = VFSExplorerCreate(frm, FileTable, FILE_ROOT);
      } else {
        debug(DEBUG_INFO, APPNAME, "refreshing vfse");
        VFSExplorerRefresh(frm, data->vfse);
      }
      FrmDrawForm(frm);
      handled = true;
      break;

    case winDisplayChangedEvent:
      frm = FrmGetActiveForm();
      resize(frm, data);
      pumpkin_set_size(APPID, event->data.winDisplayChanged.newBounds.extent.x, event->data.winDisplayChanged.newBounds.extent.y);
      handled = true;
      break;

    case tblSelectEvent:
      handled = VFSExplorerHandleEvent(data->vfse, event);
      if (!handled) {
        FrmGotoForm(EditForm);
      }
      break;

    case tblExitEvent:
      handled = VFSExplorerHandleEvent(data->vfse, event);
      break;

    case menuEvent:
      if (event->data.menu.itemID == aboutCmd) {
        AbtShowAboutPumpkin(APPID);
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormPtr frm;
  UInt16 formID;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      formID = event->data.frmLoad.formID;
      frm = FrmInitForm(formID);
      FrmSetActiveForm(frm);
      switch (formID) {
        case MainForm:
          FrmSetEventHandler(frm, MainFormHandleEvent);
          break;
        case EditForm:
          FrmSetEventHandler(frm, EditFormHandleEvent);
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop(void) {
  EventType event;
  Err err;

  do {
    EvtGetEvent(&event, evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  app_data_t *data;

  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    default:
      debug(DEBUG_INFO, APPNAME, "launch code %d ignored", cmd);
      return 0;
  }

  InitData();
  FrmCenterDialogs(true);
  FrmGotoForm(MainForm);
  EventLoop();

  data = pumpkin_get_data();
  if (data->buffer) {
    sys_free(data->buffer);
  }
  if (data->vfse) {
    debug(DEBUG_INFO, APPNAME, "destroying vfse");
    VFSExplorerDestroy(data->vfse);
  }

  FrmCloseAllForms();
  FinishData();

  return 0;
}
