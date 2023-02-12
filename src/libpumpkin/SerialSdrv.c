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

Err DrvClose(SdrvDataPtr drvrData) {
  debug(DEBUG_ERROR, "PALMOS", "DrvClose not implemented");
  return 0;
}

UInt16 DrvStatus(SdrvDataPtr drvrData) {
  debug(DEBUG_ERROR, "PALMOS", "DrvStatus not implemented");
  return 0;
}

Err DrvWriteChar(SdrvDataPtr drvrData, UInt8 aChar) {
  debug(DEBUG_ERROR, "PALMOS", "DrvWriteChar not implemented");
  return 0;
}

UInt16 DrvReadChar(SdrvDataPtr drvrData) {
  debug(DEBUG_ERROR, "PALMOS", "DrvReadChar not implemented");
  return 0;
}

