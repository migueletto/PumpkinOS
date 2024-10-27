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

Err INetLibSockHTTPReqSend(UInt16 libRefnum, MemHandle sockH, void *writeP, UInt32 writeLen, Int32 timeout) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapSockHTTPReqSend, 0, &iret, NULL, libRefnum, sockH, writeP, writeLen, timeout);
  return (Err)iret;
}
