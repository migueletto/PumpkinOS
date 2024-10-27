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

WinHandle WinCreateBitmapWindow(BitmapType *bitmapP, UInt16 *error) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapWinCreateBitmapWindow, 0, NULL, &pret, bitmapP, error);
  return (WinHandle)pret;
}
