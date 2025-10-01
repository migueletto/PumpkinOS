#include <PalmOS.h>

#include "mutex.h"
#include "vfs.h"
#include "debug.h"

#define DATAMGR_FILE_ELEMENT 1
#define DATAMGR_FILE_HEADER  2
#define DATAMGR_FILE_INDEX   3
#define DATAMGR_FILE_DATA    4
#define DATAMGR_FILE_AINFO   5
#define DATAMGR_FILE_SINFO   6
#define DATAMGR_FILE_LOCK    7

#define MAX_PATH 256

typedef struct {
  mutex_t *mutex;
  char path[MAX_PATH];
  vfs_session_t *session;
} data_module_t;

typedef struct data_db_t {
  uint32_t dbID;
  uint32_t ftype, readCount, writeCount, uniqueIDSeed;
  uint16_t mode, numRecs, protect;
  uint32_t creator, type, crDate, modDate, bckDate, modNum;
  uint32_t attributes, version, appInfoID, sortInfoID;
  char name[dmDBNameLength];
} data_db_t;

int DataMgrInitModule(mutex_t *mutex, char *path) {
  data_module_t *module;

  if ((module = sys_calloc(1, sizeof(data_module_t))) == NULL) {
    return -1;
  }

  module->mutex = mutex;
  sys_strncpy(module->path, path, MAX_PATH - 1);
  module->session = vfs_open_session();

  pumpkin_set_local_storage(data_key, module);

  return 0;
}

int DataMgrFinishModule(void) {
  data_module_t *module = (data_module_t *)pumpkin_get_local_storage(data_key);

  if (module) {
    if (module->session) vfs_close_session(module->session);
    sys_free(module);
  }

  return 0;
}

static void DataMgrName(char *path, char *name, int file, int id, uint32_t type, uint8_t attr, uint32_t uniqueId, char *buf) {
  char st[8];
  int n, i;

  sys_snprintf(buf, VFS_PATH - 1, "%s%s", path, name);
  n = sys_strlen(buf);

  switch (file) {
    case DATAMGR_FILE_HEADER:
      sys_strncat(buf, "/header", VFS_PATH-n-1);
      break;
    case DATAMGR_FILE_INDEX:
      sys_strncat(buf, "/index", VFS_PATH-n-1);
      break;
    case DATAMGR_FILE_DATA:
      sys_strncat(buf, "/data", VFS_PATH-n-1);
      break;
    case DATAMGR_FILE_AINFO:
      sys_strncat(buf, "/appInfo", VFS_PATH-n-1);
      break;
    case DATAMGR_FILE_SINFO:
      sys_strncat(buf, "/sortInfo", VFS_PATH-n-1);
      break;
    case DATAMGR_FILE_LOCK:
      sys_strncat(buf, "/lock", VFS_PATH-n-1);
      break;
    case DATAMGR_FILE_ELEMENT:
      if (type) {
        pumpkin_id2s(type, st);
        for (i = 0; i < 4; i++) {
          if (!((st[i] >= 'a' && st[i] <= 'z') || (st[i] >= 'A' && st[i] <= 'Z') || (st[i] >= '0' && st[i] <= '9'))) st[i] = '_';
        }
        sys_snprintf(&buf[n], VFS_PATH-n-1, "/%s.%08X.%d", st, type, id);
      } else {
        sys_snprintf(&buf[n], VFS_PATH-n-1, "/%08X.%02X", uniqueId, attr);
      }
      break;
    case 0:
      break;
    default:
      debug(DEBUG_ERROR, "DataMgr", "name \"%s\" invalid file type %d", name, file);
      ErrFatalDisplayEx("invalid file type", 1);
      break;
  }
}

static int DataMgrReadHeader(data_module_t *module, char *name, data_db_t *db) {
  char buf[VFS_PATH];
  char stype[8], screator[8];
  vfs_file_t *f;
  int r = -1;

  DataMgrName(module->path, name, DATAMGR_FILE_HEADER, 0, 0, 0, 0, buf);
  if ((f = vfs_open(module->session, buf, VFS_READ)) != NULL) {
    sys_memset(buf, 0, sizeof(buf));
    if (vfs_read(f, (uint8_t *)buf, sizeof(buf)-1) > 0) {
      if (sys_sscanf(buf, "name=\"%s\"\nftype=%u\ntype='%4s'\ncreator='%4s'\nattributes=%u\nuniqueIDSeed=%u\nversion=%u\ncrDate=%u\nmodDate=%u\nbckDate=%u\nmodNum=%d\n",
          db->name, &db->ftype, stype, screator, &db->attributes, &db->uniqueIDSeed, &db->version, &db->crDate, &db->modDate, &db->bckDate, &db->modNum) == 11) {
        pumpkin_s2id(&db->type, stype);
        pumpkin_s2id(&db->creator, screator);
        r = 0;
      } else {
        debug(DEBUG_ERROR, "DataMgr", "invalid header \"%s\"", buf);
      }
    }
    vfs_close(f);
  }

  return r;
}

Err DmInit(void) {
  return 0;
}

Err DmCreateDatabase(UInt16 cardNo, const Char *nameP, UInt32 creator, UInt32 type, Boolean resDB) {
  return 0;
}

Err DmCreateDatabaseFromImage(MemPtr bufferP) {
  return 0;
}

Err DmDeleteDatabase(UInt16 cardNo, LocalID dbID) {
  data_module_t *module = (data_module_t *)pumpkin_get_local_storage(data_key);
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  data_db_t db;
  SysNotifyParamType notify;
  SysNotifyDBDeletedType dbDeleted;
  char buf[VFS_PATH], buf2[VFS_PATH];
  Err err = errNone;

  if (mutex_lock(module->mutex) == 0) {
    if ((dir = vfs_opendir(module->session, module->path)) != NULL) {
      for (;;) {
        ent = vfs_readdir(dir);
        if (ent == NULL) {
          vfs_closedir(dir);
          break;
        }
        if (DataMgrReadHeader(module, ent->name, &db) != 0) continue;
        if (db.dbID != dbID) continue;
        if (db.readCount > 0 || db.writeCount > 0) {
          vfs_closedir(dir);
          debug(DEBUG_ERROR, "DataMgr", "DmDeleteDatabase database dbID %u name \"%s\" is open", dbID, db.name);
          break;
        }
        vfs_closedir(dir);
        debug(DEBUG_INFO, "DataMgr", "DmDeleteDatabase database dbID %u name \"%s\"", dbID, db.name);
        DataMgrName(module->path, db.name, 0, 0, 0, 0, 0, buf);
        if ((dir = vfs_opendir(module->session, buf)) != NULL) {
          for (;;) {
            ent = vfs_readdir(dir);
            if (ent == NULL) break;
            if (ent->type != VFS_FILE) continue;
            if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue;
            sys_strncpy(buf2, buf, VFS_PATH-1);
            sys_strncat(buf2, "/", VFS_PATH-sys_strlen(buf2)-1);
            sys_strncat(buf2, ent->name, VFS_PATH-sys_strlen(buf2)-1);
            vfs_unlink(module->session, buf2);
          }
          vfs_closedir(dir);
        }
        DataMgrName(module->path, db.name, 0, 0, 0, 0, 0, buf);
        vfs_unlink(module->session, buf);

        sys_memset(&dbDeleted, 0, sizeof(dbDeleted));
        dbDeleted.oldDBID = 0; // dbID's are not advertised, since they are local to the task
        dbDeleted.creator = db.creator;
        dbDeleted.type = db.type;
        dbDeleted.attributes = db.attributes;
        sys_strncpy(dbDeleted.dbName, db.name, dmDBNameLength-1);

        sys_memset(&notify, 0, sizeof(notify));
        notify.notifyType = sysNotifyDBDeletedEvent;
        notify.broadcaster = 0;
        notify.notifyDetailsP = &dbDeleted;
        SysNotifyBroadcast(&notify);
        break;
      }
    }
    mutex_unlock(module->mutex);
  }

  return err;
}

UInt16 DmNumDatabases(UInt16 cardNo) {
  data_module_t *module = (data_module_t *)pumpkin_get_local_storage(data_key);
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  data_db_t db;
  UInt16 numDatabases = 0;

  if (mutex_lock(module->mutex) == 0) {
    if ((dir = vfs_opendir(module->session, module->path)) != NULL) {
      for (;;) {
        ent = vfs_readdir(dir);
        if (ent == NULL) break;
        if (DataMgrReadHeader(module, ent->name, &db) != 0) continue;
        numDatabases++;
      }
      vfs_closedir(dir);
    }
    mutex_unlock(module->mutex);
  }

  return numDatabases;
}

LocalID DmGetDatabase(UInt16 cardNo, UInt16 index) {
  return 0;
}

LocalID DmFindDatabase(UInt16 cardNo, const Char *nameP) {
  return 0;
}

Err DmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP, UInt32 type, UInt32 creator, Boolean onlyLatestVers, UInt16 *cardNoP, LocalID *dbIDP) {
  return 0;
}

Err DmDatabaseInfo(UInt16 cardNo, LocalID dbID, Char *nameP, UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP, UInt32 *bckUpDateP, UInt32 *modNumP, LocalID *appInfoIDP, LocalID *sortInfoIDP, UInt32 *typeP, UInt32 *creatorP) {
  return 0;
}

Err DmSetDatabaseInfo(UInt16 cardNo, LocalID dbID, const Char *nameP, UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP, UInt32 *bckUpDateP, UInt32 *modNumP, LocalID *appInfoIDP, LocalID *sortInfoIDP, UInt32 *typeP, UInt32 *creatorP) {
  return 0;
}

Err DmDatabaseSize(UInt16 cardNo, LocalID dbID, UInt32 *numRecordsP, UInt32 *totalBytesP, UInt32 *dataBytesP) {
  return 0;
}

Err DmDatabaseProtect(UInt16 cardNo, LocalID dbID, Boolean protect) {
  return 0;
}

DmOpenRef DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  return 0;
}

DmOpenRef DmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode) {
  return 0;
}

DmOpenRef DmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  return 0;
}

Err DmCloseDatabase(DmOpenRef dbP) {
  return 0;
}

DmOpenRef DmNextOpenDatabase(DmOpenRef currentP) {
  return 0;
}

Err DmOpenDatabaseInfo(DmOpenRef dbP, LocalID *dbIDP, UInt16 *openCountP, UInt16 *modeP, UInt16 *cardNoP, Boolean *resDBP) {
  return 0;
}

LocalID DmGetAppInfoID(DmOpenRef dbP) {
  return 0;
}

void DmGetDatabaseLockState(DmOpenRef dbR, UInt8 *highest, UInt32 *count, UInt32 *busy) {
}

Err DmResetRecordStates(DmOpenRef dbP) {
  return 0;
}

Err DmGetLastErr(void) {
  return 0;
}

UInt16 DmNumRecords(DmOpenRef dbP) {
  return 0;
}

UInt16 DmNumRecordsInCategory(DmOpenRef dbP, UInt16 category) {
  return 0;
}

Err DmRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP, LocalID *chunkIDP) {
  return 0;
}

Err DmSetRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP) {
  return 0;
}

Err DmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP) {
  return 0;
}

Err DmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  return 0;
}

Err DmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to) {
  return 0;
}

MemHandle DmNewRecord(DmOpenRef dbP, UInt16 *atP, UInt32 size) {
  return 0;
}

Err DmRemoveRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

Err DmDeleteRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

Err DmArchiveRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

MemHandle DmNewHandle(DmOpenRef dbP, UInt32 size) {
  return 0;
}

Err DmRemoveSecretRecords(DmOpenRef dbP) {
  return 0;
}

Err DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, UInt16 *indexP) {
  return 0;
}

MemHandle DmQueryRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

MemHandle DmGetRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

MemHandle DmQueryNextInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 category) {
  return 0;
}

UInt16 DmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category) {
  return 0;
}

Err DmSeekRecordInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category) {
  return 0;
}

MemHandle DmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize) {
  return 0;
}

Err DmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty) {
  return 0;
}

UInt16 DmSearchRecord(MemHandle recH, DmOpenRef *dbPP) {
  return 0;
}

Err DmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty) {
  return 0;
}

Err DmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum) {
  return 0;
}

Err DmWriteCheck(void *recordP, UInt32 offset, UInt32 bytes) {
  return 0;
}

Err DmWrite(void *recordP, UInt32 offset, const void *srcP, UInt32 bytes) {
  return 0;
}

Err DmStrCopy(void *recordP, UInt32 offset, const Char *srcP) {
  return 0;
}

Err DmSet(void *recordP, UInt32 offset, UInt32 bytes, UInt8 value) {
  return 0;
}

MemHandle DmGetResource(DmResType type, DmResID resID) {
  return 0;
}

MemHandle DmGet1Resource(DmResType type, DmResID resID) {
  return 0;
}

Err DmReleaseResource(MemHandle resourceH) {
  return 0;
}

MemHandle DmResizeResource(MemHandle resourceH, UInt32 newSize) {
  return 0;
}

DmOpenRef DmNextOpenResDatabase(DmOpenRef dbP) {
  return 0;
}

UInt16 DmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex) {
  return 0;
}

UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH) {
  return 0;
}

UInt16 DmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP) {
  return 0;
}

UInt16 DmNumResources(DmOpenRef dbP) {
  return 0;
}

Err DmResourceInfo(DmOpenRef dbP, UInt16 index, DmResType *resTypeP, DmResID *resIDP, LocalID *chunkLocalIDP) {
  return 0;
}

Err DmSetResourceInfo(DmOpenRef dbP, UInt16 index, DmResType *resTypeP, DmResID *resIDP) {
  return 0;
}

Err DmAttachResource(DmOpenRef dbP, MemHandle newH, DmResType resType, DmResID resID) {
  return 0;
}

Err DmDetachResource(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  return 0;
}

MemHandle DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size) {
  return 0;
}

Err DmRemoveResource(DmOpenRef dbP, UInt16 index) {
  return 0;
}

MemHandle DmGetResourceIndex(DmOpenRef dbP, UInt16 index) {
  return 0;
}

Err DmQuickSort(DmOpenRef dbP, DmComparF *compar, Int16 other) {
  return 0;
}

Err DmInsertionSort(DmOpenRef dbR, DmComparF *compar, Int16 other) {
  return 0;
}

UInt16 DmFindSortPosition(DmOpenRef dbP, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other) {
  return 0;
}

UInt16 DmFindSortPositionV10(DmOpenRef dbP, void *newRecord, DmComparF *compar, Int16 other) {
  return 0;
}
