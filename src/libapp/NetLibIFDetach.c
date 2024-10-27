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

Err NetLibIFDetach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapIFDetach, 0, &iret, NULL, libRefNum, ifCreator, ifInstance, timeout);
  return (Err)iret;
}
