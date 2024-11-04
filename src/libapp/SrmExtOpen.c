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

Err SrmExtOpen(UInt32 port, SrmOpenConfigType *configP, UInt16 configSize, UInt16 *newPortIdP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSerialDispatch, sysSerialOpenV4, &iret, NULL, port, configP, configSize, newPortIdP);
  return (Err)iret;
}