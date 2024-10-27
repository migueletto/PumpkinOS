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

Err FSFileWrite(UInt16 fsLibRefNum, FileRef fileRef, UInt32 numBytes, const void *dataP, UInt32 *numBytesWrittenP) {
  uint64_t iret;
  pumpkin_system_call_p(FS_LIB, FSTrapFileWrite, 0, &iret, NULL, fsLibRefNum, fileRef, numBytes, dataP, numBytesWrittenP);
  return (Err)iret;
}
