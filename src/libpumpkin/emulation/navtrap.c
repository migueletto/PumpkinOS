#include <PalmOS.h>
#include <VFSMgr.h>
#include <HsNavCommon.h>
#include <HsExt.h>
#include <HsNav.h>
    
#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"
    
void palmos_navtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err;
    
  switch (sel) {
    case NavSelectorFrmNavObjectTakeFocus: {
      // void FrmNavObjectTakeFocus(const FormType* formP, UInt16 objID)
      uint32_t formP = ARG32;
      uint16_t objID = ARG16;
      FormType *form = emupalmos_trap_sel_in(formP, sysTrapNavSelector, sel, 0);
      FrmNavObjectTakeFocus(form, objID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNavObjectTakeFocus(0x%08X, %d)", formP, objID);
      }
      break;
    case NavSelectorFrmNavGetFocusRingInfo: {
      // Err FrmNavGetFocusRingInfo(const FormType *formP, UInt16 *objectIDP,
      //   Int16 *extraInfoP, RectangleType *boundsInsideRingP,
      //   FrmNavFocusRingStyleEnum *ringStyleP)
      uint32_t formP = ARG32;
      uint32_t objectIDP = ARG32;
      uint32_t extraInfoP = ARG32;
      uint32_t boundsInsideRingP = ARG32;
      uint32_t ringStyleP = ARG32;
      emupalmos_trap_sel_in(formP, sysTrapNavSelector, sel, 0);
      emupalmos_trap_sel_in(objectIDP, sysTrapNavSelector, sel, 1);
      emupalmos_trap_sel_in(extraInfoP, sysTrapNavSelector, sel, 2);
      emupalmos_trap_sel_in(boundsInsideRingP, sysTrapNavSelector, sel, 3);
      emupalmos_trap_sel_in(ringStyleP, sysTrapNavSelector, sel, 4);
      if (objectIDP) m68k_write_memory_16(objectIDP, frmInvalidObjectId);
      if (extraInfoP) m68k_write_memory_16(extraInfoP, frmNavFocusRingNoExtraInfo);
      if (boundsInsideRingP) {
        m68k_write_memory_16(boundsInsideRingP, 0);
        m68k_write_memory_16(boundsInsideRingP + 2, 0);
        m68k_write_memory_16(boundsInsideRingP + 4, 0);
        m68k_write_memory_16(boundsInsideRingP + 6, 0);
      }
      if (ringStyleP) m68k_write_memory_16(ringStyleP, frmNavFocusRingStyleInvalid);
      err = uilibErrObjectNotFound;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNavGetFocusRingInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d",
        formP, objectIDP, extraInfoP, boundsInsideRingP, ringStyleP, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "NavSelector selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
