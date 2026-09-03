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

void palmos_PenSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapPenResetCalibration: {
      // Err PenResetCalibration(void)
      Err err = PenResetCalibration();
      debug(DEBUG_TRACE, "EmuPalmOS", "PenResetCalibration(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapPenCalibrate: {
      // Err PenCalibrate(PointType *digTopLeftP, PointType *digBotRightP, PointType *scrTopLeftP, PointType *scrBotRightP)
      Err err = PenCalibrate(NULL, NULL, NULL, NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "PenCalibrate %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapPenSleep: {
      // Err PenSleep(void)
      Err err = PenSleep();
      debug(DEBUG_TRACE, "EmuPalmOS", "PenSleep(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapPenWake: {
      // Err PenWake(void)
      Err err = PenWake();
      debug(DEBUG_TRACE, "EmuPalmOS", "PenWake(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
  }
}
