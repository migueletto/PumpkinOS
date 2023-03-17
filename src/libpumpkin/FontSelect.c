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
    case stdFont:       controlID = 11903; break;
    case boldFont:      controlID = 11904; break;
    case largeBoldFont: controlID = 11905; break;
    case mono6x10Font:  controlID = 11908; break;
    case mono8x14Font:  controlID = 11909; break;
    case mono16x16Font: controlID = 11910; break;
    case mono8x16Font:  controlID = 11911; break;
    default:            controlID = 11903; break;
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
    else if (checkFont(frm, 11908)) fontID = mono6x10Font;
    else if (checkFont(frm, 11909)) fontID = mono8x14Font;
    else if (checkFont(frm, 11910)) fontID = mono16x16Font;
    else if (checkFont(frm, 11911)) fontID = mono8x16Font;
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return fontID;
}
