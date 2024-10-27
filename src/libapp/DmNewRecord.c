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

MemHandle DmNewRecord(DmOpenRef dbP, UInt16 *atP, UInt32 size) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapDmNewRecord, 0, NULL, &pret, dbP, atP, size);
  return (MemHandle)pret;
}
