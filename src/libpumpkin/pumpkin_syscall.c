#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <VFSMgr.h>
#include <ExpansionMgr.h>
#include <DLServer.h>
#include <SerialMgrOld.h>
#include <UDAMgr.h>
#include <PceNativeCall.h>
#include <FixedMath.h>
#include <BmpGlue.h>
#include <FrmGlue.h>
#include <CtlGlue.h>
#include <FldGlue.h>
#include <LstGlue.h>
#include <TblGlue.h>
#include <WinGlue.h>

#include <CPMLib.h>
#include <GPSLib68K.h>
#include <GPDLib.h>
#include <PdiLib.h>

#include "debug.h"

int pumpkin_system_call(syscall_lib_e lib, uint32_t id, uint32_t sel, uint64_t *iret, void **pret, ...) {
  sys_va_list ap;
  int r = 0;

  debug(DEBUG_TRACE, PUMPKINOS, "syscall 0x%04X %d (lib %d)", id, sel, lib);
  sys_va_start(ap, pret);

  if (lib == 0) {
    switch (id) {
      case sysCallTxtLowerChar: {
        WChar c = sys_va_arg(ap, uint32_t);
        c = TxtLowerChar(c);
        *iret = c;
        }
        break;
      case sysCallTxtUpperChar: {
        WChar c = sys_va_arg(ap, uint32_t);
        c = TxtUpperChar(c);
        *iret = c;
        }
        break;
#include "syscall_switch.c"
      default:
        debug(DEBUG_ERROR, PUMPKINOS, "invalid syscall 0x%04X %d", id, sel);
        r = -1;
        break;
    }
  } else {
    switch (lib) {
      case CPM_LIB:
        switch (id) {
#include "cpmlib_switch.c"
          default:
            debug(DEBUG_ERROR, PUMPKINOS, "invalid cpmlib syscall 0x%04X", id);
            r = -1;
            break;
        }
        break;
      case GPS_LIB:
        switch (id) {
#include "gpslib_switch.c"
          default:
            debug(DEBUG_ERROR, PUMPKINOS, "invalid gpslib syscall 0x%04X", id);
            r = -1;
            break;
        }
        break;
      case NET_LIB:
        switch (id) {
#include "netlib_switch.c"
          default:
            debug(DEBUG_ERROR, PUMPKINOS, "invalid netlib syscall 0x%04X", id);
            r = -1;
            break;
        }
        break;
      case GPD_LIB:
        switch (id) {
#include "gpdlib_switch.c"
          default:
            debug(DEBUG_ERROR, PUMPKINOS, "invalid gpdlib syscall 0x%04X", id);
            r = -1;
            break;
        } 
        break;
      case PDI_LIB:
        switch (id) {
#include "pdilib_switch.c"
          default:
            debug(DEBUG_ERROR, PUMPKINOS, "invalid pdilib syscall 0x%04X", id);
            r = -1;
            break;
        } 
        break;
      case BT_LIB:
      case EXG_LIB:
      case FS_LIB:
      case INET_LIB:
      case IR_LIB:
      case SER_LIB:
      case SLOT_LIB:
      case SSL_LIB:
        break;
    }
  }

  sys_va_end(ap);

  return r;
}
