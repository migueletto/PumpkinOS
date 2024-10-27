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

Err BtLibGetSelectedDevices(UInt16 btLibRefNum, BtLibDeviceAddressType *selectedDeviceArray, UInt8 arraySize, UInt8 *numDevicesReturned) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapGetSelectedDevices, 0, &iret, NULL, btLibRefNum, selectedDeviceArray, arraySize, numDevicesReturned);
  return (Err)iret;
}
