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

Err SlotCardSectorWrite(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 sectorNumber, UInt8 *bufferP, UInt32 *numSectorsP) {
  uint64_t iret;
  pumpkin_system_call_p(SLOT_LIB, SlotTrapCardSectorWrite, 0, &iret, NULL, slotLibRefNum, slotRefNum, sectorNumber, bufferP, numSectorsP);
  return (Err)iret;
}
