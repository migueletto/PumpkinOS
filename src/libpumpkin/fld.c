inline void FldCopy(const FieldType *fld) {
  pumpkin_system_call(sysTrapFldCopy, 0, NULL, NULL, fld);
}

inline void FldCut(FieldType *fld) {
  pumpkin_system_call(sysTrapFldCut, 0, NULL, NULL, fld);
}

inline void FldDrawField(FieldType *fld) {
  pumpkin_system_call(sysTrapFldDrawField, 0, NULL, NULL, fld);
}

inline void FldEraseField(FieldType *fld) {
  pumpkin_system_call(sysTrapFldEraseField, 0, NULL, NULL, fld);
}

inline void FldFreeMemory(FieldType *fld) {
  pumpkin_system_call(sysTrapFldFreeMemory, 0, NULL, NULL, fld);
}

inline void FldGetBounds(const FieldType *fld, RectangleType *rect) {
  pumpkin_system_call(sysTrapFldGetBounds, 0, NULL, NULL, fld, rect);
}

inline char *FldGetTextPtr(const FieldType *fld) {
  void *pret;
  pumpkin_system_call(sysTrapFldGetTextPtr, 0, NULL, &pret, fld);
  return (char *)pret;
}

inline void FldGetSelection(const FieldType *fld, UInt16 *startPosition, UInt16 *endPosition) {
  pumpkin_system_call(sysTrapFldGetSelection, 0, NULL, NULL, fld, startPosition, endPosition);
}

inline Boolean FldHandleEvent(FieldType *fld, EventType *event) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldHandleEvent, 0, &iret, NULL, fld, event);
  return (Boolean)iret;
}

inline void FldPaste(FieldType *fld) {
  pumpkin_system_call(sysTrapFldPaste, 0, NULL, NULL, fld);
}

inline void FldRecalculateField(FieldType *fld, Boolean redraw) {
  pumpkin_system_call(sysTrapFldRecalculateField, 0, NULL, NULL, fld, redraw);
}

inline void FldSetBounds(FieldType *fld, const RectangleType *rect) {
  pumpkin_system_call(sysTrapFldSetBounds, 0, NULL, NULL, fld, rect);
}

inline void FldSetText(FieldType *fld, MemHandle textHandle, UInt16 offset, UInt16 size) {
  pumpkin_system_call(sysTrapFldSetText, 0, NULL, NULL, fld, textHandle, offset, size);
}

inline FontID FldGetFont(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetFont, 0, &iret, NULL, fld);
  return (FontID)iret;
}

inline void FldSetFont(FieldType *fld, FontID fontID) {
  pumpkin_system_call(sysTrapFldSetFont, 0, NULL, NULL, fld, fontID);
}

inline void FldSetSelection(FieldType *fld, UInt16 startPosition, UInt16 endPosition) {
  pumpkin_system_call(sysTrapFldSetSelection, 0, NULL, NULL, fld, startPosition, endPosition);
}

inline void FldGrabFocus(FieldType *fld) {
  pumpkin_system_call(sysTrapFldGrabFocus, 0, NULL, NULL, fld);
}

inline void FldReleaseFocus(FieldType *fld) {
  pumpkin_system_call(sysTrapFldReleaseFocus, 0, NULL, NULL, fld);
}

inline UInt16 FldGetInsPtPosition(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetInsPtPosition, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline void FldSetInsPtPosition(FieldType *fld, UInt16 pos) {
  pumpkin_system_call(sysTrapFldSetInsPtPosition, 0, NULL, NULL, fld, pos);
}

inline void FldSetScrollPosition(FieldType *fld, UInt16 pos) {
  pumpkin_system_call(sysTrapFldSetScrollPosition, 0, NULL, NULL, fld, pos);
}

inline UInt16 FldGetScrollPosition(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetScrollPosition, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline UInt16 FldGetTextHeight(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetTextHeight, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline UInt16 FldGetTextAllocatedSize(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetTextAllocatedSize, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline UInt16 FldGetTextLength(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetTextLength, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline void FldScrollField(FieldType *fld, UInt16 linesToScroll, WinDirectionType direction) {
  pumpkin_system_call(sysTrapFldScrollField, 0, NULL, NULL, fld, linesToScroll, direction);
}

inline Boolean FldScrollable(const FieldType *fld, WinDirectionType direction) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldScrollable, 0, &iret, NULL, fld, direction);
  return (Boolean)iret;
}

inline UInt16 FldGetVisibleLines(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetVisibleLines, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline void FldGetAttributes(const FieldType *fld, FieldAttrPtr attrP) {
  pumpkin_system_call(sysTrapFldGetAttributes, 0, NULL, NULL, fld, attrP);
}

inline void FldSetAttributes(FieldType *fld, const FieldAttrType *attrP) {
  pumpkin_system_call(sysTrapFldSetAttributes, 0, NULL, NULL, fld, attrP);
}

inline void FldSendChangeNotification(const FieldType *fld) {
  pumpkin_system_call(sysTrapFldSendChangeNotification, 0, NULL, NULL, fld);
}

inline UInt16 FldCalcFieldHeight(const char *chars, UInt16 maxWidth) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldCalcFieldHeight, 0, &iret, NULL, chars, maxWidth);
  return (UInt16)iret;
}

inline void *FldGetTextHandle(const FieldType *fld) {
  void *pret;
  pumpkin_system_call(sysTrapFldGetTextHandle, 0, NULL, &pret, fld);
  return (void *)pret;
}

inline void FldCompactText(FieldType *fld) {
  pumpkin_system_call(sysTrapFldCompactText, 0, NULL, NULL, fld);
}

inline Boolean FldDirty(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldDirty, 0, &iret, NULL, fld);
  return (Boolean)iret;
}

inline UInt16 FldWordWrap(const char *chars, Int16 maxWidth) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldWordWrap, 0, &iret, NULL, chars, maxWidth);
  return (UInt16)iret;
}

inline void FldSetTextAllocatedSize(FieldType *fld, UInt16 allocatedSize) {
  pumpkin_system_call(sysTrapFldSetTextAllocatedSize, 0, NULL, NULL, fld, allocatedSize);
}

inline void FldSetTextHandle(FieldType *fld, MemHandle textHandle) {
  pumpkin_system_call(sysTrapFldSetTextHandle, 0, NULL, NULL, fld, textHandle);
}

inline void FldSetTextPtr(FieldType *fld, char *text) {
  pumpkin_system_call(sysTrapFldSetTextPtr, 0, NULL, NULL, fld, text);
}

inline UInt16 FldGetMaxChars(const FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldGetMaxChars, 0, &iret, NULL, fld);
  return (UInt16)iret;
}

inline void FldSetMaxChars(FieldType *fld, UInt16 maxChars) {
  pumpkin_system_call(sysTrapFldSetMaxChars, 0, NULL, NULL, fld, maxChars);
}

inline void FldSetUsable(FieldType *fld, Boolean usable) {
  pumpkin_system_call(sysTrapFldSetUsable, 0, NULL, NULL, fld, usable);
}

inline Boolean FldInsert(FieldType *fld, const char *insertChars, UInt16 insertLen) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldInsert, 0, &iret, NULL, fld, insertChars, insertLen);
  return (Boolean)iret;
}

inline void FldDelete(FieldType *fld, UInt16 start, UInt16 end) {
  pumpkin_system_call(sysTrapFldDelete, 0, NULL, NULL, fld, start, end);
}

inline void FldUndo(FieldType *fld) {
  pumpkin_system_call(sysTrapFldUndo, 0, NULL, NULL, fld);
}

inline void FldSetDirty(FieldType *fld, Boolean dirty) {
  pumpkin_system_call(sysTrapFldSetDirty, 0, NULL, NULL, fld, dirty);
}

inline void FldSendHeightChangeNotification(const FieldType *fld, UInt16 pos, Int16 numLines) {
  pumpkin_system_call(sysTrapFldSendHeightChangeNotification, 0, NULL, NULL, fld, pos, numLines);
}

inline Boolean FldMakeFullyVisible(FieldType *fld) {
  uint64_t iret;
  pumpkin_system_call(sysTrapFldMakeFullyVisible, 0, &iret, NULL, fld);
  return (Boolean)iret;
}
