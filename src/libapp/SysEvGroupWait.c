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

Err SysEvGroupWait(UInt32 evID, UInt32 mask, UInt32 value, Int32 matchType, Int32 timeout) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSysEvGroupWait, 0, &iret, NULL, evID, mask, value, matchType, timeout);
  return (Err)iret;
}
