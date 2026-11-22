#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <VFSMgr.h>
#include <INetMgr.h>
#include <BtLib.h>

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

void palmos_btlibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      // Err BtLibOpen(UInt16 btLibRefNum, Boolean allowStackToFail)
      uint16_t refNum = ARG16;
      uint8_t allowStackToFail = ARG8;
      err = BtLibOpen(refNum, allowStackToFail);
      debug(DEBUG_TRACE, "EmuPalmOS", "BtLibOpen(refNum=%u, allowStackToFail=%d): %d", refNum, allowStackToFail, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      // Err BtLibClose(UInt16 refnum)
      uint16_t refNum = ARG16;
      err = BtLibClose(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "BtLibClose(refNum=%u): %d", refNum, err);
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
    case btLibTrapRegisterManagementNotification: {
      // Err BtLibRegisterManagementNotification(UInt16 btLibRefNum, BtLibManagementProcPtr callbackP, UInt32 refCon)
      uint16_t refNum = ARG16;
      uint32_t callbackP = ARG32;
      uint32_t refCon = ARG32;
      BtLibManagementProcPtr callback = emupalmos_trap_in(callbackP, trap, 1);
      err = BtLibRegisterManagementNotification(refNum, callback, refCon);
      debug(DEBUG_TRACE, "EmuPalmOS", "BtLibRegisterManagementNotification(refNum=%u, callback=0x%08X, refCon=%u): %d", refNum, callbackP, refCon, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case btLibTrapSocketCreate: {
      // Err BtLibSocketCreate(UInt16 btLibRefNum, BtLibSocketRef *socketRefP, BtLibSocketProcPtr callbackP, UInt32 refCon, BtLibProtocolEnum socketProtocol)
      uint16_t refNum = ARG16;
      uint32_t socketRefP = ARG32;
      uint32_t callbackP = ARG32;
      uint32_t refCon = ARG32;
      uint32_t socketProtocol = ARG8;
      emupalmos_trap_in(socketRefP, trap, 1);
      BtLibSocketProcPtr callback = emupalmos_trap_in(callbackP, trap, 2);
      BtLibSocketRef socketRef;
      err = BtLibSocketCreate(refNum, &socketRef, callback, refCon, socketProtocol);
      debug(DEBUG_TRACE, "EmuPalmOS", "BtLibSocketCreate(refNum=%u, sockeRef=0x%08X, callback=0x%08X, refCon=%u, socketProtocol=%d): %d",
        refNum, socketRefP, callbackP, refCon, socketProtocol, err);
      if (err == errNone && socketRefP) m68k_write_memory_16(socketRefP, socketRef);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case btLibTrapSocketListen: {
      // Err BtLibSocketListen(UInt16 btLibRefNum, BtLibSocketRef socket, BtLibSocketListenInfoType *listenInfo)
      uint16_t refNum = ARG16;
      uint16_t socket = ARG16;
      uint32_t listenInfoP = ARG32;
      emupalmos_trap_in(listenInfoP, trap, 2);
      BtLibSocketListenInfoType listenInfo;
      err = BtLibSocketListen(refNum, socket, &listenInfo);
      debug(DEBUG_TRACE, "EmuPalmOS", "BtLibSocketListen(refNum=%u, socket=%d, listenInfo=0x%08X): %d", refNum, socket, listenInfoP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case btLibTrapSdpServiceRecordCreate: {
      // Err BtLibSdpServiceRecordCreate(UInt16 btLibRefNum, BtLibSdpRecordHandle *recordH)
      uint16_t refNum = ARG16;
      uint32_t recordH = ARG32;
      emupalmos_trap_in(recordH, trap, 1);
      BtLibSdpRecordHandle record;
      err = BtLibSdpServiceRecordCreate(refNum, &record);
      debug(DEBUG_TRACE, "EmuPalmOS", "BtLibSdpServiceRecordCreate(refNum=%u, recordH=0x%08X): %d", refNum, recordH, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;

    default:
      sys_snprintf(buf, sizeof(buf)-1, "BtLib trap 0x%04X (%d) not mapped", trap, trap - sysLibTrapCustom);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
