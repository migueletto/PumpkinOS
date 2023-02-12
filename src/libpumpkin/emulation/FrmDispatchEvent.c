#include <PalmOS.h>

// m68k-palmos-gcc -O2 -Wall -Wno-switch -palmos5 -c FrmDispatchEvent.c
// m68k-palmos-objdump -s FrmDispatchEvent.o

extern FormEventHandlerType *FrmGetFormEventHandler(FormType *formP) SYS_TRAP(0xA500);

Boolean FrmDispatchEvent(EventType *eventP) {
  FormType *formP = FrmGetActiveForm();
  FormGadgetTypeInCallback *gadget;
  FormEventHandlerType *handlerP;
  Boolean handled = false;

  if (formP) {
    handlerP = FrmGetFormEventHandler(formP);
    if (handlerP && handlerP(eventP)) {
      return true;
    }

    switch (eventP->eType) {
      case frmGadgetEnterEvent: gadget = (FormGadgetTypeInCallback *)eventP->data.gadgetEnter.gadgetP; break;
      case frmGadgetMiscEvent:  gadget = (FormGadgetTypeInCallback *)eventP->data.gadgetMisc.gadgetP;  break;
      default: gadget = NULL; break;
    }

    if (gadget && gadget->handler) {
      handled = gadget->handler((FormGadgetTypeInCallback *)gadget, formGadgetHandleEventCmd, eventP);
    }

    if (!handled) {
      handled = FrmHandleEvent(formP, eventP);
    }
  }

  return handled;
}
