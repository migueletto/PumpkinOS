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

Err				INetLibClose (UInt16 libRefnum, MemHandle inetH) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibClose not implemented");
  return 0;
}

Err				INetLibSleep (UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibSleep not implemented");
  return 0;
}

Err				INetLibWake (UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibWake not implemented");
  return 0;
}

Err				INetLibSockClose(UInt16 libRefnum, MemHandle socketH) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibSockClose not implemented");
  return 0;
}

Err				INetLibURLCrack(UInt16 libRefnum, UInt8 * urlTextP, INetURLType* urlP) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibURLCrack not implemented");
  return 0;
}

Int16 			INetLibURLsCompare(UInt16 libRefnum, Char * URLStr1, Char * URLStr2) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibURLsCompare not implemented");
  return 0;
}

Boolean 		INetLibWiCmd(UInt16 refNum, UInt16 /*WiCmdEnum*/ cmd, int enableOrX, int y) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibWiCmd not implemented");
  return 0;
}

Boolean 		INetLibWirelessIndicatorCmd(UInt16 refNum, MemHandle inetH, UInt16 /*WiCmdEnum*/ cmd, int enableOrX, int y) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibWirelessIndicatorCmd not implemented");
  return 0;
}

Err				INetLibCheckAntennaState(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibCheckAntennaState not implemented");
  return 0;
}

Err 			INetLibCachePurge(UInt16 libRefnum, MemHandle clientParamH, UInt8 * urlTextP, UInt32 uniqueID) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibCachePurge not implemented");
  return 0;
}

Err 			INetLibPrepareCacheForHistory(UInt16 libRefnum, MemHandle clientParamH) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibPrepareCacheForHistory not implemented");
  return 0;
}

Err				INetLibConfigDelete( UInt16 refNum, UInt16 index) {
  debug(DEBUG_ERROR, "PALMOS", "INetLibConfigDelete not implemented");
  return 0;
}

