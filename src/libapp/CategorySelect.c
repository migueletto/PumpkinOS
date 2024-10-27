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

Boolean CategorySelect(DmOpenRef db, const FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, UInt16 *categoryP, Char *categoryName, UInt8 numUneditableCategories, UInt32 editingStrID) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapCategorySelect, 0, &iret, NULL, db, frm, ctlID, lstID, title, categoryP, categoryName, numUneditableCategories, editingStrID);
  return (Boolean)iret;
}
