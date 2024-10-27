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

Err INetLibCachePurge(UInt16 libRefnum, MemHandle clientParamH, UInt8 *urlTextP, UInt32 uniqueID) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapCachePurge, 0, &iret, NULL, libRefnum, clientParamH, urlTextP, uniqueID);
  return (Err)iret;
}
