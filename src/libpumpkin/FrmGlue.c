#include <PalmOS.h>

#include "debug.h"

Boolean FrmGlueGetObjectUsable(const FormType *formP, UInt16 objIndex) {
  FormObjectType obj;
  Boolean usable = false;

  if (formP && objIndex < formP->numObjects) {
    obj = formP->objects[objIndex].object;

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:     usable = obj.field->attr.usable;     break;
      case frmControlObj:   usable = obj.control->attr.usable;   break;
      case frmLabelObj:     usable = obj.label->attr.usable;     break;
      case frmListObj:      usable = obj.list->attr.usable;      break;
      case frmTableObj:     usable = obj.table->attr.usable;     break;
      case frmGadgetObj:    usable = obj.gadget->attr.usable;    break;
      case frmScrollBarObj: usable = obj.scrollBar->attr.usable; break;
      default: break;
    }
  }

  return usable;
}
