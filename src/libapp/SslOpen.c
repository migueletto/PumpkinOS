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

Err SslOpen(UInt16 refnum, SslContext *ctx, UInt16 mode, UInt32 timeout) {
  uint64_t iret;
  pumpkin_system_call_p(SSL_LIB, kSslOpen, 0, &iret, NULL, refnum, ctx, mode, timeout);
  return (Err)iret;
}
