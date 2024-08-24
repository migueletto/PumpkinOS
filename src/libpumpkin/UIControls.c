#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define NCOLS 18

#define PALMOS_MODULE "UI"

static void drawColorCell(RectangleType *rect, UInt16 i, Boolean border) {
  RGBColorType rgb, prev;
  IndexedColorType oldf;
  RectangleType cr;
  UInt16 cell, col, row;

  cell = rect->extent.x / NCOLS;
  col = i % NCOLS;
  row = i / NCOLS;
  cr.topLeft.x = rect->topLeft.x + 6 + col * cell;
  cr.topLeft.y = rect->topLeft.y + 3 + row * cell;
  cr.extent.x = cell - 1;
  cr.extent.y = cell - 1;

  if (border) {
    WinIndexToRGB(i, &rgb);
    rgb.r = 255 - rgb.r;
    rgb.g = 255 - rgb.g;
    rgb.b = 255 - rgb.b;
    WinSetForeColorRGB(&rgb, &prev);
    WinPaintRectangleFrame(simpleFrame, &cr);
    WinSetForeColorRGB(&prev, NULL);
  } else {
    oldf = WinSetForeColor(i);
    WinPaintRectangle(&cr, 0);
    WinSetForeColor(oldf);
  }
}

static Boolean getSliderValues(FormType *frm, UInt16 controlID, UInt16 *minValue, UInt16 *maxValue, UInt16 *value) {
  ControlType *ctl;
  UInt16 index;
  Boolean r = false;

  if ((index = FrmGetObjectIndex(frm, controlID)) != frmInvalidObjectId) {
    if ((ctl = (ControlType *)FrmGetObjectPtr(frm, index)) != NULL) {
      CtlGetSliderValues(ctl, minValue, maxValue, NULL, value);
      r = true;
    }
  }

  return r;
}

static UInt16 getSliderColor(FormType *frm, UInt16 controlID, char *buf, UInt16 max) {
  UInt16 minValue, maxValue, value, color = 0;

  if (getSliderValues(frm, controlID, &minValue, &maxValue, &value)) {
    color = maxValue > minValue ? (255 * (value - minValue)) / (maxValue - minValue) : 0;
    if (buf) StrNPrintF(buf, max, "%02d", value);
  }

  return color;
}

static void setSliderColor(FormType *frm, UInt16 controlID, UInt16 labelID, UInt16 color) {
  ControlType *ctl;
  char buf[8];
  UInt16 index, minValue, maxValue, value;

  if ((index = FrmGetObjectIndex(frm, controlID)) != frmInvalidObjectId) {
    if ((ctl = (ControlType *)FrmGetObjectPtr(frm, index)) != NULL) {
      CtlGetSliderValues(ctl, &minValue, &maxValue, NULL, NULL);
      value = minValue + (color * (maxValue - minValue)) / 255;
      CtlSetSliderValues(ctl, NULL, NULL, NULL, &value);
      StrNPrintF(buf, sizeof(buf)-1, "%02d", value);
      FrmCopyLabel(frm, labelID, buf);
    }
  }
}

static void drawGrid(RectangleType *rect, IndexedColorType sel) {
  IndexedColorType oldf;
  UInt16 i;

  oldf = WinSetForeColor(sel);
  WinPaintRectangle(rect, 0);
  WinSetForeColor(oldf);

  for (i = 0; i < 256; i++) {
    drawColorCell(rect, i, false);
  }
  drawColorCell(rect, sel, true);
}

static Boolean ColorGridGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  EventType *event;
  IndexedColorType *indexP;
  RGBColorType rgb;
  RectangleType rect;
  Int16 i, cell, col, row, index;

  if (cmd == formGadgetDeleteCmd) return true;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, 13305); // color grid gadget
  FrmGetObjectBounds(frm, index, &rect);

  indexP = FrmGetGadgetData(frm, index);
  debug(DEBUG_INFO, PALMOS_MODULE, "UIPickColor grid gadget %d,%d cmd %d", rect.extent.x, rect.extent.y, cmd);

  switch (cmd) {
    case formGadgetDrawCmd:
      drawGrid(&rect, *indexP);
      break;

    case formGadgetEraseCmd:
      WinEraseRectangle(&rect, 0);
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        if (RctPtInRectangle(event->screenX, event->screenY, &rect)) {
          cell = rect.extent.x / NCOLS;
          col = (event->screenX - rect.topLeft.x - 6) / cell;
          row = (event->screenY - rect.topLeft.y - 3) / cell;
          debug(DEBUG_TRACE, PALMOS_MODULE, "UIPickColor grid select (%d,%d) col=%d row=%d", event->screenX, event->screenY, col, row);
          if (col >= 0 && row >= 0) {
            i = row * NCOLS + col;
            if (i >= 0 && i < 256) {
              debug(DEBUG_TRACE, PALMOS_MODULE, "UIPickColor grid select i=%d", i);
              drawGrid(&rect, i);
              *indexP = i;
              WinIndexToRGB(i, &rgb);
              setSliderColor(frm, 13306, 13313, rgb.r);
              setSliderColor(frm, 13308, 13314, rgb.g);
              setSliderColor(frm, 13310, 13315, rgb.b);
            }
          }
        }
      }
      break;
  }

  return true;
}

static void drawColorPanel(RectangleType *rect, RGBColorType *rgb) {
  RGBColorType prev;

  WinSetForeColorRGB(rgb, &prev);
  WinPaintRectangle(rect, 0);
  WinSetForeColorRGB(&prev, NULL);
}

static Boolean ColorGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  RectangleType rect;
  RGBColorType rgb;
  UInt16 index;

  if (cmd == formGadgetDeleteCmd) return true;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, 13312); // color gadget
  FrmGetObjectBounds(frm, index, &rect);

  switch (cmd) {
    case formGadgetDrawCmd:
      rgb.index = 0;
      rgb.r = getSliderColor(frm, 13306, NULL, 0);
      rgb.g = getSliderColor(frm, 13308, NULL, 0);
      rgb.b = getSliderColor(frm, 13310, NULL, 0);
      drawColorPanel(&rect, &rgb);
      break;

    case formGadgetEraseCmd:
      WinEraseRectangle(&rect, 0);
      break;
  }

  return true;
}

static void showObject(FormType *frm, UInt16 index, Boolean show) {
  if (frm == FrmGetActiveForm()) {
    if (show) {
      FrmShowObject(frm, index);
    } else {
      FrmHideObject(frm, index);
    }
  } else {
    FrmSetUsable(frm, index, show);
  }
}

static void UIPickColorSetControls(FormType *frm, Boolean rgb, RGBColorType *rgbP, Boolean list, void *data) {
  ControlType *ctl;
  ListType *lst;
  UInt16 index, gridIndex, listIndex;

  gridIndex = FrmGetObjectIndex(frm, 13305); // color grid gadget
  if (!rgb) {
    FrmSetGadgetHandler(frm, gridIndex, ColorGridGadgetCallback);
  }
  if (data) {
    FrmSetGadgetData(frm, gridIndex, data);
  }

  if (rgb) {
    showObject(frm, gridIndex, false);
  }

  index = FrmGetObjectIndex(frm, 13303);     // popup trigger
  showObject(frm, index, list);
  listIndex = FrmGetObjectIndex(frm, 13304); // list
  showObject(frm, listIndex, false);

  if (list) {
    lst = (ListType *)FrmGetObjectPtr(frm, listIndex);
    LstSetSelection(lst, rgb ? 1 : 0);
    ctl = (ControlType *)FrmGetObjectPtr(frm, index);
    CtlSetLabel(ctl, LstGetSelectionText(lst, rgb ? 1 : 0));
  }

  index = FrmGetObjectIndex(frm, 13306); // red slider
  showObject(frm, index, rgb);
  index = FrmGetObjectIndex(frm, 13308); // green slider
  showObject(frm, index, rgb);
  index = FrmGetObjectIndex(frm, 13310); // blue slider
  showObject(frm, index, rgb);

  index = FrmGetObjectIndex(frm, 13307); // red label
  showObject(frm, index, rgb);
  index = FrmGetObjectIndex(frm, 13309); // green label
  showObject(frm, index, rgb);
  index = FrmGetObjectIndex(frm, 13311); // blue label
  showObject(frm, index, rgb);

  if (rgbP) {
    setSliderColor(frm, 13306, 13313, rgbP->r);
    setSliderColor(frm, 13308, 13314, rgbP->g);
    setSliderColor(frm, 13310, 13315, rgbP->b);
  }

  index = FrmGetObjectIndex(frm, 13312); // color gadget
  if (rgb) {
    FrmSetGadgetHandler(frm, index, ColorGadgetCallback);
  }
  showObject(frm, index, rgb);

  index = FrmGetObjectIndex(frm, 13313); // red value label
  showObject(frm, index, rgb);
  index = FrmGetObjectIndex(frm, 13314); // green value label
  showObject(frm, index, rgb);
  index = FrmGetObjectIndex(frm, 13315); // blue value label
  showObject(frm, index, rgb);

  if (!rgb) {
    showObject(frm, gridIndex, true);
  }
}

static void updateLabel(FormType *frm, UInt16 labelID, char *buf) {
  UInt16 index;

  FrmCopyLabel(frm, labelID, buf);
  index = FrmGetObjectIndex(frm, labelID);
  FrmEraseObject(frm, index);
  FrmDrawObject(frm, index, false);
}

static void ctlSelectHandlEvent(FormType *frm, UInt16 controlID) {
  RectangleType rect;
  RGBColorType rgb;
  IndexedColorType *indexP;
  UInt16 index;
  char buf[8];

  switch (controlID) {
    case 13306:
    case 13308:
    case 13310:
      rgb.r = getSliderColor(frm, 13306, buf, sizeof(buf)-1);
      updateLabel(frm, 13313, buf);

      rgb.g = getSliderColor(frm, 13308, buf, sizeof(buf)-1);
      updateLabel(frm, 13314, buf);

      rgb.b = getSliderColor(frm, 13310, buf, sizeof(buf)-1);
      updateLabel(frm, 13315, buf);

      index = FrmGetObjectIndex(frm, 13312); // color panel gadget
      FrmGetObjectBounds(frm, index, &rect);
      drawColorPanel(&rect, &rgb);
    
      index = FrmGetObjectIndex(frm, 13305); // color grid gadget
      indexP = (IndexedColorType *)FrmGetGadgetData(frm, index);
      if (indexP) *indexP = WinRGBToIndex(&rgb);
  }
}

static Boolean PickColorHandleEvent(EventType *eventP) {
  FormType *frm;
  Boolean handled = false;

  switch (eventP->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;
    case popSelectEvent:
      if (eventP->data.popSelect.listID == 13304) {
        frm = FrmGetActiveForm();
        switch (eventP->data.popSelect.selection) {
          case 0: // palette
            UIPickColorSetControls(frm, false, NULL, true, NULL);
            break;
          case 1: // RGB
            UIPickColorSetControls(frm, true, NULL, true, NULL);
            break;
        }
      }
      break;
    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      ctlSelectHandlEvent(frm, eventP->data.ctlSelect.controlID);
      break;
    case ctlRepeatEvent:
      frm = FrmGetActiveForm();
      ctlSelectHandlEvent(frm, eventP->data.ctlRepeat.controlID);
      break;
    default:
      break;
  }

  return handled;
}

Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP, UIPickColorStartType start, const Char *titleP, const Char *tipP) {
  FormType *frm, *previous;
  IndexedColorType dummy;
  Boolean r = false;

  debug(DEBUG_INFO, PALMOS_MODULE, "UIPickColor start %d title \"%s\"", start, titleP ? titleP : "");

  frm = FrmInitForm(13300);
  FrmSetTitle(frm, (char *)titleP);
  FrmSetEventHandler(frm, PickColorHandleEvent);

  if (indexP != NULL && rgbP == NULL) {
    // The palette version of the dialog displays a series of squares, each containing a different color defined on the system palette.
    // The indexP value contains the index of the square that is initially selected.
    debug(DEBUG_INFO, PALMOS_MODULE, "UIPickColor index %d used", *indexP);
    start = UIPickColorStartPalette;
    UIPickColorSetControls(frm, false, NULL, false, indexP);

  } else if (indexP == NULL && rgbP != NULL) {
    // The RGB version of the dialog displays three sliders that allow the
    // user to select the level of red, green, and blue in the color. The rgbP
    // parameter contains the red, green, and blue values initially shown
    // in the dialog. The sliders only allow values that are defined in the
    // current system color table.
    debug(DEBUG_INFO, PALMOS_MODULE, "UIPickColor rgb %d,%d,%d (%d) used", rgbP->r, rgbP->g, rgbP->b, rgbP->index);
    start = UIPickColorStartRGB;
    UIPickColorSetControls(frm, true, rgbP, false, &dummy);

  } else if (indexP != NULL && rgbP != NULL) {
    debug(DEBUG_INFO, PALMOS_MODULE, "UIPickColor index %d used", *indexP);
    debug(DEBUG_INFO, PALMOS_MODULE, "UIPickColor rgb %d,%d,%d (%d) used", rgbP->r, rgbP->g, rgbP->b, rgbP->index);

    switch (start) {
      case UIPickColorStartPalette:
        UIPickColorSetControls(frm, false, rgbP, true, indexP);
        break;
      case UIPickColorStartRGB:
        UIPickColorSetControls(frm, true, rgbP, true, indexP);
        break;
      default:
        debug(DEBUG_ERROR, PALMOS_MODULE, "UIPickColor invalid start type %d", start);
        start = UIPickColorStartPalette;
        UIPickColorSetControls(frm, false, rgbP, true, indexP);
        break;
    }

  } else {
    debug(DEBUG_ERROR, PALMOS_MODULE, "UIPickColor neither index / rgb used", *indexP);
    start = UIPickColorStartPalette;
    UIPickColorSetControls(frm, false, NULL, false, &dummy);
  }

  previous = FrmGetActiveForm();
  if (FrmDoDialog(frm) == 13301) { // "OK" button
    // Palm OS 3.5 supports a maximum of 256 colors. The number of
    // possible RGB colors greatly exceeds this amount. For this reason,
    // the chosen RGB may not have an exact match. If this is the case, the
    // indexP parameter (if not NULL) contains the closest match using a
    // luminance best-fit if the color lookup table is entirely grayscale (red,
    // green, and blue values for each entry are identical), or a shortestdistance
    // fit in the RGB space is used if the palette contains colors.

    if (rgbP) {
      rgbP->r = getSliderColor(frm, 13306, NULL, 0);
      rgbP->g = getSliderColor(frm, 13308, NULL, 0);
      rgbP->b = getSliderColor(frm, 13310, NULL, 0);
    }

    r = true;
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return r;
}

void UIContrastAdjust(void) {
  FormType *frm, *previous;

  debug(DEBUG_ERROR, PALMOS_MODULE, "UIContrastAdjust not implemented");
  frm = FrmInitForm(13100);

  previous = FrmGetActiveForm();
  if (FrmDoDialog(frm) == 13101) { // "OK" button
  }
  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);
}

void UIBrightnessAdjust(void) {
  FormType *frm, *previous;

  debug(DEBUG_ERROR, PALMOS_MODULE, "UIBrightnessAdjust not implemented");
  frm = FrmInitForm(13150);

  previous = FrmGetActiveForm();
  if (FrmDoDialog(frm) == 13111) { // "OK" button
  }
  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);
}
