#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <VFSMgr.h>
#include <ExpansionMgr.h>
#include <DLServer.h>
#include <SerialMgrOld.h>
#include <UDAMgr.h>
#include <PceNativeCall.h>
#include <FixedMath.h>
#include <CPMLib.h>
#include <GPSLib68K.h>
#include <GPDLib.h>
#include <PdiLib.h>
#include <BtLib.h>
#include <FSLib.h>
#include <SslLib.h>
#include <INetMgr.h>
#include <SlotDrvrLib.h>

Err NetLibTracePrintF(UInt16 libRefNum, const Char *formatStr, ...) {
  sys_va_list ap;
  sys_va_start(ap, formatStr);
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapTracePrintF, 0, &iret, NULL, libRefNum, formatStr, ap);
  sys_va_end(ap);
  return (Err)iret;
}
