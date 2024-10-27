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

PdiDictionary *PdiDefineWriterDictionary(UInt16 libRefnum, PdiWriterType *ioWriter, PdiDictionary *dictionary, Boolean disableMainDictionary) {
  void *pret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapDefineWriterDictionary, 0, NULL, &pret, libRefnum, ioWriter, dictionary, disableMainDictionary);
  return (PdiDictionary *)pret;
}
