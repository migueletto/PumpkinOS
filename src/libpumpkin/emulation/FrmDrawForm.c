#include <PalmOS.h>

extern FormGadgetTypeInCallback *FrmGetGadgetPtr(FormType *formP, UInt16 objIndex) SYS_TRAP(0xA502);

// special version of FrmDrawForm that draws only gadgets, to be called from m68k code that installs a gadget handler

void FrmDrawForm(FormType *formP) {
  FormGadgetTypeInCallback *gadget;
  FormObjectKind objType;
  UInt16 objIndex;

  if (formP) {
    for (objIndex = 0;; objIndex++) {
      gadget = FrmGetGadgetPtr(formP, objIndex);
      if (gadget == NULL) break;
      objType = FrmGetObjectType(formP, objIndex);
      if (objType == frmGadgetObj) {
        if (gadget->handler) {
          gadget->handler(gadget, formGadgetDrawCmd, NULL);
        }
      }
    }
  }
}
