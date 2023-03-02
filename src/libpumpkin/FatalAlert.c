#include <PalmOS.h>

#include "debug.h"

void SysFatalAlertInit(void) {
}

void SysFatalAlertFinish(void) {
}

UInt16 SysFatalAlert(const Char *msg) {
  FormType *formP, *previous;
  FieldType *fldP;
  FieldAttrType attr;
  UInt16 index;

  debug(DEBUG_ERROR, "System", "SysFatalAlert: %s", msg);
  WinSetCoordinateSystem(kCoordinatesStandard);

  if ((formP = FrmInitForm(10400)) != NULL) {
    FrmSetTitle(formP, "Fatal Exception");
    index = FrmGetObjectIndex(formP, 10404);
    FrmHideObject(formP, index);
    index = FrmGetObjectIndex(formP, 10405);
    FrmHideObject(formP, index);

    index = FrmGetObjectIndex(formP, 10402);
    fldP = FrmGetObjectPtr(formP, index);

    if (fldP != NULL) {
      FldGetAttributes(fldP, &attr);
      attr.editable = false;
      FldSetAttributes(fldP, &attr);
      FldSetFont(fldP, boldFont);
      FldSetTextPtr(fldP, (char *)msg);
    } else {
      debug(DEBUG_ERROR, "System", "field not found on form");
    }
    previous = FrmGetActiveForm();

    FrmDoDialog(formP);
    FrmSetTitle(formP, NULL);
    if (fldP != NULL) {
      FldSetTextPtr(fldP, NULL);
    }

    FrmDeleteForm(formP);
    FrmSetActiveForm(previous);
  }

  return 0;
}
