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

Err CPMLibExportHashInfo(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  uint64_t iret;
  pumpkin_system_call_p(CPM_LIB, cpmLibTrapExportHashInfo, 0, &iret, NULL, refnum, hashInfoP, encoding, exportDataP, dataLenP);
  return (Err)iret;
}
