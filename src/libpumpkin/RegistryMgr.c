#include <PalmOS.h>

#include "mutex.h"
#include "storage.h"
#include "RegistryMgr.h"
#include "debug.h"

#define REGISTRY_DB "RegistryDB"
#define sysFileTRegistry 'regt'

struct RegMgrType {
  mutex_t *mutex;
};

RegMgrType *RegInit(void) {
  DmSearchStateType stateInfo;
  DmOpenRef reg_dbRef, dbRef;
  MemHandle h;
  Boolean newSearch;
  UInt16 cardNo, numRecs, imported, index;
  LocalID reg_dbID, dbID;
  DmResType resType;
  DmResID resID;
  RegMgrType *rm;
  char name[dmDBNameLength], stype[8];
  void *r;

  if ((rm = sys_calloc(1, sizeof(RegMgrType))) != NULL) {
    if ((rm->mutex = mutex_create("RegMgr")) != NULL) {
      if ((reg_dbID = DmFindDatabase(0, REGISTRY_DB)) == 0) {
        // create RegistryDB if it does not exist
        debug(DEBUG_INFO, "Registry", "creating %s", REGISTRY_DB);
        DmCreateDatabase(0, REGISTRY_DB, sysFileCSystem, sysFileTRegistry, true);
        reg_dbID = DmFindDatabase(0, REGISTRY_DB);
      }

      if (reg_dbID) {
        // open RegistryDB in write mode
        if ((reg_dbRef = DmOpenDatabase(0, reg_dbID, dmModeWrite)) != NULL) {
          // search other registry databases (if any)
          for (newSearch = true;; newSearch = false) {
            if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, sysFileTRegistry, sysFileCSystem, false, &cardNo, &dbID) != errNone) break;
            // ignore database if it is RegistryDB
            if (dbID == reg_dbID) continue;
            if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != errNone) continue;
            // open the database in read mode
            if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
              numRecs = DmNumRecords(dbRef);
              debug(DEBUG_INFO, "Registry", "importing %u registry entries from database \"%s\"", numRecs, name);
              // import all resources from the database into RegistryDB
              for (index = 0, imported = 0; index < numRecs; index++) {
                if (DmResourceInfo(dbRef, index, &resType, &resID, NULL) == errNone) {
                  if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
                    if ((r = MemHandleLock(h)) != NULL) {
                      pumpkin_id2s(resType, stype);
                      debug(DEBUG_INFO, "Registry", "importing registry '%s' %u", stype, resID);
                      DmNewResourceEx(reg_dbRef, resType, resID, MemHandleSize(h), r);
                      MemHandleUnlock(h);
                      imported++;
                    }
                    DmReleaseResource(h);
                  }
                }
              }
              debug(DEBUG_INFO, "Registry", "imported %u registry entries from \"%s\"", imported, name);
              // close the database
              DmCloseDatabase(dbRef);
            }
            // remove the database, now that all its resources were imported
            debug(DEBUG_INFO, "Registry", "removing database \"%s\"", name);
            DmDeleteDatabase(0, dbID);
          }
          // close RegistryDB
          DmCloseDatabase(reg_dbRef);
        }
      } else {
        sys_free(rm);
        rm = NULL;
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
