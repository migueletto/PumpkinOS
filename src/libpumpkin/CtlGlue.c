#include <PalmOS.h>

#include "debug.h"

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

FontID CtlGlueGetFont(const ControlType *ctlP) {
  return ctlP ? ctlP->font : 0;
}

ControlStyleType CtlGlueGetControlStyle(const ControlType *ctlP) {
  return ctlP ? ctlP->style : 0;
}

Boolean CtlGlueIsGraphical(ControlType *ctlP) {
  return ctlP ? ctlP->attr.graphical : 0;
}

void CtlGlueGetGraphics(const ControlType *ctlP, DmResID *bitmapID, DmResID *selectedBitmapID) {
  if (ctlP && ctlP->attr.graphical) {
    if (bitmapID) *bitmapID = ctlP->bitmapID;
    if (selectedBitmapID) *selectedBitmapID = ctlP->selectedBitmapID;
  }
}

void CtlGlueSetFrameStyle(ControlType * ctlP, ButtonFrameType frameStyle) {
  if (ctlP) {
    ctlP->attr.frame = frameStyle;
  }
}

SliderControlType *CtlGlueNewSliderControl(void **formPP, UInt16 ID, ControlStyleType style, DmResID thumbID, DmResID backgroundID, Coord x, Coord y, Coord width, Coord height, UInt16 minValue, UInt16 maxValue, UInt16 pageSize, UInt16 value) {
  debug(DEBUG_ERROR, "Control", "CtlGlueNewSliderControl not implemented");
  return NULL;
}
