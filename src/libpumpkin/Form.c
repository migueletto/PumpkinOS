#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "emupalmosinc.h"
#include "pumpkin.h"
#include "AppRegistry.h"
#include "storage.h"
#include "debug.h"
#include "xalloc.h"

#define MAX_STACK  8

typedef struct FormList {
  FormType *formP;
  struct FormList *next;
} FormList;

typedef struct {
  FormType *currentForm;
  FormList *list;
  Boolean centerDialogs;
} frm_module_t;

static const char *frmObjType[] = {
  "frmFieldObj",
  "frmControlObj",
  "frmListObj",
  "frmTableObj",
  "frmBitmapObj",
  "frmLineObj",
  "frmFrameObj",
  "frmRectangleObj",
  "frmLabelObj",
  "frmTitleObj",
  "frmPopupObj",
  "frmGraffitiStateObj",
  "frmGadgetObj",
  "frmScrollBarObj"
};

int FrmInitModule(void) {
  frm_module_t *module;

  if ((module = xcalloc(1, sizeof(frm_module_t))) == NULL) {
    return -1;
  }

  pumpkin_set_local_storage(frm_key, module);

  return 0;
}

void *FrmReinitModule(void *module) {
  frm_module_t *old = NULL;

  if (module) {
    FrmFinishModule();
    pumpkin_set_local_storage(frm_key, module);
  } else {
    old = (frm_module_t *)pumpkin_get_local_storage(frm_key);
    FrmInitModule();
  }

  return old;
}

int FrmFinishModule(void) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

static const char *FrmObjectTypeName(UInt16 type) {
  return (type >= frmFieldObj && type <= frmScrollBarObj) ? frmObjType[type] : "unknown";
}

static void FrmCenterForm(FormType *formP) {
  UInt32 swidth, sheight;
  UInt16 index;
  FormObjectType obj;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  formP->window.windowBounds.topLeft.x = (swidth  - formP->window.windowBounds.extent.x) / 2;
  formP->window.windowBounds.topLeft.y = (sheight - formP->window.windowBounds.extent.y) / 2;

  for (index = 0; index < formP->numObjects; index++) {
    if (formP->objects[index].objectType == frmListObj) {
      obj = formP->objects[index].object;
      obj.list->popupWin->windowBounds.topLeft.x = formP->window.windowBounds.topLeft.x + obj.list->bounds.topLeft.x; // absolute screen coordinate
      obj.list->popupWin->windowBounds.topLeft.y = formP->window.windowBounds.topLeft.y + obj.list->bounds.topLeft.y; // absolute screen coordinate
    }
  }
}

Boolean FrmGetCenterDialogs(void) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  Boolean center = false;

  if (module) {
    center = module->centerDialogs;
  }

  return center;
}

void FrmCenterDialogs(Boolean center) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);

  if (module) {
    module->centerDialogs = center;
  }
}

void FrmGotoForm(UInt16 formId) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  EventType event;

  debug(DEBUG_TRACE, "Form", "FrmGotoForm %d", formId);
  InsPtEnable(false);

  if (module->currentForm) {
    xmemset(&event, 0, sizeof(EventType));
    event.eType = frmCloseEvent;
    event.data.frmClose.formID = module->currentForm->formId;
    EvtAddEventToQueue(&event);
  }

  EvtFlushPenQueue();

  xmemset(&event, 0, sizeof(EventType));
  event.eType = frmLoadEvent;
  event.data.frmLoad.formID = formId;
  EvtAddEventToQueue(&event);

  xmemset(&event, 0, sizeof(EventType));
  event.eType = frmOpenEvent;
  event.data.frmOpen.formID = formId;
  EvtAddEventToQueue(&event);
}

static Err FrmInitFormInternal(FormType *formP) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  RectangleType rect;
  FormList *p;
  Coord width, height, xmargin, ymargin, w, h;
  UInt32 density, depth;
  Err err = errNone;

  if (formP->window.windowFlags.modal) {
    if (formP->window.windowBounds.topLeft.x >= 2) {
      xmargin = 2;
    } else {
      xmargin = formP->window.windowBounds.topLeft.x;
    }
    if (formP->window.windowBounds.topLeft.y >= 2) {
      ymargin = 2;
    } else {
      ymargin = formP->window.windowBounds.topLeft.y;
    }
  } else {
    xmargin = ymargin = 0;
  }

  w = width = formP->window.windowBounds.extent.x;
  h = height = formP->window.windowBounds.extent.y;
  formP->bitsBehindForm = WinCreateOffscreenWindow(width + 2*xmargin, height + 2*ymargin, nativeFormat, &err);
  WinAdjustCoords(&w, &h);

  WinScreenGetAttribute(winScreenDensity, &density);
  WinScreenMode(winScreenModeGet, NULL, NULL, &depth, NULL);
  formP->window.bitmapP = BmpCreate3(w, h, 0, density, depth, false, 0, NULL, &err);
  formP->window.density = density;

  RctSetRectangle(&rect, 0, 0, width, height);
  WinSetClipingBounds(&formP->window, &rect);

  WinDirectAccessHack(&formP->window, 0, 0, width, height);

  formP->selectedObject = -1;
  formP->activeList = -1;
  formP->activeField = -1;
  formP->mbar = formP->menuRscId ? MenuInit(formP->menuRscId) : NULL;
  formP->diaPolicy = frmDIAPolicyStayOpen;

  p = xcalloc(1, sizeof(FormList));
  p->formP = formP;
  p->next = NULL;

  if (module->list == NULL) {
    module->list = p;
  } else {
    p->next = module->list;
    module->list = p;
  }

  return err;
}

FormType *FrmInitForm(UInt16 rscID) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  FormType *formP = NULL;
  MemHandle h;
  UInt16 size;
  void *p, *rsrc = NULL;

  // Forms live in the heap but are not chunks.
  // You can not use MemPtrRecoverHandle with a form pointer.

  if ((h = DmGetResource(formRscType, rscID)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      size = MemHandleSize(h);
      rsrc = pumpkin_heap_alloc(size, "form_rsrc");
      MemMove(rsrc, p, size);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  if (rsrc) {
    formP = pumpkin_create_form(rsrc, size);
    debug(DEBUG_TRACE, "Form", "FrmInitForm %d (modal %d)", rscID, formP->window.windowFlags.modal);
    FrmInitFormInternal(formP);
    if (formP->window.windowFlags.modal && module->centerDialogs) {
      FrmCenterForm(formP);
    }
  }

  return formP;
}

// Set the active form. All input (key and pen) is directed to the active form and all drawing occurs there.

void FrmSetActiveForm(FormType *formP) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  WinHandle wh;
  EventType event;

  if (formP) {
    debug(DEBUG_TRACE, "Form", "FrmSetActiveForm %d", formP->formId);
    module->currentForm = formP;
    wh = WinGetActiveWindow();
    WinSetActiveWindow(&module->currentForm->window);
    WinSetDrawWindow(&module->currentForm->window);
    MenuSetActiveMenu(module->currentForm->mbar);

    xmemset(&event, 0, sizeof(EventType));
    event.eType = winExitEvent;
    event.data.winExit.exitWindow = wh;
    event.data.winExit.enterWindow = WinGetActiveWindow();
    EvtAddEventToQueue(&event);

    xmemset(&event, 0, sizeof(EventType));
    event.eType = winEnterEvent;
    event.data.winEnter.exitWindow = wh;
    event.data.winEnter.enterWindow = WinGetActiveWindow();
    EvtAddEventToQueue(&event);
  }
}

FormType *FrmGetActiveForm(void) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  return module->currentForm;
}

void FrmSetEventHandler(FormType *formP, FormEventHandlerType *handler) {
  if (formP) {
    formP->handler = handler;
  }
}

// Dispatch an event to the application’s handler for the form.
// If the form’s event handler returns false, the event is passed to FrmHandleEvent.
// Returns the Boolean value returned by the form’s event handler or FrmHandleEvent.
// XXX The event is dispatched to the current form’s handler unless the form ID is specified in the event data, as, for example, with frmOpenEvent or frmGotoEvent.

static Boolean FrmDispatchEventInternal(FormType *formP, EventType *eventP) {
  Boolean r = false;

  if (formP) {
    if (formP->m68k_handler) {
      if (CallFormHandler(formP->m68k_handler, eventP)) r = true;
    } else if (formP->handler) {
      if (formP->handler(eventP)) r = true;
    }

    if (formP->diaPolicy == frmDIAPolicyStayOpen && eventP && eventP->eType == frmOpenEvent) {
      if (PINGetInputAreaState() == pinInputAreaClosed) {
        PINSetInputAreaState(pinInputAreaOpen);
      }
      if (PINGetInputTriggerState() == pinInputTriggerEnabled) {
        PINSetInputTriggerState(pinInputTriggerDisabled);
      }
    }
  }

  return r;
}

Boolean FrmDispatchEvent(EventType *eventP) {
  FormType *formP = FrmGetActiveForm();
  Boolean r = false;

  if (formP) {
    r = FrmDispatchEventInternal(formP, eventP);
    if (!r && formP) {
      r = FrmHandleEvent(formP, eventP);
    }
  }

  return r;
}

void FrmEraseObject(FormType *formP, UInt16 objIndex, Boolean setUsable) {
  FormObjectType obj;
  RectangleType rect;
  IndexedColorType formFill, oldb;
  MemHandle h;
  BitmapPtr bitmapP;
  FontID old;
  Coord width, height;
  UInt16 totalLines, max;
  Boolean erase = false;

  if (formP && objIndex < formP->numObjects) {
    pumpkin_dirty_region_mode(dirtyRegionBegin);

    obj = formP->objects[objIndex].object;

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:
        if (setUsable) obj.field->attr.usable = false;
        FldEraseField(obj.field);
        break;

      case frmControlObj:
        if (setUsable) obj.control->attr.usable = false;
        CtlEraseControl(obj.control);
        break;

      case frmTitleObj:
        MemMove(&rect, &obj.title->rect, sizeof(RectangleType));
        erase = true;
        break;

      case frmLabelObj:
        if (setUsable) obj.label->attr.usable = false;
        if (formP->attr.visible) {
          if (obj.label->text) {
            old = FntSetFont(obj.label->fontID);
            max = formP->window.windowBounds.extent.x - obj.label->pos.x + 1;
            RctSetRectangle(&rect, obj.label->pos.x, obj.label->pos.y, max, FntCharHeight()*5);
            WinDrawCharBox(obj.label->text, StrLen(obj.label->text), obj.label->fontID, &rect, false, &totalLines, NULL, &max, NULL, 0);
            RctSetRectangle(&rect, obj.label->pos.x, obj.label->pos.y, max, FntCharHeight()*totalLines);
            FntSetFont(old);
            erase = true;
          }
        }
        break;

      case frmBitmapObj:
        if ((h = DmGetResource(bitmapRsc, obj.bitmap->rscID)) != NULL) {
          if ((bitmapP = MemHandleLock(h)) != NULL) {
            BmpGetDimensions(bitmapP, &width, &height, NULL);
            RctSetRectangle(&rect, obj.bitmap->pos.x, obj.bitmap->pos.y, width, height);
            erase = true;
            MemHandleUnlock(h);
          }
          DmReleaseResource(h);
        }
        break;

      case frmGadgetObj:
        if (setUsable) obj.gadget->attr.usable = false;
        if (formP->attr.visible && (obj.gadget->handler || obj.gadget->m68k_handler)) {
          formFill = UIColorGetTableEntryIndex(UIFormFill);
          oldb = WinSetBackColor(formFill);
          WinPushDrawState();
          if (obj.gadget->m68k_handler) {
            CallGadgetHandler(obj.gadget->m68k_handler, (FormGadgetTypeInCallback *)obj.gadget, formGadgetEraseCmd, NULL);
          } else {
            obj.gadget->handler((FormGadgetTypeInCallback *)obj.gadget, formGadgetEraseCmd, NULL);
          }
          WinPopDrawState();
          WinSetBackColor(oldb);
          obj.gadget->attr.visible = 0;
        }
        break;

      case frmListObj:
        if (setUsable) obj.list->attr.usable = 0;
        if (obj.list->attr.visible) {
          LstEraseList(obj.list);
        }
        break;

      case frmTableObj:
        if (setUsable) obj.table->attr.usable = 0;
        if (obj.table->attr.visible) {
          TblEraseTable(obj.table);
        }
        break;

      case frmScrollBarObj:
        if (setUsable) obj.scrollBar->attr.usable = 0;
        if (obj.scrollBar->attr.visible) {
          MemMove(&rect, &obj.scrollBar->bounds, sizeof(RectangleType));
          erase = true;
        }
        break;

      default:
        debug(DEBUG_ERROR, "Form", "FrmEraseObject type %d not supported", formP->objects[objIndex].objectType);
        break;
    }

    if (erase) {
      formFill = UIColorGetTableEntryIndex(UIFormFill);
      oldb = WinSetBackColor(formFill);
      WinEraseRectangle(&rect, 0);
      WinSetBackColor(oldb);
    }

    pumpkin_dirty_region_mode(dirtyRegionEnd);
  }
}

void FrmDrawObject(FormType *formP, UInt16 objIndex, Boolean setUsable) {
  FormObjectType obj;
  IndexedColorType formTitle, formFrame, formFill, objFill, fieldText, oldb, oldt;
  WinDrawOperation mode;
  RectangleType rect;
  Coord bw, bh;
  MemHandle h;
  BitmapPtr bitmapP;
  FontID old;
  UInt16 totalLines, max, graffitiState;
  Int16 x, y, tw, th;
  Boolean formVisible;

  if (formP && objIndex < formP->numObjects) {
    pumpkin_dirty_region_mode(dirtyRegionBegin);

    obj = formP->objects[objIndex].object;
    formTitle = UIColorGetTableEntryIndex(UIObjectSelectedForeground);
    formFill = UIColorGetTableEntryIndex(UIFormFill);
    formFrame = UIColorGetTableEntryIndex(UIFormFrame);
    objFill = UIColorGetTableEntryIndex(UIObjectFill);
    fieldText = UIColorGetTableEntryIndex(UIFieldText);

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:
        if (setUsable) obj.field->attr.usable = 1;
        if (obj.field->attr.usable) {
          FldDrawField(obj.field);
        }
        break;
      case frmControlObj:
        if (setUsable) obj.control->attr.usable = 1;
        if (obj.control->attr.usable) {
          CtlDrawControl(obj.control);
        }
        break;
      case frmLabelObj:
        if (setUsable) obj.label->attr.usable = 1;
        if (obj.label->attr.usable) {
          old = FntSetFont(obj.label->fontID);
          oldb = WinSetBackColor(objFill);
          oldt = WinSetTextColor(fieldText);
          max = formP->window.windowBounds.extent.x - obj.label->pos.x + 1;
          RctSetRectangle(&rect, obj.label->pos.x, obj.label->pos.y, max, FntCharHeight()*5);
          WinDrawCharBox(obj.label->text, StrLen(obj.label->text), obj.label->fontID, &rect, false, &totalLines, NULL, &max, NULL, 0);
          rect.extent.y = FntCharHeight()*totalLines;
          obj.label->extent.x = max;
          obj.label->extent.y = rect.extent.y;
          WinDrawCharBox(obj.label->text, StrLen(obj.label->text), obj.label->fontID, &rect, true, NULL, NULL, NULL, NULL, 0);
          WinSetBackColor(oldb);
          WinSetTextColor(oldt);
          FntSetFont(old);
        }
        break;
      case frmGadgetObj:
        if (setUsable) obj.gadget->attr.usable = 1;
        if (obj.gadget->attr.usable) {
          if (obj.gadget->m68k_handler || obj.gadget->handler) {
            oldb = WinSetBackColor(formFill);
            WinPushDrawState();
            if (obj.gadget->m68k_handler) {
              CallGadgetHandler(obj.gadget->m68k_handler, (FormGadgetTypeInCallback *)obj.gadget, formGadgetDrawCmd, NULL);
            } else {
              obj.gadget->handler((FormGadgetTypeInCallback *)obj.gadget, formGadgetDrawCmd, NULL);
            }
            WinPopDrawState();
            WinSetBackColor(oldb);
            obj.gadget->attr.visible = 1;
          } else {
          }
        }
        break;
      case frmListObj:
        formVisible = formP->attr.visible;
/*
2024-08-14 00:30:18.236620 I 31096 Plucker  XXX: drawObject list 3242, formVisible 0, visible 0, usable 0, setUsable 1
2024-08-14 00:30:18.236622 I 31096 Plucker  XXX: drawObject draw!

2024-08-14 00:43:17.487624 I 31953 Plucker  XXX: drawObject list 3242, formVisible 0, visible 0, usable 1, setUsable 0
2024-08-14 00:43:17.487626 I 31953 Plucker  XXX: drawObject draw!

2024-08-14 00:34:11.389664 I 31312 ChemTabl XXX: drawObject list 1505, formVisible 1, visible 0, usable 0, setUsable 1
2024-08-14 00:34:11.389666 I 31312 ChemTabl XXX: drawObject draw!

2024-08-14 00:35:12.018367 I 31416 eReader  XXX: drawObject list 1503, formVisible 0, visible 0, usable 1, setUsable 0
2024-08-14 00:35:12.018377 I 31416 eReader  XXX: drawObject draw!
*/

        debug(DEBUG_TRACE, "List", "drawObject list %d, formVisible %d, visible %d, usable %d, setUsable %d",
          obj.list->id, formP->attr.visible, obj.list->attr.visible, obj.list->attr.usable, setUsable);
        if (setUsable && formVisible) obj.list->attr.usable = 1;
        if (!obj.list->bitsBehind && obj.list->attr.usable) {
          debug(DEBUG_TRACE, "List", "drawObject draw!");
          LstDrawList(obj.list);
        }
        break;
      case frmTableObj:
        if (setUsable) obj.table->attr.usable = 1;
        if (obj.table->attr.usable) {
          TblDrawTable(obj.table);
        }
        break;
      case frmBitmapObj:
        if (setUsable) obj.bitmap->attr.usable = 1;
        if (obj.bitmap->attr.usable) {
          if ((h = DmGetResource(bitmapRsc, obj.bitmap->rscID)) != NULL) {
            if ((bitmapP = MemHandleLock(h)) != NULL) {
              mode = WinSetDrawMode(winPaint);
              WinPaintBitmap(bitmapP, obj.bitmap->pos.x, obj.bitmap->pos.y);
              WinSetDrawMode(mode);
              MemHandleUnlock(h);
            }
            DmReleaseResource(h);
          }
        }
        break;
      case frmLineObj:
        break;
      case frmFrameObj:
        break;
      case frmRectangleObj:
        break;
      case frmTitleObj:
        // PalmOS draws a 2-pixel line on the border between the title bar and the data area
        old = FntSetFont(boldFont);
        oldb = WinSetBackColor(formFrame);
        oldt = WinSetTextColor(formTitle);
        tw = obj.title->text ? FntCharsWidth(obj.title->text, sys_strlen(obj.title->text)) : 4;
        th = FntCharHeight();
        if (formP->window.windowFlags.modal) {
          RctSetRectangle(&rect, 0, 0, formP->window.windowBounds.extent.x, th+1);
          WinEraseRectangle(&rect, 0);
          MemMove(&obj.title->rect, &rect, sizeof(RectangleType));
          x = (formP->window.windowBounds.extent.x - tw) / 2;
          y = 0;
        } else {
          x = 2;
          y = 2;

          // erase right rounded border
          WinSetBackColor(formFill);
          RctSetRectangle(&rect, tw+2, 0, 2, th+4);
          WinEraseRectangle(&rect, 0);
          WinSetBackColor(formFrame);

          RctSetRectangle(&rect, 0, 0, tw+4, th+4);
          WinEraseRectangle(&rect, 1);
          MemMove(&obj.title->rect, &rect, sizeof(RectangleType));
          RctSetRectangle(&rect, 0, th+2, formP->window.windowBounds.extent.x, 2);
          WinEraseRectangle(&rect, 0);
        }
        if (obj.title->text) {
          WinDrawOperation prev = WinSetDrawMode(winOverlay);
          WinPaintChars(obj.title->text, sys_strlen(obj.title->text), x, y);
          WinSetDrawMode(prev);
        }
        if (formP->window.windowFlags.modal && formP->helpRscId) {
          FntSetFont(symbol11Font);
          bw = FntCharWidth(0x04);
          bh = FntCharHeight();
          RctSetRectangle(&formP->helpRect, formP->window.windowBounds.extent.x - bw, 0, bw, bh);
          WinSetTextColor(formFrame);
          WinSetBackColor(formTitle);
          WinPaintChar(0x04, formP->helpRect.topLeft.x, formP->helpRect.topLeft.y);
        }
        WinSetBackColor(oldb);
        WinSetTextColor(oldt);
        FntSetFont(old);
        break;
      case frmPopupObj:
        break;
      case frmGraffitiStateObj:
        old = FntSetFont(symbolFont);
        bw = FntCharWidth(13);
        bh = FntCharHeight();
        oldb = WinSetBackColor(formFill);
        RctSetRectangle(&rect, obj.grfState->pos.x, obj.grfState->pos.y, bw, bh);
        WinEraseRectangle(&rect, 0);
        WinSetBackColor(oldb);
        graffitiState = KbdGrfGetState();
        if (graffitiState) {
          WinPaintChar(graffitiState, obj.grfState->pos.x, obj.grfState->pos.y);
        }
        FntSetFont(old);
        break;
      case frmScrollBarObj:
        if (setUsable) obj.scrollBar->attr.usable = 1;
        if (obj.scrollBar->attr.usable) {
          SclDrawScrollBar(obj.scrollBar);
        }
        break;
      default:
        debug(DEBUG_ERROR, "Form", "FrmDrawObject type %d not supported", formP->objects[objIndex].objectType);
        break;
    }

    pumpkin_dirty_region_mode(dirtyRegionEnd);
  }
}

void FrmSetUsable(FormType *formP, UInt16 objIndex, Boolean usable) {
  FormObjectType obj;

  if (formP && objIndex < formP->numObjects) {
    obj = formP->objects[objIndex].object;

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:     obj.field->attr.usable     = usable; break;
      case frmControlObj:   obj.control->attr.usable   = usable; break;
      case frmLabelObj:     obj.label->attr.usable     = usable; break;
      case frmListObj:      obj.list->attr.usable      = usable; break;
      case frmTableObj:     obj.table->attr.usable     = usable; break;
      case frmGadgetObj:    obj.gadget->attr.usable    = usable; break;
      case frmScrollBarObj: obj.scrollBar->attr.usable = usable; break;
      default: break;
    }
  }
}

Boolean FrmGetUsable(FormType *formP, UInt16 objIndex) {
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

void FrmSetVisible(FormType *formP, UInt16 objIndex, Boolean visible) {
  FormObjectType obj;

  if (formP && objIndex < formP->numObjects) {
    obj = formP->objects[objIndex].object;

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:     obj.field->attr.visible     = visible; break;
      case frmControlObj:   obj.control->attr.visible   = visible; break;
      case frmListObj:      obj.list->attr.visible      = visible; break;
      case frmTableObj:     obj.table->attr.visible     = visible; break;
      case frmGadgetObj:    obj.gadget->attr.visible    = visible; break;
      case frmScrollBarObj: obj.scrollBar->attr.visible = visible; break;
      default: break;
    }
  }
}

Boolean FrmGetVisible(FormType *formP, UInt16 objIndex) {
  FormObjectType obj;
  Boolean visible = false;

  if (formP && objIndex < formP->numObjects) {
    obj = formP->objects[objIndex].object;

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:     visible = obj.field->attr.visible;     break;
      case frmControlObj:   visible = obj.control->attr.visible;   break;
      case frmListObj:      visible = obj.list->attr.visible;      break;
      case frmTableObj:     visible = obj.table->attr.visible;     break;
      case frmGadgetObj:    visible = obj.gadget->attr.visible;    break;
      case frmScrollBarObj: visible = obj.scrollBar->attr.visible; break;
      default: break;
    }
  }

  return visible;
}

void FrmSetColorTrigger(FormType *formP, UInt16 id, RGBColorType *rgb, Boolean draw) {
  ControlType *ctl;
  UInt16 index;
  char *label, buf[16];

  index = FrmGetObjectIndex(formP, id);
  ctl = (ControlType *)FrmGetObjectPtr(formP, index);
  label = (char *)CtlGetLabel(ctl);
  StrNPrintF(buf, sizeof(buf)-1, "#%02X%02X%02X", rgb->r, rgb->g, rgb->b);
  StrNCopy(label, buf, 7);

  if (draw) {
    FrmDrawObject(formP, index, false);
  }
}

static void FrmReleaseFocus(FormType *formP) {
  FormObjectType obj;

  if (formP && formP->selectedObject != -1) {
    switch (formP->objects[formP->selectedObject].objectType) {
      case frmTableObj:
        obj = formP->objects[formP->selectedObject].object;
        TblReleaseFocus(obj.table);
        break;
      default:
        break;
    }
  }
}

Boolean FrmTrackPenUp(UInt32 x, UInt32 y) {
  FormType *formP;
  FormObjectType obj;
  RectangleType rect;
  EventType event;
  Boolean handled = false;

  formP = FrmGetActiveForm();
  if (formP && formP->selectedObject != -1) {
    obj = formP->objects[formP->selectedObject].object;

    MemSet(&event, sizeof(EventType), 0);
    event.eType = penUpEvent;
    event.screenX = x;
    event.screenY = y;

    switch (formP->objects[formP->selectedObject].objectType) {
      case frmFieldObj:
        handled = FldHandleEvent(obj.field, &event);
        break;
      case frmControlObj:
        handled = CtlHandleEvent(obj.control, &event);
        break;
      case frmScrollBarObj:
        handled = SclHandleEvent(obj.scrollBar, &event);
        break;
      case frmTableObj:
        handled = TblHandleEvent(obj.table, &event);
        break;
      case frmListObj:
        handled = LstHandleEvent(obj.list, &event);
        break;
      case frmTitleObj:
        RctSetRectangle(&rect, obj.title->rect.topLeft.x, obj.title->rect.topLeft.y, obj.title->rect.extent.x, obj.title->rect.extent.y);
        if (RctPtInRectangle(x, y, &rect)) {
          event.eType = frmTitleSelectEvent;
          event.data.frmTitleSelect.formID = formP->formId;
          EvtAddEventToQueue(&event);
          handled = true;
        }
        break;
      case frmGadgetObj:
        if (!pumpkin_is_m68k()) {
          event.eType = frmGadgetMiscEvent;
          event.data.gadgetMisc.gadgetID = obj.gadget->id;
          event.data.gadgetMisc.gadgetP = obj.gadget;
          event.data.gadgetMisc.selector = 0xFFFF;
          event.data.gadgetMisc.dataP = NULL;
          EvtAddEventToQueue(&event);
          handled = true;
        }
        break;
      default:
        break;
    }
    formP->selectedObject = -1;
  }

  return handled;
}

// Handle the event that has occurred in the form.
// Never call this function directly. Call FrmDispatchEvent instead.
// FrmDispatchEvent passes events to a form’s custom event
// handler and then, if the event was not handled, to this function.

Boolean FrmHandleEvent(FormType *formP, EventType *eventP) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  EventType event;
  RectangleType rect;
  FormObjectType obj;
  FieldType *fldP;
  Boolean handled = false;
  UInt16 length;
  int i;

  switch (eventP->eType) {
    case frmUpdateEvent:
      // Calls FrmDrawForm to redraw the form.
      FrmDrawForm(formP);
      handled = true;
      break;

    case keyDownEvent:
      // Passes the event to the handler for the object that has the focus. If no object has the focus, the event is ignored.
      if (formP->activeList == -1) {
        if ((fldP = FldGetActiveField()) != NULL) {
          handled = FldHandleEvent(fldP, eventP);
        }
      }
      break;

    case penDownEvent:
      // Checks the list of objects contained by the form to determine if the pen is within the bounds of one.
      // If it is, the appropriate handler is called to handle the event, for example, if the pen is in a control,
      // CtlHandleEvent is called. If the pen isn’t within the bounds of an object, the event is ignored by the form.
      // If the pen is within the bounds of the help icon, it is tracked until it is lifted, and if it’s still within the help
      // icon bounds, the help dialog is displayed.

      if (formP->window.windowFlags.modal && formP->helpRscId) {
        if (RctPtInRectangle(eventP->screenX, eventP->screenY, &formP->helpRect)) {
          FrmHelp(formP->helpRscId);
          handled = true;
          break;
        }
      }

      for (i = 0; i < formP->numObjects && !handled; i++) {
        obj = formP->objects[i].object;

        switch (formP->objects[i].objectType) {
          case frmFieldObj:
            if (formP->activeList != -1) break;
            handled = FldHandleEvent(obj.field, eventP);
            if (handled) {
              FrmReleaseFocus(formP);
              formP->selectedObject = i;
            }
            break;
          case frmControlObj:
            FrmReleaseFocus(formP);
            if (formP->activeList != -1) break;
            handled = CtlHandleEvent(obj.control, eventP);
            if (handled) {
              FldSetActiveField(NULL);
              formP->selectedObject = i;
            }
            break;
          case frmListObj:
            handled = LstHandleEvent(obj.list, eventP);
            if (handled) {
              FrmReleaseFocus(formP);
              FldSetActiveField(NULL);
              formP->selectedObject = i;
            }
            break;
          case frmScrollBarObj:
            if (formP->activeList != -1) break;
            handled = SclHandleEvent(obj.scrollBar, eventP);
            if (handled) {
              FrmReleaseFocus(formP);
              FldSetActiveField(NULL);
              formP->selectedObject = i;
            }
            break;
          case frmTableObj:
            if (formP->activeList != -1) break;
            handled = TblHandleEvent(obj.table, eventP);
            if (handled) {
              FldSetActiveField(NULL);
              formP->selectedObject = i;
            }
            break;
          case frmTitleObj:
            if (formP->activeList != -1) break;
            RctSetRectangle(&rect, obj.title->rect.topLeft.x, obj.title->rect.topLeft.y, obj.title->rect.extent.x, obj.title->rect.extent.y);
            if (RctPtInRectangle(eventP->screenX, eventP->screenY, &rect)) {
              FrmReleaseFocus(formP);
              //FldSetActiveField(NULL);
              formP->selectedObject = i;
              xmemset(&event, 0, sizeof(EventType));
              event.eType = frmTitleEnterEvent;
              event.screenX = eventP->screenX;
              event.screenY = eventP->screenY;
              event.data.frmTitleEnter.formID = formP->formId;
              EvtAddEventToQueue(&event);
              handled = true;
            }
            break;
          case frmGadgetObj:
            if (formP->activeList != -1) break;
            if (obj.gadget->attr.usable) {
              if (RctPtInRectangle(eventP->screenX, eventP->screenY, &obj.gadget->rect)) {
                FrmReleaseFocus(formP);
                formP->selectedObject = i;
                FldSetActiveField(NULL);
                MemSet(&event, sizeof(EventType), 0);
                event.eType = frmGadgetEnterEvent;
                event.screenX = eventP->screenX;
                event.screenY = eventP->screenY;
                event.data.gadgetEnter.gadgetID = obj.gadget->id;
                event.data.gadgetEnter.gadgetP = obj.gadget;
                EvtAddEventToQueue(&event);
                handled = true;
              }
            }
            break;
          default:
            break;
        }
      }

      if (!handled) {
        FldSetActiveField(NULL);
      }
      break;

    case penMoveEvent:
      if (formP->selectedObject != -1) {
        obj = formP->objects[formP->selectedObject].object;
        switch (formP->objects[formP->selectedObject].objectType) {
          case frmFieldObj:
            handled = FldHandleEvent(obj.field, eventP);
            break;
          case frmControlObj:
            handled = CtlHandleEvent(obj.control, eventP);
            break;
          case frmScrollBarObj:
            handled = SclHandleEvent(obj.scrollBar, eventP);
            break;
          default:
            break;
        }
      }
      break;
    case frmTitleEnterEvent:
      // Tracks the pen until it is lifted. If it is lifted within the bounds of the form title, adds a frmTitleSelectEvent event to the event queue.
      handled = true;
      break;
    case frmTitleSelectEvent:
      // Adds a keyDownEvent with the vchrMenu character to the event queue.
      EvtEnqueueKey(vchrMenu, 0, commandKeyMask);
      handled = true;
      break;
    case frmGadgetEnterEvent:
      // Passes the event to the gadget’s callback function if the gadget has one.
      if (eventP->data.gadgetEnter.gadgetP->m68k_handler) {
        handled = CallGadgetHandler(eventP->data.gadgetEnter.gadgetP->m68k_handler, (FormGadgetTypeInCallback *)eventP->data.gadgetEnter.gadgetP, formGadgetHandleEventCmd, eventP);
      } else if (eventP->data.gadgetEnter.gadgetP->handler) {
        handled = eventP->data.gadgetEnter.gadgetP->handler((FormGadgetTypeInCallback *)eventP->data.gadgetEnter.gadgetP, formGadgetHandleEventCmd, eventP);
      }
      break;
    case frmGadgetMiscEvent:
      // Passes the event to the gadget’s callback function if the gadget has one.
      if (eventP->data.gadgetMisc.gadgetP->m68k_handler) {
        handled = CallGadgetHandler(eventP->data.gadgetMisc.gadgetP->m68k_handler, (FormGadgetTypeInCallback *)eventP->data.gadgetMisc.gadgetP, formGadgetHandleEventCmd, eventP);
      } else if (eventP->data.gadgetMisc.gadgetP->handler) {
        handled = eventP->data.gadgetMisc.gadgetP->handler((FormGadgetTypeInCallback *)eventP->data.gadgetMisc.gadgetP, formGadgetHandleEventCmd, eventP);
      }
      break;
    case ctlEnterEvent:
      // Passes the event and a pointer to the object the event occurred in to CtlHandleEvent.
      // The object pointer is obtained from the event data. If the control is part of an exclusive control group,
      // it deselects the currently selected control of the group first.
      handled = CtlHandleEvent(eventP->data.ctlEnter.pControl, eventP);
      break;
    case ctlRepeatEvent:
      // Passes the event and a pointer to the object the event occurred in to CtlHandleEvent. The object pointer is obtained from the event data.
      CtlHandleEvent(eventP->data.ctlRepeat.pControl, eventP);
      break;
    case ctlSelectEvent:
      // Checks if the control is a Popup Trigger Control. If it is, the list associated with the popup trigger is
      // displayed until the user makes a selection or touches the pen outside the bounds of the list. If a selection is
      // made, a popSelectEvent is added to the event queue.
      CtlHandleEvent(eventP->data.ctlSelect.pControl, eventP);
      break;
    case ctlExitEvent:
      handled = CtlHandleEvent(eventP->data.ctlExit.pControl, eventP);
      break;
    case sclEnterEvent:
      // Passes the event and a pointer to the object the event occurred in to SclHandleEvent.
      handled = SclHandleEvent(eventP->data.sclEnter.pScrollBar, eventP);
      break;
    case sclRepeatEvent:
      // Passes the event and a pointer to the object the event occurred in to SclHandleEvent.
      handled = SclHandleEvent(eventP->data.sclRepeat.pScrollBar, eventP);
      break;
    case lstEnterEvent:
      // Passes the event and a pointer to the object the event occurred in to LstHandleEvent. The object pointer is obtained from the event data.
      handled = LstHandleEvent(eventP->data.lstEnter.pList, eventP);
      break;
    case lstSelectEvent:
      handled = LstHandleEvent(eventP->data.lstSelect.pList, eventP);
      break;
    case popSelectEvent:
      // Sets the label of the popup trigger to the current selection of the popup list.
      handled = LstHandleEvent(eventP->data.popSelect.listP, eventP);
      break;
    case lstExitEvent:
      handled = LstHandleEvent(eventP->data.lstExit.pList, eventP);
      break;
    case tblEnterEvent:
      // Passes the event and a pointer to the object the event occurred in to TblHandleEvent. The object pointer is obtained from the event data.
      handled = TblHandleEvent(eventP->data.tblEnter.pTable, eventP);
      break;
    case tblSelectEvent:
      handled = TblHandleEvent(eventP->data.tblSelect.pTable, eventP);
      break;
    case tblExitEvent:
      handled = TblHandleEvent(eventP->data.tblExit.pTable, eventP);
      break;
    case fldEnterEvent:
      // Checks if a field object or a table object has the focus and passes the event to the appropriate handler
      // (FldHandleEvent or TblHandleEvent). The table object is also a container object, which may contain a
      // field object. If TblHandleEvent receives a field event, it passes the event to the field object contained within it.
      handled = FldHandleEvent(eventP->data.fldEnter.pField, eventP);
      break;
    case menuEvent:
      // Checks if the menu command is one of the system edit menu commands. The system provides a standard
      // edit menu that contains the commands Undo, Cut, Copy, Paste, Select All, and Keyboard. FrmHandleEvent responds to these commands.
      switch (eventP->data.menu.itemID) {
        case sysEditMenuSelectAllCmd:
          if ((fldP = FldGetActiveField()) != NULL) {
            if ((length = FldGetTextLength(fldP)) >= 0) {
              FldSetSelection(fldP, 0, length);
            }
            handled = true;
          }
          break;
        case sysEditMenuCutCmd:
          if ((fldP = FldGetActiveField()) != NULL) {
            FldCut(fldP);
            handled = true;
          }
          break;
        case sysEditMenuCopyCmd:
          if ((fldP = FldGetActiveField()) != NULL) {
            FldCopy(fldP);
            handled = true;
          }
          break;
        case sysEditMenuPasteCmd:
          if ((fldP = FldGetActiveField()) != NULL) {
            FldPaste(fldP);
            handled = true;
          }
          break;
        case sysEditMenuUndoCmd:
          if ((fldP = FldGetActiveField()) != NULL) {
            FldUndo(fldP);
            handled = true;
          }
          break;
        case sysEditMenuKeyboardCmd:
          if (FldGetActiveField() != NULL) {
            SysKeyboardDialog(kbdAlpha);
            FrmDrawForm(formP);
            handled = true;
          }
          break;
        case sysEditMenuGraffitiCmd:
          break;
      }
      break;
    case frmCloseEvent:
      // Erases the form and releases any memory allocated for it.
      if (formP == module->currentForm) {
        WinSetActiveWindow(WinGetDisplayWindow());
        WinSetDrawWindow(WinGetDisplayWindow());
        module->currentForm = NULL;
        MenuSetActiveMenu(NULL);
      }
      FrmDeleteForm(formP);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void FrmDeleteFormInternal(FormType *formP) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);

  debug(DEBUG_TRACE, "Form", "FrmDeleteFormInternal form %d", formP->formId);

  if (formP == module->currentForm) {
    WinSetActiveWindow(NULL);
    WinSetDrawWindow(NULL);
    module->currentForm = NULL;
    MenuSetActiveMenu(NULL);
  }

  if (formP->window.bitmapP) {
    debug(DEBUG_TRACE, "Form", "FrmDeleteFormInternal BmpDelete %p", formP->window.bitmapP);
    BmpDelete(formP->window.bitmapP);
  }

  if (formP->bitsBehindForm) {
    debug(DEBUG_TRACE, "Form", "FrmDeleteFormInternal WinDeleteWindow %p", formP->bitsBehindForm);
    WinDeleteWindow(formP->bitsBehindForm, false);
  }

  if (formP->mbar) {
    MenuDispose(formP->mbar);
  }

  pumpkin_destroy_form(formP);
}

// Release the memory occupied by a form. Any memory allocated to objects in the form is also released.
// This function does not modify the display.

void FrmDeleteForm(FormType *formP) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  FormList *p, *q;

  if (formP) {
    debug(DEBUG_TRACE, "Form", "FrmDeleteForm %d", formP->formId);
    for (p = module->list, q = NULL; p; q = p, p = p->next) {
      if (p->formP == formP) {
        if (q) {
          q->next = p->next;
        } else {
          module->list = p->next;
        }
        xfree(p);
        break;
      }
    }

    FrmDeleteFormInternal(formP);
  }
}

void FrmDrawEmptyDialog(FormType *formP, RectangleType *rect, Int16 margin, WinHandle wh) {
  RectangleType aux;
  IndexedColorType formFill, formFrame, old;
  PatternType oldPattern;
  WinHandle oldDraw, oldActive;

  // draw dialog border
  if (formP->window.windowFlags.modal) {
    oldActive = WinGetActiveWindow();
    oldDraw = WinSetDrawWindow(wh);
    WinSetActiveWindow(wh);
    formFrame = UIColorGetTableEntryIndex(UIFormFrame);
    old = WinSetForeColor(formFrame);
    oldPattern = WinGetPatternType();
    WinSetPatternType(blackPattern);
    WinDrawRectangle(rect, margin);
    WinSetPatternType(oldPattern);
    WinSetForeColor(old);
    WinSetActiveWindow(oldActive);
    WinSetDrawWindow(oldDraw);
  }

  // erase form background
  MemMove(&aux, &formP->window.windowBounds, sizeof(RectangleType));
  aux.topLeft.x = 0;
  aux.topLeft.y = 0;
  formFill = UIColorGetTableEntryIndex(UIFormFill);
  old = WinSetBackColor(formFill);
  WinEraseRectangle(&aux, 0);
  WinSetBackColor(old);
}

void FrmDrawForm(FormType *formP) {
  WinHandle oldd, olda;
  IndexedColorType formFill;
  RectangleType rect;
  UInt16 objIndex;
  Int16 margin;

  if (formP) {
    debug(DEBUG_TRACE, "Form", "FrmDrawForm %d", formP->formId);
    pumpkin_dirty_region_mode(dirtyRegionBegin);

    // if this is not done, SimCity draws scrollbars in white
    formFill = UIColorGetTableEntryIndex(UIFormFill);
    WinSetForeColor(0xFF);
    WinSetBackColor(formFill);
    WinSetPatternType(blackPattern);

    margin = WinGetBorderRect(&formP->window, &rect);

    if (!formP->attr.visible) {
      WinSaveRectangle(formP->bitsBehindForm, &rect);
    }

    olda = WinGetActiveWindow();
    WinSetActiveWindow(FrmGetWindowHandle(formP));
    oldd = WinSetDrawWindow(FrmGetWindowHandle(formP));
    FrmDrawEmptyDialog(formP, &rect, margin, WinGetDisplayWindow());

    // draw form objects
    for (objIndex = 0; objIndex < formP->numObjects; objIndex++) {
      if (formP->objects[objIndex].objectType == frmControlObj) {
        // CtlInvertControl uses the current visible state to perform color inversion.
        // Since all controls were erased by FrmDrawEmptyDialog above, the visible status
        // must be set here, otherwise the inversion routine would produce garbled pixels.
        FrmSetVisible(formP, objIndex, false);
      }
      FrmDrawObject(formP, objIndex, false);
    }
    formP->attr.visible = 1;

    WinSetDrawWindow(oldd);
    WinSetActiveWindow(olda);

    pumpkin_dirty_region_mode(dirtyRegionEnd);
  }
}

// Erase a form from the display.
// If the region obscured by the form was saved by FrmDrawForm, this function restores that region.

void FrmEraseForm(FormType *formP) {
  RectangleType rect;

  if (formP) {
    debug(DEBUG_TRACE, "Form", "FrmEraseForm %d", formP->formId);
    MemMove(&rect, &formP->window.windowBounds, sizeof(RectangleType));
    if (formP->window.windowFlags.modal) {
      rect.topLeft.x -= 2;
      rect.topLeft.y -= 2;
      rect.extent.x += 4;
      rect.extent.y += 4;
    }
    WinRestoreRectangle(formP->bitsBehindForm, &rect);

    formP->attr.visible = 0;
  }
}

UInt16 FrmGetActiveFormID(void) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  return module->currentForm ? module->currentForm->formId : 0;
}

FieldType *FrmGetActiveField(const FormType* formP) {
  FieldType *fldP = NULL;

  fldP = FldGetActiveField();

  return fldP;
}

Boolean FrmGetUserModifiedState(const FormType *formP) {
  // system use only
  debug(DEBUG_ERROR, "Form", "FrmGetUserModifiedState not implemented");
  return 0;
}

void FrmSetNotUserModified(FormType *formP) {
  // system use only
  debug(DEBUG_ERROR, "Form", "FrmSetNotUserModified not implemented");
}

UInt16 FrmGetFocus(const FormType *formP) {
  UInt16 i, objIndex = noFocus;

  if (formP) {
    for (i = 0; i < formP->numObjects; i++) {
      if (formP->objects[i].objectType == frmFieldObj && formP->objects[i].object.field->attr.hasFocus) {
        objIndex = i;
        break;
      }
    }
  }

  return objIndex;
}

void FrmSetFocus(FormType *formP, UInt16 fieldIndex) {
  if (formP) {
    FldSetActiveField(NULL);

    if (fieldIndex < formP->numObjects) {
      if (formP->objects[fieldIndex].objectType == frmFieldObj) {
        FldSetActiveField(formP->objects[fieldIndex].object.field);
      }
    }
  }
}

void FrmGetFormBounds(const FormType *formP, RectangleType *rP) {
  if (formP && rP) {
    MemMove(rP, &formP->window.windowBounds, sizeof(RectangleType));
    WinScaleRectangle(rP);
  }
}

void FrmSetFormBounds(const FormType *formP, RectangleType *rP) {
  if (formP && rP) {
    MemMove((RectangleType *)&formP->window.windowBounds, rP, sizeof(RectangleType));
    WinUnscaleRectangle((RectangleType *)&formP->window.windowBounds);
  }
}

WinHandle FrmGetWindowHandle(const FormType *_formP) {
  FormType *formP = (FormType *)_formP;
  WinHandle wh = formP ? &formP->window : NULL;
  return wh;
}

UInt16 FrmGetFormId(const FormType *formP) {
  return formP ? formP->formId : 0;
}

FormType *FrmGetFormPtr(UInt16 formId) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  FormList *p;

  for (p = module->list; p; p = p->next) {
    if (p->formP->formId == formId) {
      return p->formP;
    }
  }

  return NULL;
}

FormType *FrmGetFirstForm(void) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  return module->list ? module->list->formP : NULL;
}

UInt16 FrmGetNumberOfObjects(const FormType *formP) {
  return formP ? formP->numObjects : 0;
}

UInt16 FrmGetObjectIndex(const FormType *formP, UInt16 objID) {
  UInt16 index = frmInvalidObjectId;

  if (formP) {
    for (index = 0; index < formP->numObjects; index++) {
      if (formP->objects[index].id == objID) break;
    }
    if (index == formP->numObjects) index = frmInvalidObjectId;
  }

  return index;
}

UInt16 FrmGetObjectIndexFromPtr(const FormType *formP, void *objP) {
  UInt16 index = frmInvalidObjectId;

  if (formP && objP) {
    for (index = 0; index < formP->numObjects; index++) {
      if (formP->objects[index].object.ptr == objP) break;
    }
    if (index == formP->numObjects) index = frmInvalidObjectId;
  }

  return index;
}

UInt16 FrmGetObjectId(const FormType *formP, UInt16 objIndex) {
  UInt16 id = frmInvalidObjectId;
  
  if (formP) {
    if (objIndex < formP->numObjects) {
      id = formP->objects[objIndex].id;
    }
  }

  return id;
}

FormObjectKind FrmGetObjectType(const FormType *formP, UInt16 objIndex) {
  FormObjectKind kind = 0;

  if (formP && objIndex < formP->numObjects) {
    kind = formP->objects[objIndex].objectType;
  }

  return kind;
}

void *FrmGetObjectPtr(const FormType *formP, UInt16 objIndex) {
  FormObjectType obj;
  void *p = NULL;

  if (formP && objIndex < formP->numObjects) {
    obj = formP->objects[objIndex].object;
    p = obj.ptr;
  }

  return p;
}

void FrmSetObjectPtr(const FormType *formP, UInt16 objIndex, void *p) {
  if (formP && objIndex < formP->numObjects) {
    formP->objects[objIndex].object.ptr = p;
  }
}

void FrmHideObject(FormType *formP, UInt16 objIndex) {
  if (formP && objIndex < formP->numObjects) {
    debug(DEBUG_TRACE, "Form", "FrmHideObject form %d object %d %s", formP->formId, objIndex, FrmObjectTypeName(formP->objects[objIndex].objectType));
    FrmEraseObject(formP, objIndex, true);
  }
}

void FrmShowObject(FormType *formP, UInt16 objIndex) {
  if (formP && objIndex < formP->numObjects) {
    debug(DEBUG_TRACE, "Form", "FrmShowObject form %d object %d %s", formP->formId, objIndex, FrmObjectTypeName(formP->objects[objIndex].objectType));
    FrmDrawObject(formP, objIndex, true);
  }
}

Int16 FrmGetControlValue(const FormType *formP, UInt16 objIndex) {
  Int16 value = 0;

  if (formP && objIndex < formP->numObjects) {
    if (formP->objects[objIndex].objectType == frmControlObj) {
      value = CtlGetValue(formP->objects[objIndex].object.control);
    }
  }

  return value;
}

const Char *FrmGetTitle(const FormType *formP) {
  int i;

  if (formP) {
    for (i = 0; i < formP->numObjects; i++) {
      if (formP->objects[i].objectType == frmTitleObj) {
        return formP->objects[i].object.title->text;
      }
    }
  }

  return NULL;
}

/*
This function saves the pointer passed in newTitle; it does not
make a copy. The value of newTitle must not be a pointer to a
stack-based object
*/
void FrmSetTitle(FormType *formP, Char *newTitle) {
  RectangleType old, rect;
  FormObjectType obj;
  UInt16 i, width;

  if (formP && newTitle) {
    for (i = 0; i < formP->numObjects; i++) {
      if (formP->objects[i].objectType == frmTitleObj) {
        debug(DEBUG_TRACE, "Form", "FrmSetTitle %d \"%s\"", formP->formId, newTitle);
        formP->objects[i].object.title->text = newTitle;
        if (formP->attr.visible) {
          obj = formP->objects[i].object;
          MemMove(&old, &obj.title->rect, sizeof(RectangleType));

          // draw new title
          FrmDrawObject(formP, formP->objects[i].object.title->objIndex, false);
          MemMove(&rect, &obj.title->rect, sizeof(RectangleType));

          // if new title is shorter than old title
          if (rect.extent.x < old.extent.x) {
            // erase excess
            width = old.extent.x - rect.extent.x;
            rect.topLeft.x += rect.extent.x;
            rect.extent.x = width;
            rect.extent.y -= 2;
            WinEraseRectangle(&rect, 0);
          }
        }
        break;
      }
    }
  }
}

void FrmCopyTitle(FormType *formP, const Char *newTitle) {
  UInt16 i;

  if (formP && newTitle) {
    for (i = 0; i < formP->numObjects; i++) {
      if (formP->objects[i].objectType == frmTitleObj) {
        debug(DEBUG_TRACE, "Form", "FrmCopyTitle %d \"%s\"", formP->formId, newTitle);
        if (formP->attr.visible) {
          FrmEraseObject(formP, formP->objects[i].object.title->objIndex, false);
        }
        StrCopy(formP->objects[i].object.title->text, newTitle);
        if (formP->attr.visible) {
          FrmDrawObject(formP, formP->objects[i].object.title->objIndex, false);
        }
        break;
      }
    }
  }
}

/*
boot forms:
10000: Edit Categories (big)
10100: Go To Date
10200: Set Time (big)
10300: Keyboard
10400: Tips
10500: Find (small)
10600: Find (big)
10900: Edit Memo ???
10950: Edit Memo ???
11000: About Palm
11100: Edit Categories (small)
11200: Graffiti
11500: blank
11600: Exchance Progress
11700: Exchange
11900: Select Font
12100: Set Time (small)
13100: Adjust Contrast
13150: Adjust Brightness
13200: Change Security
13300: Pick Color
13400: Set Time Zone
13500: Attention
*/

static UInt16 FrmDoDialogResponse(FormType *formP, Int32 timeout, UInt16 fieldID, FormCheckResponseFuncPtr callback) {
  FormType *previous;
  EventType event;
  FormObjectType obj;
  FieldType *fld;
  UInt16 index, buttonID;
  Boolean stop, found;
  Err err;

  buttonID = 0;

  if (formP) {
    previous = FrmGetActiveForm();
    FrmSetActiveForm(formP);

    if (formP->handler || formP->m68k_handler) {
      MemSet(&event, sizeof(EventType), 0);
      event.eType = frmOpenEvent;
      event.data.frmOpen.formID = formP->formId;
      if (!FrmDispatchEvent(&event)) {
        FrmDrawForm(formP);
      }
    } else {
      FrmDrawForm(formP);
    }

    for (index = 0, found = false; index < formP->numObjects; index++) {
      if (formP->objects[index].objectType == frmFieldObj) {
        obj = formP->objects[index].object;
        if (obj.field->attr.hasFocus) {
          FldSetActiveField(obj.field);
          found = true;
          break;
        }
      }
    }

    if (!found) {
      FldSetActiveField(NULL);
    }

    for (stop = false; !stop;) {
      EvtGetEvent(&event, timeout);
      if (SysHandleEvent(&event)) continue;
      if (MenuHandleEvent(NULL, &event, &err)) continue;

      if (!FrmDispatchEvent(&event)) {
        switch (event.eType) {
          case ctlSelectEvent:
            if (event.data.ctlSelect.pControl->style == buttonCtl) {
              buttonID = event.data.ctlSelect.controlID;
              if (fieldID && callback) {
                // the dialog closes only if the callback returns true
                index = FrmGetObjectIndex(formP, fieldID);
                fld = index != 0xffff ? (FieldType *)FrmGetObjectPtr(formP, index) : NULL;
                stop = fld ? callback(buttonID, FldGetTextPtr(fld)) : true;
              } else {
                stop = true;
              }
            }
            break;
          case appStopEvent:
            stop = true;
            break;
          default:
            break;
        }
      }
    }

    FldSetActiveField(NULL);
    FrmEraseForm(formP);
    MenuSetActiveMenu(NULL);
    FrmSetActiveForm(previous);

    // changed from EvtEmptyQueue() to EvtFlushPenQueue() here, otherwise apps may hang
    // (for example: FreeJong)
    EvtFlushPenQueue();
  }

  return buttonID;
}

UInt16 FrmDoDialogEx(FormType *formP, Int32 timeout) {
  return FrmDoDialogResponse(formP, timeout, 0, NULL);
}

UInt16 FrmDoDialog(FormType *formP) {
  return FrmDoDialogEx(formP, evtWaitForever);
}

/*
  UInt16    alertType;
  UInt16    helpRscID;
  UInt16    numButtons;
  UInt16    defaultButton;
  Char      *title;
  Char      *message;
  Char      *button[8];
*/
/*
  10004: information
  10005: question mark
  10006: warning
  10007: stop
*/
static UInt16 FrmShowAlert(UInt16 id, AlertTemplateType *alert, char *msg, char *entry, Int16 entryLen, FormCheckResponseFuncPtr callback) {
  FormType *formP;
  FieldType *fld;
  RectangleType rect;
  UInt16 bitmapID, len, tw, th, x;
  UInt16 labelX, labelY, labelH, labelW, formH, totalLines;
  UInt16 bmpX, bmpY, fieldX, fieldY;
  BitmapType *bitmapP;
  Coord bmpW, bmpH;
  MemHandle h;
  FontID old;
  char *s;
  int i, r = 0;

  if (alert && msg) {
    debug(DEBUG_TRACE, "Form", "Alert %d \"%s\"", id, msg);
    switch (alert->alertType) {
      case informationAlert:  bitmapID = 10004; break;
      case confirmationAlert: bitmapID = 10005; break;
      case warningAlert:      bitmapID = 10006; break;
      case errorAlert:        bitmapID = 10007; break;
      default: bitmapID = 0; break;
    }

    old = FntSetFont(boldFont);
    labelX = 32;
    labelY = 20;

    labelW = 156 - labelX;
    RctSetRectangle(&rect, labelX, labelY, labelW, FntCharHeight()*5);
    if (msg && msg[0]) {
      WinDrawCharBox(msg, StrLen(msg), boldFont, &rect, false, &totalLines, NULL, NULL, NULL, 0);
    } else {
      totalLines = 1;
    }
    labelH = FntCharHeight() * totalLines;
    formH = 46 + labelH;
    if (entry && entryLen > 0) {
      formH += FntCharHeight() + 8;
    }

    if ((formP = FrmNewForm(1000, alert->title, 2, 158-formH, 156, formH, true, 1000+alert->defaultButton, alert->helpRscID, 0)) != NULL) {
      // bitmap
      bmpX = 8;
      bmpY = 20;

      if (bitmapID) {
        FrmNewBitmap(&formP, bitmapID, bitmapID, bmpX, bmpY);
      }

      // label
      if (msg && msg[0]) {
        FrmNewLabel(&formP, 2000, msg, labelX, labelY, boldFont);
      }

      FntSetFont(stdFont);

      if (entry && entryLen > 0) {
        // field

        bmpH = 0;
        if (bitmapID) {
          if ((h = DmGetResource(bitmapRsc, bitmapID)) != NULL) {
            if ((bitmapP = MemHandleLock(h)) != NULL) {
              BmpGetDimensions(bitmapP, &bmpW, &bmpH, NULL);
              MemHandleUnlock(h);
            }
            DmReleaseResource(h);
          }
        }

        if (bmpH > 0) {
          fieldY = bmpY + bmpH + 6; 
        } else {
          fieldY = labelY + labelH + 2; 
        }
        fieldX = 8;

        fld = FldNewField((void **)&formP, 3000, fieldX, fieldY,
          140, FntCharHeight(), stdFont, entryLen, true, true,
          true, false, leftAlign, false, false, false);
      } else {
        fld = NULL;
      }

      // buttons
      th = FntCharHeight();
      for (i = 0, x = 6; i < alert->numButtons; i++) {
        len = StrLen(alert->button[i]);
        tw = FntCharsWidth(alert->button[i], len) + 10;
        if (tw < 36) tw = 36;
        CtlNewControl((void **)&formP, 1000+i, buttonCtl, alert->button[i], x, formH - th - 6, tw, th, stdFont, 0, true);
        x += tw + 6;
      }

      if (fld) {
        FldGrabFocus(fld);
        r = FrmDoDialogResponse(formP, evtWaitForever, 3000, callback);
        if ((s = FldGetTextPtr(fld)) != NULL) {
          StrNCopy(entry, s, entryLen);
        }
      } else {
        r = FrmDoDialog(formP);
      }

      FrmDeleteForm(formP);
      r -= 1000;
    }

    FntSetFont(old);
  }

  return r;
}

UInt16 FrmAlert(UInt16 alertId) {
  AlertTemplateType *alert;
  MemHandle h;
  UInt16 r = 0;

  if ((h = DmGetResource(alertRscType, alertId)) != NULL) {
    if ((alert = MemHandleLock(h)) != NULL) {
      r = FrmShowAlert(alertId, alert, alert->message, NULL, 0, NULL);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return r;
}

UInt16 FrmCustomAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3) {
  AlertTemplateType *alert;
  MemHandle h, t;
  char *s;
  UInt16 r = 0;

  if ((h = DmGetResource(alertRscType, alertId)) != NULL) {
    if ((alert = MemHandleLock(h)) != NULL) {
      if (alert->message != NULL) {
        if ((s = TxtParamString(alert->message, NULL, s1, s2, s3)) != NULL) {
          r = FrmShowAlert(alertId, alert, s, NULL, 0, NULL);
          t = MemPtrRecoverHandle(s);
          MemHandleUnlock(t);
          MemHandleFree(t);
        }
      }
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return r;
}

/*
The dialog it displays contains a text field for user entry.
The text that the user enters is returned in the entryStringBuf parameter.
When the user taps a button, the callback function is called
and is passed the button number and entryStringBuf.
The dialog is only dismissed if the callback returns true.
This behavior allows you to perform error checking on the
string that the user entered and give the user a chance to reenter the string.
The callback function is also called with special constants
when the alert dialog is being initialized and when it is being
deallocated. This allows the callback to perform any
necessary initialization and cleanup.
*/
UInt16 FrmCustomResponseAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3, Char *entryStringBuf, Int16 entryStringBufLength, FormCheckResponseFuncPtr callback) {
  AlertTemplateType *alert;
  MemHandle h, t;
  char *s;
  UInt16 r = 0;
 
  if (entryStringBuf && entryStringBufLength > 0) {
    if ((h = DmGetResource(alertRscType, alertId)) != NULL) {
      if ((alert = MemHandleLock(h)) != NULL) {
        if ((s = TxtParamString(alert->message, NULL, s1, s2, s3)) != NULL) {
          if (callback) callback(frmResponseCreate, NULL);
          r = FrmShowAlert(alertId, alert, s, entryStringBuf, entryStringBufLength, callback);
          if (callback) callback(frmResponseQuit, NULL);
          t = MemPtrRecoverHandle(s);
          MemHandleUnlock(t);
          MemHandleFree(t);
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }

  return r;
}

static void frmHelpPage(Boolean up) {
  FormType *formP;
  FieldType *fldP;
  UInt16 index, n;

  formP = FrmGetActiveForm();
  index = FrmGetObjectIndex(formP, 10402);
  fldP = (FieldType *)FrmGetObjectPtr(formP, index);
  n = FldGetVisibleLines(fldP);
  FldScrollField(fldP, n, up ? winUp : winDown);
}

static Boolean frmHelpEventHandler(EventType *eventP) {
  Boolean handled = false;

  switch (eventP->eType) {
    case ctlSelectEvent:
    case ctlRepeatEvent:
      switch (eventP->data.ctlSelect.controlID) {
        case 10404: // up arrow
          frmHelpPage(true);
          handled = true;
          break;
        case 10405: // down arrow
          frmHelpPage(false);
          handled = true;
          break;
      }
      break;
    default:
      break;
  }

  return handled;
}

void FrmHelp(UInt16 helpMsgId) {
  FormType *formP, *previous;
  FieldType *fldP;
  FieldAttrType attr;
  MemHandle h;
  Boolean old;
  UInt16 index;

  if ((h = DmGetResource(strRsc, helpMsgId)) != NULL) {
    if ((formP = FrmInitForm(10400)) != NULL) {
      if ((index = FrmGetObjectIndex(formP, 10402)) != frmInvalidObjectId) {
        if (formP->objects[index].objectType == frmFieldObj) {
          fldP = formP->objects[index].object.field;
          FldGetAttributes(fldP, &attr);
          old = attr.editable;
          attr.editable = true;
          FldSetAttributes(fldP, &attr);
          FldSetTextHandle(fldP, h);
          attr.editable = old;
          FldSetAttributes(fldP, &attr);
          previous = FrmGetActiveForm();

          if (!FldScrollable(fldP, winUp)) {
            index = FrmGetObjectIndex(formP, 10404);
            FrmHideObject(formP, index);
          }
          if (!FldScrollable(fldP, winDown)) {
            index = FrmGetObjectIndex(formP, 10405);
            FrmHideObject(formP, index);
          }

          FrmSetEventHandler(formP, frmHelpEventHandler);
          FrmDoDialog(formP);

          attr.editable = true;
          FldSetAttributes(fldP, &attr);
          FldSetTextHandle(fldP, NULL);
          attr.editable = old;
          FldSetAttributes(fldP, &attr);
          FrmDeleteForm(formP);
          FrmSetActiveForm(previous);
        }
      }
    }
    DmReleaseResource(h);
  }
}

Boolean FrmVisible(const FormType *formP) {
  return formP ? formP->attr.visible : false;
}

void FrmPopupForm(UInt16 formId) {
  EventType event;

  debug(DEBUG_TRACE, "Form", "FrmPopupForm %d", formId);
  InsPtEnable(false);

  xmemset(&event, 0, sizeof(EventType));
  event.eType = frmLoadEvent;
  event.data.frmLoad.formID = formId;
  EvtAddEventToQueue(&event);

  xmemset(&event, 0, sizeof(EventType));
  event.eType = frmOpenEvent;
  event.data.frmOpen.formID = formId;
  EvtAddEventToQueue(&event);
}

void FrmReturnToForm(UInt16 formId) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  FormList *p;

  // Erase and delete the currently active form and make the specified form the active form.

  // It is assumed that the form being returned to is already loaded into
  // memory and initialized. Passing a form ID of 0 returns to the first
  // form in the window list, which is the last form to be loaded.

  // FrmReturnToForm does not generate a frmCloseEvent when
  // called from a modal form's event handler. It assumes that you have
  // already handled cleaning up your form's variables since you are
  // explicitly calling FrmReturnToForm.

  debug(DEBUG_TRACE, "Form", "FrmReturnToForm %d", formId);

  if (module->currentForm) {
    FrmEraseForm(module->currentForm);
    FrmDeleteForm(module->currentForm);
    module->currentForm = NULL;
    MenuSetActiveMenu(NULL);
  }

  for (p = module->list; p; p = p->next) {
    if (formId == 0 || p->formP->formId == formId) {
      FrmSetActiveForm(p->formP);
      break;
    }
  }
}

void FrmUpdateForm(UInt16 formId, UInt16 updateCode) {
  EventType event;

  debug(DEBUG_TRACE, "Form", "FrmUpdateForm %d code %u", formId, updateCode);
  MemSet(&event, sizeof(EventType), 0);
  event.eType = frmUpdateEvent;
  event.data.frmUpdate.formID = formId;
  event.data.frmUpdate.updateCode = updateCode;
  EvtAddEventToQueue(&event);
}

void FrmCloseAllForms(void) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  FormList *p, *q;
  EventType event;

  debug(DEBUG_TRACE, "Form", "FrmCloseAllForms");
  WinEraseWindow();

  for (p = module->list; p;) {
    debug(DEBUG_TRACE, "Form", "FrmCloseAllForms form %d", p->formP->formId);
    q = p->next;

    xmemset(&event, 0, sizeof(EventType));
    event.eType = frmCloseEvent;
    event.data.frmClose.formID = p->formP->formId;
    if (!FrmDispatchEventInternal(p->formP, &event)) {
      FrmDeleteFormInternal(p->formP);
      xfree(p);
    }

    p = q;
  }
  module->currentForm = NULL;
  MenuSetActiveMenu(NULL);
  module->list = NULL;

  WinSetActiveWindow(WinGetDisplayWindow());
  WinSetDrawWindow(WinGetDisplayWindow());
}

// Send a frmSaveEvent to all open forms.
// XXX how can I send the event to all forms if the event has no data ?
void FrmSaveAllForms(void) {
  EventType event;

  debug(DEBUG_TRACE, "Form", "FrmSaveAllForms");
  MemSet(&event, sizeof(EventType), 0);
  event.eType = frmSaveEvent;
  EvtAddEventToQueue(&event);
}

Boolean FrmPointInTitle(const FormType *formP, Coord x, Coord y) {
  UInt16 index;
  Boolean r = false;

  if (formP) {
    for (index = 0; index < formP->numObjects; index++) {
      if (formP->objects[index].objectType == frmTitleObj) {
        r = RctPtInRectangle(x, y, &formP->objects[index].object.title->rect);
        break;
      }
    }
  }

  return r;
}

void FrmSetMenu(FormType *formP, UInt16 menuRscID) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);

  if (formP) {
    if (formP->mbar) {
      MenuDispose(formP->mbar);
    }
    formP->mbar = MenuInit(menuRscID);
    if (formP == module->currentForm) {
      MenuSetActiveMenu(formP->mbar);
    }
  }
}

Boolean FrmValidatePtr(const FormType *formP) {
  // system use only
  debug(DEBUG_ERROR, "Form", "FrmValidatePtr not implemented");
  return 0;
}

Err FrmRemoveObject(FormType **formPP, UInt16 objIndex) {
  debug(DEBUG_ERROR, "Form", "FrmRemoveObject not implemented");
  return 0;
}

typedef struct {
  FormType *current;
} FormActiveStateTypeEx;

Err FrmActiveState(FormActiveStateType *stateP, Boolean save) {
  FormActiveStateTypeEx *state = (FormActiveStateTypeEx *)stateP;

  if (save) {
    state->current = FrmGetActiveForm();
  } else {
    FrmSetActiveForm(state->current);
  }
  
  return errNone;
}

FrmGraffitiStateType *FrmNewGsi(FormType **formPP, Coord x, Coord y) {
  debug(DEBUG_ERROR, "Form", "FrmNewGsi not implemented");
  return 0;
}

UInt16 FrmGetDIAPolicyAttr(FormPtr formP) {
  return formP ? formP->diaPolicy : 0;
}

Err FrmSetDIAPolicyAttr(FormPtr formP, UInt16 diaPolicy) {
  Err err = sysErrParamErr;

  // With Pen Input Manager version 1.1, if you set an input area policy of frmDIAPolicyCustom
  // but don't call PINSetInputTriggerState to enable or disable the trigger or call PINSetInputAreaState
  // to open or close the input area, then the system automatically restores the last user-selected input
  // area state and enables the trigger (1.0 doesn't do this).
  // If PINSetInputAreaState and/or PINSetInputTriggerState is called by the application, however,
  // then the form's resulting state is restored when the form is updated due to a call to FrmDrawForm or a WinEnterEvent.

  if (formP) {
    switch (diaPolicy) {
      case frmDIAPolicyStayOpen:
      case frmDIAPolicyCustom:
        formP->diaPolicy = diaPolicy;
        err = errNone;
        break;
      default:
        debug(DEBUG_ERROR, "Form", "FrmSetDIAPolicyAttr invalid policy %d", diaPolicy);
        break;
    }
  }

  return err;
}

const Char *FrmGetLabel(const FormType *formP, UInt16 labelID) {
  FormLabelType *label;
  UInt16 index;
  Char *s = NULL;

  if (formP) {
    if ((index = FrmGetObjectIndex(formP, labelID)) != frmInvalidObjectId) {
      if (formP->objects[index].objectType == frmLabelObj) {
        label = formP->objects[index].object.label;
        s = label->text;
      }
    }
  }

  return s;
}

// This function redraws the label if the form's usable attribute and the label's visible attribute are set.
// This function redraws the label but does not erase the old one first.
// If the new label is shorter than the old one, the end of the old label will still be visible.

void FrmCopyLabel(FormType *formP, UInt16 labelID, const Char *newLabel) {
  FormLabelType *label;
  ControlType *control;
  UInt16 index;

  if (formP && newLabel) {
    if ((index = FrmGetObjectIndex(formP, labelID)) != frmInvalidObjectId) {
      if (formP->objects[index].objectType == frmLabelObj) {
        label = formP->objects[index].object.label;
        if (label->text) {
          StrNCopy(label->text, newLabel, label->len);
          // XXX there is no visible attribute on the Label object!
          if (formP->attr.usable && formP->attr.visible) {
            FrmDrawObject(formP, index, false);
          }
        }
      } else if (formP->objects[index].objectType == frmControlObj) {
        // XXX Napalm Racing uses FrmCopyLabel to set the label of a Control, which is not right.
        // I hope this StrCopy does not have any bad side effects.
        control = formP->objects[index].object.control;
        StrCopy(control->text, newLabel);
      }
    }
  }
}

void FrmSetControlValue(const FormType *formP, UInt16 objIndex, Int16 newValue) {
  if (formP && objIndex < formP->numObjects) {
    switch (formP->objects[objIndex].objectType) {
      case frmControlObj:
        CtlSetValue(formP->objects[objIndex].object.control, newValue ? 1 : 0);
        break;
      default:
        break;
    }
  }
}

UInt16 FrmGetControlGroupSelection(const FormType *formP, UInt8 groupNum) {
  UInt16 objIndex = 0;

  if (formP) {
    for (objIndex = 0; objIndex < formP->numObjects; objIndex++) {
      if (formP->objects[objIndex].objectType == frmControlObj &&
          formP->objects[objIndex].object.control->group == groupNum &&
          formP->objects[objIndex].object.control->attr.on) {
         break;
      }
    }
  }

  return objIndex;
}

void FrmSetControlGroupSelection(const FormType *formP, UInt8 groupNum, UInt16 controlID) {
  ControlType *control;
  UInt16 index;

  if (formP) {
    index = FrmGetObjectIndex(formP, controlID);
    if (index != frmInvalidObjectId) {
      if (FrmGetObjectType(formP, index) == frmControlObj) {
        control = (ControlType *)FrmGetObjectPtr(formP, index);
        if (control && control->group == groupNum) {
          if (control->style == pushButtonCtl) {
            CtlUpdateGroup(control, true);
          } else if (control->style == checkboxCtl) {
            CtlUpdateCheckboxGroup(control, true);
          }
        }
      }
    }
  }
}

// Set the category label displayed on the title line of a form.
// If the form's visible attribute is set, redraw the label.

void FrmSetCategoryLabel(const FormType *formP, UInt16 objIndex, Char *newLabel) {
  FormObjectKind kind;
  ControlType *control;

  if (formP && newLabel) {
    if ((kind = FrmGetObjectType(formP, objIndex)) == frmControlObj) {
      if ((control = (ControlType *)FrmGetObjectPtr(formP, objIndex)) != NULL) {
        CtlSetLabel(control, newLabel);
      }
    }
  }
}

void FrmSetGadgetHandler(FormType *formP, UInt16 objIndex, FormGadgetHandlerType *attrP) {
  if (formP && objIndex < formP->numObjects && formP->objects[objIndex].objectType == frmGadgetObj) {
    formP->objects[objIndex].object.gadget->handler = attrP;
  }
}
 
void *FrmGetGadgetData(const FormType *formP, UInt16 objIndex) {
  return (formP && objIndex < formP->numObjects && formP->objects[objIndex].objectType == frmGadgetObj) ? (void *)formP->objects[objIndex].object.gadget->data : NULL;
}

void FrmSetGadgetData(FormType *formP, UInt16 objIndex, const void *data) {
  if (formP && objIndex < formP->numObjects && formP->objects[objIndex].objectType == frmGadgetObj) {
    formP->objects[objIndex].object.gadget->data = data;
  }
}

// Set the bounds or position of an object. Window-relative bounds.
// For the following objects, this sets only the position of the top-left corner: label, bitmap, and Graffiti state indicator.

void FrmSetObjectBounds(FormType *formP, UInt16 objIndex, const RectangleType *bounds) {
  ListType *lst;
  FontID old;
  Err err;

  if (formP && objIndex < formP->numObjects) {
    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:
        MemMove(&formP->objects[objIndex].object.field->rect, bounds, sizeof(RectangleType));
        break;
      case frmControlObj:
        MemMove(&formP->objects[objIndex].object.control->bounds, bounds, sizeof(RectangleType));
        break;
      case frmLabelObj:
        formP->objects[objIndex].object.label->pos.x = bounds->topLeft.x;
        formP->objects[objIndex].object.label->pos.y = bounds->topLeft.y;
        break;
      case frmListObj:
        debug(DEBUG_TRACE, "Form", "FrmSetObjectBounds list %d (%d,%d,%d,%d)",
          formP->objects[objIndex].object.list->id, bounds->topLeft.x, bounds->topLeft.y, bounds->extent.x, bounds->extent.y);
        MemMove(&formP->objects[objIndex].object.list->bounds, bounds, sizeof(RectangleType));
        WinDeleteWindow(formP->objects[objIndex].object.list->popupWin, true);
        formP->objects[objIndex].object.list->popupWin = WinCreateOffscreenWindow(bounds->extent.x, bounds->extent.y, nativeFormat, &err);
        formP->objects[objIndex].object.list->popupWin->windowBounds.topLeft.x = formP->window.windowBounds.topLeft.x + bounds->topLeft.x;
        formP->objects[objIndex].object.list->popupWin->windowBounds.topLeft.y = formP->window.windowBounds.topLeft.y + bounds->topLeft.y;

        lst = (ListType *)FrmGetObjectPtr(formP, objIndex);
        old = FntSetFont(lst->font);
        LstSetHeight(lst, bounds->extent.y / FntCharHeight());
        FntSetFont(old);
        break;
      case frmGadgetObj:
        MemMove(&formP->objects[objIndex].object.gadget->rect, bounds, sizeof(RectangleType));
        break;
      case frmTableObj:
        MemMove(&formP->objects[objIndex].object.table->bounds, bounds, sizeof(RectangleType));
        break;
      case frmBitmapObj:
        formP->objects[objIndex].object.bitmap->pos.x = bounds->topLeft.x;
        formP->objects[objIndex].object.bitmap->pos.y = bounds->topLeft.y;
        break;
      case frmScrollBarObj:
        MemMove(&formP->objects[objIndex].object.scrollBar->bounds, bounds, sizeof(RectangleType));
        break;
      case frmTitleObj:
        debug(DEBUG_TRACE, "Form", "FrmSetObjectBounds title (%d,%d,%d,%d)",
          bounds->topLeft.x, bounds->topLeft.y, bounds->extent.x, bounds->extent.y);
        MemMove(&formP->objects[objIndex].object.title->rect, bounds, sizeof(RectangleType));
        break;
      case frmGraffitiStateObj:
        formP->objects[objIndex].object.grfState->pos.x = bounds->topLeft.x;
        formP->objects[objIndex].object.grfState->pos.y = bounds->topLeft.y;
        break;
      default:
        debug(DEBUG_ERROR, "Form", "FrmSetObjectBounds object type %d not supported", formP->objects[objIndex].objectType);
        break;
    }
  }
}

void FrmGetObjectBounds(const FormType *formP, UInt16 objIndex, RectangleType *rP) {
  MemHandle h;
  BitmapPtr bitmapP;
  FontID old;
  Coord width, height;

  if (objIndex < formP->numObjects) {
    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:
        MemMove(rP, &formP->objects[objIndex].object.field->rect, sizeof(RectangleType));
        break;
      case frmControlObj:
        MemMove(rP, &formP->objects[objIndex].object.control->bounds, sizeof(RectangleType));
        break;
      case frmLabelObj:
        rP->topLeft.x = formP->objects[objIndex].object.label->pos.x;
        rP->topLeft.y = formP->objects[objIndex].object.label->pos.y;
        rP->extent.x = formP->objects[objIndex].object.label->extent.x;
        rP->extent.y = formP->objects[objIndex].object.label->extent.y;
        break;
      case frmListObj:
        MemMove(rP, &formP->objects[objIndex].object.list->bounds, sizeof(RectangleType));
        break;
      case frmGadgetObj:
        MemMove(rP, &formP->objects[objIndex].object.gadget->rect, sizeof(RectangleType));
        break;
      case frmTableObj:
        MemMove(rP, &formP->objects[objIndex].object.table->bounds, sizeof(RectangleType));
        break;
      case frmBitmapObj:
        if ((h = DmGetResource(bitmapRsc, formP->objects[objIndex].object.bitmap->rscID)) != NULL) {
          if ((bitmapP = MemHandleLock(h)) != NULL) {
            BmpGetDimensions(bitmapP, &width, &height, NULL);
            rP->topLeft.x = formP->objects[objIndex].object.bitmap->pos.x;
            rP->topLeft.y = formP->objects[objIndex].object.bitmap->pos.y;
            rP->extent.x = width;
            rP->extent.y = height;
            MemHandleUnlock(h);
          }
          DmReleaseResource(h);
        }
        break;
      case frmScrollBarObj:
        MemMove(rP, &formP->objects[objIndex].object.scrollBar->bounds, sizeof(RectangleType));
        break;
      case frmTitleObj:
        MemMove(rP, &formP->objects[objIndex].object.title->rect, sizeof(RectangleType));
        break;
      case frmGraffitiStateObj:
        old = FntSetFont(symbolFont);
        rP->topLeft.x = formP->objects[objIndex].object.grfState->pos.x;
        rP->topLeft.y = formP->objects[objIndex].object.grfState->pos.y;
        rP->extent.x = FntCharWidth(13);
        rP->extent.y = FntCharHeight();
        FntSetFont(old);
        break;
      default:
        debug(DEBUG_ERROR, "Form", "FrmGetObjectBounds object type %d not supported", formP->objects[objIndex].objectType);
        return;
    }
    WinScaleRectangle(rP);
  }
}

void FrmGetObjectPosition(const FormType *formP, UInt16 objIndex, Coord *x, Coord *y) {
  RectangleType rect;

  FrmGetObjectBounds(formP, objIndex, &rect);
  if (x) *x = rect.topLeft.x;
  if (y) *y = rect.topLeft.y;
}

Int16 FrmObjectRightAlign(FormType *formP, UInt16 objIndex, Int16 d) {
  RectangleType rect;
  UInt32 swidth;

  if (objIndex < formP->numObjects) {
    FrmGetObjectBounds(formP, objIndex, &rect);

    if (d < 0) {
      WinScreenMode(winScreenModeGet, &swidth, NULL, NULL, NULL);
      d = swidth - 160;
    }

    rect.topLeft.x += d;
    FrmSetObjectBounds(formP, objIndex, &rect);
  }

  return d;
}

Int16 FrmObjectBottomAlign(FormType *formP, UInt16 objIndex, Int16 d) {
  RectangleType rect;
  UInt32 sheight;

  if (objIndex < formP->numObjects) {
    FrmGetObjectBounds(formP, objIndex, &rect);

    if (d < 0) {
      WinScreenMode(winScreenModeGet, NULL, &sheight, NULL, NULL);
      d = sheight - 160;
    }

    rect.topLeft.y += d;
    FrmSetObjectBounds(formP, objIndex, &rect);
  }

  return d;
}

void FrmUpdateScrollers(FormType *formP, UInt16 upIndex, UInt16 downIndex, Boolean scrollableUp, Boolean scrollableDown) {
  if (formP && upIndex < formP->numObjects && downIndex < formP->numObjects) {
    if (scrollableUp) {
      FrmShowObject(formP, upIndex);
    } else {
      FrmHideObject(formP, upIndex);
    }
    if (scrollableDown) {
      FrmShowObject(formP, downIndex);
    } else {
      FrmHideObject(formP, downIndex);
    }
  }
}

void FrmSetObjectPosition(FormType *formP, UInt16 objIndex, Coord x, Coord y) {
  RectangleType rect;

  FrmGetObjectBounds(formP, objIndex, &rect);
  rect.topLeft.x = x;
  rect.topLeft.y = y;
  FrmSetObjectBounds(formP, objIndex, &rect);
}

Err FrmAddSpaceForObject(FormType **formPP, MemPtr *objectPP, FormObjectKind objectKind, UInt16 objectSize) {
  // system use only
  debug(DEBUG_ERROR, "Form", "FrmAddSpaceForObject not implemented");
  return sysErrParamErr;
}

FormType *FrmNewForm(UInt16 formID, const Char *titleStrP, Coord x, Coord y, Coord width, Coord height, Boolean modal, UInt16 defaultButton, UInt16 helpRscID, UInt16 menuRscID) {
  frm_module_t *module = (frm_module_t *)pumpkin_get_local_storage(frm_key);
  FormTitleType *title;
  FormType *formP;
  int len;

  if ((formP = pumpkin_heap_alloc(sizeof(FormType), "Form")) != NULL) {
    formP->formId = formID;
    formP->attr.usable = true;
    formP->attr.enabled = true;
    formP->attr.saveBehind = true;
    formP->defaultButton = defaultButton;
    formP->helpRscId = helpRscID;
    formP->menuRscId = menuRscID;
    formP->numObjects = 0;
    formP->diaPolicy = frmDIAPolicyStayOpen;

    formP->window.windowFlags.modal = modal;
    formP->window.windowFlags.enabled = true;

    formP->window.windowBounds.topLeft.x = x;
    formP->window.windowBounds.topLeft.y = y;
    formP->window.windowBounds.extent.x = width;
    formP->window.windowBounds.extent.y = height;

    FrmInitFormInternal(formP);

    if (formP->window.windowFlags.modal && module->centerDialogs) {
      FrmCenterForm(formP);
    }

    if (titleStrP) {
      if ((title = pumpkin_heap_alloc(sizeof(FormTitleType), "Title")) != NULL) {
        len = StrLen(titleStrP);
        title->text = MemPtrNew(len + 1);
        StrNCopy(title->text, titleStrP, len);
        title->objIndex = 0;
        formP->objects = xcalloc(1, sizeof(FormObjListType));
        formP->objects[0].objectType = frmTitleObj;
        formP->objects[0].object.title = title;
        formP->numObjects++;
      }
    }
  }

  return formP;
}

FormBitmapType *FrmNewBitmap(FormType **formPP, UInt16 ID, UInt16 rscID, Coord x, Coord y) {
  FormBitmapType *formBitmapP = NULL;
  FormType *formP;

  if (formPP) {
    formP = *formPP;

    if (formP) {
      if ((formBitmapP = pumpkin_heap_alloc(sizeof(FormBitmapType), "FormBitmap")) != NULL) {
        formBitmapP->attr.usable = true;
        formBitmapP->pos.x = x;
        formBitmapP->pos.y = y;
        formBitmapP->rscID = rscID;

        if (formP->numObjects == 0) {
          formP->objects = xcalloc(1, sizeof(FormObjListType));
        } else {
          formP->objects = xrealloc(formP->objects, (formP->numObjects + 1) * sizeof(FormObjListType));
        }
        formP->objects[formP->numObjects].objectType = frmBitmapObj;
        formP->objects[formP->numObjects].object.bitmap = formBitmapP;
        formP->objects[formP->numObjects].id = ID;
        formP->numObjects++;
      }
    }
  }

  return formBitmapP;
}

FormLabelType *FrmNewLabel(FormType **formPP, UInt16 id, const Char *textP, Coord x, Coord y, FontID font) {
  FormLabelType *labelP = NULL;
  FormType *formP;

  if (formPP && textP) {
    formP = *formPP;

    if (formP) {
      if ((labelP = pumpkin_heap_alloc(sizeof(FormLabelType) + StrLen(textP) + 1, "Label")) != NULL) {
        labelP->id = id;
        labelP->pos.x = x;
        labelP->pos.y = y;
        labelP->attr.usable = true;
        labelP->fontID = font;
        labelP->marker = 0x5A5A;
        labelP->text = &labelP->buf[0];
        labelP->len = StrLen((char *)textP) + 1;
        StrCopy(labelP->text, textP);

        if (formP->numObjects == 0) {
          formP->objects = xcalloc(1, sizeof(FormObjListType));
        } else {
          formP->objects = xrealloc(formP->objects, (formP->numObjects + 1) * sizeof(FormObjListType));
        }
        formP->objects[formP->numObjects].objectType = frmLabelObj;
        formP->objects[formP->numObjects].object.label = labelP;
        formP->objects[formP->numObjects].id = formP->objects[formP->numObjects].object.label->id;
        formP->numObjects++;
      }
    }
  }

  return labelP;
}

FormGadgetType *FrmNewGadget(FormType **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height) {
  FormGadgetType *gadgetP = NULL;
  FormType *formP;

  if (formPP) {
    formP = *formPP;

    if (formP) {
      if ((gadgetP = pumpkin_heap_alloc(sizeof(FormGadgetType), "Gadget")) != NULL) {
        gadgetP->id = id;
        gadgetP->rect.topLeft.x = x;
        gadgetP->rect.topLeft.y = y;
        gadgetP->rect.extent.x = width;
        gadgetP->rect.extent.y = height;
        gadgetP->attr.usable = true;

        if (formP->numObjects == 0) {
          formP->objects = xcalloc(1, sizeof(FormObjListType));
        } else {
          formP->objects = xrealloc(formP->objects, (formP->numObjects + 1) * sizeof(FormObjListType));
        }
        formP->objects[formP->numObjects].objectType = frmGadgetObj;
        formP->objects[formP->numObjects].object.gadget = gadgetP;
        formP->objects[formP->numObjects].id = formP->objects[formP->numObjects].object.gadget->id;
        formP->numObjects++;
      }
    }
  }
                                                                                                                                                                                              
  return gadgetP;
}

static int palign(int a, int i) {
  int r = i % a;
  return r ? (a - r) : 0;
}

static FieldType *pumpkin_create_field(uint8_t *p, int *i) {
  FieldType *c = NULL;
  uint8_t dummy8, font;
  uint16_t dummy16, attr, id, x, y, w, h, max;
  uint32_t dummy32;

  // szRCFieldBA16 "w,w4,uuuuuuuu,u2u2uuuzu,p,zl,zl,zw,zw,w,zw,zw,zw,zw,b,zb"
  *i += get2b(&id, p, *i);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  *i += get2b(&attr, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&max, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get1(&font, p, *i);
  *i += get1(&dummy8, p, *i);
  debug(DEBUG_TRACE, "Form",  "field id %d max %d font %d at (%d,%d,%d,%d)", id, max, font, x, y, w, h);

  if ((c = pumpkin_heap_alloc(sizeof(FieldType), "Field")) != NULL) {
    c->id = id;
    c->rect.topLeft.x = x;
    c->rect.topLeft.y = y;
    c->rect.extent.x = w;
    c->rect.extent.y = h;
    c->attr.usable        = (attr & 0x8000) ? 1 : 0;
    c->attr.visible       = (attr & 0x4000) ? 1 : 0;
    c->attr.editable      = (attr & 0x2000) ? 1 : 0;
    c->attr.singleLine    = (attr & 0x1000) ? 1 : 0;
    c->attr.hasFocus      = (attr & 0x0800) ? 1 : 0;
    c->attr.dynamicSize   = (attr & 0x0400) ? 1 : 0;
    c->attr.insPtVisible  = (attr & 0x0200) ? 1 : 0;
    c->attr.dirty         = (attr & 0x0100) ? 1 : 0;
    c->attr.underlined    = (attr & 0x00C0) >> 6;
    c->attr.justification = (attr & 0x0030) >> 4;
    c->attr.autoShift     = (attr & 0x0008) ? 1 : 0;
    c->attr.hasScrollBar  = (attr & 0x0004) ? 1 : 0;
    c->attr.numeric       = (attr & 0x0002) ? 1 : 0;
    c->attr.reserved      = 0;
    c->maxChars = max;
    c->fontID = font;
  }

  return c;
}

static void fill_attr(uint16_t attr, ControlAttrType *ctlAttr) {
  ctlAttr->usable          = (attr & 0x8000) ? 1 : 0;
  ctlAttr->enabled         = (attr & 0x4000) ? 1 : 0;
  ctlAttr->visible         = (attr & 0x2000) ? 1 : 0;
  ctlAttr->on              = (attr & 0x1000) ? 1 : 0;
  ctlAttr->leftAnchor      = (attr & 0x0800) ? 1 : 0;
  ctlAttr->frame           = (attr & 0x0700) >> 8;
  ctlAttr->drawnAsSelected = (attr & 0x0080) ? 1 : 0;
  ctlAttr->graphical       = (attr & 0x0040) ? 1 : 0;
  ctlAttr->vertical        = (attr & 0x0020) ? 1 : 0;
  ctlAttr->reserved        = 0;
}

static ControlType *pumpkin_create_control(uint8_t *p, int *i) {
  ControlType *c = NULL;
  SliderControlType *sc = NULL;
  uint8_t dummy8, style, font, group;
  uint16_t attr, id, x, y, w, h, bitmapId, selBitmapId, len;
  uint16_t minValue, maxValue, pageSize, value;
  uint32_t dummy32;
  char *text;

  // szRCControlBA16          "w,w4,ssp,uuuuuu3,uuuzu5,b,b,b,zb"
  // szRCSliderControlBA16    "w,w4,w,w,uuuuuu3,uuuzu5,b,zb,w,w,w,w,zl"

  *i += get2b(&id, p, *i);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  *i += get2b(&bitmapId, p, *i);
  *i += get2b(&selBitmapId, p, *i);
  *i += get2b(&attr, p, *i);
  *i += get1(&style, p, *i);

  if (style == sliderCtl || style == feedbackSliderCtl) {
    *i += get1(&dummy8, p, *i);
    *i += get2b(&minValue, p, *i);
    *i += get2b(&maxValue, p, *i);
    *i += get2b(&pageSize, p, *i);
    *i += get2b(&value, p, *i);
    *i += get4b(&dummy32, p, *i);
  } else {
    *i += get1(&font, p, *i);
    *i += get1(&group, p, *i);
    *i += get1(&dummy8, p, *i);
  }

  if (style == sliderCtl || style == feedbackSliderCtl) {
    debug(DEBUG_TRACE, "Form",  "slider control id %d style %d attr 0x%04X bitmaps %d,%d min %d max %d page %d value %d at (%d,%d,%d,%d)",
      id, style, attr, bitmapId, selBitmapId, minValue, maxValue, pageSize, value, x, y, w, h);
    if ((sc = pumpkin_heap_alloc(sizeof(SliderControlType), "Control")) != NULL) {
      sc->id = id;
      sc->bounds.topLeft.x = x;
      sc->bounds.topLeft.y = y;
      sc->bounds.extent.x = w;
      sc->bounds.extent.y = h;
      sc->thumbID = bitmapId;
      sc->backgroundID = selBitmapId;
      if (sc->thumbID == 0) sc->thumbID = 13350;
      if (sc->backgroundID == 0) sc->backgroundID = 13351;
      fill_attr(attr, &sc->attr);
      sc->style = style;
      sc->minValue = minValue;
      sc->maxValue = maxValue;
      sc->pageSize = pageSize;
      sc->value = value;
      c = (ControlType *)sc;
    }
  } else if (attr & 0x0040) {
    debug(DEBUG_TRACE, "Form",  "graphical control id %d font %d style %d attr 0x%04X at (%d,%d,%d,%d)", id, font, style, attr, x, y, w, h);
    if ((c = pumpkin_heap_alloc(sizeof(GraphicControlType), "Control")) != NULL) {
      c->id = id;
      c->bounds.topLeft.x = x;
      c->bounds.topLeft.y = y;
      c->bounds.extent.x = w;
      c->bounds.extent.y = h;
      c->bitmapID = bitmapId;
      c->selectedBitmapID = selBitmapId;
      fill_attr(attr, &c->attr);
      c->style = style;
      c->group = group;
    }
  } else {
    *i += pumpkin_getstr(&text, p, *i);
    *i += palign(2, *i);
    len = sys_strlen(text);
    if ((c = pumpkin_heap_alloc(sizeof(ControlType) + len + 1, "Control")) != NULL) {
      c->id = id;
      c->bounds.topLeft.x = x;
      c->bounds.topLeft.y = y;
      c->bounds.extent.x = w;
      c->bounds.extent.y = h;
      fill_attr(attr, &c->attr);
      c->style = style;
      c->group = group;
      c->font = font;
      c->text = &c->buf[0];
      c->len = len;
      xmemcpy(c->text, text, len);
      if (StrCompare(c->text, "#000000") == 0) {
        c->style = colorTriggerCtl;
      }
      debug(DEBUG_TRACE, "Form", "control id %d font %d style %d attr 0x%04X text \"%s\" at (%d,%d,%d,%d)", id, font, style, attr, text ? text : "", x, y, w, h);
    }
  }

  put2b(id, (uint8_t *)c, 0);

  return c;
}

static ListType *pumpkin_create_list(uint8_t *p, int *i, FormType *form, uint32_t size) {
  ListType *c = NULL;
  uint8_t dummy8, font;
  uint16_t dummy16, attr, id, x, y, w, h, th, numItems;
  uint32_t dummy32;
  FontID old;
  char *text;
  Err err;
  int j;

  // szRCListBA16 "w,w4,uuuuuuzu10,p,w,zw,zw,bzb,zl,zl"
  *i += get2b(&id, p, *i);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  *i += get2b(&attr, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get2b(&numItems, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get1(&font, p, *i);
  *i += get1(&dummy8, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get4b(&dummy32, p, *i);
  debug(DEBUG_TRACE, "Form", "list id %d (%d,%d,%d,%d)", id, x, y, w, h);

  // the list border must be drawn outside the list area, so adjust position and dimensions
  if (x >= 1) x -= 1;
  w += 2;
  if (y >= 1) y -= 1;
  h += 2;

  for (j = 0; j < numItems; j++) {
    *i += get4b(&dummy32, p, *i);
  }

  if ((c = pumpkin_heap_alloc(sizeof(ListType) + numItems * sizeof(char *), "List")) != NULL) {
    old = FntSetFont(font);
    th = FntCharHeight();
    FntSetFont(old);
    if (h == 0) h = th;

    if (w > form->window.windowBounds.extent.x) {
      w = form->window.windowBounds.extent.x;
    }
    if (h > form->window.windowBounds.extent.y) {
      h = form->window.windowBounds.extent.y;
    }
    if (x + w > form->window.windowBounds.extent.x) {
      x = form->window.windowBounds.extent.x - w;
    }
    if (y + h > form->window.windowBounds.extent.y) {
      y = form->window.windowBounds.extent.y - h;
    }

    c->id = id;
    c->bounds.topLeft.x = x; // form relative coordinate
    c->bounds.topLeft.y = y; // form relative coordinate
    c->bounds.extent.x = w;
    c->bounds.extent.y = h;
    c->attr.usable       = (attr & 0x8000) ? 1 : 0;
    c->attr.enabled      = (attr & 0x4000) ? 1 : 0;
    c->attr.visible      = (attr & 0x2000) ? 1 : 0;
    c->attr.poppedUp     = (attr & 0x1000) ? 1 : 0;
    c->attr.hasScrollBar = (attr & 0x0800) ? 1 : 0;
    c->attr.search       = (attr & 0x0400) ? 1 : 0;
    c->attr.reserved     = 0;

    c->numItems = numItems;
    c->font = font;
    c->visibleItems = h / th;
    c->formP = form;
    debug(DEBUG_TRACE, "Form", "list id %d numItems %d visibleItems %d usable %d", c->id, c->numItems, c->visibleItems, c->attr.usable);

    c->popupWin = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
    c->popupWin->windowBounds.topLeft.x = form->window.windowBounds.topLeft.x + c->bounds.topLeft.x; // absolute screen coordinate
    c->popupWin->windowBounds.topLeft.y = form->window.windowBounds.topLeft.y + c->bounds.topLeft.y; // absolute screen coordinate

    if (!c->attr.usable) {
      c->bitsBehind = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
    }
    debug(DEBUG_TRACE, "Form", "list popupwin %d,%d,%d,%d",
      c->popupWin->windowBounds.topLeft.x, c->popupWin->windowBounds.topLeft.y, c->popupWin->windowBounds.extent.x, c->popupWin->windowBounds.extent.y);

    if (numItems > 0) {
      if (pumpkin_is_m68k()) {
        for (j = 0; j < numItems; j++) {
          debug(DEBUG_TRACE, "Form", "list item %d offset %d (m68k)", j, *i);
          *i += pumpkin_getstr(&text, p, *i);
          put4b((uint8_t *)text - emupalmos_ram(), (uint8_t *)c->aux, j*4);
          debug(DEBUG_TRACE, "Form", "list item %d \"%s\"", j, text);
        }
      } else {
        for (j = 0; j < numItems; j++) {
          debug(DEBUG_TRACE, "Form", "list item %d offset %d", j, *i);
          *i += pumpkin_getstr(&text, p, *i);
          c->aux[j] = text;
          debug(DEBUG_TRACE, "Form", "list item %d \"%s\"", j, text);
        }
      }
    }
    c->itemsText = c->aux;
    *i += palign(2, *i);
  }

  return c;
}

static int pumpkin_destroy_list(ListType *list) {
  if (list) {
    if (list->popupWin) WinDeleteWindow(list->popupWin, false);
    if (list->bitsBehind) WinDeleteWindow(list->bitsBehind, false);

    pumpkin_heap_free(list, "List");
  }

  return 0;
}

static TableType *pumpkin_create_table(uint8_t *p, int *i) {
  TableType *c = NULL;
  uint16_t dummy16, attr, id, x, y, w, h, cols, rows, col, row, top;
  uint16_t cw, cattr, cspacing;
  uint16_t rid, rh, rattr;
  uint32_t dummy32, rdata, p1, p2;
  int j;

  // szRCTableBA16 "w,w4,uuuuuzu11,w,w,w,w,w,zl,zl,zl"
  *i += get2b(&id, p, *i);
  debug(DEBUG_TRACE, "Form",  "table id %d", id);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  debug(DEBUG_TRACE, "Form",  "table bounds (%d,%d,%d,%d)", x, y, w, h);
  *i += get2b(&attr, p, *i);
  *i += get2b(&cols, p, *i);
  *i += get2b(&rows, p, *i);
  *i += get2b(&row, p, *i);
  *i += get2b(&col, p, *i);
  *i += get2b(&top, p, *i);
  debug(DEBUG_TRACE, "Form",  "table %dx%d current %d,%d top %d", cols, rows, col, row, top);
  *i += get4b(&dummy32, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get4b(&dummy32, p, *i);

  // szRCFieldBA16 "w,w4,uuuuuuuu,u2u2uuuzu,p,zl,zl,zw,zw,w,zw,zw,zw,zw,b,zb"
  for (j = 0; j < 20; j++) {
    *i += get2b(&dummy16, p, *i);
    debug(DEBUG_TRACE, "Form",  "table w%d %d", j, dummy16);
  }

  if ((c = pumpkin_heap_alloc(sizeof(TableType), "Table")) != NULL) {
    c->id = id;
    c->bounds.topLeft.x = x;
    c->bounds.topLeft.y = y;
    c->bounds.extent.x = w;
    c->bounds.extent.y = h;
    c->attr.visible      = (attr & 0x8000) ? 1 : 0;
    c->attr.editable     = (attr & 0x4000) ? 1 : 0;
    c->attr.editing      = (attr & 0x2000) ? 1 : 0;
    c->attr.selected     = (attr & 0x1000) ? 1 : 0;
    c->attr.hasScrollBar = (attr & 0x0800) ? 1 : 0;
    c->attr.usable       = (attr & 0x0400) ? 1 : 0;
    c->attr.reserved     = 0;
    debug(DEBUG_TRACE, "Form", "table attr 0x%04X (visible=%d, editable=%d, usable=%d)", attr, c->attr.visible, c->attr.editable, c->attr.usable);
    c->numColumns = cols;
    c->numRows = rows;
    c->currentRow = col;
    c->currentColumn = row;
    c->topRow = top;
    c->columnAttrs = xcalloc(cols, sizeof(TableColumnAttrType));
    c->rowAttrs = xcalloc(rows, sizeof(TableRowAttrType));
    c->items = xcalloc(cols*rows, sizeof(TableItemType));

    // szRCTableColumnAttrBA16 "w,zt5tttzb,w,zl,zl,zl"
    for (j = 0; j < cols; j++) {
      *i += get2b(&cw, p, *i);
      *i += get2b(&cattr, p, *i);
      *i += get2b(&cspacing, p, *i);
      *i += get4b(&dummy32, p, *i);
      *i += get4b(&dummy32, p, *i);
      *i += get4b(&dummy32, p, *i);

      //c->columnAttrs[j].usable = 0;
      //c->columnAttrs[j].editIndicator = 0;
      //c->columnAttrs[j].masked = 0;

      c->columnAttrs[j].reserved1     = 0;
      c->columnAttrs[j].masked        = (cattr & 0x0400) ? 1 : 0;
      c->columnAttrs[j].editIndicator = (cattr & 0x0200) ? 1 : 0;
      c->columnAttrs[j].usable        = (cattr & 0x0100) ? 1 : 0;
      c->columnAttrs[j].reserved2     = 0;

      c->columnAttrs[j].width = cw;
      c->columnAttrs[j].spacing = cspacing;
      debug(DEBUG_TRACE, "Form",  "table col %d width %d attr 0x%04X", j, cw, cattr);
    }

    // szRCTABLEROWATTR "w,w,zl,zt7t,zt4tttt,zw"
    for (j = 0; j < rows; j++) {
      *i += get2b(&rid, p, *i);
      *i += get2b(&rh, p, *i);
      *i += get4b(&rdata, p, *i);
      *i += get2b(&rattr, p, *i);
      *i += get2b(&dummy16, p, *i);
      c->rowAttrs[j].id = rid;
      c->rowAttrs[j].height = rh;
      c->rowAttrs[j].data = rdata;
      c->rowAttrs[j].reserved1    = 0;
      c->rowAttrs[j].usable       = (rattr & 0x0100) ? 1 : 0;
      c->rowAttrs[j].reserved2    = 0;
      c->rowAttrs[j].masked       = (rattr & 0x0008) ? 1 : 0;
      c->rowAttrs[j].invalid      = (rattr & 0x0004) ? 1 : 0;
      c->rowAttrs[j].staticHeight = (rattr & 0x0002) ? 1 : 0;
      c->rowAttrs[j].selectable   = (rattr & 0x0001) ? 1 : 0;
      debug(DEBUG_TRACE, "Form",  "table row %d id %d height %d attr 0x%04X (usable=%d, masked=%d, selectable=%d)",
        j, rid, rh, rattr, c->rowAttrs[j].usable, c->rowAttrs[j].masked, c->rowAttrs[j].selectable);
    }

    for (j = 0; j < cols*rows; j++) {
      *i += get4b(&p1, p, *i);
      *i += get4b(&p2, p, *i);
      debug(DEBUG_TRACE, "Form",  "table padding %d %d %d", j, p1, p2);
    }
  }

  return c;
}

static int pumpkin_destroy_table(TableType *table) {
  if (table) {
    if (table->items) xfree(table->items);
    if (table->rowAttrs) xfree(table->rowAttrs);
    if (table->columnAttrs) xfree(table->columnAttrs);
    pumpkin_heap_free(table, "Table");
  }

  return 0;
}

static FormBitmapType *pumpkin_create_formbitmap(uint8_t *p, int *i) {
  FormBitmapType *c = NULL;
  uint16_t attr, id, x, y;

  // szRCFormBitMapBA16 "uzu15,w2,w"
  *i += get2b(&attr, p, *i);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&id, p, *i);
  debug(DEBUG_TRACE, "Form", "form bitmap id %d at (%d,%d)", id, x, y);

  if ((c = pumpkin_heap_alloc(sizeof(FormBitmapType), "FormBitmap")) != NULL) {
    c->attr.usable = (attr & 0x8000) ? 1 : 0;
    c->attr.reserved = 0;
    c->pos.x = x;
    c->pos.y = y;
    c->rscID = id;
  }

  return c;
}

static FormLabelType *pumpkin_create_label(uint8_t *p, int *i) {
  FormLabelType *c = NULL;
  uint8_t dummy8, font;
  uint16_t attr, id, x, y, len;
  uint32_t dummy32;
  char *text;

  // szRCFormLabelBA16 "w,w2,uzu15,b,zb,p"
  *i += get2b(&id, p, *i);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&attr, p, *i);
  *i += get1(&font, p, *i);
  *i += get1(&dummy8, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += pumpkin_getstr(&text, p, *i);
  *i += palign(2, *i);
  len = sys_strlen(text);
  if (len < 32) len = 32; // XXX I had to do this so that FrmCopyLabel in Find in Launcher works
  debug(DEBUG_TRACE, "Form", "label id %d text \"%s\" font %d at (%d,%d)", id, text, font, x, y);

  if ((c = pumpkin_heap_alloc(sizeof(FormLabelType) + len + 1, "Label")) != NULL) {
    c->id = id;
    c->pos.x = x;
    c->pos.y = y;
    c->attr.usable = (attr & 0x8000) ? 1 : 0;
    c->attr.reserved = 0;
    c->fontID = font;
    c->marker = 0x5A5A;
    c->text = &c->buf[0];
    c->len = len;
    xmemcpy(c->text, text, len);
  }

  return c;
}

static FormTitleType *pumpkin_create_title(uint8_t *p, int *i) {
  FormTitleType *c = NULL;
  uint16_t x, y, w, h, len;
  uint32_t dummy32;
  char *title;

  // szRCFORMTITLE "w4,p"
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += pumpkin_getstr(&title, p, *i);
  len = sys_strlen(title);
  debug(DEBUG_TRACE, "Form",  "title \"%s\" at (%d,%d,%d,%d)", title, x, y, w, h);
  *i += palign(2, *i);

  if ((c = pumpkin_heap_alloc(sizeof(FormTitleType), "Title")) != NULL) {
    c->rect.topLeft.x = x;
    c->rect.topLeft.y = y;
    c->rect.extent.x = w;
    c->rect.extent.y = h;
    c->text = MemPtrNew(len + 1); // MemoPad calls MemPtrFree(oldTitle), so this must use MemPtrNew()
    xmemcpy(c->text, title, len);
  }

  return c;
}

static FormPopupType *pumpkin_create_popup(uint8_t *p, int *i) {
  FormPopupType *c = NULL;
  uint16_t controlId, listId;

  //  szRCFORMPOPUP "ww"
  *i += get2b(&controlId, p, *i);
  *i += get2b(&listId, p, *i);
  debug(DEBUG_TRACE, "Form",  "popup id %d list %d", controlId, listId);

  if ((c = pumpkin_heap_alloc(sizeof(FormPopupType), "Popup")) != NULL) {
    c->controlID = controlId;
    c->listID = listId;
  }

  return c;
}

static FrmGraffitiStateType *pumpkin_create_grfstate(uint8_t *p, int *i) {
  FrmGraffitiStateType *c = NULL;
  uint16_t x, y;

  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);

  if ((c = pumpkin_heap_alloc(sizeof(FrmGraffitiStateType), "GrfState")) != NULL) {
    c->pos.x = x;
    c->pos.y = y;
  }

  return c;
}

static FormGadgetType *pumpkin_create_gadget(uint8_t *p, int *i) {
  FormGadgetType *c = NULL;
  uint16_t attr, id, x, y, w, h;
  uint32_t dummy32;

  // szRCFORMGADGET "w,uuuzu13,w4,zl,zl"
  *i += get2b(&id, p, *i);
  *i += get2b(&attr, p, *i);
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  *i += get4b(&dummy32, p, *i);
  *i += get4b(&dummy32, p, *i);

  if ((c = pumpkin_heap_alloc(sizeof(FormGadgetType), "Gadget")) != NULL) {
    c->id = id;
    c->attr.usable   = (attr & 0x8000) ? 1 : 0;
    c->attr.extended = (attr & 0x4000) ? 1 : 0;
    c->attr.visible  = (attr & 0x2000) ? 1 : 0;
    c->attr.reserved = 0;
    c->rect.topLeft.x = x;
    c->rect.topLeft.y = y;
    c->rect.extent.x = w;
    c->rect.extent.y = h;
    debug(DEBUG_TRACE, "Form",  "gadget id %d at (%d,%d,%d,%d) usable %d", id, x, y, w, h, c->attr.usable);
  }

  return c;
}

static ScrollBarType *pumpkin_create_scrollbar(uint8_t *p, int *i) {
  ScrollBarType *c = NULL;
  uint16_t dummy16, attr, id, x, y, w, h, value, min, max, page;

  // szRCSCROLLBAR "w4,w,ttttt4,zb,w,w,w,w,zw,zw"
  *i += get2b(&x, p, *i);
  *i += get2b(&y, p, *i);
  *i += get2b(&w, p, *i);
  *i += get2b(&h, p, *i);
  *i += get2b(&id, p, *i);
  *i += get2b(&attr, p, *i);
  *i += get2b(&value, p, *i);
  *i += get2b(&min, p, *i);
  *i += get2b(&max, p, *i);
  *i += get2b(&page, p, *i);
  *i += get2b(&dummy16, p, *i);
  *i += get2b(&dummy16, p, *i);
  debug(DEBUG_TRACE, "Form",  "scrollBar id %d at (%d,%d,%d,%d)", id, x, y, w, h);

  if ((c = pumpkin_heap_alloc(sizeof(ScrollBarType), "ScrollBar")) != NULL) {
    put2b(x, (uint8_t *)c, 0);
    put2b(y, (uint8_t *)c, 2);
    put2b(w, (uint8_t *)c, 4);
    put2b(h, (uint8_t *)c, 6);

    c->bounds.topLeft.x = x;
    c->bounds.topLeft.y = y;
    c->bounds.extent.x = w;
    c->bounds.extent.y = h;
    c->id = id;
    c->attr.usable       = (attr & 0x8000) ? 1 : 0;
    c->attr.visible      = (attr & 0x4000) ? 1 : 0;
    c->attr.hilighted    = (attr & 0x2000) ? 1 : 0;
    c->attr.shown        = (attr & 0x1000) ? 1 : 0;
    c->attr.activeRegion = (attr & 0x0F00) >> 8;
    c->attr.reserved     = 0;
    c->value = value;
    c->minValue = min;
    c->maxValue = max;
    c->pageSize = page;
  }

  return c;
}

void pumpkin_fix_popups(FormType *form) {
  int j, k;

  for (j = 0; j < form->numObjects; j++) {
    if (form->objects[j].objectType == frmPopupObj) {
      for (k = 0; k < form->numObjects; k++) {
        if (form->objects[k].objectType == frmControlObj) {
          if (form->objects[k].object.control->id == form->objects[j].object.popup->controlID) {
            debug(DEBUG_TRACE, "Form",  "popup control %d -> list %d ", form->objects[j].object.popup->controlID, form->objects[j].object.popup->listID);
            form->objects[k].object.control->listID = form->objects[j].object.popup->listID;
          }
        }
        if (form->objects[k].objectType == frmListObj) {
          if (form->objects[k].object.list->id == form->objects[j].object.popup->listID) {
            debug(DEBUG_TRACE, "Form",  "popup list %d -> control %d ", form->objects[j].object.popup->listID, form->objects[j].object.popup->controlID);
            form->objects[k].object.list->controlID = form->objects[j].object.popup->controlID;
          }
        }
      }
    }
  }
}

FormType *pumpkin_create_form(uint8_t *p, uint32_t formSize) {
  FormType *form;
  ControlType *control;
  SliderControlType *slider;
  //RectangleType rect;
  uint8_t dummy8, objectType;
  uint16_t dummy16, formId, defaultButton, helpRscId, menuRscId, numObjects;
  uint16_t displayWidthV20, displayHeightV20, wx, wy, ww, wh, x0, x1, y0, y1, frameType;
  uint32_t dummy32, listSize, *offset;
  uint16_t flags;
  uint32_t attr;
  WindowFlagsType windowFlags;
  FormAttrType formAttr;
  int i, j;

  if ((form = pumpkin_heap_alloc(sizeof(FormType), "Form")) != NULL) {
    form->rsrc = p;

    // szRCWindowBA16 "w,w,zl,zuzuuuzuzuuzuzu8,w4,zw4,zl,u8zu3uu2u2,zl,zl"
    i = 0;
    i += get2b(&displayWidthV20, p, i);
    i += get2b(&displayHeightV20, p, i);
    i += get4b(&dummy32, p, i);
    i += get2b(&flags, p, i);
    i += get2b(&wx, p, i);
    i += get2b(&wy, p, i);
    i += get2b(&ww, p, i);
    i += get2b(&wh, p, i);
    i += get2b(&y0, p, i);
    i += get2b(&x0, p, i);
    i += get2b(&x1, p, i);
    i += get2b(&y1, p, i);
    i += get4b(&dummy32, p, i);
    i += get2b(&frameType, p, i);
    i += get4b(&dummy32, p, i);
    i += get4b(&dummy32, p, i);

    windowFlags.format     = (flags & 0x8000) ? 1 : 0;
    windowFlags.offscreen  = (flags & 0x4000) ? 1 : 0;
    windowFlags.modal      = (flags & 0x2000) ? 1 : 0;
    windowFlags.focusable  = (flags & 0x1000) ? 1 : 0;
    windowFlags.enabled    = (flags & 0x0800) ? 1 : 0;
    windowFlags.visible    = (flags & 0x0400) ? 1 : 0;
    windowFlags.dialog     = (flags & 0x0200) ? 1 : 0;
    windowFlags.freeBitmap = (flags & 0x0100) ? 1 : 0;
    windowFlags.reserved   = 0;

    form->window.displayWidthV20 = displayWidthV20;
    form->window.displayHeightV20 = displayHeightV20;
    form->window.windowFlags = windowFlags;
    form->window.windowBounds.topLeft.x = wx;
    form->window.windowBounds.topLeft.y = wy;
    form->window.windowBounds.extent.x = ww;
    form->window.windowBounds.extent.y = wh;
    //RctSetRectangle(&rect, x0, y0, x1-x0+1, y1-y0+1);
    //WinSetClipingBounds(form->wh, &rect);
    form->window.frameType.word = frameType;
    debug(DEBUG_TRACE, "Form", "form window bounds (%d,%d,%d,%d)", wx, wy, ww, wh);

    // szRCFormBA16 szRCWindowBA16 ",w,uuuuuuuuuzu7,zw,zl,zl,zw,w,w,w,w,zl"
    i += get2b(&formId, p, i);
    i += get4b(&attr, p, i);
    i += get4b(&dummy32, p, i);
    i += get4b(&dummy32, p, i);
    i += get2b(&dummy16, p, i);
    i += get2b(&defaultButton, p, i);
    i += get2b(&helpRscId, p, i);
    i += get2b(&menuRscId, p, i);
    i += get2b(&numObjects, p, i);
    i += get4b(&dummy32, p, i);

    formAttr.usable             = (flags & 0x80000000) ? 1 : 0;
    formAttr.enabled            = (flags & 0x40000000) ? 1 : 0;
    formAttr.visible            = (flags & 0x20000000) ? 1 : 0;
    formAttr.dirty              = (flags & 0x10000000) ? 1 : 0;
    formAttr.saveBehind         = (flags & 0x08000000) ? 1 : 0;
    formAttr.graffitiShift      = (flags & 0x04000000) ? 1 : 0;
    formAttr.globalsAvailable   = (flags & 0x02000000) ? 1 : 0;
    formAttr.doingDialog        = (flags & 0x01000000) ? 1 : 0;
    formAttr.exitDialog         = (flags & 0x00800000) ? 1 : 0;
    formAttr.attnIndicator      = (flags & 0x00400000) ? 1 : 0;
    formAttr.reserved           = 0;
    formAttr.frmDIAPolicy       = (flags & 0x00008000) ? 1 : 0;
    formAttr.inputAreaState     = (flags & 0x00007000) >> 12;
    formAttr.statusState        = (flags & 0x00000800) ? 1 : 0;
    formAttr.inputTrigger       = (flags & 0x00000400) ? 1 : 0;
    formAttr.orientation        = (flags & 0x00000380) >> 7;
    formAttr.orientationTrigger = (flags & 0x00000040) ? 1 : 0;
    formAttr.reserved2          = 0;

    debug(DEBUG_TRACE, "Form", "form id %d attr usable %d save %d", formId, formAttr.usable, formAttr.saveBehind);
    debug(DEBUG_TRACE, "Form", "form id %d wflags dialog %d modal %d", formId, windowFlags.dialog, windowFlags.modal);
    debug(DEBUG_TRACE, "Form", "form id %d defbtn %d, help %d, menu %d", formId, defaultButton, helpRscId, menuRscId);

    form->formId = formId;
    form->attr = formAttr;
    form->defaultButton = defaultButton;
    form->helpRscId = helpRscId;
    form->menuRscId = menuRscId;
    form->numObjects = numObjects;
    form->diaPolicy = frmDIAPolicyStayOpen;
    debug(DEBUG_TRACE, "Form", "form id %d has %d objects", form->formId, form->numObjects);

    form->objects = xcalloc(form->numObjects, sizeof(FormObjListType));
    offset = xcalloc(form->numObjects, sizeof(uint32_t));
    for (j = 0; j < form->numObjects; j++) {
      // szRCFormObjListBA16 "b,zb,l"
      i += get1(&objectType, p, i);
      i += get1(&dummy8, p, i);
      i += get4b(&offset[j], p, i);
      form->objects[j].objectType = objectType;
      debug(DEBUG_TRACE, "Form", "object %d type %d (%s) offset %u", j, objectType, FrmObjectTypeName(objectType), offset[j]);
    }

    for (j = 0; j < form->numObjects; j++) {
      debug(DEBUG_TRACE, "Form",  "reading object %d", j);
      i = offset[j];
      switch (form->objects[j].objectType) {
        case frmFieldObj:
          form->objects[j].object.field = pumpkin_create_field(p, &i);
          debug(DEBUG_TRACE, "Form",  "object %d is a Field %d", j, form->objects[j].object.field->id);
          form->objects[j].id = form->objects[j].object.field->id;
          form->objects[j].object.field->formP = form;
          form->objects[j].object.field->objIndex = j;
          break;
        case frmControlObj:
          control = pumpkin_create_control(p, &i);
          control->formP = form;
          control->objIndex = j;

          if (control->style == sliderCtl || control->style == feedbackSliderCtl) {
            slider = (SliderControlType *)control;
            form->objects[j].object.sliderControl = slider;
            form->objects[j].id = form->objects[j].object.sliderControl->id;
            debug(DEBUG_TRACE, "Form",  "object %d is a SliderControl %d bitmaps %d,%d", j, form->objects[j].object.sliderControl->id, form->objects[j].object.sliderControl->thumbID, form->objects[j].object.sliderControl->backgroundID);
          } else if (control->attr.graphical) {
            form->objects[j].object.graphicControl = control;
            form->objects[j].id = form->objects[j].object.graphicControl->id;
            debug(DEBUG_TRACE, "Form",  "object %d is a GraphicControl %d bitmap %d", j, form->objects[j].object.graphicControl->id, control->bitmapID);
          } else {
            form->objects[j].object.control = control;
            form->objects[j].id = form->objects[j].object.control->id;
            debug(DEBUG_TRACE, "Form",  "object %d is a Control %d group %d", j, form->objects[j].object.control->id, form->objects[j].object.control->group);
          }
          break;
        case frmListObj:
          listSize = (form->numObjects > 1 && j < form->numObjects-1) ? offset[j+1] - offset[j] : formSize - offset[j];
          form->objects[j].object.list = pumpkin_create_list(p, &i, form, listSize);
          form->objects[j].id = form->objects[j].object.list->id;
          debug(DEBUG_TRACE, "Form",  "object %d is a List %d (usable %d)", j, form->objects[j].object.list->id, form->objects[j].object.list->attr.usable);
          break;
        case frmTableObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a Table", j);
          form->objects[j].object.table = pumpkin_create_table(p, &i);
          form->objects[j].id = form->objects[j].object.table->id;
          break;
        case frmBitmapObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a FormBitmap", j);
          form->objects[j].object.bitmap = pumpkin_create_formbitmap(p, &i);
          form->objects[j].id = form->objects[j].object.bitmap->rscID;
          break;
        case frmLabelObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a Label", j);
          form->objects[j].object.label = pumpkin_create_label(p, &i);
          form->objects[j].id = form->objects[j].object.label->id;
          break;
        case frmTitleObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a Title", j);
          form->objects[j].object.title = pumpkin_create_title(p, &i);
          form->objects[j].object.title->objIndex = j;
          break;
        case frmPopupObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a Popup", j);
          form->objects[j].object.popup = pumpkin_create_popup(p, &i);
          break;
        case frmGraffitiStateObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a GraffitiStateIndicator", j);
          form->objects[j].object.grfState = pumpkin_create_grfstate(p, &i);
          break;
        case frmGadgetObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a Gadget", j);
          form->objects[j].object.gadget = pumpkin_create_gadget(p, &i);
          form->objects[j].id = form->objects[j].object.gadget->id;
          break;
        case frmScrollBarObj:
          debug(DEBUG_TRACE, "Form",  "object %d is a ScrollBar", j);
          form->objects[j].object.scrollBar = pumpkin_create_scrollbar(p, &i);
          form->objects[j].id = form->objects[j].object.scrollBar->id;
          break;
        default:
          debug(DEBUG_ERROR, "Form",  "object %d is of unknown type (%d)", j, objectType);
          break;
      }
    }
    xfree(offset);

    pumpkin_fix_popups(form);
  }

  return form;
}

void pumpkin_destroy_alert(void *p) {
  if (p) {
    MemChunkFree(p);
  }
}

static void pumpkin_destroy_form_object(FormObjListType *obj) {
  if (obj && obj->object.ptr) {

    switch (obj->objectType) {
      case frmFieldObj:
        debug(DEBUG_TRACE, "Form", "free form field");
        FldFreeMemory(obj->object.field);
        pumpkin_heap_free(obj->object.field, "Field");
        break;
      case frmControlObj:
        debug(DEBUG_TRACE, "Form", "free form control");
        pumpkin_heap_free(obj->object.control, "Control");
        break;
      case frmListObj:
        debug(DEBUG_TRACE, "Form", "free form list");
        pumpkin_destroy_list(obj->object.list);
        break;
      case frmTableObj:
        debug(DEBUG_TRACE, "Form", "free form table");
        pumpkin_destroy_table(obj->object.table);
        break;
      case frmBitmapObj:
        debug(DEBUG_TRACE, "Form", "free form bitmap");
        pumpkin_heap_free(obj->object.bitmap, "FormBitmap");
        break;
      case frmLabelObj:
        debug(DEBUG_TRACE, "Form", "free form label");
        pumpkin_heap_free(obj->object.label, "Label");
        break;
      case frmTitleObj:
        debug(DEBUG_TRACE, "Form", "free form title");
        pumpkin_heap_free(obj->object.title, "Title");
        break;
      case frmPopupObj:
        debug(DEBUG_TRACE, "Form", "free form popup");
        pumpkin_heap_free(obj->object.popup, "Popup");
        break;
      case frmGraffitiStateObj:
        debug(DEBUG_TRACE, "Form", "free form graffitiState");
        pumpkin_heap_free(obj->object.grfState, "GrfState");
        break;
      case frmGadgetObj:
        debug(DEBUG_TRACE, "Form", "free form gadget");
        if (obj->object.gadget->handler) {
          if (obj->object.gadget->m68k_handler) {
            CallGadgetHandler(obj->object.gadget->m68k_handler, (FormGadgetTypeInCallback *)obj->object.gadget, formGadgetDeleteCmd, NULL);
          } else {
            obj->object.gadget->handler((FormGadgetTypeInCallback *)obj->object.gadget, formGadgetDeleteCmd, NULL);
          }
        }
        pumpkin_heap_free(obj->object.gadget, "Gadget");
        break;
      case frmScrollBarObj:
        debug(DEBUG_TRACE, "Form", "free form scrollBar");
        pumpkin_heap_free(obj->object.scrollBar, "ScrollBar");
        break;
      default:
        debug(DEBUG_ERROR, "Form", "free form object of unknown type %d", obj->objectType);
        break;
    }
  }
}

void pumpkin_destroy_form(FormType *formP) {
  int j;

  if (formP) {
    debug(DEBUG_TRACE, "Form", "pumpkin_destroy_form %d with %d objects", formP->formId, formP->numObjects);
    if (formP->objects) {
      for (j = 0; j < formP->numObjects; j++) {
        pumpkin_destroy_form_object(&formP->objects[j]);
      }
      xfree(formP->objects);
    }
    if (formP->rsrc) {
      pumpkin_heap_free(formP->rsrc, "form_rsrc");
    }
    pumpkin_heap_free(formP, "Form");
  }
}

AlertTemplateType *pumpkin_create_alert(void *h, uint8_t *p, uint32_t *dsize) {
  AlertTemplateType *alert = NULL;
  uint16_t alertType, helpRscID, numButtons, defaultButton;
  char *title, *message, *btn;
  int i, j;

  if ((alert = StoNewDecodedResource(h, sizeof(AlertTemplateType), 0, 0)) != NULL) {
    i = 0;
    i += get2b(&alertType, p, i);
    i += get2b(&helpRscID, p, i);
    i += get2b(&numButtons, p, i);
    i += get2b(&defaultButton, p, i);
    i += pumpkin_getstr(&title, p, i);
    i += pumpkin_getstr(&message, p, i);

    alert->alertType = alertType;
    alert->helpRscID = helpRscID;
    alert->numButtons = numButtons;
    alert->defaultButton = defaultButton;
    alert->title = title;
    alert->message = message;
    debug(DEBUG_TRACE, "Form", "alert title \"%s\", message \"%s\", type %d, help %d, buttons %d, defbutton %d", title, message, alertType, helpRscID, numButtons, defaultButton);

    for (j = 0; j < numButtons; j++) {
      i += pumpkin_getstr(&btn, p, i);
      alert->button[j] = btn;
      debug(DEBUG_TRACE, "Form", "alert button %d \"%s\"", j, btn);
    }
  }

  *dsize = sizeof(AlertTemplateType);
  return alert;
}
