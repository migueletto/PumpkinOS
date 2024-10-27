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

void HostTraceOutputTL(UInt16 a, const char *b, ...) {
  sys_va_list ap;
  sys_va_start(ap, b);
  pumpkin_system_call_p(0, sysTrapHostControl, hostSelectorTraceOutputTL, NULL, NULL, a, b, ap);
  sys_va_end(ap);
}
