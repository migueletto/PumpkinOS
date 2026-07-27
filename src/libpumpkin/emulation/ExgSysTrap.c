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

void palmos_ExgSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapExgInit:
    case sysTrapExgConnect:
    case sysTrapExgPut:
    case sysTrapExgGet:
    case sysTrapExgAccept:
    case sysTrapExgDisconnect:
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
    case sysTrapExgSend:
    case sysTrapExgReceive:
      m68k_set_reg(M68K_REG_D0, 0);
    break;
  }
}
