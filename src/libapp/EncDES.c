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

Err EncDES(UInt8 *srcP, UInt8 *keyP, UInt8 *dstP, Boolean encrypt) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapEncDES, 0, &iret, NULL, srcP, keyP, dstP, encrypt);
  return (Err)iret;
}
