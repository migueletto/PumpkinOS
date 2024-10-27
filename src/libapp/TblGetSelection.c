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

Boolean TblGetSelection(const TableType *tableP, Int16 *rowP, Int16 *columnP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapTblGetSelection, 0, &iret, NULL, tableP, rowP, columnP);
  return (Boolean)iret;
}
