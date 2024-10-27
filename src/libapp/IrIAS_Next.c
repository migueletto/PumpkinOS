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

UInt8 *IrIAS_Next(UInt16 refNum, IrIasQuery *token) {
  void *pret;
  pumpkin_system_call_p(IR_LIB, irLibTrapIAS_Next, 0, NULL, &pret, refNum, token);
  return (UInt8 *)pret;
}
