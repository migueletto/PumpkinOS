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

IrStatus IrTestReq(UInt16 refNum, IrDeviceAddr devAddr, IrConnect *con, IrPacket *packet) {
  uint64_t iret;
  pumpkin_system_call_p(IR_LIB, irLibTrapTestReq, 0, &iret, NULL, refNum, devAddr, con, packet);
  return (IrStatus)iret;
}
