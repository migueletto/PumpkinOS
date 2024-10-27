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

Err FSVolumeInfo(UInt16 fsLibRefNum, UInt16 volRefNum, VolumeInfoType *volInfoP) {
  uint64_t iret;
  pumpkin_system_call_p(FS_LIB, FSTrapVolumeInfo, 0, &iret, NULL, fsLibRefNum, volRefNum, volInfoP);
  return (Err)iret;
}
