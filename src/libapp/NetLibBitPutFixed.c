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

void NetLibBitPutFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt32 value, UInt16 numBits) {
  pumpkin_system_call_p(NET_LIB, netLibTrapBitPutFixed, 0, NULL, NULL, libRefNum, dstP, dstBitOffsetP, value, numBits);
}
