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

Err FtrGetByIndex(UInt16 index, Boolean romTable, UInt32 *creatorP, UInt16 *numP, UInt32 *valueP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapFtrGetByIndex, 0, &iret, NULL, index, romTable, creatorP, numP, valueP);
  return (Err)iret;
}
