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

Err PdiWritePropertyStr(UInt16 libRefnum, PdiWriterType *ioWriter, const Char *propertyName, UInt8 writeMode, UInt8 requiredFields) {
  uint64_t iret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapWritePropertyStr, 0, &iret, NULL, libRefnum, ioWriter, propertyName, writeMode, requiredFields);
  return (Err)iret;
}
