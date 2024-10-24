inline void AbtShowAbout(UInt32 creator) {
  pumpkin_system_call_p(sysTrapAbtShowAbout, 0, NULL, NULL, creator);
}

inline void AttnIndicatorEnable(Boolean enableIt) {
  pumpkin_system_call_p(sysTrapAttnIndicatorEnable, 0, NULL, NULL, enableIt);
}

inline void CategoryCreateList(DmOpenRef db, ListType *listP, UInt16 currentCategory, Boolean showAll, Boolean showUneditables, UInt8 numUneditableCategories, UInt32 editingStrID, Boolean resizeList) {
  pumpkin_system_call_p(sysTrapCategoryCreateList, 0, NULL, NULL, db, listP, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
}

inline MemHandle DmGetResource(DmResType type, DmResID resID) {
  void *pret;
  pumpkin_system_call_p(sysTrapDmGetResource, 0, NULL, &pret, type, resID);
  return (MemHandle)pret;
}

inline Err DmReleaseResource(MemHandle resourceH) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapDmReleaseResource, 0, &iret, NULL, resourceH);
  return (Err)iret;
}

inline void ErrDisplayFileLineMsg(const Char *filename, UInt16 lineNo, const Char *msg) {
  pumpkin_system_call_p(sysTrapErrDisplayFileLineMsg, 0, NULL, NULL, filename, lineNo, msg);
}

inline void EvtGetEvent(EventType *event, Int32 timeout) {
  pumpkin_system_call_p(sysTrapEvtGetEvent, 0, NULL, NULL, event, timeout);
}

inline void EvtGetPen(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown) {
  pumpkin_system_call_p(sysTrapEvtGetPen, 0, NULL, NULL, pScreenX, pScreenY, pPenDown);
}

inline Int16 FntCharsWidth(const Char *chars, Int16 len) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFntCharsWidth, 0, &iret, NULL, chars, len);
  return (Int16)iret;
}

inline Int16 FntLineHeight(void) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFntLineHeight, 0, &iret, NULL);
  return (Int16)iret;
}

inline FontID FntSetFont(FontID font) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFntSetFont, 0, &iret, NULL, font);
  return (FontID)iret;
}

inline UInt16 FrmAlert(UInt16 alertId) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmAlert, 0, &iret, NULL, alertId);
  return (UInt16)iret;
}

inline void FrmCenterDialogs(Boolean center) {
  pumpkin_system_call_p(sysCallFrmCenterDialogs, 0, NULL, NULL, center);
}

inline void FrmCloseAllForms(void) {
  pumpkin_system_call_p(sysTrapFrmCloseAllForms, 0, NULL, NULL);
}

inline void FrmDeleteForm(FormType *formP) {
  pumpkin_system_call_p(sysTrapFrmDeleteForm, 0, NULL, NULL, formP);
}

inline Boolean FrmDispatchEvent(EventType *eventP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmDispatchEvent, 0, &iret, NULL, eventP);
  return (Boolean)iret;
}

inline UInt16 FrmDoDialog(FormType *formP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmDoDialog, 0, &iret, NULL, formP);
  return (UInt16)iret;
}

inline void FrmDrawForm(FormType *formP) {
  pumpkin_system_call_p(sysTrapFrmDrawForm, 0, NULL, NULL, formP);
}

inline FormType *FrmGetActiveForm(void) {
  void *pret;
  pumpkin_system_call_p(sysTrapFrmGetActiveForm, 0, NULL, &pret);
  return (FormType *)pret;
}

inline UInt16 FrmGetControlGroupSelection(const FormType *formP, UInt8 groupNum) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmGetControlGroupSelection, 0, &iret, NULL, formP, groupNum);
  return (UInt16)iret;
}

inline Int16 FrmGetControlValue(const FormType *formP, UInt16 objIndex) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmGetControlValue, 0, &iret, NULL, formP, objIndex);
  return (Int16)iret;
}

inline UInt16 FrmGetObjectId(const FormType *formP, UInt16 objIndex) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmGetObjectId, 0, &iret, NULL, formP, objIndex);
  return (UInt16)iret;
}

inline UInt16 FrmGetObjectIndex(const FormType *formP, UInt16 objID) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFrmGetObjectIndex, 0, &iret, NULL, formP, objID);
  return (UInt16)iret;
}

inline WinHandle FrmGetWindowHandle(const FormType *formP) {
  void *pret;
  pumpkin_system_call_p(sysTrapFrmGetWindowHandle, 0, NULL, &pret, formP);
  return (WinHandle)pret;
}

inline void FrmGotoForm(UInt16 formId) {
  pumpkin_system_call_p(sysTrapFrmGotoForm, 0, NULL, NULL, formId);
}

inline void FrmHelp(UInt16 helpMsgId) {
  pumpkin_system_call_p(sysTrapFrmHelp, 0, NULL, NULL, helpMsgId);
}

inline FormType *FrmInitForm(UInt16 rscID) {
  void *pret;
  pumpkin_system_call_p(sysTrapFrmInitForm, 0, NULL, &pret, rscID);
  return (FormType *)pret;
}

inline void FrmSetActiveForm(FormType *formP) {
  pumpkin_system_call_p(sysTrapFrmSetActiveForm, 0, NULL, NULL, formP);
}

inline void FrmSetControlGroupSelection(const FormType *formP, UInt8 groupNum, UInt16 controlID) {
  pumpkin_system_call_p(sysTrapFrmSetControlGroupSelection, 0, NULL, NULL, formP, groupNum, controlID);
}

inline void FrmSetControlValue(const FormType *formP, UInt16 objIndex, Int16 newValue) {
  pumpkin_system_call_p(sysTrapFrmSetControlValue, 0, NULL, NULL, formP, objIndex, newValue);
}

inline void FrmSetEventHandler(FormType *formP, FormEventHandlerType *handler) {
  pumpkin_system_call_p(sysTrapFrmSetEventHandler, 0, NULL, NULL, formP, handler);
}

inline Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapFtrGet, 0, &iret, NULL, creator, featureNum, valueP);
  return (Err)iret;
}

inline UInt32 KeyCurrentState(void) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapKeyCurrentState, 0, &iret, NULL);
  return (UInt32)iret;
}

inline Int16 MemCmp(const void *s1, const void *s2, Int32 numBytes) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapMemCmp, 0, &iret, NULL, s1, s2, numBytes);
  return (Int16)iret;
}

inline MemPtr MemHandleLock(MemHandle h) {
  void *pret;
  pumpkin_system_call_p(sysTrapMemHandleLock, 0, NULL, &pret, h);
  return (MemPtr)pret;
}

inline Err MemPtrUnlock(MemPtr p) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapMemPtrUnlock, 0, &iret, NULL, p);
  return (Err)iret;
}

inline void MenuEraseStatus(MenuBarType *menuP) {
  pumpkin_system_call_p(sysTrapMenuEraseStatus, 0, NULL, NULL, menuP);
}

inline MenuBarType *MenuGetActiveMenu(void) {
  void *pret;
  pumpkin_system_call_p(sysTrapMenuGetActiveMenu, 0, NULL, &pret);
  return (MenuBarType *)pret;
}

inline Boolean MenuHandleEvent(MenuBarType *menuP, EventType *event, UInt16 *error) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapMenuHandleEvent, 0, &iret, NULL, menuP, event, error);
  return (Boolean)iret;
}

inline Int16 PrefGetAppPreferences(UInt32 creator, UInt16 id, void *prefs, UInt16 *prefsSize, Boolean saved) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapPrefGetAppPreferences, 0, &iret, NULL, creator, id, prefs, prefsSize, saved);
  return (Int16)iret;
}

inline void PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, const void *prefs, UInt16 prefsSize, Boolean saved) {
  pumpkin_system_call_p(sysTrapPrefSetAppPreferences, 0, NULL, NULL, creator, id, version, prefs, prefsSize, saved);
}

inline Boolean RctPtInRectangle(Coord x, Coord y, const RectangleType *rP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapRctPtInRectangle, 0, &iret, NULL, x, y, rP);
  return (Boolean)iret;
}

inline void RctSetRectangle(RectangleType *rP, Coord left, Coord top, Coord width, Coord height) {
  pumpkin_system_call_p(sysTrapRctSetRectangle, 0, NULL, NULL, rP, left, top, width, height);
}

inline Char *StrIToA(Char *s, Int32 i) {
  void *pret;
  pumpkin_system_call_p(sysTrapStrIToA, 0, NULL, &pret, s, i);
  return (Char *)pret;
}

inline UInt16 StrLen(const Char *src) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapStrLen, 0, &iret, NULL, src);
  return (UInt16)iret;
}

inline Boolean SysHandleEvent(EventPtr eventP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapSysHandleEvent, 0, &iret, NULL, eventP);
  return (Boolean)iret;
}

inline Int16 SysRandom(Int32 newSeed) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapSysRandom, 0, &iret, NULL, newSeed);
  return (Int16)iret;
}

inline Err SysTaskDelay(Int32 delay) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapSysTaskDelay, 0, &iret, NULL, delay);
  return (Err)iret;
}

inline UInt16 SysTicksPerSecond(void) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapSysTicksPerSecond, 0, &iret, NULL);
  return (UInt16)iret;
}

inline UInt32 TimGetSeconds(void) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapTimGetSeconds, 0, &iret, NULL);
  return (UInt32)iret;
}

inline UInt32 TimGetTicks(void) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapTimGetTicks, 0, &iret, NULL);
  return (UInt32)iret;
}

inline IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapUIColorGetTableEntryIndex, 0, &iret, NULL, which);
  return (IndexedColorType)iret;
}

inline void UIColorGetTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP) {
  pumpkin_system_call_p(sysTrapUIColorGetTableEntryRGB, 0, NULL, NULL, which, rgbP);
}

inline Err UIColorSetTableEntry(UIColorTableEntries which, const RGBColorType *rgbP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapUIColorSetTableEntry, 0, &iret, NULL, which, rgbP);
  return (Err)iret;
}

inline void WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, const RectangleType *srcRect, Coord dstX, Coord dstY, WinDrawOperation mode) {
  pumpkin_system_call_p(sysTrapWinCopyRectangle, 0, NULL, NULL, srcWin, dstWin, srcRect, dstX, dstY, mode);
}

inline WinHandle WinCreateOffscreenWindow(Coord width, Coord height, WindowFormatType format, UInt16 *error) {
  void *pret;
  pumpkin_system_call_p(sysTrapWinCreateOffscreenWindow, 0, NULL, &pret, width, height, format, error);
  return (WinHandle)pret;
}

inline void WinDeleteWindow(WinHandle winHandle, Boolean eraseIt) {
  pumpkin_system_call_p(sysTrapWinDeleteWindow, 0, NULL, NULL, winHandle, eraseIt);
}

inline void WinDisplayToWindowPt(Coord *extentX, Coord *extentY) {
  pumpkin_system_call_p(sysTrapWinDisplayToWindowPt, 0, NULL, NULL, extentX, extentY);
}

inline void WinDrawBitmap(BitmapType *bitmapP, Coord x, Coord y) {
  pumpkin_system_call_p(sysTrapWinDrawBitmap, 0, NULL, NULL, bitmapP, x, y);
}

inline void WinDrawChars(const Char *chars, Int16 len, Coord x, Coord y) {
  pumpkin_system_call_p(sysTrapWinDrawChars, 0, NULL, NULL, chars, len, x, y);
}

inline void WinDrawLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  pumpkin_system_call_p(sysTrapWinDrawLine, 0, NULL, NULL, x1, y1, x2, y2);
}

inline void WinEraseRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  pumpkin_system_call_p(sysTrapWinEraseRectangle, 0, NULL, NULL, rP, cornerDiam);
}

inline void WinGetClip(RectangleType *rP) {
  pumpkin_system_call_p(sysTrapWinGetClip, 0, NULL, NULL, rP);
}

inline WinHandle WinGetDrawWindow(void) {
  void *pret;
  pumpkin_system_call_p(sysTrapWinGetDrawWindow, 0, NULL, &pret);
  return (WinHandle)pret;
}

inline void WinIndexToRGB(IndexedColorType i, RGBColorType *rgbP) {
  pumpkin_system_call_p(sysTrapWinIndexToRGB, 0, NULL, NULL, i, rgbP);
}

inline void WinPaintBitmap(BitmapType *bitmapP, Coord x, Coord y) {
  pumpkin_system_call_p(sysTrapWinPaintBitmap, 0, NULL, NULL, bitmapP, x, y);
}

inline void WinPaintLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  pumpkin_system_call_p(sysTrapWinPaintLine, 0, NULL, NULL, x1, y1, x2, y2);
}

inline void WinPaintRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  pumpkin_system_call_p(sysTrapWinPaintRectangle, 0, NULL, NULL, rP, cornerDiam);
}

inline void WinPopDrawState(void) {
  pumpkin_system_call_p(sysTrapWinPopDrawState, 0, NULL, NULL);
}

inline void WinPushDrawState(void) {
  pumpkin_system_call_p(sysTrapWinPushDrawState, 0, NULL, NULL);
}

inline IndexedColorType WinRGBToIndex(const RGBColorType *rgbP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapWinRGBToIndex, 0, &iret, NULL, rgbP);
  return (IndexedColorType)iret;
}

inline Err WinScreenMode(WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapWinScreenMode, 0, &iret, NULL, operation, widthP, heightP, depthP, enableColorP);
  return (Err)iret;
}

inline IndexedColorType WinSetBackColor(IndexedColorType backColor) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapWinSetBackColor, 0, &iret, NULL, backColor);
  return (IndexedColorType)iret;
}

inline void WinSetBounds(WinHandle winHandle, const RectangleType *rP) {
  pumpkin_system_call_p(sysTrapWinSetBounds, 0, NULL, NULL, winHandle, rP);
}

inline void WinSetClip(const RectangleType *rP) {
  pumpkin_system_call_p(sysTrapWinSetClip, 0, NULL, NULL, rP);
}

inline WinHandle WinSetDrawWindow(WinHandle winHandle) {
  void *pret;
  pumpkin_system_call_p(sysTrapWinSetDrawWindow, 0, NULL, &pret, winHandle);
  return (WinHandle)pret;
}

inline IndexedColorType WinSetForeColor(IndexedColorType foreColor) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapWinSetForeColor, 0, &iret, NULL, foreColor);
  return (IndexedColorType)iret;
}

inline IndexedColorType WinSetTextColor(IndexedColorType textColor) {
  uint64_t iret;
  pumpkin_system_call_p(sysTrapWinSetTextColor, 0, &iret, NULL, textColor);
  return (IndexedColorType)iret;
}
