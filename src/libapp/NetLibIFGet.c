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

Err NetLibIFGet(UInt16 libRefNum, UInt16 index, UInt32 *ifCreatorP, UInt16 *ifInstanceP) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapIFGet, 0, &iret, NULL, libRefNum, index, ifCreatorP, ifInstanceP);
  return (Err)iret;
}
