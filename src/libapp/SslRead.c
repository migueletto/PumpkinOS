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

Int32 SslRead(UInt16 refnum, SslContext *ctx, void *buffer, Int32 bufferLen, Err *errRet) {
  uint64_t iret;
  pumpkin_system_call_p(SSL_LIB, kSslRead, 0, &iret, NULL, refnum, ctx, buffer, bufferLen, errRet);
  return (Int32)iret;
}
