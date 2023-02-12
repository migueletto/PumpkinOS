#include <PalmOS.h>

void CtlGlueSetLeftAnchor(ControlType *ctlP, Boolean leftAnchor) {
  if (ctlP) {
    ctlP->attr.leftAnchor = leftAnchor;
  }
}

void CtlGlueSetFont(ControlType *ctlP, FontID fontID) {
  if (ctlP) {
    ctlP->font = fontID;
  }
}
