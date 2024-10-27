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

Err CPMLibGenerateKey(UInt16 refnum, UInt8 *keyDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  uint64_t iret;
  pumpkin_system_call_p(CPM_LIB, cpmLibTrapGenerateKey, 0, &iret, NULL, refnum, keyDataP, dataLen, keyInfoP);
  return (Err)iret;
}
