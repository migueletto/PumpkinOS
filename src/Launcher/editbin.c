#include <PalmOS.h>

#include "sys.h"
#include "mutex.h"
#include "pumpkin.h"
#include "AppRegistry.h"
#include "storage.h"
#include "resource.h"
#include "resedit.h"
#include "bytes.h"
#include "xalloc.h"
#include "debug.h"

typedef struct {
  RectangleType rect;
  FormType *frm;
  MemHandle handle;
  int index, current, nibble;
  UInt8 *p;
  UInt32 size;
  UInt32 cols, rows, maxrows, fw, fh, dw;
  Boolean down, changed, stop;
} bin_edit_t;

static const RGBColorType addr  = { 0, 0x00, 0x80, 0x00 };
static const RGBColorType even  = { 0, 0x00, 0x00, 0xff };
static const RGBColorType odd   = { 0, 0x00, 0xa0, 0xa0 };
static const RGBColorType white = { 0, 0xff, 0xff, 0xff };
static const RGBColorType gray  = { 0, 0xe0, 0xe0, 0xe0 };
static const RGBColorType red   = { 0, 0xff, 0xc0, 0xc0 };

static void printAddr(bin_edit_t *data, int row) {
  FontID old;
  char buf[8];
  int index, x, y;

  index = row * data->cols;

  if (data->index + index < data->size) {
    snprintf(buf, sizeof(buf)-1, "%04X", data->index + index);
    WinSetTextColorRGB(&addr, NULL);
    x = data->rect.topLeft.x + 1;
    y = data->rect.topLeft.y + row * data->fh;
    old = FntSetFont(monoFont1);
    WinPaintChars(buf, 4, x, y);
    FntSetFont(old);
  }
}

static void printByte(bin_edit_t *data, int col, int row, Boolean inverse) {
  FontID old;
  char buf[8], c;
  int index, x, y;

  old = FntSetFont(monoFont1);
  index = row * data->cols + col;
  WinSetBackColorRGB(inverse ? &red : &white, NULL);

  if (data->index + index < data->size) {
    snprintf(buf, sizeof(buf)-1, "%02X", data->p[data->index + index]);
    c = (char)data->p[data->index + index];
    if (c < 32 || c > 127) c = -1;
  } else {
    StrCopy(buf, "  ");
    c = ' ';
  }

  WinSetTextColorRGB((index & 1) ? &odd : &even, NULL);
  x = data->rect.topLeft.x + 1 + 4*data->fw + 1 + col * data->dw;
  y = data->rect.topLeft.y + row * data->fh;
  WinPaintChars(buf, 2, x, y);

  WinSetTextColorRGB(&even, NULL);
  x = data->rect.topLeft.x + 1 + 4*data->fw + 1 + data->cols * data->dw + 1 + col*data->fw;
  if (c >= 0) {
    WinSetBackColorRGB(inverse ? &red : &white, NULL);
    WinPaintChar(c, x, y);
  } else {
    WinSetBackColorRGB(inverse ? &red : &gray, NULL);
    WinPaintChar('.', x, y);
  }

  WinSetBackColorRGB(&white, NULL);
  FntSetFont(old);
}

static Boolean binaryGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  EventType *event;
  RGBColorType oldb, oldt, oldf;
  PatternType oldp;
  bin_edit_t *data;
  int index, col, row, x, y, x0, x1;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, binaryGad);
  data = (bin_edit_t *)FrmGetGadgetData(frm, index);
  WinSetBackColorRGB(&white, &oldb);
  WinSetTextColorRGB(&even, &oldt);
  oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);

  switch (cmd) {
    case formGadgetDrawCmd:
      for (row = 0, index = 0; row < data->rows; row++) {
        printAddr(data, row);
        for (col = 0; col < data->cols; col++, index++) {
          printByte(data, col, row, data->index + index == data->current);
        }
      }
      break;

    case formGadgetEraseCmd:
      WinSetForeColorRGB(&white, &oldf);
      WinPaintRectangle(&data->rect, 0);
      WinSetForeColorRGB(&oldf, NULL);
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        x = event->screenX - data->rect.topLeft.x;
        y = event->screenY - data->rect.topLeft.y;
        x0 = 1 + 4*data->fw + 1;
        x1 = x0 + data->cols * data->dw;
        if (x >= x0 && x < x1) {
          // binary area
          col = (x - x0) / data->dw;
          row = y / data->fh;
          index = data->index + row * data->cols + col;
          if (index >= 0 && index < data->size && index != data->current) {
            printByte(data, col, row, true);
            if (data->current >= data->index && data->current < data->index + data->cols * data->rows) {
              row = (data->current - data->index) / data->cols;
              col = (data->current - data->index) % data->cols;
              printByte(data, col, row, false);
            }
            data->current = index;
          }
        } if (x > x1+1) {
          // ascii area
        }
      } else {
      }
      break;
  }

  WinSetPatternType(oldp);
  WinSetBackColorRGB(&oldb, NULL);
  WinSetTextColorRGB(&oldt, NULL);

  return true;
}

static void changeByte(bin_edit_t *data, UInt8 b) { 
  int col, row;

  if (data->nibble) {
    data->p[data->current] &= 0xF0;
    data->p[data->current] |= b;
    data->nibble = 0;
  } else {
    data->p[data->current] &= 0x0F;
    data->p[data->current] |= (b << 4);
    data->nibble = 1;
  }

  if (data->nibble == 0 && data->current < data->size - 1) {
    row = (data->current - data->index) / data->cols;
    col = (data->current - data->index) % data->cols;
    printByte(data, col, row, false);
    data->current++;
  }
  row = (data->current - data->index) / data->cols;
  col = (data->current - data->index) % data->cols;
  printByte(data, col, row, true);

  data->changed = true;
}

static Boolean eventHandler(EventType *event) {
  UInt16 index;
  FormType *frm;
  FormGadgetTypeInCallback *gad;
  ScrollBarType *scl;
  FontID old;
  bin_edit_t *data;
  char c;
  Boolean handled = false;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, binaryGad);
  data = (bin_edit_t *)FrmGetGadgetData(frm, index);

  switch (event->eType) {
    case frmOpenEvent:
      FrmGetObjectBounds(frm, index, &data->rect);

      old = FntSetFont(monoFont1);
      data->fw = FntCharWidth('0');
      data->fh = FntCharHeight();
      FntSetFont(old);
      data->dw = 2*data->fw + 1;
      data->cols = 16;
      data->rows = data->rect.extent.y / data->fh;
      data->maxrows = (data->size + data->cols - 1) / data->cols;

      index = FrmGetObjectIndex(frm, binaryScl);
      if (data->maxrows > data->rows) {
        scl = (ScrollBarType *)FrmGetObjectPtr(frm, index);
        SclSetScrollBar(scl, 0, 0, data->maxrows - data->rows, (data->maxrows - data->rows) >= data->rows ? data->rows-1 : data->maxrows - data->rows - 1);
      } else {
        FrmHideObject(frm, index);
      }

      FrmDrawForm(frm);
      handled = true;
      break;

    case keyDownEvent:
      if (!(event->data.keyDown.modifiers & commandKeyMask)) {
        c = event->data.keyDown.chr;
        if (c >= '0' && c <= '9') {
          changeByte(data, c - '0');
          handled = true;
        } else if (c >= 'a' && c <= 'f') {
          changeByte(data, c - 'a' + 10);
          handled = true;
        } else if (c >= 'A' && c <= 'F') {
          changeByte(data, c - 'A' + 10);
          handled = true;
        }
      }
      break;

    case sclRepeatEvent:
      data->index = event->data.sclRepeat.newValue * data->cols;
      gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(frm, index);
      binaryGadgetCallback(gad, formGadgetDrawCmd, NULL);
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          data->stop = true;
          break;
        case cancelBtn:
          if (!data->changed || FrmCustomAlert(QuestionAlert, "Discard changes ?", "", "") == 0) {
            data->changed = false;
            data->stop = true;
          }
          break;
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

Boolean editBinary(FormType *frm, char *title, MemHandle h) {
  bin_edit_t data;
  FormType *previous;
  EventType event;
  UInt16 index;
  UInt8 *p;
  Err err;
  Boolean r = false;

  MemSet(&data, sizeof(bin_edit_t), 0);
  data.frm = frm;
  data.handle = h;
  data.size = MemHandleSize(h);
  data.p = xcalloc(1, data.size);

  if ((p = (UInt8 *)MemHandleLock(h)) != NULL) {
    xmemcpy(data.p, p, data.size);
    MemHandleUnlock(h);
  }

  index = FrmGetObjectIndex(frm, binaryGad);
  FrmSetGadgetHandler(frm, index, binaryGadgetCallback);
  FrmSetGadgetData(frm, index, &data);

  FrmSetTitle(frm, title);
  FrmSetEventHandler(frm, eventHandler);
  previous = FrmGetActiveForm();
  FrmSetActiveForm(frm);
  MemSet(&event, sizeof(EventType), 0);
  event.eType = frmOpenEvent;
  event.data.frmOpen.formID = frm->formId;
  FrmDispatchEvent(&event);

  do {
    EvtGetEvent(&event, 500);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (eventHandler(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent && !data.stop);

  FrmEraseForm(frm);
  FrmSetActiveForm(previous);

  if (data.changed) {
    if ((p = (UInt8 *)MemHandleLock(h)) != NULL) {
      //DmWrite(p, 0, data.p, data.size);
      MemHandleUnlock(h);
      r = true;
    }
  }

  return r;
}
