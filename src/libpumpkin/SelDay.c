#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define NCOLS 7
#define NROWS 7

#define PALMOS_MODULE "DateTime"

typedef struct {
  DateTimeType date;
  Boolean selected;
} sel_date_t;

static const char *wday[7] = { "S", "M", "T", "W", "T", "F", "S" };

static void drawDay(RectangleType *rect, Int16 i, char *buf, Boolean selected) {
  RectangleType cr;
  IndexedColorType objFore, objFill, objSelFill, oldb, oldf, oldt;
  UInt16 cellx, celly, col, row, x, y, len, tw;

  cellx = rect->extent.x / NCOLS;
  celly = rect->extent.y / NROWS;
  col = i % NCOLS;
  row = i / NCOLS;
  x = rect->topLeft.x + col * cellx;
  y = rect->topLeft.y + row * celly;
  RctSetRectangle(&cr, x, y, cellx, celly);

  objFore = UIColorGetTableEntryIndex(UIObjectForeground);
  objFill = UIColorGetTableEntryIndex(UIObjectFill);
  objSelFill = UIColorGetTableEntryIndex(UIObjectSelectedFill);

  oldf = WinSetForeColor(selected ? objSelFill : objFill);
  oldb = WinSetBackColor(objFill);
  oldt = WinSetTextColor(objFore);
  WinPaintRectangle(&cr, 0);

  if (buf) {
    len = StrLen(buf);
    tw = FntCharsWidth(buf, len);
    if (selected) {
      WinEraseChars(buf, len, x+cellx-tw-4, y);
    } else {
      WinDrawChars(buf, len, x+cellx-tw-4, y);
    }
  }

  WinSetTextColor(oldt);
  WinSetForeColor(oldf);
  WinSetBackColor(oldb);
}

static void drawGrid(RectangleType *rect, DateTimeType *date) {
  FontID old;
  UInt16 i, ndays, dow, last;
  char buf[4];

  old = FntSetFont(boldFont);
  for (i = 0; i < 7; i++) {
    drawDay(rect, i, (char *)wday[i], false);
  }

  ndays = DaysInMonth(date->month, date->year);
  dow = DayOfWeek(date->month, 1, date->year);

  FntSetFont(stdFont);
  for (i = 0; i < dow; i++) {
    drawDay(rect, i+NCOLS, NULL, false);
  }

  for (i = 0; i < ndays; i++) {
    StrPrintF(buf, "%d", i+1);
    drawDay(rect, i+NCOLS+dow, buf, i+1 == date->day);
  }
  for (last = NCOLS*NROWS; i+NCOLS+dow < last; i++) {
    drawDay(rect, i+NCOLS+dow, NULL, false);
  }
  FntSetFont(old);
}

static Boolean DayGridGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  EventType *event;
  RectangleType rect;
  sel_date_t *data;
  Int16 i, cellx, celly, col, row, index, ndays, dow;

  if (cmd == formGadgetDeleteCmd) return true;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, 10120); // gadget
  FrmGetObjectBounds(frm, index, &rect);
  data = FrmGetGadgetData(frm, index);

  switch (cmd) {
    case formGadgetDrawCmd:
      drawGrid(&rect, &data->date);
      break;

    case formGadgetEraseCmd:
      WinEraseRectangle(&rect, 0);
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        if (RctPtInRectangle(event->screenX, event->screenY, &rect)) {
          cellx = rect.extent.x / NCOLS;
          celly = rect.extent.y / NROWS;
          col = (event->screenX - rect.topLeft.x) / cellx;
          row = (event->screenY - rect.topLeft.y) / celly;
          debug(DEBUG_TRACE, PALMOS_MODULE, "SelDay grid select (%d,%d) col=%d row=%d", event->screenX, event->screenY, col, row);
          if (col >= 0 && row > 0) {
            ndays = DaysInMonth(data->date.month, data->date.year);
            dow = DayOfWeek(data->date.month, 1, data->date.year);
            i = row * NCOLS + col;
            if (i >= NCOLS+dow && i < ndays+NCOLS+dow) {
              debug(DEBUG_TRACE, PALMOS_MODULE, "SelDay grid select i=%d", i);
              data->date.day = i-NCOLS-dow+1;
              data->selected = true;
              drawGrid(&rect, &data->date);
            }
          }
        }
      }
      break;
  }

  return true;
}

static Boolean SelectDayHandleEvent(EventType *eventP) {
  FormType *frm;
  sel_date_t *data;
  RectangleType rect;
  EventType ev;
  UInt16 index, id, ndays;
  char buf[8];
  Boolean handled = false;

  switch (eventP->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      id = eventP->data.ctlSelect.controlID;
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, 10120);
      FrmGetObjectBounds(frm, index, &rect);
      data = FrmGetGadgetData(frm, index);

      if (id >= 1 && id <= 12) {
        data->date.month = id;
        ndays = DaysInMonth(data->date.month, data->date.year);
        if (data->date.day > ndays) data->date.day = ndays;
        drawGrid(&rect, &data->date);
        handled = true;
      } else {
        switch (id) {
          case 10122: // this month
          case 10121: // this week
            // XXX not implemented
            break;
          case 10118: // today
            TimSecondsToDateTime(TimGetSeconds(), &data->date);
            // fall through
          case 10103:
          case 10104:
          case 0:
            if (id == 10103) data->date.year--;
            else if (id == 10104) data->date.year++;
            ndays = DaysInMonth(data->date.month, data->date.year);
            if (data->date.day > ndays) data->date.day = ndays;
            StrPrintF(buf, "%d", data->date.year);
            FrmCopyLabel(frm, 10102, buf);
            index = FrmGetObjectIndex(frm, 10102);
            FrmEraseObject(frm, index, false);
            FrmDrawObject(frm, index, false);
            drawGrid(&rect, &data->date);
            handled = id != 0 && id != 10118;
            break;
          default:
            break;
        }
      }
      break;

    case frmGadgetEnterEvent:
      // intercept the gadget event and call FrmHandleEvent directly
      frm = FrmGetActiveForm();
      FrmHandleEvent(frm, eventP);
      index = FrmGetObjectIndex(frm, 10120);
      data = FrmGetGadgetData(frm, index);
      // check if a day was selected
      if (data->selected) {
        // if a day was selected, simulate a ctlSelect event on a dummy button,
        // so that the dialog will be closed by FrmDoDialog.
        MemSet(&ev, sizeof(EventType), 0);
        ev.eType = ctlSelectEvent;
        index = FrmGetObjectIndex(frm, 10118); // the control must be real control
        ev.data.ctlSelect.pControl = FrmGetObjectPtr(frm, index);;
        ev.data.ctlSelect.controlID = 0;       // but the controlID is not
        EvtAddEventToQueue(&ev);
        // XXX there must be a better way to close the dialog
        // when a gadget is clicked, other than simulating a click
        // on a non-existent button. The way it is now, the dialog
        // will exit with a button id of 0.
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

Boolean SelectDay(const SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char *title) {
  FormType *frm, *previous;
  ControlType *ctl;
  sel_date_t data;
  UInt16 index, button;
  char buf[16];
  Boolean r = false;

  if (month == NULL || day == NULL || year == NULL) return false;
  if (*month < 1 || *month > 12) return false;

  data.selected = false;
  data.date.year = *year;
  data.date.month = *month;
  data.date.day = *day;

  frm = FrmInitForm(10100);
  if (title) FrmSetTitle(frm, (char *)title);

  // 10102: year label
  // 10103: up repeatingButton
  // 10104: down repeatingButton
  // 1 .. 12: "Jan" .. "Dec" pushButton
  // 10118: "Today" button
  // 10119: "Cancel" button
  // 10120: gadget
  // 10121: "This week" button
  // 10122: "This month" button

  // the method by which the user should choose the day
  switch (selectDayBy) {
    case selectDayByDay:   // d/m/y
      index = FrmGetObjectIndex(frm, 10118);
      FrmSetUsable(frm, index, true);
      break;
    case selectDayByWeek:  // d/m/y with d as same day of the week
      index = FrmGetObjectIndex(frm, 10121);
      FrmSetUsable(frm, index, true);
      break;
    case selectDayByMonth: // d/m/y with d as same day of the month
      index = FrmGetObjectIndex(frm, 10122);
      FrmSetUsable(frm, index, true);
      break;
  }

  // set year label
  StrPrintF(buf, "%d", *year);
  FrmCopyLabel(frm, 10102, buf);

  // set month pushButton
  index = FrmGetObjectIndex(frm, *month);
  ctl = (ControlType *)FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, 1);

  index = FrmGetObjectIndex(frm, 10120);
  FrmSetGadgetData(frm, index, &data);
  FrmSetGadgetHandler(frm, index, DayGridGadgetCallback);
  FrmSetEventHandler(frm, SelectDayHandleEvent);

  previous = FrmGetActiveForm();
  button = FrmDoDialog(frm);

  if (button == 10101 || button == 10118 || data.selected) { // "OK" or "Today" buttons
    *year = data.date.year;
    *month = data.date.month;
    *day = data.date.day;
    r = true;
  }
  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return r;
}

Boolean SelectDayV10(Int16 *month, Int16 *day, Int16 *year, const Char *title) {
  return SelectDay(selectDayByDay, month, day, year, title);
}
