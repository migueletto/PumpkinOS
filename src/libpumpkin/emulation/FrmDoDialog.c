#include <PalmOS.h>

extern ControlStyleType CtlGetStyle(ControlType *controlP) SYS_TRAP(0xA501);

UInt16 FrmDoDialog(FormType *formP) {
  FormType *previous;
  EventType event;
  UInt16 buttonID;
  Boolean stop;
  Err err;

  buttonID = 0;

  if (formP) {
    previous = FrmGetActiveForm();
    FrmSetActiveForm(formP);
    FrmDrawForm(formP);

    for (stop = false; !stop;) {
      EvtGetEvent(&event, evtWaitForever);
      if (SysHandleEvent(&event)) continue;
      if (MenuHandleEvent(NULL, &event, &err)) continue;

      if (!FrmDispatchEvent(&event)) {
        switch (event.eType) {
          case ctlSelectEvent:
            if (CtlGetStyle(event.data.ctlSelect.pControl) == buttonCtl) {
              buttonID = event.data.ctlSelect.controlID;
              stop = true;
            }
            break;
          case appStopEvent:
            stop = true;
            break;
          default:
            break;
        }
      }
    }

    FrmEraseForm(formP);
    FrmSetActiveForm(previous);
  }

  return buttonID;
}
