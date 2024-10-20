#include <PalmOS.h>
#include <ExpansionMgr.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define SLOT_NUM 1

Err ExpInit(void) {
  // system use only
  return errNone;
}

Err ExpSlotDriverInstall(UInt32 dbCreator, UInt16 *slotLibRefNumP) {
  debug(DEBUG_ERROR, "PALMOS", "ExpSlotDriverInstall not implemented");
  return sysErrNoFreeLibSlots;
}

Err ExpSlotDriverRemove(UInt16 slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "ExpSlotDriverRemove not implemented");
  return sysErrParamErr;
}

Err ExpSlotLibFind(UInt16 slotRefNum, UInt16 *slotLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "ExpSlotLibFind not implemented");
  return expErrInvalidSlotRefNum;
}

Err ExpSlotRegister(UInt16 slotLibRefNum, UInt16 *slotRefNum) {
  // system use only
  return errNone;
}

Err ExpSlotUnregister(UInt16 slotRefNum) {
  // system use only
  return errNone;
}

Err ExpCardInserted(UInt16 slotRefNum) {
  // system use only
  return errNone;
}

Err ExpCardRemoved(UInt16 slotRefNum) {
  // system use only
  return errNone;
}

Err ExpCardPresent(UInt16 slotRefNum) {
  return slotRefNum == SLOT_NUM ? errNone : expErrCardNotPresent;
}

Err ExpCardInfo(UInt16 slotRefNum, ExpCardInfoType *infoP) {
  Err err = expErrInvalidSlotRefNum;

  if (slotRefNum == SLOT_NUM && infoP) {
    MemSet(infoP, sizeof(ExpCardInfoType), 0);
    infoP->capabilityFlags = expCapabilityHasStorage;
    StrCopy(infoP->manufacturerStr, SYSTEM_NAME);
    StrCopy(infoP->productStr, "Card");
    StrCopy(infoP->deviceClassStr, "Storage");
    StrCopy(infoP->deviceUniqueIDStr, "");
    err = errNone;
  }

  return err;
}

Err ExpSlotEnumerate(UInt16 *slotRefNumP, UInt32 *slotIteratorP) {
  Err err = sysErrParamErr;

  if (slotRefNumP && slotIteratorP && *slotIteratorP == expIteratorStart) {
    *slotRefNumP = SLOT_NUM;
    *slotIteratorP = expIteratorStop;
    err = errNone;
  }

  return err;
}

Err ExpCardGetSerialPort(UInt16 slotRefNum, UInt32* portP) {
  debug(DEBUG_ERROR, "PALMOS", "ExpCardGetSerialPort not implemented");
  return expErrInvalidSlotRefNum;
}
