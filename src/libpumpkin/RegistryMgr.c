#include <PalmOS.h>

#include "mutex.h"
#include "storage.h"
#include "RegistryMgr.h"

#define REGISTRY_DB "RegistryDB"
#define sysFileTRegistry 'regt'

struct RegMgrType {
  mutex_t *mutex;
};

RegMgrType *RegInit(void) {
  RegMgrType *rm;

  if ((rm = sys_calloc(1, sizeof(RegMgrType))) != NULL) {
    if ((rm->mutex = mutex_create("RegMgr")) != NULL) {
      if ((DmFindDatabase(0, REGISTRY_DB)) == 0) {
        if (DmCreateDatabase(0, REGISTRY_DB, sysFileCSystem, sysFileTRegistry, true) == errNone) {
        }
      }
    } else {
      sys_free(rm);
      rm = NULL;
    }
  }

  return rm;
}

void RegFinish(RegMgrType *rm) {
  if (rm) {
    if (rm->mutex) mutex_destroy(rm->mutex);
    sys_free(rm);
  }
}

void *RegGet(RegMgrType *rm, DmResType type, UInt16 id, UInt32 *size) {
  LocalID dbID;
  DmOpenRef dbRef;
  UInt16 index;
  MemHandle h;
  void *p, *r = NULL;

  if (rm && size && mutex_lock(rm->mutex) == 0) {
    if ((dbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        if ((index = DmFindResource(dbRef, type, id, NULL)) != 0xFFFF) {
          if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
            *size = MemHandleSize(h);
            if ((r = MemHandleLock(h)) != NULL) {
              if ((p = MemPtrNew(*size)) != NULL) {
                MemMove(p, r, *size);
                r = errNone;
              }
              MemHandleUnlock(h);
            }
            DmReleaseResource(h);
          }
        }
        DmCloseDatabase(dbRef);
      }
    }
    mutex_unlock(rm->mutex);
  }

  return r;
}

Err RegSet(RegMgrType *rm, DmResType type, UInt16 id, void *p, UInt32 size) {
  LocalID dbID;
  DmOpenRef dbRef;
  UInt32 currentSize;
  UInt16 index;
  MemHandle h;
  void *r;
  Err err = -1;

  if (rm && mutex_lock(rm->mutex) == 0) {
    if ((dbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadWrite)) != NULL) {
        if ((index = DmFindResource(dbRef, type, id, NULL)) == 0xFFFF) {
          h = DmNewResourceEx(dbRef, type, id, size, p);
          index = DmFindResource(dbRef, 0, 0, h);
        }
        if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
          currentSize = MemHandleSize(h);
          if (size != currentSize) {
            DmResizeResource(h, size); 
          }
          if ((r = MemHandleLock(h)) != NULL) {
            DmWrite(r, 0, p, size);
            err = errNone;
            MemHandleUnlock(h);
          }
          DmReleaseResource(h);
        }
        DmCloseDatabase(dbRef);
      }
    }
    mutex_unlock(rm->mutex);
  }

  return err;
}
