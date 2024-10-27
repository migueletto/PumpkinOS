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

Err PdiWritePropertyBinaryValue(UInt16 libRefnum, PdiWriterType *ioWriter, const Char *buffer, UInt16 size, UInt16 options) {
  uint64_t iret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapWritePropertyBinaryValue, 0, &iret, NULL, libRefnum, ioWriter, buffer, size, options);
  return (Err)iret;
}
