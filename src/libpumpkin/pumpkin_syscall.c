#include <PalmOS.h>
#include <VFSMgr.h>
#include <ExpansionMgr.h>
#include <FixedMath.h>
#include <DLServer.h>
#include <SerialMgrOld.h>
#include <UDAMgr.h>
#include <PceNativeCall.h>

#include "debug.h"

int pumpkin_system_call(syscall_lib_e lib, uint32_t id, uint32_t sel, uint64_t *iret, void **pret, ...) {
  sys_va_list ap;
  int r = 0;

  sys_va_start(ap, pret);
  switch (id) {
//#include "syscall_switch.c"
    case sysTrapAttnIndicatorEnable: {
      Boolean enableIt = sys_va_arg(ap, int32_t);
      AttnIndicatorEnable(enableIt);
      }
      break;
    case sysTrapAbtShowAbout: {
      UInt32 creator = sys_va_arg(ap, uint32_t);
      AbtShowAbout(creator);
      }
      break;
    case sysTrapCategoryCreateList: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      ListType *lst = sys_va_arg(ap, ListType *);
      UInt16 currentCategory = sys_va_arg(ap, uint32_t);
      Boolean showAll = sys_va_arg(ap, int32_t);
      Boolean showUneditables = sys_va_arg(ap, int32_t);
      UInt8 numUneditableCategories = sys_va_arg(ap, uint32_t);
      UInt32 editingStrID = sys_va_arg(ap, uint32_t);
      Boolean resizeList = sys_va_arg(ap, int32_t);
      CategoryCreateList(db, lst, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
      }
      break;
    case sysTrapDmGetResource: {
      UInt32 type = sys_va_arg(ap, uint32_t);
      UInt16 resID = sys_va_arg(ap, uint32_t);
      MemHandle h = DmGetResource(type, resID);
      *pret = h;
      }
      break;
    case sysTrapDmReleaseResource: {
      MemHandle h = sys_va_arg(ap, void *);
      Err err = DmReleaseResource(h);
      *iret = err;
      }
      break;
    case sysTrapErrDisplayFileLineMsg: {
      char *filename = sys_va_arg(ap, char *);
      UInt16 lineNo = sys_va_arg(ap, uint32_t);
      char *msg = sys_va_arg(ap, char *);
      ErrDisplayFileLineMsg(filename, lineNo, msg);
      }
      break;
    case sysTrapEvtGetEvent: {
      EventType *event = sys_va_arg(ap, EventType *);
      Int32 timeout = sys_va_arg(ap, int32_t);
      EvtGetEvent(event, timeout);
      }
      break;
    case sysTrapEvtGetPen: {
      Int16 *pScreenX = sys_va_arg(ap, Int16 *);
      Int16 *pScreenY = sys_va_arg(ap, Int16 *);
      Boolean *pPenDown = sys_va_arg(ap, Boolean *);
      EvtGetPen(pScreenX, pScreenY, pPenDown);
      }
      break;
    case sysTrapFntCharsWidth: {
      char *chars = sys_va_arg(ap, char *);
      Int16 len = sys_va_arg(ap, int32_t);
      Int16 width = FntCharsWidth(chars, len);
      *iret = width;
      }
      break;
    case sysTrapFntLineHeight: {
      Int16 height = FntLineHeight();
      *iret = height;
      }
      break;
    case sysTrapFntSetFont: {
      FontID font = sys_va_arg(ap, uint32_t);
      font = FntSetFont(font);
      *iret = font;
      }
      break;
    case sysTrapFrmAlert: {
      FontID alertId = sys_va_arg(ap, uint32_t);
      UInt16 ret = FrmAlert(alertId);
      *iret = ret;
      }
      break;
    case sysCallFrmCenterDialogs: {
      Boolean center = sys_va_arg(ap, int32_t);
      FrmCenterDialogs(center);
      }
      break;
    case sysTrapFrmCloseAllForms: {
      FrmCloseAllForms();
      }
      break;
    case sysTrapFrmDeleteForm: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FrmDeleteForm(formP);
      }
      break;
    case sysTrapFrmDispatchEvent: {
      EventType *event = sys_va_arg(ap, EventType *);
      Boolean handled = FrmDispatchEvent(event);
      *iret = handled;
      }
      break;
    case sysTrapFrmDoDialog: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmDoDialog(frm);
      *iret = ret;
      }
      break;
    case sysTrapFrmDrawForm: {
      FormType *frm = sys_va_arg(ap, FormType *);
      FrmDrawForm(frm);
      }
      break;
    case sysTrapFrmGetActiveForm: {
      FormType *frm = FrmGetActiveForm();
      *pret = frm;
      }
      break;
    case sysTrapFrmGetControlGroupSelection: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt8 groupNum = sys_va_arg(ap, uint32_t);
      UInt16 ret = FrmGetControlGroupSelection(frm, groupNum);
      *iret = ret;
      }
      break;
    case sysTrapFrmGetControlValue: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, uint32_t);
      Int16 value = FrmGetControlValue(frm, objIndex);
      *iret = value;
      }
      break;
    case sysTrapFrmGetObjectId: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, uint32_t);
      UInt16 id = FrmGetObjectId(frm, objIndex);
      *iret = id;
      }
      break;
    case sysTrapFrmGetObjectIndex: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 objID = sys_va_arg(ap, uint32_t);
      UInt16 index = FrmGetObjectIndex(frm, objID);
      *iret = index;
      }
      break;
    case sysTrapFrmGetWindowHandle: {
      FormType *frm = sys_va_arg(ap, FormType *);
      WinHandle wh = FrmGetWindowHandle(frm);
      *pret = wh;
      }
      break;
    case sysTrapFrmGotoForm: {
      UInt16 formId = sys_va_arg(ap, uint32_t);
      FrmGotoForm(formId);
      }
      break;
    case sysTrapFrmHelp: {
      UInt16 helpMsgId = sys_va_arg(ap, uint32_t);
      FrmHelp(helpMsgId);
      }
      break;
    case sysTrapFrmInitForm: {
      UInt16 rscID = sys_va_arg(ap, uint32_t);
      FormType *frm = FrmInitForm(rscID);
      *pret = frm;
      }
      break;
    case sysTrapFrmSetActiveForm: {
      FormType *frm = sys_va_arg(ap, FormType *);
      FrmSetActiveForm(frm);
      }
      break;
    case sysTrapFrmSetControlGroupSelection: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt8 groupNum = sys_va_arg(ap, uint32_t);
      UInt16 controlID = sys_va_arg(ap, uint32_t);
      FrmSetControlGroupSelection(frm, groupNum, controlID);
      }
      break;
    case sysTrapFrmSetControlValue: {
      FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, uint32_t);
      Int16 newValue = sys_va_arg(ap, int32_t);
      FrmSetControlValue(frm, objIndex, newValue);
      }
      break;
    case sysTrapFrmSetEventHandler: {
      FormType *frm = sys_va_arg(ap, FormType *);
      FormEventHandlerType *handler = sys_va_arg(ap, FormEventHandlerType *);
      FrmSetEventHandler(frm, handler);
      }
      break;
    case sysTrapFtrGet: {
      UInt32 creator = sys_va_arg(ap, uint32_t);
      UInt16 featureNum = sys_va_arg(ap, uint32_t);
      UInt32 *valueP = sys_va_arg(ap, UInt32 *);
      Err err = FtrGet(creator, featureNum, valueP);
      *iret = err;
      }
      break;
    case sysTrapKeyCurrentState: {
      UInt32 state = KeyCurrentState();
      *iret = state;
      }
      break;
    case sysTrapMemCmp: {
      void *s1 = sys_va_arg(ap, void *);
      void *s2 = sys_va_arg(ap, void *);
      Int32 numBytes = sys_va_arg(ap, int32_t);
      Int16 ret = MemCmp(s1, s2, numBytes);
      *iret = ret;
      }
      break;
    case sysTrapMemHandleLock: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      MemPtr ret = MemHandleLock(h);
      *pret = ret;
      }
      break;
    case sysTrapMemPtrUnlock: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      Err err = MemPtrUnlock(p);
      *iret = err;
      }
      break;
    case sysTrapMenuEraseStatus: {
      MenuBarType *menu = sys_va_arg(ap, MenuBarType *);
      MenuEraseStatus(menu);
      }
      break;
    case sysTrapMenuGetActiveMenu: {
      MenuBarType *menu = MenuGetActiveMenu();
      *pret = menu;
      }
      break;
    case sysTrapMenuHandleEvent: {
      MenuBarType *menu = sys_va_arg(ap, MenuBarType *);
      EventType *event = sys_va_arg(ap, EventType *);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      Boolean handled = MenuHandleEvent(menu, event, error);
      *iret = handled;
      }
      break;
    case sysTrapPrefGetAppPreferences: {
      UInt32 creator = sys_va_arg(ap, uint32_t);
      UInt16 id = sys_va_arg(ap, uint32_t);
      void *prefs = sys_va_arg(ap, void *);
      UInt16 *prefsSize = sys_va_arg(ap, UInt16 *);
      Boolean saved = sys_va_arg(ap, uint32_t);
      Int16 version = PrefGetAppPreferences(creator, id, prefs, prefsSize, saved);
      *iret = version;
      }
      break;
    case sysTrapPrefSetAppPreferences: {
      UInt32 creator = sys_va_arg(ap, uint32_t);
      UInt16 id = sys_va_arg(ap, uint32_t);
      Int16 version = sys_va_arg(ap, int32_t);
      void *prefs = sys_va_arg(ap, void *);
      UInt16 prefsSize = sys_va_arg(ap, uint32_t);
      Boolean saved = sys_va_arg(ap, uint32_t);
      PrefSetAppPreferences(creator, id, version, prefs, prefsSize, saved);
      }
      break;
    case sysTrapRctPtInRectangle: {
      Coord x = sys_va_arg(ap, int32_t);
      Coord y = sys_va_arg(ap, int32_t);
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      Boolean ret = RctPtInRectangle(x, y, rect);
      *iret = ret;
      }
      break;
    case sysTrapRctSetRectangle: {
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      Coord left = sys_va_arg(ap, int32_t);
      Coord top = sys_va_arg(ap, int32_t);
      Coord width = sys_va_arg(ap, int32_t);
      Coord height = sys_va_arg(ap, int32_t);
      RctSetRectangle(rect, left, top, width, height);
      }
      break;
    case sysTrapStrIToA: {
      char *s = sys_va_arg(ap, char *);
      Int32 i = sys_va_arg(ap, int32_t);
      s = StrIToA(s, i);
      *pret = s;
      }
      break;
    case sysTrapStrLen: {
      char *s = sys_va_arg(ap, char *);
      UInt16 len = StrLen(s);
      *iret = len;
      }
      break;
    case sysTrapSysHandleEvent: {
      EventType *event = sys_va_arg(ap, EventType *);
      Boolean handled = SysHandleEvent(event);
      *iret = handled;
      }
      break;
    case sysTrapSysRandom: {
      Int32 newSeed = sys_va_arg(ap, int32_t);
      Int16 ret = SysRandom(newSeed);
      *iret = ret;
      }
      break;
    case sysTrapSysTaskDelay: {
      Int32 delay = sys_va_arg(ap, int32_t);
      Err err = SysTaskDelay(delay);
      *iret = err;
      }
      break;
    case sysTrapSysTicksPerSecond: {
      UInt16 ticks = SysTicksPerSecond();
      *iret = ticks;
      }
      break;
    case sysTrapTimGetSeconds: {
      UInt32 seconds = TimGetSeconds();
      *iret = seconds;
      }
      break;
    case sysTrapTimGetTicks: {
      UInt32 ticks = TimGetTicks();
      *iret = ticks;
      }
      break;
    case sysTrapUIColorGetTableEntryIndex: {
      UIColorTableEntries which = sys_va_arg(ap, int32_t);
      IndexedColorType c = UIColorGetTableEntryIndex(which);
      *iret = c;
      }
      break;
    case sysTrapUIColorGetTableEntryRGB: {
      UIColorTableEntries which = sys_va_arg(ap, int32_t);
      RGBColorType *rgb = sys_va_arg(ap, RGBColorType *);
      UIColorGetTableEntryRGB(which, rgb);
      }
      break;
    case sysTrapUIColorSetTableEntry: {
      UIColorTableEntries which = sys_va_arg(ap, int32_t);
      RGBColorType *rgb = sys_va_arg(ap, RGBColorType *);
      Err err = UIColorSetTableEntry(which, rgb);
      *iret = err;
      }
      break;
    case sysTrapWinCopyRectangle: {
      WinHandle srcWin = sys_va_arg(ap, WinHandle);
      WinHandle dstWin = sys_va_arg(ap, WinHandle);
      RectangleType *srcRect = sys_va_arg(ap, RectangleType *);
      Coord dstX = sys_va_arg(ap, int32_t);
      Coord dstY = sys_va_arg(ap, int32_t);
      WinDrawOperation mode = sys_va_arg(ap, int32_t);
      WinCopyRectangle(srcWin, dstWin, srcRect, dstX, dstY, mode);
      }
      break;
    case sysTrapWinCreateOffscreenWindow: {
      Coord width = sys_va_arg(ap, int32_t);
      Coord height = sys_va_arg(ap, int32_t);
      WindowFormatType format = sys_va_arg(ap, uint32_t);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      WinHandle wh = WinCreateOffscreenWindow(width, height, format, error);
      *pret = wh;
      }
      break;
    case sysTrapWinDeleteWindow: {
      WinHandle wh = sys_va_arg(ap, WinHandle);
      Boolean eraseIt = sys_va_arg(ap, int32_t);
      WinDeleteWindow(wh, eraseIt);
      }
      break;
    case sysTrapWinDisplayToWindowPt: {
      Coord *x = sys_va_arg(ap, Coord *);
      Coord *y = sys_va_arg(ap, Coord *);
      WinDisplayToWindowPt(x, y);
      }
      break;
    case sysTrapWinDrawBitmap: {
      BitmapType *bmp = sys_va_arg(ap, BitmapType *);
      Coord x = sys_va_arg(ap, int32_t);
      Coord y = sys_va_arg(ap, int32_t);
      WinDrawBitmap(bmp, x, y);
      }
      break;
    case sysTrapWinDrawChars: {
      char *chars = sys_va_arg(ap, char *);
      Int16 len = sys_va_arg(ap, int32_t);
      Coord x = sys_va_arg(ap, int32_t);
      Coord y = sys_va_arg(ap, int32_t);
      WinDrawChars(chars, len, x, y);
      }
      break;
    case sysTrapWinDrawLine: {
      Coord x1 = sys_va_arg(ap, int32_t);
      Coord y1 = sys_va_arg(ap, int32_t);
      Coord x2 = sys_va_arg(ap, int32_t);
      Coord y2 = sys_va_arg(ap, int32_t);
      WinDrawLine(x1, y1, x2, y2);
      }
      break;
    case sysTrapWinEraseRectangle: {
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, uint32_t);
      WinEraseRectangle(rect, cornerDiam);
      }
      break;
    case sysTrapWinGetClip: {
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      WinGetClip(rect);
      }
      break;
    case sysTrapWinGetDrawWindow: {
      WinHandle wh = WinGetDrawWindow();
      *pret = wh;
      }
      break;
    case sysTrapWinIndexToRGB: {
      IndexedColorType c = sys_va_arg(ap, int32_t);
      RGBColorType *rgb = sys_va_arg(ap, RGBColorType *);
      WinIndexToRGB(c, rgb);
      }
      break;
    case sysTrapWinPaintBitmap: {
      BitmapType *bmp = sys_va_arg(ap, BitmapType *);
      Coord x = sys_va_arg(ap, int32_t);
      Coord y = sys_va_arg(ap, int32_t);
      WinPaintBitmap(bmp, x, y);
      }
      break;
    case sysTrapWinPaintLine: {
      Coord x1 = sys_va_arg(ap, int32_t);
      Coord y1 = sys_va_arg(ap, int32_t);
      Coord x2 = sys_va_arg(ap, int32_t);
      Coord y2 = sys_va_arg(ap, int32_t);
      WinPaintLine(x1, y1, x2, y2);
      }
      break;
    case sysTrapWinPaintRectangle: {
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, uint32_t);
      WinPaintRectangle(rect, cornerDiam);
      }
      break;
    case sysTrapWinPopDrawState:
      WinPopDrawState();
      break;
    case sysTrapWinPushDrawState:
      WinPushDrawState();
      break;
    case sysTrapWinRGBToIndex: {
      RGBColorType *rgb = sys_va_arg(ap, RGBColorType *);
      IndexedColorType c = WinRGBToIndex(rgb);
      *iret = c;
      }
      break;
    case sysTrapWinScreenMode: {
      WinScreenModeOperation operation = sys_va_arg(ap, int32_t);
      UInt32 *width = sys_va_arg(ap, UInt32 *);
      UInt32 *height = sys_va_arg(ap, UInt32 *);
      UInt32 *depth = sys_va_arg(ap, UInt32 *);
      Boolean *enableColor = sys_va_arg(ap, Boolean *);
      Err err = WinScreenMode(operation, width, height, depth, enableColor);
      *iret = err;
      }
      break;
    case sysTrapWinSetBackColor: {
      IndexedColorType c = sys_va_arg(ap, int32_t);
      c = WinSetBackColor(c);
      *iret = c;
      }
      break;
    case sysTrapWinSetBounds: {
      WinHandle wh = sys_va_arg(ap, WinHandle);
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      WinSetBounds(wh, rect);
      }
      break;
    case sysTrapWinSetClip: {
      RectangleType *rect = sys_va_arg(ap, RectangleType *);
      WinSetClip(rect);
      }
      break;
    case sysTrapWinSetDrawWindow: {
      WinHandle wh = sys_va_arg(ap, WinHandle);
      wh = WinSetDrawWindow(wh);
      *pret = wh;
      }
      break;
    case sysTrapWinSetForeColor: {
      IndexedColorType c = sys_va_arg(ap, int32_t);
      c = WinSetForeColor(c);
      *iret = c;
      }
      break;
    case sysTrapWinSetTextColor: {
      IndexedColorType c = sys_va_arg(ap, int32_t);
      c = WinSetTextColor(c);
      *iret = c;
      }
      break;
    default:
      debug(DEBUG_ERROR, PUMPKINOS, "invalid system call 0x%04X %d", id, sel);
      r = -1;
      break;
  }
  sys_va_end(ap);

  return r;
}
