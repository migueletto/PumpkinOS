#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "gui.h"
#include "file.h"
#include "misc.h"
#include "error.h"
#include "tracks.h"

static UInt16 top;

void TrackFillList(FileType *tracks)
{
  AppPrefs *prefs;

  DestroyFileList(tracks);
  CreateFileList(AppID, LogType, tracks, LogName);

  prefs = GetPrefs();
  if (prefs->track >= tracks->n)
    prefs->track = tracks->n ? tracks->n-1 : 0;
}

void TrackDrawCell(void *t, Int16 row, Int16 col, RectangleType *rect)
{
  AppPrefs *prefs;
  TableType *tbl;
  UInt16 i, n, len, dx, tc = 0, fg = 0, bg = 0;
  char *name, buf[16];
  
  tbl = t;
  i = TblGetRowData(tbl, row);
  
  if (i < GetTrackNum()) {
    prefs = GetPrefs();

    if (i == prefs->track) {
      tc = WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
      fg = WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
      bg = WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
      WinFillRectangle(rect, 0);
    }
    
    n = GetTrackSize(i) / sizeof(TracklogType);
    StrPrintF(buf, "%u", n);
    len = StrLen(buf);
    dx = FntCharsWidth(buf, len);

    WinSetClip(rect);
    WinPaintChars(buf, len, rect->topLeft.x + rect->extent.x - dx,
      rect->topLeft.y);

    name = GetTrackName(i);
    len = StrLen(name);

    dx = FntCharsWidth("000000", 6);
    rect->extent.x -= dx;
    WinSetClip(rect);
    WinPaintChars(name, len, rect->topLeft.x, rect->topLeft.y);

    if (i == prefs->track) {
      WinSetTextColor(tc);
      WinSetForeColor(fg);
      WinSetBackColor(bg);
    }
    
    WinResetClip();
  }
}

void TrackRefresh(FormPtr frm)
{
  UInt16 i, n;
  TableType *tbl;
  ScrollBarType *scl;

  tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, trackTbl));
  scl = (ScrollBarType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, trackScl));

  n = GetTrackNum();

  if (n > trackRows) {
    SclSetScrollBar(scl, top, 0, n-1, trackRows);
    TblHasScrollBar(tbl, true);
  } else {
    top = 0;
    SclSetScrollBar(scl, 0, 0, 0, 0);
    TblHasScrollBar(tbl, false);
  }

  for (i = 0; i < trackRows; i++) {
    TblSetRowUsable(tbl, i, top+i < n);
    TblSetItemStyle(tbl, i, 0, customTableItem);
    TblSetRowData(tbl, i, top+i);
  }

  TblSetColumnUsable(tbl, 0, true);
  TblSetCustomDrawProcedure(tbl, 0, TrackDrawCell);

  TblMarkTableInvalid(tbl);
  TblRedrawTable(tbl);
}

void TrackSetTop(UInt16 t)
{
  top = t;
}

UInt16 TrackGetTop(void)
{
  return top;
}
