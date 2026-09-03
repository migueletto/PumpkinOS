#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>
#include <INetMgr.h>

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

void palmos_PrgSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapPrgStartDialogV31: {
      // ProgressPtr PrgStartDialogV31(const Char *title, PrgCallbackFunc textCallback)
      uint32_t titleP = ARG32;
      uint32_t textCallbackP = ARG32;
      char *title = emupalmos_trap_in(titleP, trap, 0);
      void *textCallback = emupalmos_trap_in(textCallbackP, trap, 1);
      ProgressPtr prg = PrgStartDialogV31(title, textCallback);
      uint32_t prgP = emupalmos_trap_out(prg);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrgStartDialogV31(0x%08X [%s], 0x%08X): 0x%08X", titleP, title ? title : "", textCallbackP, prgP);
      m68k_set_reg(M68K_REG_A0, prgP);
    }
    break;
    case sysTrapPrgStartDialog: {
      // ProgressPtr PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP)
      uint32_t titleP = ARG32;
      uint32_t textCallbackP = ARG32;
      uint32_t userDataP = ARG32;
      char *title = emupalmos_trap_in(titleP, trap, 0);
      void *textCallback = emupalmos_trap_in(textCallbackP, trap, 1);
      void *userData = emupalmos_trap_in(userDataP, trap, 2);
      ProgressPtr prg = PrgStartDialog(title, textCallback, userData);
      uint32_t prgP = emupalmos_trap_out(prg);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrgStartDialog(0x%08X [%s], 0x%08X, 0x%08X): 0x%08X", titleP, title ? title : "", textCallbackP, userDataP, prgP);
      m68k_set_reg(M68K_REG_A0, prgP);
    }
    break;
    case sysTrapPrgStopDialog: {
      // void PrgStopDialog(ProgressPtr prgP, Boolean force)
      uint32_t prgP = ARG32;
      uint8_t force = ARG8;
      void *prg = emupalmos_trap_in(prgP, trap, 0);
      PrgStopDialog(prg, force);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrgStopDialog(0x%08X)", prgP);
    }
    break;
    case sysTrapPrgHandleEvent: {
      // Boolean PrgHandleEvent(ProgressPtr prgGP, EventType *eventP)
      uint32_t prgP = ARG32;
      uint32_t eventP = ARG32;
      void *prg = emupalmos_trap_in(prgP, trap, 0);
      emupalmos_trap_in(eventP, trap, 1);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      Boolean res = PrgHandleEvent(prg, &event);
      char *eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [%s]): %d", prgP, eventP, eventName, res);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [0x%04X]): %d", prgP, eventP, event.eType, res);
      }
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapPrgUpdateDialog: {
      // void PrgUpdateDialog(ProgressPtr prgGP, UInt16 err, UInt16 stage, const Char *messageP, Boolean updateNow)
      uint32_t prgP = ARG32;
      uint16_t err = ARG16;
      uint16_t stage = ARG16;
      uint32_t messageP = ARG32;
      uint8_t updateNow = ARG8;
      void *prg = emupalmos_trap_in(prgP, trap, 0);
      char *message = emupalmos_trap_in(messageP, trap, 3);
      PrgUpdateDialog(prg, err, stage, message, updateNow);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrgUpdateDialog(0x%08X, %d, %d, 0x%08X [%s], %d)", prgP, err, stage, messageP, messageP ? message : "", updateNow);
    }
    break;
  }
}
