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

Err CPMLibDecrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  uint64_t iret;
  pumpkin_system_call_p(CPM_LIB, cpmLibTrapDecrypt, 0, &iret, NULL, refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
  return (Err)iret;
}
