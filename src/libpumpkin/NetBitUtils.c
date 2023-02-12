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

UInt32 	NetLibBitGetUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP) {
  debug(DEBUG_ERROR, "PALMOS", "NetLibBitGetUIntV not implemented");
  return 0;
}

Int32 	NetLibBitGetIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP) {
  debug(DEBUG_ERROR, "PALMOS", "NetLibBitGetIntV not implemented");
  return 0;
}

