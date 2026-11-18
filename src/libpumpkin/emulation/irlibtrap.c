#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <VFSMgr.h>
#include <INetMgr.h>
#include <IrLib.h>

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

void palmos_irlibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case irLibTrapIrOpen: {
      // Err IrOpen(UInt16 refnum, UInt32 options)
      uint16_t refNum = ARG16;
      uint32_t options = ARG32;
      err = IrOpen(refNum, options);
      debug(DEBUG_TRACE, "EmuPalmOS", "ItOpen(refNum=%u, options=0x%08X): %d", refNum, options, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      // Err IrClose(UInt16 refnum)
      uint16_t refNum = ARG16;
      err = IrClose(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrClose(refNum=%u): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      m68k_set_reg(M68K_REG_D0, errNone);
      }
      break;
    case sysLibTrapWake: {
      m68k_set_reg(M68K_REG_D0, errNone);
      }
      break;
    case irLibTrapBind: {
      // IrStatus IrBind(UInt16 refNum, IrConnect *con, IrCallBack callBack)
      uint16_t refNum = ARG16;
      uint32_t conP = ARG32;
      uint32_t callBackP = ARG32;
      IrConnect *con = emupalmos_trap_in(conP, trap, 1);
      IrCallBack callBack = emupalmos_trap_in(callBackP, trap, 2);
      IrStatus res = IrBind(refNum, con, callBack);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrBind(refNum=%u, con=0x%08X, callBack=0x%08X): %d", refNum, conP, callBackP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case irLibTrapUnBind: {
      // IrStatus IrUnbind(UInt16 refNum, IrConnect *con)
      uint16_t refNum = ARG16;
      uint32_t conP = ARG32;
      IrConnect *con = emupalmos_trap_in(conP, trap, 1);
      IrStatus res = IrUnbind(refNum, con);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrUnbind(refNum=%u, con=0x%08X): %d", refNum, conP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case irLibTrapSetDeviceInfo: {
      // IrStatus IrSetDeviceInfo(UInt16 refNum, UInt8 *info, UInt8 len)
      uint16_t refNum = ARG16;
      uint32_t infoP = ARG32;
      uint8_t len = ARG8;
      UInt8 *info = emupalmos_trap_in(infoP, trap, 1);
      IrStatus res = IrSetDeviceInfo(refNum, info, len);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrSetDeviceInfo(refNum=%u, info=0x%08X, len=%u): %d", refNum, infoP, len, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case irLibTrapIsIrLapConnected: {
      // BOOL IrIsIrLapConnected(UInt16 refNum)
      uint16_t refNum = ARG16;
      Int16 res = IrIsIrLapConnected(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrIsIrLapConnected(refNum=%u): %d", refNum, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case irLibTrapIAS_SetDeviceName: {
      // IrStatus IrIAS_SetDeviceName(UInt16 refNum, UInt8 *name, UInt8 len)
      uint16_t refNum = ARG16;
      uint32_t nameP = ARG32;
      uint8_t len = ARG8;
      UInt8 *name = emupalmos_trap_in(nameP, trap, 1);
      IrStatus res = IrIAS_SetDeviceName(refNum, name, len);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrIAS_SetDeviceName(refNum=%u, name=0x%08X, len=%u): %d", refNum, nameP, len, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case irLibTrapIAS_Add: {
      // IrStatus IrIAS_Add(UInt16 refNum, IrIasObject *obj)
      uint16_t refNum = ARG16;
      uint32_t objP = ARG32;
      IrIasObject *obj = emupalmos_trap_in(objP, trap, 1);
      IrStatus res = IrIAS_Add(refNum, obj);
      debug(DEBUG_TRACE, "EmuPalmOS", "IrIAS_Add(refNum=%u, obj=0x%08X): %d", refNum, objP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "IrLib trap 0x%04X (%d) not mapped", trap, trap - sysLibTrapCustom);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
