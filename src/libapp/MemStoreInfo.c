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

Err MemStoreInfo(UInt16 cardNo, UInt16 storeNumber, UInt16 *versionP, UInt16 *flagsP, Char *nameP, UInt32 *crDateP, UInt32 *bckUpDateP, UInt32 *heapListOffsetP, UInt32 *initCodeOffset1P, UInt32 *initCodeOffset2P, LocalID *databaseDirIDP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapMemStoreInfo, 0, &iret, NULL, cardNo, storeNumber, versionP, flagsP, nameP, crDateP, bckUpDateP, heapListOffsetP, initCodeOffset1P, initCodeOffset2P, databaseDirIDP);
  return (Err)iret;
}
