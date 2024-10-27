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

Int16 SslReceive(UInt16 refnum, SslContext *ctx, void *buffer, UInt16 bufferLen, UInt16 flags, void *fromAddr, UInt16 fromLen, Int32 timeout, Err *errRet) {
  uint64_t iret;
  pumpkin_system_call_p(SSL_LIB, kSslReceive, 0, &iret, NULL, refnum, ctx, buffer, bufferLen, flags, fromAddr, fromLen, timeout, errRet);
  return (Int16)iret;
}
