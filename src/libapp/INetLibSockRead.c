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

Err INetLibSockRead(UInt16 libRefnum, MemHandle sockH, void *bufP, UInt32 reqBytes, UInt32 *actBytesP, Int32 timeout) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapSockRead, 0, &iret, NULL, libRefnum, sockH, bufP, reqBytes, actBytesP, timeout);
  return (Err)iret;
}
