inline void LstSetDrawFunction(ListType *lst, ListDrawDataFuncPtr func) {
  pumpkin_system_call(sysTrapLstSetDrawFunction, 0, NULL, NULL, lst, func);
}

inline void LstDrawList(ListType *lst) {
  pumpkin_system_call(sysTrapLstDrawList, 0, NULL, NULL, lst);
}

inline void LstEraseList(ListType *lst) {
  pumpkin_system_call(sysTrapLstEraseList, 0, NULL, NULL, lst);
}

inline Int16 LstGetSelection(const ListType *lst) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstGetSelection, 0, &iret, NULL, lst);
  return (Int16)iret;
}

inline char *LstGetSelectionText(const ListType *lst, Int16 itemNum) {
  void *pret;
  pumpkin_system_call(sysTrapLstGetSelectionText, 0, NULL, &pret, lst, itemNum);
  return (char *)pret;
}

inline Boolean LstHandleEvent(ListType *lst, const EventType *event) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstHandleEvent, 0, &iret, NULL, lst, event);
  return (Boolean)iret;
}

inline void LstSetHeight(ListType *lst, Int16 visibleItems) {
  pumpkin_system_call(sysTrapLstSetHeight, 0, NULL, NULL, lst, visibleItems);
}

inline void LstSetSelection(ListType *lst, Int16 itemNum) {
  pumpkin_system_call(sysTrapLstSetSelection, 0, NULL, NULL, lst, itemNum);
}

inline void LstSetListChoices(ListType *lst, char **itemsText, Int16 numItems) {
  pumpkin_system_call(sysTrapLstSetListChoices, 0, NULL, NULL, lst, itemsText, numItems);
}

inline void LstMakeItemVisible(ListType *lst, Int16 itemNum) {
  pumpkin_system_call(sysTrapLstMakeItemVisible, 0, NULL, NULL, lst, itemNum);
}

inline Int16 LstGetNumberOfItems(const ListType *lst) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstGetNumberOfItems, 0, &iret, NULL, lst);
  return (Int16)iret;
}

inline Int16 LstPopupList(ListType *lst) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstPopupList, 0, &iret, NULL, lst);
  return (Int16)iret;
}

inline void LstSetPosition(ListType *lst, Coord x, Coord y) {
  pumpkin_system_call(sysTrapLstSetPosition, 0, NULL, NULL, lst, x, y);
}

inline void LstSetTopItem(ListType *lst, Int16 itemNum) {
  pumpkin_system_call(sysTrapLstSetTopItem, 0, NULL, NULL, lst, itemNum);
}

inline Boolean LstScrollList(ListType *lst, WinDirectionType direction, Int16 itemCount) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstScrollList, 0, &iret, NULL, lst, direction, itemCount);
  return (Boolean)iret;
}
  
inline Int16 LstGetVisibleItems(const ListType *lst) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstGetVisibleItems, 0, &iret, NULL, lst);
  return (Int16)iret;
} 

inline Int16 LstGetTopItem(const ListType *lst) {
  uint64_t iret;
  pumpkin_system_call(sysTrapLstGetTopItem, 0, &iret, NULL, lst);
  return (Int16)iret;
} 
