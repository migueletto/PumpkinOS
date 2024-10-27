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

UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapDmFindResource, 0, &iret, NULL, dbP, resType, resID, resH);
  return (UInt16)iret;
}
