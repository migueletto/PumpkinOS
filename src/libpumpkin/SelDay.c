#include <PalmOS.h>

#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define NCOLS 7
#define NROWS 7

#define PALMOS_MODULE "DateTime"

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
  DateTimeType *date;
  Int16 i, cellx, celly, col, row, index, ndays, dow;

  if (cmd == formGadgetDeleteCmd) return true;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, 10120); // gadget
  FrmGetObjectBounds(frm, index, &rect);
  date = FrmGetGadgetData(frm, index);

  switch (cmd) {
    case formGadgetDrawCmd:
      drawGrid(&rect, date);
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
            ndays = DaysInMonth(date->month, date->year);
            dow = DayOfWeek(date->month, 1, date->year);
            i = row * NCOLS + col;
            if (i >= NCOLS+dow && i < ndays+NCOLS+dow) {
              debug(DEBUG_TRACE, PALMOS_MODULE, "SelDay grid select i=%d", i);
              date->day = i-NCOLS-dow+1;
              drawGrid(&rect, date);
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
  DateTimeType *date;
  RectangleType rect;
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
      date = FrmGetGadgetData(frm, index);

      if (id >= 1 && id <= 12) {
        date->month = id;
        ndays = DaysInMonth(date->month, date->year);
        if (date->day > ndays) date->day = ndays;
        drawGrid(&rect, date);
        handled = true;
      } else {
        switch (id) {
          case 10122: // this month
          case 10121: // this week
            // XXX not implemented
            break;
          case 10118: // today
            TimSecondsToDateTime(TimGetSeconds(), date);
            // fall through
          case 10103:
          case 10104:
            if (id == 10103) date->year--;
            else if (id == 10104) date->year++;
            ndays = DaysInMonth(date->month, date->year);
            if (date->day > ndays) date->day = ndays;
            StrPrintF(buf, "%d", date->year);
            FrmCopyLabel(frm, 10102, buf);
            index = FrmGetObjectIndex(frm, 10102);
            FrmEraseObject(frm, index, false);
            FrmDrawObject(frm, index, false);
            drawGrid(&rect, date);
            handled = id != 10118;
            break;
          default:
            break;
        }
      }
      break;

    default:
      break;
  }

  return handled;
}

Boolean SelectDay(const SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char *title) {
  FormType *frm, *previous;
  ControlType *ctl;
  DateTimeType date;
  UInt16 index;
  char buf[16];
  Boolean r = false;

  if (month == NULL || day == NULL || year == NULL) return false;
  if (*month < 1 || *month > 12) return false;
  date.year = *year;
  date.month = *month;
  date.day = *day;

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
  FrmSetGadgetData(frm, index, &date);
  FrmSetGadgetHandler(frm, index, DayGridGadgetCallback);
  FrmSetEventHandler(frm, SelectDayHandleEvent);

  previous = FrmGetActiveForm();
  if (FrmDoDialog(frm) == 10101) { // "OK" button
    r = true;
  }
  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return r;
}
