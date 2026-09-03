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

void palmos_ExgSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapExgInit:
    case sysTrapExgConnect:
    case sysTrapExgGet:
    case sysTrapExgAccept:
    case sysTrapExgRegisterData:
    case sysTrapExgNotifyReceiveV35:
    case sysTrapExgDBRead:
    case sysTrapExgDBWrite:
    case sysTrapExgDoDialog:
    case sysTrapExgRegisterDatatype:
    case sysTrapExgNotifyReceive:
    case sysTrapExgNotifyGoto:
    case sysTrapExgRequest:
    case sysTrapExgSetDefaultApplication:
    case sysTrapExgGetDefaultApplication:
    case sysTrapExgGetTargetApplication:
    case sysTrapExgGetRegisteredApplications:
    case sysTrapExgGetRegisteredTypes:
    case sysTrapExgNotifyPreview:
    case sysTrapExgControl:
      m68k_set_reg(M68K_REG_D0, sysErrParamErr);
      break;
    case sysTrapExgPut: {
      // Err ExgPut(ExgSocketType *socketP)
      uint32_t socketP = ARG32;
      uint8_t *p = emupalmos_trap_in(socketP, trap, 0);
      debug_bytes(DEBUG_INFO, "EmuPalmOS", p, 60);
      ExgSocketType sock;
      decode_ExgSocketType(socketP, &sock);
      Err err = ExgPut(&sock);
      m68k_set_reg(M68K_REG_D0, err);
      debug(DEBUG_INFO, "EmuPalmOS", "ExgPut(socketP=0x%08X): %d", socketP, err);
      }
      break;
    case sysTrapExgSend: {
      // UInt32 ExgSend(ExgSocketType *socketP, const void *bufP, UInt32 bufLen, Err *err)
      uint32_t socketP = ARG32;
      uint32_t bufP = ARG32;
      uint32_t bufLen = ARG32;
      uint32_t errP = ARG32;
      emupalmos_trap_in(socketP, trap, 0);
      void *buf = emupalmos_trap_in(bufP, trap, 1);
      emupalmos_trap_in(errP, trap, 3);
      ExgSocketType sock;
      decode_ExgSocketType(socketP, &sock);
      Err err;
      UInt32 res = ExgSend(&sock, buf, bufLen, &err);
      if (errP) m68k_write_memory_16(errP, err);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_INFO, "EmuPalmOS", "ExgSend(socketP=0x%08X, bufP=0x%08X, bufLen=%u, err=0x%04X): %u",
        socketP, bufP, bufLen, err, res);
      }
      break;
    case sysTrapExgDisconnect: {
      // Err ExgDisconnect(ExgSocketType *socketP, Err error)
      uint32_t socketP = ARG32;
      uint16_t error = ARG16;
      emupalmos_trap_in(socketP, trap, 0);
      ExgSocketType sock;
      decode_ExgSocketType(socketP, &sock);
      Err err = ExgDisconnect(&sock, error);
      debug(DEBUG_INFO, "EmuPalmOS", "ExgDisconnect(socketP=0x%08X, error=0x%04X): 0x%04X", socketP, error, err);
      }
      break;
    case sysTrapExgReceive:
      m68k_set_reg(M68K_REG_D0, 0);
      break;
  }
}
