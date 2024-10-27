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

Int32 FileReadLow(FileHand stream, void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapFileReadLow, 0, &iret, NULL, stream, baseP, offset, dataStoreBased, objSize, numObj, errP);
  return (Int32)iret;
}
