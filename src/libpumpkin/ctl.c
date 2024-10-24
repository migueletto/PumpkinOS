inline void CtlDrawControl(ControlType *ctl) {
  pumpkin_system_call(sysTrapCtlDrawControl, 0, NULL, NULL, ctl);
}

inline void CtlEraseControl(ControlType *ctl) {
  pumpkin_system_call(sysTrapCtlEraseControl, 0, NULL, NULL, ctl);
}

inline void CtlHideControl(ControlType *ctl) {
  pumpkin_system_call(sysTrapCtlHideControl, 0, NULL, NULL, ctl);
}

inline void CtlShowControl(ControlType *ctl) {
  pumpkin_system_call(sysTrapCtlShowControl, 0, NULL, NULL, ctl);
}

inline Int16 CtlGetValue(const ControlType *ctl) {
  uint64_t iret;
  pumpkin_system_call(sysTrapCtlGetValue, 0, &iret, NULL, ctl);
  return (Int16)iret;
}

inline void CtlSetValue(ControlType *ctl, Int16 value) {
  pumpkin_system_call(sysTrapCtlSetValue, 0, NULL, NULL, ctl, value);
}

inline const char *CtlGetLabel(const ControlType *ctl) {
  void *pret;
  pumpkin_system_call(sysTrapCtlGetLabel, 0, NULL, &pret, ctl);
  return (char *)pret;
}

inline void CtlSetLabel(ControlType *ctl, const char *text) {
  pumpkin_system_call(sysTrapCtlSetLabel, 0, NULL, NULL, ctl, text);
}

inline Boolean CtlHandleEvent(ControlType *ctl, EventType *event) {
  uint64_t iret;
  pumpkin_system_call(sysTrapCtlHandleEvent, 0, &iret, NULL, ctl, event);
  return (Boolean)iret;
}

inline void CtlHitControl(const ControlType *ctl) {
  pumpkin_system_call(sysTrapCtlHitControl, 0, NULL, NULL, ctl);
}

inline void CtlSetEnabled(ControlType *ctl, Boolean enabled) {
  pumpkin_system_call(sysTrapCtlSetEnabled, 0, NULL, NULL, ctl, enabled);
}

inline void CtlSetUsable(ControlType *ctl, Boolean usable) {
  pumpkin_system_call(sysTrapCtlSetUsable, 0, NULL, NULL, ctl, usable);
}

inline Boolean CtlEnabled(const ControlType *ctl) {
  uint64_t iret;
  pumpkin_system_call(sysTrapCtlEnabled, 0, &iret, NULL, ctl);
  return (Boolean)iret;
}

inline void CtlSetGraphics(ControlType *ctl, DmResID newBitmapID, DmResID newSelectedBitmapID) {
  pumpkin_system_call(sysTrapCtlSetGraphics, 0, NULL, NULL, ctl, newBitmapID, newSelectedBitmapID);
}

inline void CtlGetSliderValues(const ControlType *ctl, UInt16 *minValue, UInt16 *maxValue, UInt16 *pageSize, UInt16 *value) {
  pumpkin_system_call(sysTrapCtlGetSliderValues, 0, NULL, NULL, ctl, minValue, maxValue, pageSize, value);
} 
  
inline void CtlSetSliderValues(ControlType *ctl, const UInt16 *minValue, const UInt16 *maxValue, const UInt16 *pageSize, const UInt16 *value) {
  pumpkin_system_call(sysTrapCtlSetSliderValues, 0, NULL, NULL, ctl, minValue, maxValue, pageSize, value);
}

inline Boolean CtlValidatePointer(const ControlType *ctl) {
  uint64_t iret;
  pumpkin_system_call(sysTrapCtlValidatePointer, 0, &iret, NULL, ctl);
  return (Boolean)iret;
} 
