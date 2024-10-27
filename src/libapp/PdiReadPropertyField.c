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

Err PdiReadPropertyField(UInt16 libRefnum, PdiReaderType *ioReader, Char * *bufferPP, UInt16 bufferSize, UInt16 readMode) {
  uint64_t iret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapReadPropertyField, 0, &iret, NULL, libRefnum, ioReader, bufferPP, bufferSize, readMode);
  return (Err)iret;
}
