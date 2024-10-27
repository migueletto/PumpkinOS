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

ProgressType *PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapPrgStartDialog, 0, NULL, &pret, title, textCallback, userDataP);
  return (ProgressType *)pret;
}
