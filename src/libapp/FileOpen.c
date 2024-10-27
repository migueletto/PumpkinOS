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

FileHand FileOpen(UInt16 cardNo, const Char *nameP, UInt32 type, UInt32 creator, UInt32 openMode, Err *errP) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapFileOpen, 0, NULL, &pret, cardNo, nameP, type, creator, openMode, errP);
  return (FileHand)pret;
}
