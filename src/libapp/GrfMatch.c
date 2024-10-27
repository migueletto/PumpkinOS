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

Err GrfMatch(UInt16 *flagsP, void *dataPtrP, UInt16 *dataLenP, UInt16 *uncertainLenP, GrfMatchInfoPtr matchInfoP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapGrfMatch, 0, &iret, NULL, flagsP, dataPtrP, dataLenP, uncertainLenP, matchInfoP);
  return (Err)iret;
}
