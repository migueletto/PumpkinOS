#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "gui.h"
#include "ddb.h"
#include "list.h"
#include "format.h"
#include "map.h"
#include "mapdecl.h"
#include "misc.h"
#include "symbol.h"
#include "error.h"
#include "object.h"

#include "debug.h"

#define MAX_LISTS 4

static Boolean hd, pos_valid;
static UInt16 num;
static UInt16 top[MAX_LISTS];
static DmOpenRef dbRef[MAX_LISTS];
static UInt16 newForm[MAX_LISTS], editForm[MAX_LISTS], seekForm[MAX_LISTS];
static Int16 (*getobjname[MAX_LISTS])(void *, char *);
static Int16 (*getobjdetail[MAX_LISTS])(void *, char *);
static UInt32 (*getobjdata[MAX_LISTS])(void *);
static UInt32 (*getobjdynamic[MAX_LISTS])(void *);
static void (*getobjcenter[MAX_LISTS])(void *, double *, double *);
static Int16 (*compare[MAX_LISTS])(void *, void *, Int32);

void ObjectInit(Boolean highDensity)
{
  hd = highDensity;
  pos_valid = false;
}

void ObjectDefine(UInt16 num, DmOpenRef _dbRef,
                  UInt16 _newForm, UInt16 _editForm, UInt16 _seekForm,
                  Int16 (*_getobjname)(void *, char *),
                  Int16 (*_getobjdetail)(void *, char *),
                  UInt32 (*_getobjdata)(void *),
                  UInt32 (*_getobjdynamic)(void *),
                  void (*_getobjcenter)(void *, double *, double *),
                  Int16 _compare(void *, void *, Int32))
{
  top[num] = 0;
  dbRef[num] = _dbRef;
  newForm[num] = _newForm;
  editForm[num] = _editForm;
  seekForm[num] = _seekForm;
  getobjname[num] = _getobjname;
  getobjdetail[num] = _getobjdetail;
  getobjdata[num] = _getobjdata;
  getobjdynamic[num] = _getobjdynamic;
  getobjcenter[num] = _getobjcenter;
  compare[num] = _compare;

  BuildRecList(num, dbRef[num], getobjname[num], getobjdetail[num],
    getobjdata[num], getobjdynamic[num], compare[num]);
}

void ObjectSelect(UInt16 _num)
{
  num = _num;
}

static void ObjectDrawCell(void *t, Int16 row, Int16 col, RectangleType *rect)
{
  TableType *tbl;
  UInt16 i, sel, symbol, tc = 0, fg = 0, bg = 0;
  UInt16 max = 0, part = 0, dx = 0;
  UInt32 data;
  char *name, *detail;

  tbl = t;
  i = TblGetRowData(tbl, row);

  if (i < GetRecNum(num)) {
    name = GetRecName(num, i);
    detail = GetRecDetail(num, i);
    data = GetRecData(num, i);
    sel = GetRecSelection(num);

    if (i == sel) {
      tc = WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
      fg = WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
      bg = WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
      WinSetClip(rect);
      WinFillRectangle(rect, 0);
    }

    if (hd && data != 0xFFFFFFFFL) {
      symbol = hsymbolId + GetSymbolIndex(data);
      DrawBmp(symbol, rect->topLeft.x+9, rect->topLeft.y+9, true);
      rect->topLeft.x += 12;
      rect->extent.x -= 12;
    }

    if (detail) {
      max = rect->topLeft.x + rect->extent.x;
      part = FntCharWidth('a');
      dx = 12 * part;
      rect->extent.x = dx;
    }

    WinSetClip(rect);
    WinPaintChars(name, StrLen(name), rect->topLeft.x, rect->topLeft.y);

    if (detail) {
      rect->topLeft.x += dx + part;
      rect->extent.x = max - rect->topLeft.x;
      WinSetClip(rect);
      WinPaintChars(detail, StrLen(detail), rect->topLeft.x, rect->topLeft.y);
    }

    if (i == sel) {
      WinSetTextColor(tc);
      WinSetForeColor(fg);
      WinSetBackColor(bg);
    }

    WinResetClip();
  }
}

static void AlignObjectControls(FormPtr frm) {
  RectangleType rect;
  UInt32 swidth, sheight;
  UInt16 index, dx, dy;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  dx = swidth - 160;
  dy = sheight - 160;

  index = FrmGetObjectIndex(frm, objectTbl);
  FrmGetObjectBounds(frm, index, &rect);
  rect.extent.x += dx;
  rect.extent.y += dy;
  FrmSetObjectBounds(frm, index, &rect);

  index = FrmGetObjectIndex(frm, objectScl);
  FrmGetObjectBounds(frm, index, &rect);
  rect.extent.y += dy;
  FrmSetObjectBounds(frm, index, &rect);
  FrmObjectRightAlign(frm, index, -1);

  if (newForm[num]) {
    FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, newBtn), -1);
  }

  if (editForm[num]) {
    FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, editBtn), -1);
  }

  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, deleteBtn), -1);

  if (getobjcenter[num]) {
    FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, centerBtn), -1);
  }

  if (seekForm[num]) {
    FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, seekBtn), -1);
  }
}

Boolean ObjectListHandleEvent(EventPtr event)
{
  FormPtr frm;
  UInt16 i, index, size, sel, rows, chr;
  double lat, lon;
  TableType *tbl;
  char c;
  Err err;
  void *p;
  AppPrefs *prefs;
  Boolean handled, changed;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resizeForm(frm);

      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),
          LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),
          StatusGadgetCallback);
      AlignObjectControls(frm);
      AlignUpperGadgets(frm);

      FrmDrawForm(frm);
      ObjectRefresh(frm);
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      ObjectRefresh(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;
  
    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    case sclRepeatEvent: 
    case sclExitEvent: 
      top[num] = event->data.sclRepeat.newValue;
      ObjectRefresh(FrmGetActiveForm());
      break;

    case keyDownEvent:
      frm = FrmGetActiveForm();
      tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, objectTbl));
      rows = TblGetNumberOfRows(tbl);

      chr = event->data.keyDown.chr;
      size = GetRecNum(num);
      changed = false;

      if (event->data.keyDown.modifiers & commandKeyMask) {
        switch (chr) {
          case pageDownChr:
            if (size > rows) {
              top[num] += rows;
              if (top[num] >= size)
                top[num] = size-1;
              changed = true;
            }
            handled = true;
            break;
          case pageUpChr:
            if (size > rows) {
              if (top[num] >= rows)
                top[num] -= rows;
              else
                top[num] = 0;
              changed = true;
            }
            handled = true;
        }
      } else if (TxtCharIsAlNum(chr)) {
        if (chr >= 'a' && chr <= 'z')
          chr &= 0xdf;
        for (i = 0; i < size; i++) {
          c = GetRecName(num, i)[0];
          if (c >= 'a' && c <= 'z')
            c &= 0xdf;
          if (c == chr) {
            sel = i;
            if (sel < rows)
              top[num] = 0;
            else
              top[num] = sel-rows/2;
            SetRecSelection(num, i);
            changed = true;
            break;
          }
        }
        handled = true;
      }

      if (changed)
        ObjectRefresh(FrmGetActiveForm());
      break;

    case tblEnterEvent:
      tbl = event->data.tblSelect.pTable;
      rows = TblGetNumberOfRows(tbl);

      sel = GetRecSelection(num);
      size = GetRecNum(num);

      if (TblGetRowData(tbl, event->data.tblSelect.row) < size) {
        UInt16 tmp = TblGetRowData(tbl, event->data.tblSelect.row);
        if (tmp != sel) {
          SetRecSelection(num, tmp);
          TblMarkRowInvalid(tbl, event->data.tblSelect.row);
          if (sel >= top[num] && sel < top[num]+rows) {
            TblMarkRowInvalid(tbl, sel-top[num]);
          }
          TblRedrawTable(tbl);
        }
      }
      handled = true;
      break;

    case ctlSelectEvent:
      handled = true;

      frm = FrmGetActiveForm();
      sel = GetRecSelection(num);
      size = GetRecNum(num);

      switch (event->data.ctlSelect.controlID) {
        case newBtn:
          if (newForm[num])
            FrmPopupForm(newForm[num]);
          break;

        case editBtn:
          if (size == 0)
            break;

          if (editForm[num])
            FrmPopupForm(editForm[num]);
          break;

        case deleteBtn:
          if (size == 0)
            break;

          if (FrmAlert(DeleteAlert) != 0)
            break;

          index = GetRecIndex(num, sel);
          err = DbDeleteRec(dbRef[num], index);

          if (err)
            InfoDialog(ERROR, "Error deleting record (%d)", err);
          else {
            BuildRecList(num, dbRef[num], getobjname[num], getobjdetail[num],
              getobjdata[num], getobjdynamic[num], compare[num]);
            sel = GetRecSelection(num);
            size = GetRecNum(num);
            if (sel >= size)
              SetRecSelection(num, size-1);
            ObjectRefresh(frm);
            MapInvalid();
          }
          break;

        case centerBtn:
          if (size == 0)
            break;

          if (getobjcenter[num] == NULL)
            break;

          index = GetRecIndex(num, sel);

          p = (void *)DbOpenRec(dbRef[num], index, &err);
          if (p && !err) {
            getobjcenter[num](p, &lat, &lon);
            prefs = GetPrefs();
            prefs->locked = false;
            MapLock(false);
            MapCenter(lat, lon);
            DbCloseRec(dbRef[num], index, (char *)p);
            prefs->display = MapForm;
            FrmGotoForm(MapForm);
          }
          break;

        case seekBtn:
          if (size == 0)
            break;

          if (seekForm[num]) {
            if (pos_valid)
              FrmGotoForm(seekForm[num]);
            else
              InfoDialog(INFO, "There is no fix");
          }
      }
      break;

    default:
      break;
  }

  return handled;
}

void ObjectRefresh(FormPtr frm)
{
  UInt16 i, size, rows;
  TableType *tbl;
  ScrollBarType *scl;

  tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, objectTbl));
  scl = (ScrollBarType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, objectScl));

  size = GetRecNum(num);
  rows = TblGetNumberOfRows(tbl);

  if (size > rows) {
    SclSetScrollBar(scl, top[num], 0, size-1, rows);
    TblHasScrollBar(tbl, true);
  } else {
    top[num] = 0;
    SclSetScrollBar(scl, 0, 0, 0, 0);
    TblHasScrollBar(tbl, false);
  }

  for (i = 0; i < rows; i++) {
    TblSetRowUsable(tbl, i, top[num]+i < size);
    TblSetItemStyle(tbl, i, 0, customTableItem);
    TblSetRowData(tbl, i, top[num]+i);
  }

  TblSetColumnUsable(tbl, 0, true);
  TblSetCustomDrawProcedure(tbl, 0, ObjectDrawCell);

  TblMarkTableInvalid(tbl);
  TblRedrawTable(tbl);
}

void ObjectSetTop(UInt16 i)
{
  UInt16 size;

  top[num] = i;
  size = GetRecNum(num);
  if (top[num] >= size)
    top[num] = size-1;
}

void ObjectPositionValid(Boolean valid)
{
  pos_valid = valid;
}
