#include <PalmOS.h>

static Boolean checkFont(FormType *frm, UInt16 controlID) {
  ControlType *ctl;
  UInt16 index;
  Boolean selected = false;

  index = FrmGetObjectIndex(frm, controlID);
  if ((ctl = FrmGetObjectPtr(frm, index)) != NULL) {
    selected = CtlGetValue(ctl) == 1;
  }

  return selected;
}

FontID FontSelect(FontID fontID) {
  FormType *frm, *previous;
  ControlType *ctl;
  UInt16 controlID, index;

  frm = FrmInitForm(11900);

  switch (fontID) {
    case stdFont:       // Standard plain text font.
      controlID = 11903;
      break;
    case boldFont:      // Bold version of stdFont.
      controlID = 11904;
      break;
    case largeBoldFont: // Larger version of boldFont.
      controlID = 11905;
      break;
    default:
      controlID = 11903;
      break;
  }

  index = FrmGetObjectIndex(frm, controlID);
  if (index != frmInvalidObjectId) {
     if ((ctl = FrmGetObjectPtr(frm, index)) != NULL) {
       CtlSetValue(ctl, 1);
     }
  }

  previous = FrmGetActiveForm();
  if (FrmDoDialog(frm) == 11906) { // "OK" button
    if (checkFont(frm, 11903)) fontID = stdFont;
    else if (checkFont(frm, 11904)) fontID = boldFont;
    else if (checkFont(frm, 11905)) fontID = largeBoldFont;
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return fontID;
}
