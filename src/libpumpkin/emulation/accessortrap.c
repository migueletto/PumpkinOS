#include <PalmOS.h>
#include <VFSMgr.h>
#include <FrmGlue.h>
#include <CtlGlue.h>
#include <FldGlue.h>
#include <LstGlue.h>
#include <TblGlue.h>
#include <BmpGlue.h>
#include <WinGlue.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_accessortrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    case 0: {
      // ControlStyleType CtlGlueGetControlStyle(const ControlType *ctlP)
      uint32_t ctlP = ARG32;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      ControlStyleType res = CtlGlueGetControlStyle(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueGetControlStyle(0x%08X): %d", ctlP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 1: {
      // Boolean FldGlueGetLineInfo(const FieldType *fldP, UInt16 lineNum, UInt16* startP, UInt16* lengthP)
      uint32_t fldP = ARG32;
      uint16_t lineNum = ARG16;
      uint32_t startP = ARG32;
      uint32_t lengthP = ARG32;
      FieldType *fld = (FieldType *)emupalmos_trap_sel_in(fldP, sysTrapAccessorDispatch, sel, 0);
      emupalmos_trap_sel_in(startP, sysTrapAccessorDispatch, sel, 2);
      emupalmos_trap_sel_in(lengthP, sysTrapAccessorDispatch, sel, 3);
      UInt16 start, length;
      Boolean res = FldGlueGetLineInfo(fld, lineNum, &start, &length);
      if (startP) m68k_write_memory_32(startP, start);
      if (lengthP) m68k_write_memory_32(lengthP, length);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGlueGetLineInfo(0x%08X, %u, 0x%08X, 0x%08X): %d", fldP, lineNum, startP, lengthP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 2: {
      // Boolean FrmGlueGetObjectUsable(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      Boolean res = FrmGlueGetObjectUsable(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetObjectUsable(0x%08X, %d): %d", formP, objIndex, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 3: {
      // BitmapCompressionType BmpGlueGetCompressionType(const BitmapType *bitmapP)
      uint32_t bitmapP = ARG32;
      BitmapType *bitmap = (BitmapType *)emupalmos_trap_sel_in(bitmapP, sysTrapAccessorDispatch, sel, 0);
      BitmapCompressionType res = BmpGlueGetCompressionType(bitmapP ? bitmap : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGlueGetCompressionType(0x%08X): %d", bitmapP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 4: {
      // Boolean BmpGlueGetTransparentValue(const BitmapType *bitmapP, UInt32 *transparentValueP)
      uint32_t bitmapP = ARG32;
      uint32_t transparentValueP = ARG32;
      BitmapType *bitmap = (BitmapType *)emupalmos_trap_sel_in(bitmapP, sysTrapAccessorDispatch, sel, 0);
      emupalmos_trap_sel_in(transparentValueP, sysTrapAccessorDispatch, sel, 1);
      UInt32 transparentValue;
      Boolean res = BmpGlueGetTransparentValue(bitmapP ? bitmap : NULL, &transparentValue);
      if (transparentValueP) m68k_write_memory_32(transparentValueP, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGlueGetTransparentValue(0x%08X, 0x%08X [0x%08X]): %d", bitmapP, transparentValueP, transparentValue, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 5: {
      // void BmpGlueSetTransparentValue(BitmapType *bitmapP, UInt32 transparentValue)
      uint32_t bitmapP = ARG32;
      uint32_t transparentValue = ARG32;
      BitmapType *bitmap = (BitmapType *)emupalmos_trap_sel_in(bitmapP, sysTrapAccessorDispatch, sel, 0);
      BmpGlueSetTransparentValue(bitmap, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGlueSetTransparentValue(0x%08X, 0x%08X)", bitmapP, transparentValue);
      }
      break;
    case 6: {
      // FontID CtlGlueGetFont(const ControlType *ctlP)
      uint32_t ctlP = ARG32;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      FontID res = CtlGlueGetFont(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueGetFont(0x%08X): %d", ctlP, res);
      m68k_set_reg(M68K_REG_D0, res);
      break;
      }
    case 7: {
      // void CtlGlueSetFont(ControlType *ctlP, FontID fontID)
      uint32_t ctlP = ARG32;
      uint8_t fontID = ARG8;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetFont(ctl, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetFont(0x%08X, %d)", ctlP, fontID);
      }
      break;
    case 8: {
      // void CtlGlueGetGraphics(const ControlType *ctlP, DmResID *bitmapID, DmResID *selectedBitmapID)
      uint32_t ctlP = ARG32;
      uint32_t bitmapIDP = ARG32;
      uint32_t selectedBitmapIDP = ARG32;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      DmResID bitmapID, selectedBitmapID;
      CtlGlueGetGraphics(ctl, &bitmapID, &selectedBitmapID);
      if (bitmapIDP) m68k_write_memory_32(bitmapIDP, bitmapID);
      if (selectedBitmapIDP) m68k_write_memory_32(selectedBitmapIDP, selectedBitmapID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueGetGraphics(0x%08X, 0x%08X, 0x%08X)", ctlP, bitmapIDP, selectedBitmapIDP);
      }
      break;
    case 9: {
      // SliderControlType *CtlGlueNewSliderControl(void **formPP, UInt16 ID, ControlStyleType style, DmResID thumbID, DmResID backgroundID, Coord x, Coord y, Coord width, Coord height, UInt16 minValue, UInt16 maxValue, UInt16 pageSize, UInt16 value)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint8_t style = ARG8;
      uint16_t thumbID = ARG16;
      uint16_t backgroundID = ARG16;
      int16_t x = ARG16;
      int16_t y = ARG16;
      int16_t width = ARG16;
      int16_t height = ARG16;
      uint16_t minValue = ARG16;
      uint16_t maxValue = ARG16;
      uint16_t pageSize = ARG16;
      uint16_t value = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      void *form = emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      SliderControlType *ctl = CtlGlueNewSliderControl(&form, id, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value);
      uint32_t a = emupalmos_trap_out(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueNewSliderControl(0x%08X, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X", formPP, id, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case 10: {
      // void CtlGlueSetLeftAnchor(ControlType *ctlP, Boolean leftAnchor)
      uint32_t ctlP = ARG32;
      uint8_t leftAnchor = ARG8;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetLeftAnchor(ctl, leftAnchor);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetLeftAnchor(0x%08X, %d)", ctlP, leftAnchor);
      }
      break;
    case 11: {
      // UInt16 FrmGlueGetDefaultButtonID(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      UInt16 res = FrmGlueGetDefaultButtonID(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetDefaultButtonID(0x%08X): %u", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 12: {
      // void FrmGlueSetDefaultButtonID(FormType *formP, UInt16 defaultButton)
      uint32_t formP = ARG32;
      uint16_t defaultButton = ARG16;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      FrmGlueSetDefaultButtonID(form, defaultButton);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueSetDefaultButtonID(0x%08X, %u)", formP, defaultButton);
      }
      break;
    case 13: {
      // UInt16 FrmGlueGetHelpID(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      UInt16 res = FrmGlueGetHelpID(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetHelpID(0x%08X): %u", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 14: {
      // void FrmGlueSetHelpID(FormType *formP, UInt16 helpRscID)
      uint32_t formP = ARG32;
      uint16_t helpRscID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      FrmGlueSetHelpID(form, helpRscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueSetHelpID(0x%08X, %u)", formP, helpRscID);
      }
      break;
    case 15: {
      // UInt16 FrmGlueGetMenuBarID(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      UInt16 res = FrmGlueGetMenuBarID(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetMenuBarID(0x%08X): %u", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 16: {
      // FontID FrmGlueGetLabelFont(const FormType *formP, UInt16 labelID)
      uint32_t formP = ARG32;
      uint16_t labelID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      UInt16 res = FrmGlueGetLabelFont(form, labelID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetLabelFont(0x%08X, %u): %u", formP, labelID, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 17: {
      // void FrmGlueSetLabelFont(FormType *formP, UInt16 labelID, FontID fontID)
      uint32_t formP = ARG32;
      uint16_t labelID = ARG16;
      uint8_t fontID = ARG8;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      FrmGlueSetLabelFont(form, labelID, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueSetLabelFont(0x%08X, %u, %u)", formP, labelID, fontID);
      }
      break;
    case 18: {
      // FormEventHandlerType *FrmGlueGetEventHandler(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_sel_in(formP, sysTrapAccessorDispatch, sel, 0);
      UInt32 handler = form->m68k_handler;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetEventHandler(0x%08X): 0x%08X", formP, handler);
      m68k_set_reg(M68K_REG_A0, handler);
      }
      break;
    case 19: {
      // FontID LstGlueGetFont(const ListType *listP)
      uint32_t listP = ARG32;
      ListType *list = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      FontID font = LstGlueGetFont(list);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetFont(0x%08X): %d", listP, font);
      m68k_set_reg(M68K_REG_D0, font);
      }
      break;
    case 20: {
      // void LstGlueSetFont(ListType *listP, FontID fontID)
      uint32_t listP = ARG32;
      uint8_t fontID = ARG8;
      ListType *lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      LstGlueSetFont(lst, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueSetFont(0x%08X, %u)", listP, fontID);
      }
      break;
    case 21: {
      // Char **LstGlueGetItemsText(const ListType *listP)
      uint32_t listP = ARG32;
      ListType *lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      Char **itemsText = LstGlueGetItemsText(lst);
      uint32_t a = emupalmos_trap_out(itemsText);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetItemsText(0x%08X): 0x%08X", listP, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case 22: {
      // void LstGlueSetIncrementalSearch(ListType *listP, Boolean incrementalSearch)
      uint32_t listP = ARG32;
      uint8_t incrementalSearch = ARG8;
      ListType *lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      LstGlueSetIncrementalSearch(lst, incrementalSearch);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueSetIncrementalSearch(0x%08X, %u)", listP, incrementalSearch);
      }
      break;
    case 23: {
      // Boolean TblGlueGetColumnMasked(const TableType *tableP, Int16 column)
      uint32_t tableP = ARG32;
      int16_t column = ARG8;
      TableType *tbl = (TableType *)emupalmos_trap_sel_in(tableP, sysTrapAccessorDispatch, sel, 0);
      Boolean res = TblGlueGetColumnMasked(tbl, column);
      debug(DEBUG_TRACE, "EmuPalmOS", "TblGlueGetColumnMasked(0x%08X, %d): %u", tableP, column, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 24: {
      // FrameType WinGlueGetFrameType(const WinHandle winH)
      uint32_t winH = ARG32;
      WinHandle wh = (WinHandle)emupalmos_trap_sel_in(winH, sysTrapAccessorDispatch, sel, 0);
      FrameType res = WinGlueGetFrameType(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGlueGetFrameType(0x%08X): %u", winH, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 25: {
      // void WinGlueSetFrameType(WinHandle winH, FrameType frame)
      uint32_t winH = ARG32;
      uint16_t frame = ARG16;
      WinHandle wh = (WinHandle)emupalmos_trap_sel_in(winH, sysTrapAccessorDispatch, sel, 0);
      WinGlueSetFrameType(wh, frame);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGlueSetFrameType(0x%08X, %u)", winH, frame);
      }
      break;
    case 26: {
      // UInt16 FrmGlueGetObjIDFromObjPtr(void *formObjP, FormObjectKind objKind)
      uint32_t formObjP = ARG32;
      uint16_t objKind = ARG16;
      void *obj = emupalmos_trap_sel_in(formObjP, sysTrapAccessorDispatch, sel, 0);
      UInt16 res = FrmGlueGetObjIDFromObjPtr(obj, objKind);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGlueGetObjIDFromObjPtr(0x%08X, %u): %u", formObjP, objKind, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 27: {
      // void *LstGlueGetDrawFunction(ListType* listP)
      uint32_t listP = ARG32;
      ListType *lst = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      void *res = LstGlueGetDrawFunction(lst);
      uint32_t a = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetDrawFunction(0x%08X): 0x%08X", listP, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case 28: {
      // Boolean CtlGlueIsGraphical(ControlType* controlP)
      uint32_t ctlP = ARG32;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      Boolean res = CtlGlueIsGraphical(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueIsGraphical(0x%08X): %u", ctlP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case 29: {
      // void CtlGlueSetFrameStyle(ControlType* controlP, ButtonFrameType frameStyle)
      uint32_t ctlP = ARG32;
      uint8_t frameStyle = ARG8;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetFrameStyle(ctl, frameStyle);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetFrameStyle(0x%08X, %u)", ctlP, frameStyle);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "AccessorDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
