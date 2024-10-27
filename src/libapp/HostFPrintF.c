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

long HostFPrintF(HostFILEType *fileP, const char *fmt, ...) {
  sys_va_list ap;
  sys_va_start(ap, fmt);
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapHostControl, hostSelectorFPrintF, &iret, NULL, fileP, fmt, ap);
  sys_va_end(ap);
  return (long)iret;
}
