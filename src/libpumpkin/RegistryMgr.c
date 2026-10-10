#include <PalmOS.h>

#include "mutex.h"
#include "storage.h"
#include "RegistryMgr.h"
#include "debug.h"

#define REGISTRY_DB "RegistryDB"
#define COMPAT_DB   "CompatDB"

#define sysFileTRegistry 'regt'

struct RegMgrType {
  mutex_t *mutex;
};

RegMgrType *RegInit(void) {
  RegMgrType *rm;
  LocalID dbID;

  if ((rm = sys_calloc(1, sizeof(RegMgrType))) != NULL) {
    if ((rm->mutex = mutex_create("RegMgr")) != NULL) {
      if ((dbID = DmFindDatabase(0, REGISTRY_DB)) == 0) {
        // create RegistryDB if it does not exist
        debug(DEBUG_INFO, "Registry", "creating %s", REGISTRY_DB);
        DmCreateDatabase(0, REGISTRY_DB, sysFileCSystem, sysFileTRegistry, true);
        dbID = DmFindDatabase(0, REGISTRY_DB);
      }
      if (dbID == 0) {
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

void RegImport(RegMgrType *rm, UInt32 creator) {
  DmOpenRef regDbReg, compatDbRef;
  UInt16 imported, index, nremove, i;
  LocalID regDbID, compatDbID;
  DmResID resID, remove[lastRegID];
  MemHandle h;
  Char screator[8];
  void *r;

  if (rm && mutex_lock(rm->mutex) == 0) {
    if ((regDbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
      // open RegistryDB in write mode
      if ((regDbReg = DmOpenDatabaseEx(0, regDbID, dmModeWrite, false)) != NULL) {
        if ((compatDbID = DmFindDatabase(0, COMPAT_DB)) != 0) {
          // open CompatDB in read mode
          if ((compatDbRef = DmOpenDatabaseEx(0, compatDbID, dmModeReadOnly, false)) != NULL) {
            pumpkin_id2s(creator, screator);
            debug(DEBUG_INFO, "Registry", "searching registry entries for '%s'", screator);
            // import resources from CompatDB into RegistryDB
            for (i = 0, imported = 0, nremove = 0; ; i++) {
              if ((index = DmFindResourceType(compatDbRef, creator, i)) == 0xFFFF) break;
              if (DmResourceInfo(compatDbRef, index, NULL, &resID, NULL) == errNone) {
                if ((h = DmGetResourceIndex(compatDbRef, index)) != NULL) {
                  if ((r = MemHandleLock(h)) != NULL) {
                    debug(DEBUG_INFO, "Registry", "importing registry '%s' %u", screator, resID);
                    DmNewResourceEx(regDbReg, creator, resID, MemHandleSize(h), r);
                    MemHandleUnlock(h);
                    remove[nremove++] = resID;
                    imported++;
                  }
                  DmReleaseResource(h);
                }
              }
            }
            // close CompatDB
            DmCloseDatabase(compatDbRef);

            if (imported) {
              debug(DEBUG_INFO, "Registry", "imported %u registry entries for '%s'", imported, screator);
              if ((compatDbRef = DmOpenDatabaseEx(0, compatDbID, dmModeWrite, false)) != NULL) {
                for (i = 0; i < nremove; i++) {
                  if ((index = DmFindResource(compatDbRef, creator, remove[i], NULL)) != 0xFFFF) {
                    debug(DEBUG_INFO, "Registry", "removing imported registry '%s' %u", screator, remove[i]);
                    DmRemoveResource(compatDbRef, index);
                  }
                }
                DmCloseDatabase(compatDbRef);
              }
            } else {
              debug(DEBUG_INFO, "Registry", "no registry entries found for '%s'", screator);
            }
          }
        }
        // close RegistryDB
        DmCloseDatabase(regDbReg);
      }
    }
    mutex_unlock(rm->mutex);
  }
}

void RegImportDBs(void) {
  DmSearchStateType stateInfo, appStateInfo;
  DmOpenRef regDbReg, dbRef;
  MemHandle h;
  Boolean newSearch;
  UInt16 cardNo, numRecs, imported, index;
  LocalID regDbID, dbID;
  DmResType resType;
  DmResID resID;
  char name[dmDBNameLength], stype[8];
  void *r;

  if ((regDbID = DmFindDatabase(0, REGISTRY_DB)) != 0) {
    // open RegistryDB in write mode
    if ((regDbReg = DmOpenDatabase(0, regDbID, dmModeWrite)) != NULL) {
      // search other registry databases (if any)
      for (newSearch = true;; newSearch = false) {
        if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, sysFileTRegistry, sysFileCSystem, false, &cardNo, &dbID) != errNone) break;
        // ignore database if it is RegistryDB
        if (dbID == regDbID) continue;
        if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != errNone) continue;
        // open the database in read mode
        if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
          numRecs = DmNumRecords(dbRef);
          debug(DEBUG_INFO, "Registry", "importing %u registry entries from database \"%s\"", numRecs, name);
          // import all resources from the database into RegistryDB
          for (index = 0, imported = 0; index < numRecs; index++) {
            if (DmResourceInfo(dbRef, index, &resType, &resID, NULL) == errNone) {
              pumpkin_id2s(resType, stype);
              // if the app does not exist, ignore the registry entry
              if (DmGetNextDatabaseByTypeCreator(true, &appStateInfo, sysFileTApplication, resType, false, NULL, NULL) != errNone) {
                debug(DEBUG_INFO, "Registry", "ignoring registry '%s' %u", stype, resID);
                continue;
              }
              if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
                if ((r = MemHandleLock(h)) != NULL) {
                  debug(DEBUG_INFO, "Registry", "importing registry '%s' %u", stype, resID);
                  DmNewResourceEx(regDbReg, resType, resID, MemHandleSize(h), r);
                  MemHandleUnlock(h);
                  imported++;
                }
                DmReleaseResource(h);
              }
            }
          }
          debug(DEBUG_INFO, "Registry", "imported %u registry entries from database \"%s\"", imported, name);
          // close the database
          DmCloseDatabase(dbRef);
        }
        // remove the database, now that all its resources were imported
        debug(DEBUG_INFO, "Registry", "removing database \"%s\"", name);
        DmDeleteDatabase(0, dbID);
      }
      // close RegistryDB
      DmCloseDatabase(regDbReg);
    }
  }
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
      if ((dbRef = DmOpenDatabaseEx(0, dbID, dmModeReadOnly, false)) != NULL) {
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
      if ((dbRef = DmOpenDatabaseEx(0, dbID, dmModeReadOnly, false)) != NULL) {
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
      if ((dbRef = DmOpenDatabaseEx(0, dbID, dmModeReadWrite, false)) != NULL) {
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
      if ((dbRef = DmOpenDatabaseEx(0, dbID, dmModeReadWrite, false)) != NULL) {
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
