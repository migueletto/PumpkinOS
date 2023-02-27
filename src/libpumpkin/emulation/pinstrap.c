#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_pinstrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    case pinPINSetInputAreaState: {
      // Err PINSetInputAreaState(UInt16 state)
      uint16_t state = ARG16;
      Err err = PINSetInputAreaState(state);
      debug(DEBUG_TRACE, "EmuPalmOS", "PINSetInputAreaState(%d): %d", state, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinPINGetInputAreaState: {
      // UInt16 PINGetInputAreaState(void)
      UInt16 state = PINGetInputAreaState();
      debug(DEBUG_TRACE, "EmuPalmOS", "PINGetInputAreaState(): %d", state);
      m68k_set_reg(M68K_REG_D0, state);
    }
    break;
    case pinPINSetInputTriggerState: {
      // Err PINSetInputTriggerState(UInt16 state)
      uint16_t state = ARG16;
      Err err = PINSetInputTriggerState(state);
      debug(DEBUG_TRACE, "EmuPalmOS", "PINSetInputTriggerState(%d): %d", state, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinPINGetInputTriggerState:
    //case pinPINAltInputSystemEnabled:
    //case pinPINGetCurrentPinletName:
    //case pinPINSwitchToPinlet:
    //case pinPINCountPinlets:
    //case pinPINGetPinletInfo:
    //case pinPINSetInputMode:
    //case pinPINGetInputMode:
    //case pinPINClearPinletState:
    //case pinPINShowReferenceDialog:
    case pinWinSetConstraintsSize: {
      // Err WinSetConstraintsSize(WinHandle winH, Coord minH, Coord prefH, Coord maxH, Coord minW, Coord prefW, Coord maxW)
      uint32_t w = ARG32;
      uint16_t minH = ARG16;
      uint16_t prefH = ARG16;
      uint16_t maxH = ARG16;
      uint16_t minW = ARG16;
      uint16_t prefW = ARG16;
      uint16_t maxW = ARG16;
      WinHandle wh = emupalmos_trap_sel_in(w, sysTrapPinsDispatch, sel, 0);
      Err err = WinSetConstraintsSize(wh, minH, prefH, maxH, minW, prefW, maxW);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinSetConstraintsSize(0x%08X, %d, %d, %d, %d, %d, %d): %d", w, minH, prefH, maxH, minW, prefW, maxW, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinFrmSetDIAPolicyAttr: {
      // Err FrmSetDIAPolicyAttr(FormPtr formP, UInt16 diaPolicy)
      uint32_t formP = ARG32;
      uint16_t diaPolicy = ARG16;
      FormType *form = emupalmos_trap_sel_in(formP, sysTrapPinsDispatch, sel, 0);
      Err err = FrmSetDIAPolicyAttr(form, diaPolicy);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetDIAPolicyAttr(0x%08X, %d): %d", formP, diaPolicy, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinFrmGetDIAPolicyAttr:
    case pinStatHide: {
      // Err StatHide(void)
      Err err = StatHide();
      debug(DEBUG_TRACE, "EmuPalmOS", "StatHide(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinStatShow: {
      // Err StatShow(void)
      Err err = StatShow();
      debug(DEBUG_TRACE, "EmuPalmOS", "StatShow(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case pinStatGetAttribute: {
      // Err StatGetAttribute(UInt16 selector, UInt32 *dataP)
      uint16_t selector = ARG16;
      uint32_t dataP = ARG32;
      UInt32 data;
      Err err = StatGetAttribute(selector, dataP ? &data : NULL);
      if (dataP) m68k_write_memory_32(dataP, data);
      debug(DEBUG_TRACE, "EmuPalmOS", "StatGetAttribute(%d, 0x%08X): %d", selector, dataP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinSysGetOrientation:
    case pinSysSetOrientation: {
      // Err SysSetOrientation(UInt16 orientation)
      uint16_t orientation = ARG16;
      Err err = SysSetOrientation(orientation);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysSetOrientation(%d): %d", orientation, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    //case pinSysGetOrientationTriggerState
    case pinSysSetOrientationTriggerState: {
      // Err SysSetOrientationTriggerState(UInt16 triggerState)
      uint16_t triggerState = ARG16;
      Err err = SysSetOrientationTriggerState(triggerState);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysSetOrientationTriggerState(%d): %d", triggerState, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "PinsDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
