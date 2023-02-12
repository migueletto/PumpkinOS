#include <PalmOS.h>
#include <BtLib.h>
#include <CPMLib.h>
#include <DLServer.h>
#include <ExpansionMgr.h>
#include <FSLib.h>
#include <INetMgr.h>
#include <PceNativeCall.h>
#include <PdiLib.h>
#include <SerialMgrOld.h>
#include <SerialSdrv.h>
#include <SerialVdrv.h>
#include <SlotDrvrLib.h>
#include <SslLib.h>

#include "debug.h"

TsmFepModeType TsmGetFepMode(void* nullParam) {
  debug(DEBUG_ERROR, "PALMOS", "TsmGetFepMode not implemented");
  return 0;
}

TsmFepModeType TsmSetFepMode(void* nullParam, TsmFepModeType inNewMode) {
  debug(DEBUG_ERROR, "PALMOS", "TsmSetFepMode not implemented");
  return 0;
}

