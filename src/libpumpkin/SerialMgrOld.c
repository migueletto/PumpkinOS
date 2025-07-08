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

Err SerOpen(UInt16 refNum, UInt16 port, UInt32 baud) {
  debug(DEBUG_ERROR, "PALMOS", "SerOpen not implemented");
  return 0;
}

Err SerClose(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "SerClose not implemented");
  return 0;
}

Err SerSleep(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "SerSleep not implemented");
  return 0;
}

Err SerWake(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "SerWake not implemented");
  return 0;
}

Err SerGetSettings(UInt16 refNum, SerSettingsPtr settingsP) {
  debug(DEBUG_ERROR, "PALMOS", "SerGetSettings not implemented");
  return 0;
}

Err SerSetSettings(UInt16 refNum, SerSettingsPtr settingsP) {
  debug(DEBUG_ERROR, "PALMOS", "SerSetSettings not implemented");
  return 0;
}

Err SerClearErr(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "SerClearErr not implemented");
  return 0;
}

Err SerSend10(UInt16 refNum, const void * bufP, UInt32 size) {
  debug(DEBUG_ERROR, "PALMOS", "SerSend10 not implemented");
  return 0;
}

Err SerSendWait(UInt16 refNum, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SerSendWait not implemented");
  return 0;
}

Err SerSendCheck(UInt16 refNum, UInt32 * numBytesP) {
  debug(DEBUG_ERROR, "PALMOS", "SerSendCheck not implemented");
  return 0;
}

Err SerSendFlush(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "SerSendFlush not implemented");
  return 0;
}

Err SerReceive10(UInt16 refNum, void * bufP, UInt32 bytes, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceive10 not implemented");
  return 0;
}

Err SerReceiveWait(UInt16 refNum, UInt32 bytes, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceiveWait not implemented");
  return 0;
}

Err SerReceiveCheck(UInt16 refNum, UInt32 * numBytesP) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceiveCheck not implemented");
  return 0;
}

void SerReceiveFlush(UInt16 refNum, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceiveFlush not implemented");
}

Err SerSetReceiveBuffer(UInt16 refNum, void * bufP, UInt16 bufSize) {
  debug(DEBUG_ERROR, "PALMOS", "SerSetReceiveBuffer not implemented");
  return 0;
}

Boolean SerReceiveISP(void) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceiveISP not implemented");
  return 0;
}

Err	SerReceiveWindowOpen(UInt16 refNum, UInt8 ** bufPP, UInt32 * sizeP) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceiveWindowOpen not implemented");
  return 0;
}

Err	SerReceiveWindowClose(UInt16 refNum, UInt32 bytesPulled) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceiveWindowClose not implemented");
  return 0;
}

Err	SerPrimeWakeupHandler(UInt16 refNum, UInt16 minBytes) {
  debug(DEBUG_ERROR, "PALMOS", "SerPrimeWakeupHandler not implemented");
  return 0;
}

Err	SerControl(UInt16 refNum, UInt16 op, void * valueP, UInt16 * valueLenP) {
  debug(DEBUG_ERROR, "PALMOS", "SerControl not implemented");
  return 0;
}

UInt32 SerSend(UInt16 refNum, const void * bufP, UInt32 count, Err* errP) {
  debug(DEBUG_ERROR, "PALMOS", "SerSend not implemented");
  return 0;
}

UInt32 SerReceive(UInt16 refNum, void * bufP, UInt32 count, Int32 timeout, Err* errP) {
  debug(DEBUG_ERROR, "PALMOS", "SerReceive not implemented");
  return 0;
}

UInt16 SerGetStatus(UInt16 refNum, Boolean * ctsOnP, Boolean * dsrOnP) {
  debug(DEBUG_ERROR, "PALMOS", "SerGetStatus not implemented");
  return 0;
}
