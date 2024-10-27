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

Err INetLibSockFileGetByIndex(UInt16 libRefnum, MemHandle sockH, UInt32 index, MemHandle *handleP, UInt32 *offsetP, UInt32 *lengthP) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapSockFileGetByIndex, 0, &iret, NULL, libRefnum, sockH, index, handleP, offsetP, lengthP);
  return (Err)iret;
}
