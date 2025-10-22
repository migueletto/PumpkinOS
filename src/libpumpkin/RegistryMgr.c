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
  void *r, *p = NULL;

  if (rm && size && mutex_lock(rm->mutex) == 0) {
    if ((dbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        if ((index = DmFindResource(dbRef, type, id, NULL)) != 0xFFFF) {
          if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
            *size = MemHandleSize(h);
            if ((r = MemHandleLock(h)) != NULL) {
              if ((p = MemPtrNew(*size)) != NULL) {
                MemMove(p, r, *size);
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

  return p;
}

void *RegGetById(RegMgrType *rm, UInt16 id, UInt32 *size) {
  LocalID dbID;
  DmOpenRef dbRef;
  MemHandle h;
  UInt32 allocSize, resSize, offset;
  UInt16 i, index;
  UInt8 *p = NULL;
  void *r;

  if (rm && mutex_lock(rm->mutex) == 0) {
    if ((dbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        allocSize = 65536;
        offset = 0;
        p = MemPtrNew(allocSize);
        *size = 0;

        for (i = 0; p; i++) {
          if ((index = DmFindResourceID(dbRef, id, i)) == 0xFFFF) break;
          if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
            if ((r = MemHandleLock(h)) != NULL) {
              resSize = MemHandleSize(h);
              if (offset + resSize > allocSize) {
                allocSize = offset + resSize + 65536;
                if (MemPtrResize(p, allocSize) != errNone) {
                  MemPtrFree(p);
                  p = NULL;
                }
              }
              if (p) {
                MemMove(p + offset, r, resSize);
                offset += resSize;
              }
              MemHandleUnlock(h);
            }
            DmReleaseResource(h);
          }
        }
        DmCloseDatabase(dbRef);

        if (p) {
          if (MemPtrResize(p, offset) == errNone) {
            *size = offset;
          } else {
            MemPtrFree(p);
            p = NULL;
          }
        }
      }
    }
    mutex_unlock(rm->mutex);
  }

  return p;
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

Err RegDelete(RegMgrType *rm, DmResType type) {
  LocalID dbID;
  DmOpenRef dbRef;
  UInt16 i, index;
  Err err = -1;

  if (rm && mutex_lock(rm->mutex) == 0) {
    if ((dbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadWrite)) != NULL) {
        for (i = 0; ; i++) {
          if ((index = DmFindResourceType(dbRef, type, i)) == 0xFFFF) break;
          DmRemoveResource(dbRef, index);
        }
        DmCloseDatabase(dbRef);
      }
    }
    mutex_unlock(rm->mutex);
  }

  return err;
}
