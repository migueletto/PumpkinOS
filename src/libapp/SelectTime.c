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

Boolean SelectTime(TimeType *startTimeP, TimeType *EndTimeP, Boolean untimed, const Char *titleP, Int16 startOfDay, Int16 endOfDay, Int16 startOfDisplay) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSelectTime, 0, &iret, NULL, startTimeP, EndTimeP, untimed, titleP, startOfDay, endOfDay, startOfDisplay);
  return (Boolean)iret;
}
