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

Err SysTaskCreate(UInt32 *taskIDP, UInt32 *creator, ProcPtr codeP, MemPtr stackP, UInt32 stackSize, UInt32 attr, UInt32 priority, UInt32 tSlice) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSysTaskCreate, 0, &iret, NULL, taskIDP, creator, codeP, stackP, stackSize, attr, priority, tSlice);
  return (Err)iret;
}
