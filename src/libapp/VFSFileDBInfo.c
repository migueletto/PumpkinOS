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

Err VFSFileDBInfo(FileRef ref, Char *nameP, UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP, UInt32 *bckUpDateP, UInt32 *modNumP, MemHandle *appInfoHP, MemHandle *sortInfoHP, UInt32 *typeP, UInt32 *creatorP, UInt16 *numRecordsP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapVFSMgr, vfsTrapFileDBInfo, &iret, NULL, ref, nameP, attributesP, versionP, crDateP, modDateP, bckUpDateP, modNumP, appInfoHP, sortInfoHP, typeP, creatorP, numRecordsP);
  return (Err)iret;
}
