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

Int16 NetLibSocketOptionSet(UInt16 libRefnum, NetSocketRef socket, UInt16 level, UInt16 option, void *optValueP, UInt16 optValueLen, Int32 timeout, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapSocketOptionSet, 0, &iret, NULL, libRefnum, socket, level, option, optValueP, optValueLen, timeout, errP);
  return (Int16)iret;
}
