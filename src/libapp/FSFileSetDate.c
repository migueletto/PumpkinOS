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

Err FSFileSetDate(UInt16 fsLibRefNum, FileRef fileRef, UInt16 whichDate, UInt32 date) {
  uint64_t iret;
  pumpkin_system_call_p(FS_LIB, FSTrapFileSetDate, 0, &iret, NULL, fsLibRefNum, fileRef, whichDate, date);
  return (Err)iret;
}
