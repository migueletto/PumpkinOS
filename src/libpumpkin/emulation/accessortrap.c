#include <PalmOS.h>
#include <VFSMgr.h>
#include <FrmGlue.h>
#include <CtlGlue.h>
#include <LstGlue.h>
#include <BmpGlue.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_accessortrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
#if 0
    case 0:
      // CtlGlueGetControlStyle
      break;
    case 1:
      // FldGlueGetLineInfo
      break;
#endif
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
#if 0
    case 5:
      // BmpGlueSetTransparentValue
      break;
    case 6:
      // CtlGlueGetFont
      break;
#endif
    case 7: {
      // void CtlGlueSetFont(ControlType *ctlP, FontID fontID)
      uint32_t ctlP = ARG32;
      uint8_t fontID = ARG8;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetFont(ctl, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetFont(0x%08X, %d)", ctlP, fontID);
      }
      break;
#if 0
    case 8:
      // CtlGlueGetGraphics
      break;
    case 9:
      // CtlGlueNewSliderControl
      break;
#endif
    case 16: {
      // void CtlGlueSetLeftAnchor(ControlType *ctlP, Boolean leftAnchor)
      uint32_t ctlP = ARG32;
      uint8_t leftAnchor = ARG8;
      ControlType *ctl = (ControlType *)emupalmos_trap_sel_in(ctlP, sysTrapAccessorDispatch, sel, 0);
      CtlGlueSetLeftAnchor(ctl, leftAnchor);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGlueSetLeftAnchor(0x%08X, %d)", ctlP, leftAnchor);
      }
      break;
#if 0
    case 17:
      // FrmGlueGetDefaultButtonID
      break;
    case 18:
      // FrmGlueSetDefaultButtonID
      break;
    case 19:
      // FrmGlueGetHelpID
      break;
    case 20:
      // FrmGlueSetHelpID
      break;
    case 21:
      // FrmGlueGetMenuBarID
      break;
    case 22:
      // FrmGlueGetLabelFont
      break;
    case 23:
      // FrmGlueSetLabelFont
      break;
    case 24:
      // FrmGlueGetEventHandler
      break;
#endif
    case 25: {
      // FontID LstGlueGetFont(const ListType *listP)
      uint32_t listP = ARG32;
      ListType *list = (ListType *)emupalmos_trap_sel_in(listP, sysTrapAccessorDispatch, sel, 0);
      FontID font = LstGlueGetFont(list);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstGlueGetFont(0x%08X): %d", listP, font);
      m68k_set_reg(M68K_REG_D0, font);
      }
      break;
#if 0
    case 32:
      // LstGlueSetFont
      break;
    case 33:
      // LstGlueGetItemsText
      break;
    case 34:
      // LstGlueSetIncrementalSearch
      break;
    case 35:
      // TblGlueGetColumnMasked
      break;
    case 36:
      // WinGlueGetFrameType
      break;
    case 37:
      // WinGlueSetFrameType
      break;
    case 38:
      // FrmGlueGetObjIDFromObjPtr
      break;
    case 39:
      // LstGlueGetDrawFunction
      break;
    case 40:
      // CtlGlueIsGraphical
      break;
    case 41:
      // CtlGlueSetFrameStyle
      break;
#endif
    default:
      sys_snprintf(buf, sizeof(buf)-1, "AccessorDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
