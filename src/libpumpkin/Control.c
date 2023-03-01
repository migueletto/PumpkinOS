#include <PalmOS.h>

#include "sys.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

static void CtlInvertControl(ControlType *controlP) {
  RectangleType rect;
  UInt16 corner;

  if (controlP && controlP->attr.visible) {
    MemMove(&rect, &controlP->bounds, sizeof(RectangleType));
    corner = 0;

    switch (controlP->style) {
      case pushButtonCtl:
      case selectorTriggerCtl:
        break;
      case buttonCtl:
      case repeatingButtonCtl:
        switch (controlP->attr.frame) {
          case standardButtonFrame:
          case boldButtonFrame:
            corner = 3;
        }
        break;
      default:
        return;
    }

    debug(DEBUG_TRACE, "Control", "CtlInvertControl style %d control %d corner %d", controlP->style, controlP->id, corner);
    WinInvertRect(&rect, corner);
  }
}

void CtlDrawControl(ControlType *controlP) {
  MemHandle h;
  SliderControlType *sc;
  BitmapType *bmp;
  WinDrawOperation mode;
  PatternType oldPattern;
  IndexedColorType objFill, objFore, oldb, oldf, oldt;
  Coord bw, bh;
  UInt16 rb;
  FontID old;
  Int16 tw, th, x, y;

  if (controlP) {
    debug(DEBUG_TRACE, "Control", "CtlDrawControl control %d style %d on %d", controlP->id, controlP->style, controlP->attr.on);
    objFill = UIColorGetTableEntryIndex(UIObjectFill);
    objFore = UIColorGetTableEntryIndex(UIObjectForeground);
    oldb = WinSetBackColor(objFill);
    oldf = WinSetForeColor(objFore);
    oldt = WinSetTextColor(objFore);
    oldPattern = WinGetPatternType();                                                                                                                                                       
    WinSetPatternType(blackPattern);                                                                                                                                                        
    controlP->attr.visible = true;

    switch (controlP->style) {
      case buttonCtl:
      case pushButtonCtl:
      case repeatingButtonCtl:
      case selectorTriggerCtl:
        if (controlP->attr.graphical) {
          debug(DEBUG_TRACE, "Control", "GraphicControl id=%d, bitmapID=%d, selectedBitmapID=%d, selected=%d", controlP->id, controlP->bitmapID, controlP->selectedBitmapID, controlP->attr.on);
          if ((h = DmGetResource(bitmapRsc, (controlP->attr.on && controlP->selectedBitmapID) ? controlP->selectedBitmapID : controlP->bitmapID)) != NULL) {
            if ((bmp = MemHandleLock(h)) != NULL) {
              debug(DEBUG_TRACE, "Control", "GraphicControl draw id=%d, bitmapID=%d, selectedBitmapID=%d", controlP->id, controlP->bitmapID, controlP->selectedBitmapID);
              BmpGetDimensions(bmp, &bw, &bh, &rb);
              x = controlP->bounds.topLeft.x + (controlP->bounds.extent.x - bw) / 2;
              y = controlP->bounds.topLeft.y + (controlP->bounds.extent.y - bh) / 2;
              WinDrawBitmap(bmp, x, y);
              MemHandleUnlock(h);
            }
            DmReleaseResource(h);
          }
        } else {
          if (controlP->text && controlP->text[0]) {
            old = FntSetFont(controlP->font);
            tw = FntCharsWidth(controlP->text, StrLen(controlP->text));
            th = FntCharHeight();
            x = controlP->bounds.topLeft.x + (controlP->bounds.extent.x - tw) / 2;
            y = controlP->bounds.topLeft.y + (controlP->bounds.extent.y - th) / 2;
            WinDrawChars(controlP->text, StrLen(controlP->text), x, y);
            FntSetFont(old);
          }
        }

        switch (controlP->style) {
          case buttonCtl:
          case repeatingButtonCtl:
            switch (controlP->attr.frame) {
              case standardButtonFrame:
              case boldButtonFrame:
                WinDrawRectangleFrame(roundFrame, &controlP->bounds);
                break;
              case rectangleButtonFrame:
                WinDrawRectangleFrame(simpleFrame, &controlP->bounds);
                break;
              default:
                break;
            }
            break;
          case pushButtonCtl:
            if (controlP->attr.frame != noButtonFrame) {
              WinDrawRectangleFrame(simpleFrame, &controlP->bounds);
            }
            break;
          case selectorTriggerCtl:
            WinDrawGrayRectangleFrame(simpleFrame, &controlP->bounds);
            break;
          default:
            break;
        }

        if (controlP->attr.on) {
          CtlInvertControl(controlP);
        }
        break;

      case checkboxCtl:
        old = FntSetFont(checkboxFont);
        WinDrawChar(controlP->attr.on ? 1 : 0, controlP->bounds.topLeft.x, controlP->bounds.topLeft.y);
        if (controlP->text) {
          tw = FntCharWidth(1);
          FntSetFont(controlP->font);
          WinDrawChars(controlP->text, StrLen(controlP->text), controlP->bounds.topLeft.x + tw + 2, controlP->bounds.topLeft.y);
        }
        FntSetFont(old);
        break;

      case popupTriggerCtl:
        if (controlP->text) {
          WinSetTextColor(objFore);
          WinEraseRectangle(&controlP->bounds, 0); // XXX it is not erasing all of it
          old = FntSetFont(controlP->font);
          x = controlP->bounds.topLeft.x + 2*FntCharWidth('w');
          WinDrawChars(controlP->text, StrLen(controlP->text), x, controlP->bounds.topLeft.y);
          FntSetFont(symbol7Font);
          WinDrawChar(2, controlP->bounds.topLeft.x, controlP->bounds.topLeft.y+2);
          FntSetFont(old);
        }
        break;

      case sliderCtl:
      case feedbackSliderCtl:
        sc = (SliderControlType *)controlP;
        debug(DEBUG_TRACE, "Control", "CtlDrawControl slider %d (%d,%d)", sc->id, sc->backgroundID, sc->thumbID);
        mode = WinSetDrawMode(winPaint);
        if ((h = DmGetResource(bitmapRsc, sc->backgroundID)) != NULL) {
          if ((bmp = MemHandleLock(h)) != NULL) {
            WinPaintBitmap(bmp, controlP->bounds.topLeft.x, controlP->bounds.topLeft.y);
            MemHandleUnlock(h);
          }
          DmReleaseResource(h);
        }
        if ((h = DmGetResource(bitmapRsc, sc->thumbID)) != NULL) {
          if ((bmp = MemHandleLock(h)) != NULL) {
            BmpGetDimensions(bmp, &bw, &bh, &rb);
            x = ((sc->value - sc->minValue) * (controlP->bounds.extent.x - bw)) / (sc->maxValue - sc->minValue);
            WinPaintBitmap(bmp, controlP->bounds.topLeft.x + x, controlP->bounds.topLeft.y);
            MemHandleUnlock(h);
          }
          DmReleaseResource(h);
        }
        WinSetDrawMode(mode);
        break;
    }

    WinSetPatternType(oldPattern);                                                                                                                                                        
    WinSetBackColor(oldb);
    WinSetForeColor(oldf);
    WinSetTextColor(oldt);
  }
}

void CtlEraseControl(ControlType *controlP) {
  IndexedColorType objFill, oldb, oldf;

  if (controlP) {
    if (controlP->attr.visible) {
      debug(DEBUG_TRACE, "Control", "CtlEraseControl control %d style %d", controlP->id, controlP->style);
      objFill = UIColorGetTableEntryIndex(UIObjectFill);
      oldb = WinSetBackColor(objFill);
      oldf = WinSetForeColor(objFill);

      switch (controlP->style) {
        case buttonCtl:
        case repeatingButtonCtl:
          switch (controlP->attr.frame) {
            case standardButtonFrame:
            case boldButtonFrame:
              WinDrawRectangleFrame(roundFrame, &controlP->bounds);
              break;
            case rectangleButtonFrame:
              WinDrawRectangleFrame(simpleFrame, &controlP->bounds);
              break;
            default:
              break;
          }
          break;
        case pushButtonCtl:
          WinDrawRectangleFrame(simpleFrame, &controlP->bounds);
          break;
        case selectorTriggerCtl:
          WinDrawGrayRectangleFrame(simpleFrame, &controlP->bounds);
          break;
        default:
          break;
      }

      WinEraseRectangle(&controlP->bounds, 0);
      WinSetBackColor(oldf);
      WinSetBackColor(oldb);
      controlP->attr.visible = 0;
    }
  }
}

// Set a control’s usable attribute to false and erase the control from the screen.

void CtlHideControl(ControlType *controlP) {
  if (controlP) {
    debug(DEBUG_TRACE, "Control", "CtlHideControl control %d style %d", controlP->id, controlP->style);
    controlP->attr.usable = false;
    CtlEraseControl(controlP);
  }
}

// Set a control’s usable attribute to true and draw the control on the screen.
// This function calls CtlDrawControl.

void CtlShowControl(ControlType *controlP) {
  if (controlP) {
    debug(DEBUG_TRACE, "Control", "CtlShowControl control %d style %d", controlP->id, controlP->style);
    controlP->attr.usable = true;
    CtlDrawControl(controlP);
  }
}

Boolean CtlEnabled(const ControlType *controlP) {
  return controlP ? controlP->attr.enabled : false;
}

void CtlSetEnabled(ControlType *controlP, Boolean usable) {
  if (controlP) {
    controlP->attr.enabled = usable;
  }
}

void CtlSetUsable(ControlType *controlP, Boolean usable) {
  if (controlP) {
    controlP->attr.usable = usable;
  }
}

Int16 CtlGetValue(const ControlType *controlP) {
  SliderControlType *slider;
  Int16 value = 0;

  if (controlP) {
    switch (controlP->style) {
      case sliderCtl:
      case feedbackSliderCtl:
        slider = (SliderControlType *)controlP;
        value = slider->value;
        break;
      default:
        value = controlP->attr.on ? 1 : 0;
        break;
    }
  }

  return value;
}

void CtlSetValue(ControlType *controlP, Int16 newValue) {
  SliderControlType *slider;

  if (controlP) {
    /*if (controlP->attr.graphical) {
      debug(DEBUG_TRACE, "Control", "CtlSetValue graphic control %d value %d visible %d", controlP->id, newValue, controlP->attr.visible);
      CtlInvertControl(controlP);
      controlP->attr.on = newValue ? true : false;
    } else {*/
      switch (controlP->style) {
        case sliderCtl:
        case feedbackSliderCtl:
          debug(DEBUG_TRACE, "Control", "CtlSetValue slider %d value %d visible %d", controlP->id, newValue, controlP->attr.visible);
          slider = (SliderControlType *)controlP;
          slider->value = newValue;
          if (controlP->attr.visible) {
            CtlDrawControl(controlP);
          }
          break;
        case pushButtonCtl:
          debug(DEBUG_TRACE, "Control", "CtlSetValue pushButton %d value %d visible %d", controlP->id, newValue, controlP->attr.visible);
          CtlUpdateGroup(controlP, newValue != 0);
          break;
        case checkboxCtl:
          debug(DEBUG_TRACE, "Control", "CtlSetValue checkBox %d value %d visible %d", controlP->id, newValue, controlP->attr.visible);
          controlP->attr.on = newValue ? true : false;
          if (controlP->attr.visible) {
            CtlDrawControl(controlP);
          }
          break;
        default:
          debug(DEBUG_TRACE, "Control", "CtlSetValue type %d control %d value %d visible %d", controlP->style, controlP->id, newValue, controlP->attr.visible);
          break;
      }
    //}
  }
}

void CtlSetSliderValues(ControlType *ctlP, const UInt16 *minValueP, const UInt16 *maxValueP, const UInt16 *pageSizeP, const UInt16 *valueP) {
  SliderControlType *sc;

  if (ctlP && (ctlP->style == sliderCtl || ctlP->style == feedbackSliderCtl)) {
    sc = (SliderControlType *)ctlP;
    if (minValueP) sc->minValue = *minValueP;
    if (maxValueP) sc->maxValue = *maxValueP;
    if (pageSizeP) sc->pageSize = *pageSizeP;
    if (valueP) sc->value = *valueP;
  }
}

void CtlGetSliderValues(const ControlType *ctlP, UInt16 *minValueP, UInt16 *maxValueP, UInt16 *pageSizeP, UInt16 *valueP) SYS_TRAP(sysTrapCtlGetSliderValues) {
  SliderControlType *sc;

  if (ctlP && (ctlP->style == sliderCtl || ctlP->style == feedbackSliderCtl)) {
    sc = (SliderControlType *)ctlP;
    if (minValueP) *minValueP = sc->minValue;
    if (maxValueP) *maxValueP = sc->maxValue;
    if (pageSizeP) *pageSizeP = sc->pageSize;
    if (valueP) *valueP = sc->value;
  }
}

const Char *CtlGetLabel(const ControlType *controlP) {
  char *label = NULL;

  if (controlP) {
    label = controlP->text;
  }

  return label;
}

/*
This function stores the newLabel pointer in the control’s data
structure. It doesn’t make a copy of the string that is passed in.
Therefore, if you use CtlSetLabel, you must manage the string
yourself. You must ensure that it persists for as long as it is being
displayed (that is, for as long as the control is displayed or until you
call CtlSetLabel with a new string), and you must free the string
after it is no longer in use (typically after the form containing the
control is freed).
*/
void CtlSetLabel(ControlType *controlP, const Char *newLabel) {
  Boolean visible;
  Int16 x;

  if (controlP && newLabel) {
    visible = controlP->attr.visible;
    if (visible) {
      CtlEraseControl(controlP);
    }
    controlP->text = (char *)newLabel;
    if (controlP->style == popupTriggerCtl || controlP->style == selectorTriggerCtl) {
      x = controlP->bounds.topLeft.x + controlP->bounds.extent.x;
      controlP->bounds.extent.x = controlP->text ? FntCharsWidth(controlP->text, StrLen(controlP->text)) : 0;
      controlP->bounds.extent.x += 2*FntCharWidth('w');
      if (!controlP->attr.leftAnchor) {
        controlP->bounds.topLeft.x = x - controlP->bounds.extent.x;
      }
    }
    if (visible) {
      CtlDrawControl(controlP);
    }
  }
}

void CtlHitControl(const ControlType *controlP) {
  EventType event;

  debug(DEBUG_TRACE, "Control", "CtlHitControl control %d style %d", controlP->id, controlP->style);
  event.eType = ctlSelectEvent;
  event.data.ctlSelect.controlID = controlP->id;
  event.data.ctlSelect.pControl = (ControlType *)controlP;
  event.data.ctlSelect.on = true;
  EvtAddEventToQueue(&event);
}

void CtlUpdateGroup(ControlType *controlP, Boolean value) {
  FormType *formP;
  ControlType *control2P;
  UInt16 objIndex;

  if (controlP->attr.on != value) {
    controlP->attr.on = true;
    debug(DEBUG_TRACE, "Control", "CtlUpdateGroup control %d ON group %d visible %d", controlP->id, controlP->group, controlP->attr.visible);
    CtlInvertControl(controlP);
    formP = (FormType *)controlP->formP;

    if (formP && controlP->group) {
      for (objIndex = 0; objIndex < formP->numObjects; objIndex++) {
        if (formP->objects[objIndex].objectType == frmControlObj) {
          control2P = formP->objects[objIndex].object.control;
          if (control2P->id != controlP->id && control2P->attr.on && control2P->group == controlP->group) {
            debug(DEBUG_TRACE, "Control", "CtlUpdateGroup control %d OFF group %d visible %d", controlP->id, controlP->group, controlP->attr.visible);
            CtlInvertControl(control2P);
            control2P->attr.on = !controlP->attr.on;
          }
        }
      }
    }
  }
}

Boolean CtlHandleEvent(ControlType *controlP, EventType *pEvent) {
  SliderControlType *sc;
  EventType event;
  UInt16 x, index;
  Int16 selection;
  FormType *frm;
  ListType *list;
  Boolean handled = false;

  switch (pEvent->eType) {
    case penDownEvent:
      if (controlP->attr.usable && controlP->attr.enabled) {
        if (RctPtInRectangle(pEvent->screenX, pEvent->screenY, &controlP->bounds)) {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent penDown inside control %d", controlP->id);
          MemSet(&event, sizeof(EventType), 0);
          event.screenX = pEvent->screenX;
          event.screenY = pEvent->screenY;
          event.eType = ctlEnterEvent;
          event.data.ctlEnter.controlID = controlP->id;
          event.data.ctlEnter.pControl = controlP;
          EvtAddEventToQueue(&event);
          handled = true;
        }
      }
      break;

    case penMoveEvent:
      if (controlP->attr.usable && controlP->attr.enabled) {
        x = pEvent->screenX;
        if (x < controlP->bounds.topLeft.x) {
          x = controlP->bounds.topLeft.x;
        } else if (x >= controlP->bounds.topLeft.x + controlP->bounds.extent.x) {
          x = controlP->bounds.topLeft.x + controlP->bounds.extent.x - 1;
        }
        if (RctPtInRectangle(x, pEvent->screenY, &controlP->bounds)) {
          if (controlP->style == feedbackSliderCtl) {
            sc = (SliderControlType *)controlP;
            MemSet(&event, sizeof(EventType), 0);
            event.eType = ctlRepeatEvent;
            event.screenX = pEvent->screenX;
            event.screenY = pEvent->screenY;
            event.data.ctlRepeat.controlID = controlP->id;
            event.data.ctlRepeat.pControl = controlP;
            event.data.ctlRepeat.time = TimGetTicks();
            event.data.ctlRepeat.value = sc->minValue + ((x - sc->bounds.topLeft.x + 1) * sc->maxValue) / sc->bounds.extent.x;
            EvtAddEventToQueue(&event);
            handled = true;
          }
        }
      }
      break;

    case penUpEvent:
      if (controlP->attr.usable && controlP->attr.enabled) {
        MemSet(&event, sizeof(EventType), 0);
        event.screenX = pEvent->screenX;
        event.screenY = pEvent->screenY;
        if (RctPtInRectangle(pEvent->screenX, pEvent->screenY, &controlP->bounds)) {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent penUp inside control %d", controlP->id);
          if (controlP->style == repeatingButtonCtl) {
            event.eType = ctlSelectEvent;
            event.data.ctlRepeat.controlID = controlP->id;
            event.data.ctlRepeat.pControl = controlP;
            event.data.ctlRepeat.time = TimGetTicks();
          } else {
            event.eType = ctlSelectEvent;
            event.data.ctlSelect.controlID = controlP->id;
            event.data.ctlSelect.pControl = controlP;
            event.data.ctlSelect.on = true;
          }
          if (controlP->style == sliderCtl || controlP->style == feedbackSliderCtl) {
            sc = (SliderControlType *)controlP;
            event.data.ctlSelect.value = sc->minValue + ((pEvent->screenX - sc->bounds.topLeft.x + 1) * sc->maxValue) / sc->bounds.extent.x;
          }
        } else {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent penUp outside control %d", controlP->id);
          event.eType = ctlExitEvent;
          event.data.ctlExit.controlID = controlP->id;
          event.data.ctlExit.pControl = controlP;
        }
        EvtAddEventToQueue(&event);

        // Unhighlight the control here, because if the form handler handle ctlSelectEvent,
        // we will never see ctlSelectEvent and the control will remain highlighted.
        // There must be a better way to do this, though.
        if (controlP->style != pushButtonCtl && controlP->style != checkboxCtl) {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent inverting control %d to 0", controlP->id);
          if (controlP->attr.on) {
            CtlInvertControl(controlP);
            controlP->attr.on = false;
          }
        }
        handled = true;
      }
      break;

    case ctlEnterEvent:
      debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlEnter control %d", controlP->id);
      if (controlP->style == pushButtonCtl) {
        debug(DEBUG_TRACE, "Control", "CtlHandleEvent updating pushButton %d to %d", controlP->id, !controlP->attr.on);
        if (!controlP->group || !controlP->attr.on) {
          CtlUpdateGroup(controlP, !controlP->attr.on);
        }
      } else if (controlP->style == checkboxCtl) {
        debug(DEBUG_TRACE, "Control", "CtlHandleEvent updating checkBox %d to %d", controlP->id, !controlP->attr.on);
        controlP->attr.on = !controlP->attr.on;
        if (controlP->attr.visible) {
          CtlDrawControl(controlP);
        }
      } else {
        if (!controlP->attr.on) {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent inverting control %d to 1", controlP->id);
          CtlInvertControl(controlP);
          controlP->attr.on = true;
        }
      }
      handled = true;
      break;

    case ctlSelectEvent:
      debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlSelect control %d style %d listID %d", controlP->id, controlP->style, controlP->listID);
      if (controlP->style == popupTriggerCtl && controlP->listID) {
        if ((frm = (FormType *)controlP->formP) != NULL) {
          index = FrmGetObjectIndex(frm, controlP->listID);
          if ((list = FrmGetObjectPtr(frm, index)) != NULL) {
            debug(DEBUG_TRACE, "Control", "CtlHandleEvent popup show list %d", controlP->listID);
            if ((selection = LstPopupList(list)) != -1) {
              debug(DEBUG_TRACE, "Control", "CtlHandleEvent popup set control %d label", controlP->id);
              CtlSetLabel(controlP, LstGetSelectionText(list, selection));
            } else {
              debug(DEBUG_TRACE, "Control", "CtlHandleEvent popup nothing selected");
            }
          } else {
            debug(DEBUG_ERROR, "Control", "CtlHandleEvent popup list %d not found", controlP->listID);
          }
        }

      } else if (controlP->style != pushButtonCtl && controlP->style != checkboxCtl) {
        controlP->attr.on = false;
        if (controlP->attr.visible) {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlSelect draw control %d on 0", controlP->id);
          CtlSetSliderValues(controlP, NULL, NULL, NULL, &pEvent->data.ctlSelect.value);
          CtlDrawControl(controlP);
        }
      }
      break;

    case ctlRepeatEvent:
      if (controlP->attr.visible) {
        if (controlP->style == feedbackSliderCtl) {
          debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlRepeat draw control %d on %d", controlP->id, controlP->attr.on);
          CtlSetSliderValues(controlP, NULL, NULL, NULL, &pEvent->data.ctlRepeat.value);
          CtlDrawControl(controlP);
        }
      }
      break;

    case ctlExitEvent:
      debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlExit control %d", controlP->id);
      // XXX se e pushButton de grupo multiplo, precisa retornar para o membro que estava selecionado anteriormente
      if (controlP->style != pushButtonCtl && controlP->style != checkboxCtl) {
        if (controlP->attr.on) {
          if (controlP->style == buttonCtl && controlP->attr.visible) {
            debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlExit invert control %d to 0", controlP->id);
            CtlInvertControl(controlP);
          }
          controlP->attr.on = false;
        } else {
          if (controlP->attr.visible) {
            debug(DEBUG_TRACE, "Control", "CtlHandleEvent ctlExit draw control %d on 0", controlP->id);
            CtlDrawControl(controlP);
          }
        }
        handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

// Returns true if the specified pointer references a valid control object.
// For debugging purposes; do not include this function in commercial products.
// In debug builds, this function displays a dialog and waits for the debugger when an error occurs.

Boolean CtlValidatePointer(const ControlType *controlP) {
  debug(DEBUG_ERROR, "Control", "CtlValidatePointer not implemented");
  return false;
}

void CtlSetGraphics(ControlType *ctlP, DmResID newBitmapID, DmResID newSelectedBitmapID) {
  if (ctlP && ctlP->attr.graphical) {
    ctlP->bitmapID = newBitmapID;
    ctlP->selectedBitmapID = newSelectedBitmapID;
    debug(DEBUG_TRACE, "Control", "CtlSetGraphics id=%d, bitmapID=%d, selectedBitmapID=%d", ctlP->id, ctlP->bitmapID, ctlP->selectedBitmapID);
    if (ctlP->attr.visible) {
      CtlDrawControl(ctlP);
    }
  }
}

ControlType *CtlNewControl(void **formPP, UInt16 ID, ControlStyleType style, const Char *textP, Coord x, Coord y, Coord width, Coord height, FontID font, UInt8 group, Boolean leftAnchor) {
  ControlType *controlP = NULL;
  FormType **fpp, *formP;

  if (formPP && textP) {
    fpp = (FormType **)formPP;
    formP = *fpp;

    if (formP) {
      if ((controlP = pumpkin_heap_alloc(sizeof(ControlType), "Control")) != NULL) {
        controlP->id = ID;
        controlP->bounds.topLeft.x = x;
        controlP->bounds.topLeft.y = y;
        controlP->bounds.extent.x = width;
        controlP->bounds.extent.y = height;
        controlP->attr.usable = true;
        controlP->attr.enabled = true;
        controlP->attr.leftAnchor = leftAnchor;
        controlP->attr.frame = standardButtonFrame;
        controlP->style = style;
        controlP->font = font;
        controlP->group = group;
        controlP->text = (char *)textP;
        controlP->formP = formP;
        controlP->objIndex = formP->numObjects;

        if (formP->numObjects == 0) {
          formP->objects = xcalloc(1, sizeof(FormObjListType));
        } else {
          formP->objects = xrealloc(formP->objects, (formP->numObjects + 1) * sizeof(FormObjListType));
        }
        formP->objects[formP->numObjects].objectType = frmControlObj;
        formP->objects[formP->numObjects].object.control = controlP;
        formP->objects[formP->numObjects].id = formP->objects[formP->numObjects].object.control->id;
        formP->numObjects++;
      }
    }
  }

  return controlP;
}
