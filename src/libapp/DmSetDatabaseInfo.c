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

Err DmSetDatabaseInfo(UInt16 cardNo, LocalID dbID, const Char *nameP, UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP, UInt32 *bckUpDateP, UInt32 *modNumP, LocalID *appInfoIDP, LocalID *sortInfoIDP, UInt32 *typeP, UInt32 *creatorP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapDmSetDatabaseInfo, 0, &iret, NULL, cardNo, dbID, nameP, attributesP, versionP, crDateP, modDateP, bckUpDateP, modNumP, appInfoIDP, sortInfoIDP, typeP, creatorP);
  return (Err)iret;
}
