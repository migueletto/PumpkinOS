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

PdiWriterType *PdiWriterNew(UInt16 libRefnum, UDAWriterType *output, UInt16 version) {
  void *pret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapWriterNew, 0, NULL, &pret, libRefnum, output, version);
  return (PdiWriterType *)pret;
}
