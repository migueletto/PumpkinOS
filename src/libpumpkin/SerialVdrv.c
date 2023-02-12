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

Err VDrvOpen(VdrvDataPtr *drvrData, VdrvConfigPtr configP, DrvrHWRcvQPtr rcvQP) {
  debug(DEBUG_ERROR, "PALMOS", "VDrvOpen not implemented");
  return 0;
}

Err VDrvClose(VdrvDataPtr drvrData) {
  debug(DEBUG_ERROR, "PALMOS", "VDrvClose not implemented");
  return 0;
}

UInt16 VDrvStatus(VdrvDataPtr drvrData) {
  debug(DEBUG_ERROR, "PALMOS", "VDrvStatus not implemented");
  return 0;
}

