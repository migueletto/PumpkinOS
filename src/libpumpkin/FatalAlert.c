#include <PalmOS.h>
#include <UIResourcesPrv.h>

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
  pumpkin_dirty_region_mode(dirtyRegionReset);
  WinSetCoordinateSystem(kCoordinatesStandard);

  // XXX this is using HelpForm, but it should have its own form

  if ((formP = FrmInitForm(HelpForm)) != NULL) {
    FrmSetTitle(formP, "Fatal Exception");
    index = FrmGetObjectIndex(formP, HelpUpButton);
    FrmHideObject(formP, index);
    index = FrmGetObjectIndex(formP, HelpDownButton);
    FrmHideObject(formP, index);

    index = FrmGetObjectIndex(formP, HelpField);
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
