#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "mutex.h"
#include "storage.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_FrmSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapFrmNewForm: {
      // FormType *FrmNewForm(UInt16 formID, const Char *titleStrP, Coord x, Coord y, Coord width, Coord height, Boolean modal, UInt16 defaultButton, UInt16 helpRscID, UInt16 menuRscID)
      uint16_t formID = ARG16;
      uint32_t titleStrP = ARG32;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint16_t width = ARG16;
      uint16_t height = ARG16;
      uint8_t modal = ARG8;
      uint16_t defaultButton = ARG16;
      uint16_t helpRscID = ARG16;
      uint16_t menuRscID = ARG16;
      char *titleStr = (char *)emupalmos_trap_in(titleStrP, trap, 1);
      FormType *form = FrmNewForm(formID, titleStr, x, y, width, height, modal, defaultButton, helpRscID, menuRscID);
      uint32_t formP = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewForm(%d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X", formID, titleStrP, titleStr, x, y, width, height, modal, defaultButton, helpRscID, menuRscID, formP);
      m68k_set_reg(M68K_REG_A0, formP);
    }
    break;
    case sysTrapFrmInitForm: {
      // FormType *FrmInitForm(UInt16 rscID)
      uint16_t rscID = ARG16;
      FormType *form = FrmInitForm(rscID);
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmInitForm(%d): 0x%08X", rscID, f);
      m68k_set_reg(M68K_REG_A0, f);
    }
    break;
    case sysTrapFrmDeleteForm: {
      // void FrmDeleteForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmDeleteForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDeleteForm(0x%08X)", formP);
    }
    break;
    case sysTrapFrmGetFormId: {
      // UInt16 FrmGetFormId(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 id = FrmGetFormId(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormId(0x%08X): %d", formP, id);
      m68k_set_reg(M68K_REG_D0, id);
    }
    break;
    case sysTrapFrmGetFirstForm: {
      // FormType *FrmGetFirstForm(void)
      FormType *form = FrmGetFirstForm();
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFirstForm(): 0x%08X", f);
      m68k_set_reg(M68K_REG_A0, f);
    }
    break;
    case sysTrapFrmGetFormPtr: {
      // FormType *FrmGetFormPtr(UInt16 formId)
      uint16_t formId = ARG16;
      FormType *form = FrmGetFormPtr(formId);
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormPtr(%d): 0x%08X", formId, f);
      m68k_set_reg(M68K_REG_A0, f);
    }
    break;
    case sysTrapFrmGetObjectIndexFromPtr: {
      // UInt16 FrmGetObjectIndexFromPtr(const FormType *formP, void *objP)
      uint32_t formP = ARG32;
      uint32_t objP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      void *obj = emupalmos_trap_in(objP, trap, 1);
      UInt16 res = FrmGetObjectIndexFromPtr(form, obj);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndexFromPtr(0x%08X, 0x%08X): %d", formP, objP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmGetActiveField: {
      // FieldType *FrmGetActiveField(const FormType* formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FieldType *fld = FrmGetActiveField(form);
      uint32_t f = emupalmos_trap_out(fld);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveField(0x%08X): 0x%08X", formP, f);
      m68k_set_reg(M68K_REG_A0, f);
    }
    break;
    case sysTrapFrmGotoForm: {
      // void FrmGotoForm(UInt16 formId)
      uint16_t formId = ARG16;
      FrmGotoForm(formId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGotoForm(%d)", formId);
    }
    break;
    case sysTrapFrmUpdateForm: {
      // void FrmUpdateForm(UInt16 formId, UInt16 updateCode)
      uint16_t formId = ARG16;
      uint16_t updateCode = ARG16;
      FrmUpdateForm(formId, updateCode);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmUpdateForm(%d, %d)", formId, updateCode);
    }
    break;
    case sysTrapFrmDrawForm: {
      // void FrmDrawForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm begin");
      FrmDrawForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm(0x%08X)", formP);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm end");
    }
    break;
    case sysTrapFrmEraseForm: {
      // void FrmEraseForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmEraseForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmEraseForm(0x%08X)", formP);
    }
    break;
    case sysTrapFrmVisible: {
      // Boolean FrmVisible(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      Boolean res = FrmVisible(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmVisible(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmHideObject: {
      // void FrmHideObject(FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t index = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmHideObject(form, index);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHideObject(0x%08X, %d)", formP, index);
      // XXX must handle 68K code because of gadget handler
    }
    break;
    case sysTrapFrmShowObject: {
      // void FrmShowObject(FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t index = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmShowObject(form, index);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmShowObject(0x%08X, %d)", formP, index);
      // XXX must handle 68K code because of gadget handler
    }
    break;
    case sysTrapFrmGetFocus: {
      // UInt16 FrmGetFocus(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetFocus(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFocus(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmSetMenu: {
      // void FrmSetMenu(FormType *formP, UInt16 menuRscID)
      uint32_t formP = ARG32;
      uint16_t menuRscID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetMenu(form, menuRscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetMenu(0x%08X, %d)", formP, menuRscID);
    }
    break;
    case sysTrapFrmGetTitle: {
      // const Char *FrmGetTitle(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *title = (char *)FrmGetTitle(form);
      uint32_t s = emupalmos_trap_out(title);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetTitle(0x%08X): 0x%08X \"%s\"", formP, s, title ? title : "");
      m68k_set_reg(M68K_REG_A0, s);
    }
    break;
    case sysTrapFrmCopyTitle: {
      // void FrmCopyTitle(FormType *formP, const Char *newTitle)
      uint32_t formP = ARG32;
      uint32_t newTitleP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
      FrmCopyTitle(form, newTitle);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
    }
    break;
    case sysTrapFrmSetTitle: {
      // void FrmSetTitle(FormType *formP, Char *newTitle)
      uint32_t formP = ARG32;
      uint32_t newTitleP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
      FrmSetTitle(form, newTitle);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
    }
    break;
    case sysTrapFrmUpdateScrollers: {
      // void FrmUpdateScrollers(FormType *formP, UInt16 upIndex, UInt16 downIndex, Boolean scrollableUp, Boolean scrollableDown)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      uint16_t upIndex = ARG16;
      uint16_t downIndex = ARG16;
      uint8_t scrollableUp = ARG8;
      uint8_t scrollableDown = ARG8;
      FrmUpdateScrollers(form, upIndex, downIndex, scrollableUp, scrollableDown);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmUpdateScrollers(0x%08X, %d, %d, %d, %d)", formP, upIndex, downIndex, scrollableUp, scrollableDown);
    }
    break;
    case sysTrapFrmSetActiveForm: {
      // void FrmSetActiveForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetActiveForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetActiveForm(0x%08X)", formP);
    }
    break;
    case sysTrapFrmSetEventHandler: {
      // void FrmSetEventHandler(FormType *formP, FormEventHandlerType *handler)
      uint32_t formP = ARG32;
      uint32_t handlerP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) form->m68k_handler = handlerP;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetEventHandler(0x%08X, 0x%08X)", formP, handlerP);
    }
    break;
    case sysTrapFrmGetEventHandler68K: {
      // FormEventHandlerType *FrmGetEventHandler68K(FormType *formP)
      // custom trap created for use in 68K code
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      uint32_t handlerP = form ? form->m68k_handler : 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetEventHandler68K(0x%08X): 0x%08X", formP, handlerP);
      m68k_set_reg(M68K_REG_A0, handlerP);
    }
    break;
    case sysTrapFrmSetGadgetHandler: {
      // void FrmSetGadgetHandler(FormType *formP, UInt16 objIndex, FormGadgetHandlerType *attrP)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t handlerP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) gadget->m68k_handler = handlerP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetGadgetHandler(0x%08X, %d, 0x%08X)", formP, objIndex, handlerP);
    }
    break;
    case sysTrapFrmGetGadgetData: {
      // void *FrmGetGadgetData(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      uint32_t dataP = 0;
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) dataP = gadget->m68k_data;
    }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetGadgetData(0x%08X, %d): 0x%08X", formP, objIndex, dataP);
      m68k_set_reg(M68K_REG_A0, dataP);
      }
    break;
    case sysTrapFrmSetGadgetData: {
      // void FrmSetGadgetData(FormType *formP, UInt16 objIndex, const void *data)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t dataP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) gadget->m68k_data = dataP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetGadgetData(0x%08X, %d, 0x%08X)", formP, objIndex, dataP);
    }
    break;
    case sysTrapFrmGetGadgetPtr68K: {
      // FormGadgetTypeInCallback *FrmGetGadgetPtr68k(FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t gadgetP = 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) {
          gadgetP = emupalmos_trap_out(gadget);
          if (FrmGetObjectType(form, objIndex) == frmGadgetObj) {
            encode_gadget(gadgetP, gadget);
          }
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetGadgetPtr68k(0x%08X, %d): 0x%08X", formP, objIndex, gadgetP);
      m68k_set_reg(M68K_REG_A0, gadgetP);
    }
    break;
    case sysTrapFrmGetWindowHandle: {
      // WinHandle FrmGetWindowHandle(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      WinHandle wh = FrmGetWindowHandle(form);
      uint32_t w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetWindowHandle(0x%08X): 0x%08X", formP, w);
      m68k_set_reg(M68K_REG_A0, w);
    }
    break;
    case sysTrapFrmGetFormBounds: {
      // void FrmGetFormBounds(const FormType *formP, RectangleType *rP)
      uint32_t formP = ARG32;
      uint32_t rP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      RectangleType rect;
      FrmGetFormBounds(form, &rect);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormBounds(0x%08X, 0x%08X)", formP, rP);
    }
    break;
    case sysTrapFrmSetObjectBounds: {
      // void FrmSetObjectBounds(FormType *formP, UInt16 objIndex, const RectangleType *bounds)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t rP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      RectangleType rect;
      decode_rectangle(rP, &rect);
      FrmSetObjectBounds(form, objIndex, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetObjectBounds(0x%08X, %d, 0x%08X)", formP, objIndex, rP);
    }
    break;
    case sysTrapFrmGetNumberOfObjects: {
      // UInt16 FrmGetNumberOfObjects(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetNumberOfObjects(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetNumberOfObjects(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmSetObjectPosition: {
      // void FrmSetObjectPosition(FormType *formP, UInt16 objIndex, Coord x, Coord y)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      Coord x = ARG16;
      Coord y = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetObjectPosition(form, objIndex, x, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetObjectPosition(0x%08X, %u, %d, %d)", formP, objIndex, x, y);
    }
    break;
    case sysTrapFrmGetObjectId: {
      // UInt16 FrmGetObjectId(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetObjectId(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectId(0x%08X, %d): %d", formP, objIndex, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmGetObjectPosition: {
      // void FrmGetObjectPosition(const FormType *formP, UInt16 objIndex, Coord *x, Coord *y)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t xP = ARG32;
      uint32_t yP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      Coord x, y;
      FrmGetObjectPosition(form, objIndex, xP ? &x : NULL, yP ? &y : NULL);
      if (xP) m68k_write_memory_16(xP, x);
      if (yP) m68k_write_memory_16(yP, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectPosition(0x%08X, %d, 0x%08X, 0x%08X)", formP, objIndex, xP, yP);
    }
    break;
    case sysTrapFrmGetObjectBounds: {
      // void FrmGetObjectBounds(const FormType *formP, UInt16 objIndex, RectangleType *rP)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t rP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      RectangleType rect;
      FrmGetObjectBounds(form, objIndex, &rect);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectBounds(0x%08X, %d, 0x%08X)", formP, objIndex, rP);
    }
    break;
    case sysTrapFrmGetControlGroupSelection: {
      // UInt16 FrmGetControlGroupSelection(const FormType *formP, UInt8 groupNum)
      uint32_t formP = ARG32;
      uint8_t groupNum = ARG8;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetControlGroupSelection(form, groupNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlGroupSelection(0x%08X, %u): %u", formP, groupNum, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmGetActiveForm: {
      // FormType *FrmGetActiveForm(void)
      FormType *form = FrmGetActiveForm();
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveForm(): 0x%08X", f);
      m68k_set_reg(M68K_REG_A0, f);
    }
    break;
    case sysTrapFrmGetActiveFormID: {
      // UInt16 FrmGetActiveFormID(void)
      UInt16 id = FrmGetActiveFormID();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveFormID(): %d", id);
      m68k_set_reg(M68K_REG_D0, id);
    }
    break;
    case sysTrapFrmGetObjectIndex: {
      // UInt16 FrmGetObjectIndex(const FormType *formP, UInt16 objID)
      uint32_t formP = ARG32;
      uint16_t objID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 index = FrmGetObjectIndex(form, objID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndex(0x%08X, %d): %d", formP, objID, index);
      m68k_set_reg(M68K_REG_D0, index);
    }
    break;
    case sysTrapFrmGetObjectPtr: {
      // void *FrmGetObjectPtr(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      void *p = FrmGetObjectPtr(form, objIndex);
      uint32_t ptr = emupalmos_trap_out(p);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectPtr(0x%08X, %d): 0x%08X", formP, objIndex, ptr);
      m68k_set_reg(M68K_REG_A0, ptr);
    }
    break;
    case sysTrapFrmGetObjectType: {
      // FormObjectKind FrmGetObjectType(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FormObjectKind objType = FrmGetObjectType(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectType(0x%08X, %d): %d", formP, objIndex, objType);
      m68k_set_reg(M68K_REG_D0, objType);
    }
    break;
    case sysTrapFrmGetLabel: {
      // const Char *FrmGetLabel(const FormType *formP, UInt16 labelID)
      uint32_t formP = ARG32;
      uint16_t labelID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      const Char *label = FrmGetLabel(form, labelID);
      uint32_t a = emupalmos_trap_out((void *)label);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetLabel(0x%08X, %d): 0x%08X", formP, labelID, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapFrmSetFocus: {
      // void FrmSetFocus(FormType *formP, UInt16 fieldIndex)
      uint32_t formP = ARG32;
      uint16_t fieldIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetFocus(form, fieldIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetFocus(0x%08X, %d)", formP, fieldIndex);
    }
    break;
    case sysTrapFrmGetControlValue: {
      // Int16 FrmGetControlValue(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      Int16 value = FrmGetControlValue(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlValue(0x%08X, %d): %d", formP, objIndex, value);
      m68k_set_reg(M68K_REG_D0, value);
    }
    break;
    case sysTrapFrmSetControlValue: {
      // void FrmSetControlValue(const FormType *formP, UInt16 objIndex, Int16 newValue)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      int16_t newValue = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetControlValue(form, objIndex, newValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlValue(0x%08X, %d, %d)", formP, objIndex, newValue);
    }
    break;
    case sysTrapFrmSetControlGroupSelection: {
      // void FrmSetControlGroupSelection(const FormType *formP, UInt8 groupNum, UInt16 controlID)
      uint32_t formP = ARG32;
      uint8_t groupNum = ARG8;
      uint16_t controlID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetControlGroupSelection(form, groupNum, controlID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlGroupSelection(0x%08X, %d, %d)", formP, groupNum, controlID);
    }
    break;
    case sysTrapFrmDispatchEvent: {
      // Boolean FrmDispatchEvent(EventType *eventP)
      uint32_t eventP = ARG32;
      EventType event;
      if (eventP) decode_event(eventP, &event);
      Boolean res = FrmDispatchEvent(&event);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDispatchEvent(0x%08X): %d", eventP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmHandleEvent: {
      // Boolean FrmHandleEvent(FormType *formP, EventType *eventP)
      uint32_t formP = ARG32;
      uint32_t eventP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      Boolean res = FrmHandleEvent(form, &event);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHandleEvent(0x%08X, 0x%08X): %d", formP, eventP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmCopyLabel: {
      // void FrmCopyLabel(FormType *formP, UInt16 labelID, const Char *newLabel)
      uint32_t formP = ARG32;
      uint16_t labelID = ARG16;
      uint32_t newLabelP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *s = (char *)emupalmos_trap_in(newLabelP, trap, 1);
      FrmCopyLabel(form, labelID, s);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyLabel(0x%08X, %d, 0x%08X \"%s\")", formP, labelID, newLabelP, s ? s : "");
    }
    break;
    case sysTrapFrmSaveAllForms:
      // void FrmSaveAllForms(void)
      FrmSaveAllForms();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSaveAllForms()");
    break;
    case sysTrapFrmCloseAllForms:
      // void FrmCloseAllForms(void)
      FrmCloseAllForms();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCloseAllForms()");
    break;
    case sysTrapFrmPopupForm: {
      // void FrmPopupForm(UInt16 formId)
      uint16_t formId = ARG16;
      FrmPopupForm(formId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmPopupForm(%d)", formId);
    }
    break;
    case sysTrapFrmDoDialog: {
      // UInt16 FrmDoDialog(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmDoDialog(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDoDialog(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmReturnToForm: {
      // void FrmReturnToForm(UInt16 formId)
      uint16_t formId = ARG16;
      FrmReturnToForm(formId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmReturnToForm(%d)", formId);
    }
    break;
    case sysTrapFrmHelp: {
      // void FrmHelp(UInt16 helpMsgId)
      uint16_t helpMsgId = ARG16;
      FrmHelp(helpMsgId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHelp(%d)", helpMsgId);
    }
    break;
    case sysTrapFrmCustomAlert: {
      // UInt16 FrmCustomAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3)
      uint16_t alertId = ARG16;
      uint32_t s1P = ARG32;
      uint32_t s2P = ARG32;
      uint32_t s3P = ARG32;
      char *s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
      char *s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
      char *s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
      UInt16 res = FrmCustomAlert(alertId, s1, s2, s3);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomAlert(%d, 0x%08X, 0x%08X, 0x%08X): %d", alertId, s1P, s2P, s3P, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmCustomResponseAlert: {
      // UInt16 FrmCustomResponseAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3, Char *entryStringBuf, Int16 entryStringBufLength, FormCheckResponseFuncPtr callback)
      uint16_t alertId = ARG16;
      uint32_t s1P = ARG32;
      uint32_t s2P = ARG32;
      uint32_t s3P = ARG32;
      uint32_t entryStringBufP = ARG32;
      int16_t entryStringBufLength = ARG16;
      uint32_t callbackP = ARG32;
      char *s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
      char *s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
      char *s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
      char *entryStringBuf = (char *)emupalmos_trap_in(entryStringBufP, trap, 4);
      FormCheckResponseFuncPtr callback = (FormCheckResponseFuncPtr)emupalmos_trap_in(callbackP, trap, 6);
      UInt16 res = FrmCustomResponseAlert(alertId, s1, s2, s3, entryStringBuf, entryStringBufLength, callback);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomResponseAlert(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d, 0x%08X): %d", alertId, s1P, s2P, s3P, entryStringBufP, entryStringBufLength, callbackP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmAlert: {
      // UInt16 FrmAlert(UInt16 alertId)
      uint16_t alertId = ARG16;
      UInt16 res = FrmAlert(alertId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmAlert(%d): %d", alertId, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapFrmNewBitmap: {
      // FormBitmapType *FrmNewBitmap(FormType **formPP, UInt16 ID, UInt16 rscID, Coord x, Coord y)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint16_t rscId = ARG16;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FormBitmapType *bitmap = FrmNewBitmap(&form, id, rscId, x, y);
      uint32_t a = emupalmos_trap_out(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewBitmap(0x%08X, %u, %u, %d, %d): 0x%08X", formPP, id, rscId, x, y, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapFrmNewGadget: {
      // FormGadgetType *FrmNewGadget(FormType **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint16_t width = ARG16;
      uint16_t height = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FormGadgetType *gadget = FrmNewGadget(&form, id, x, y, width, height);
      uint32_t a = emupalmos_trap_out(gadget);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGadget(0x%08X, %u, %d, %d, %d, %d): 0x%08X", formPP, id, x, y, width, height, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
    break;
    case sysTrapFrmActiveState: {
      // Err FrmActiveState(FormActiveStateType *stateP, Boolean save)
      uint32_t stateP = ARG32;
      uint8_t save = ARG8;
      FormActiveStateType *state = (FormActiveStateType *)emupalmos_trap_in(stateP, trap, 0);
      Err err = FrmActiveState(state, save);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmActiveState(0x%08X, %d)", stateP, save);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapFrmNewGsi: {
      // FrmGraffitiStateType *FrmNewGsi(FormType **formPP, Coord x, Coord y)
      uint32_t formPP = ARG32;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmGraffitiStateType *gsi = FrmNewGsi(&form, x, y);
      uint32_t a = emupalmos_trap_out(gsi);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGsi(0x%08X, %d, %d): 0x%08X", formPP, x, y, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
  }
}
