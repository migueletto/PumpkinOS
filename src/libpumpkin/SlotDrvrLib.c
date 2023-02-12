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

Err SlotOpen(UInt16 slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotOpen not implemented");
  return 0;
}

Err SlotClose(UInt16 slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotClose not implemented");
  return 0;
}

Err SlotSleep(UInt16 slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotSleep not implemented");
  return 0;
}

Err SlotWake(UInt16 slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotWake not implemented");
  return 0;
}

UInt32 SlotLibAPIVersion(UInt16 slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotLibAPIVersion not implemented");
  return 0;
}

Err SlotCardPresent(UInt16 slotLibRefNum, UInt16 slotRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardPresent not implemented");
  return 0;
}

Err SlotCardInfo(UInt16 slotLibRefNum, UInt16 slotRefNum, ExpCardInfoType *infoP) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardInfo not implemented");
  return 0;
}

Err SlotCardMediaType(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 *mediaTypeP) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardMediaType not implemented");
  return 0;
}

Err SlotMediaType(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 *mediaTypeP) {
  debug(DEBUG_ERROR, "PALMOS", "SlotMediaType not implemented");
  return 0;
}

Err SlotCardReserve(UInt16 slotLibRefNum, UInt16 slotRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardReserve not implemented");
  return 0;
}

Err SlotCardRelease(UInt16 slotLibRefNum, UInt16 slotRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardRelease not implemented");
  return 0;
}

Err SlotCardGetSerialPort(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32* portP) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardGetSerialPort not implemented");
  return 0;
}

Boolean SlotCardIsFilesystemSupported(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 filesystemType) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardIsFilesystemSupported not implemented");
  return 0;
}

Err SlotCardMetrics(UInt16 slotLibRefNum, UInt16 slotRefNum, CardMetricsPtr cardMetricsP) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardMetrics not implemented");
  return 0;
}

Err SlotCardLowLevelFormat(UInt16 slotLibRefNum, UInt16 slotRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "SlotCardLowLevelFormat not implemented");
  return 0;
}

