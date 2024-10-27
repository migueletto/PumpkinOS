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

Err DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, UInt16 *indexP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapDmFindRecordByID, 0, &iret, NULL, dbP, uniqueID, indexP);
  return (Err)iret;
}
