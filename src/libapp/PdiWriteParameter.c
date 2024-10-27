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

Err PdiWriteParameter(UInt16 libRefnum, PdiWriterType *ioWriter, UInt16 parameter, Boolean parameterName) {
  uint64_t iret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapWriteParameter, 0, &iret, NULL, libRefnum, ioWriter, parameter, parameterName);
  return (Err)iret;
}
