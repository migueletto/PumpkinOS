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

PdiDictionary *PdiDefineReaderDictionary(UInt16 libRefnum, PdiReaderType *ioReader, PdiDictionary *dictionary, Boolean disableMainDictionary) {
  void *pret;
  pumpkin_system_call_p(PDI_LIB, PdiLibTrapDefineReaderDictionary, 0, NULL, &pret, libRefnum, ioReader, dictionary, disableMainDictionary);
  return (PdiDictionary *)pret;
}
