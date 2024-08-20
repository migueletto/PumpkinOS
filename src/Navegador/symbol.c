#include <PalmOS.h>
#include <BmpGlue.h>

#include "map.h"
#include "gps.h"
#include "app.h"
#include "gui.h"
#include "misc.h"
#include "symbol.h"
#include "gpc.h"
#include "label.h"
#include "error.h"

#include "debug.h"

typedef struct {
  UInt16 symbol, index;
} SymbolIndex;

typedef struct {
  MemHandle hbmp;
  BitmapPtr bmp;
} SymbolBitmap;

static UInt16 first;
static MemHandle hsymbol = NULL;
static UInt16 *psymbol = NULL;
static MemHandle hcategory = NULL;
static UInt16 *pcategory = NULL;
static SymbolIndex *pindex = NULL;
static SymbolBitmap *pbitmap = NULL;

static Int16 compare(void *e1, void *e2, Int32 other);
static Int16 ccompare(const void *e1, const void *e2, Int32 other);

Err SymbolInit(Boolean highDensity)
{
  UInt16 i;

  if ((hsymbol = DmGetResource(wrdListRscType, symbolType)) == NULL) {
    return -1;
  }

  if ((psymbol = MemHandleLock(hsymbol)) == NULL) {
    DmReleaseResource(hsymbol);
    return -1;
  }

  if ((hcategory = DmGetResource(wrdListRscType, symbolCategory)) == NULL) {
    MemHandleUnlock(hsymbol);
    DmReleaseResource(hsymbol);
    return -1;
  }

  if ((pcategory = MemHandleLock(hcategory)) == NULL) {
    MemHandleUnlock(hsymbol);
    DmReleaseResource(hsymbol);
    DmReleaseResource(hcategory);
    return -1;
  }

  if ((pindex = MemPtrNew(psymbol[0] * sizeof(SymbolIndex))) == NULL) {
    MemHandleUnlock(hsymbol);
    DmReleaseResource(hsymbol);
    MemHandleUnlock(hcategory);
    DmReleaseResource(hcategory);
    return -1;
  }

  if ((pbitmap = MemPtrNew(psymbol[0] * sizeof(SymbolBitmap))) == NULL) {
    MemPtrFree(pindex);
    MemHandleUnlock(hsymbol);
    DmReleaseResource(hsymbol);
    MemHandleUnlock(hcategory);
    DmReleaseResource(hcategory);
    return -1;
  }

  for (i = 0; i < psymbol[0]; i++) {
    pindex[i].symbol = psymbol[i+1];
    pindex[i].index = i;
  }

  SysQSort(pindex, psymbol[0], sizeof(SymbolIndex), compare, 0);

  for (i = 0; i < psymbol[0]; i++) {
    pbitmap[i].hbmp = NULL;
    pbitmap[i].bmp = NULL;
  }

  return 0;
}

void SymbolFinish(void)
{
  Int16 i;

  if (pbitmap) {
    for (i = 0; i < psymbol[0]; i++) {
      if (pbitmap[i].hbmp) {
        MemHandleUnlock(pbitmap[i].hbmp);
        DmReleaseResource(pbitmap[i].hbmp);
      }
    }
    MemPtrFree(pbitmap);
  }

  if (pindex)
    MemPtrFree(pindex);

  if (hsymbol) {
    MemHandleUnlock(hsymbol);
    DmReleaseResource(hsymbol);
  }

  if (hcategory) {
    MemHandleUnlock(hcategory);
    DmReleaseResource(hcategory);
  }
}

UInt16 GetSymbolCount(void)
{
  return ResLoadConstant(symbolId);
}

UInt16 GetSymbolIndex(UInt16 symbol)
{
  SymbolIndex rec;
  Int32 pos;

  rec.symbol = symbol;

  if (!SysBinarySearch(pindex, psymbol[0], sizeof(SymbolIndex), ccompare, &rec, 0, &pos, true)) {
    return 0xFFFF;
  }

  return pindex[pos].index;
}

UInt16 GetSymbolType(UInt16 i)
{
  return i < psymbol[0] ? psymbol[i+1] : 0;
}

UInt16 GetSymbolCategory(UInt16 i)
{
  return i < pcategory[0] ? pcategory[i+1] : 0;
}

char *GetSymbolName(UInt16 symbol)
{
  static char name[32];
  StrCopy(name, "Not found");
  SysStringByIndex(symbolId, symbol, name, sizeof(name)-1);
  return name;
}

void DrawSymbolTable(RectangleType *bounds, Int16 first)
{
  UInt16 i, n, bx, by;
  FormPtr frm;
  ControlPtr ctl;
  RectangleType rect;

  frm = FrmGetActiveForm();
  n = GetSymbolCount();
  bx = by = 0;

  for (i = 0; i < 40; i++) {
    RctSetRectangle(&rect, bounds->topLeft.x+bx, bounds->topLeft.y+by, 16, 16);
    WinEraseRectangle(&rect, 0);
    if ((first + i) < n) {
      DrawSymbol(symbolId + first + i,  bounds->topLeft.x+bx+8, bounds->topLeft.y+by+8, true, NULL, 0, 0, true);
    }
    bx += 16;
    if (bx >= bounds->extent.x) {
      bx = 0;
      by += 16;
    }   
    if (by >= bounds->extent.y) break;
  }
  
  ctl = (ControlPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, upBtn));
  if (first >= 40) {
    CtlSetLabel(ctl, "\001");
    CtlSetEnabled(ctl, true);
  } else {  
    CtlSetLabel(ctl, "\003");
    CtlSetEnabled(ctl, false);
  }
  ctl = (ControlPtr)FrmGetObjectPtr(frm,FrmGetObjectIndex(frm,downBtn));
  if ((first + 40) < n) {
    CtlSetLabel(ctl, "\002");
    CtlSetEnabled(ctl, true);
  } else {
    CtlSetLabel(ctl, "\004");
    CtlSetEnabled(ctl, false);
  }
}

Int16 SelectSymbol(Int16 ex, Int16 ey, Int16 first)
{
  FormPtr frm;
  UInt16 index, n, i, x, y;
  RectangleType rect;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, symbolTbl);
  FrmGetObjectBounds(frm, index, &rect);

  if (RctPtInRectangle(ex, ey, &rect)) {
    n = GetSymbolCount();
    x = (ex - rect.topLeft.x) / 16;
    y = (ey - rect.topLeft.y) / 16;
    i = y * 8 + x;
    if ((first + i) < n) {
      SndPlaySystemSound(sndClick);
      FldInsertStr(frm, symbolFld, GetSymbolName(first+i));
      return first+i;
    }
  }

  return -1;
}

Boolean SymbolFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  UInt16 n;
  Boolean handled;
  Int16 i;
  static Int16 selected = 0;
  static RectangleType rect;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, symbolTbl), &rect);
      selected = GetSymbolIndex(GetSelectedSymbol());
      first = (selected / 40) * 40;
      FrmDrawForm(frm);
      DrawSymbolTable(&rect, first);
      FldInsertStr(frm, symbolFld, GetSymbolName(selected));
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case penUpEvent:
      i = SelectSymbol(event->screenX, event->screenY, first);
      if (i != -1) {
        selected = i;
        handled = true;
      }
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case upBtn:
          if (first >= 40) {
            first -= 40;
            DrawSymbolTable(&rect, first);
          }
          break;
        case downBtn:
          n = GetSymbolCount();
          if ((first + 40) < n) {
            first += 40;
            DrawSymbolTable(&rect, first);
          }
          break;
        case okBtn:
          SetSelectedSymbol(GetSymbolType(selected));
          PopForm();
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

void DrawSymbol(UInt16 id, Int16 x, Int16 y, Boolean centered,
                char *name, FontID font, UInt16 color, Boolean now)
{
  MemHandle h;
  BitmapPtr bmp;
  UInt16 bwidth, bheight, rowBytes;
  Int16 dx, len, index;
  
  if (id) {
    index = id - symbolId;

    if (!pbitmap[index].hbmp) {
      if ((h = DmGetResource(bitmapRsc, id)) == NULL)
        return;

      if ((bmp = (BitmapPtr)MemHandleLock(h)) == NULL) {
        DmReleaseResource(h);
        return;
      }

      pbitmap[index].hbmp = h;
      pbitmap[index].bmp = bmp;
    } else
      bmp = pbitmap[index].bmp;
    
    BmpGlueGetDimensions(bmp, (Coord *)&bwidth, (Coord *)&bheight, &rowBytes);

    if (now) {
      if (centered)
        WinDrawBitmap(bmp, x-bwidth/2, y-bheight/2);
      else
        WinDrawBitmap(bmp, x, y);
    } else {
      if (centered)
        LabelAdd(-1, x-bwidth/2, y-bheight/2, font, color, 0, (char *)bmp);
      else
        LabelAdd(-1, x, y, font, color, 0, (char *)bmp);
    }
  } else
    bheight = 0;
  
  if (name) {
    FntSetFont(font);
    len = StrLen(name);
    dx = FntCharsWidth(name, len);

    if (now) {
      WinSetTextColor(color);
      WinSetDrawMode(winOverlay);
      WinPaintChars(name, len, x-dx/2, y+bheight/2);
    } else
      LabelAdd(MAP_POINT, x-dx/2, y+bheight/2, font, color, 0, name);
  }
}

static Int16 compare(void *e1, void *e2, Int32 other)
{
  SymbolIndex *i1 = (SymbolIndex *)e1;
  SymbolIndex *i2 = (SymbolIndex *)e2;
  if (i1->symbol < i2->symbol) return -1;
  if (i1->symbol > i2->symbol) return 1;
  return 0;
}

static Int16 ccompare(const void *e1, const void *e2, Int32 other) {
  return compare((void *)e1, (void *)e2, other);
}
