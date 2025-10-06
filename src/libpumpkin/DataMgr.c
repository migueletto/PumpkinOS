#include <PalmOS.h>

#include "mutex.h"
#include "vfs.h"
#include "bytes.h"
#include "DDm.h"
#include "debug.h"

#define DATAMGR_FILE_ELEMENT 1
#define DATAMGR_FILE_HEADER  2
#define DATAMGR_FILE_INDEX   3
#define DATAMGR_FILE_DATA    4
#define DATAMGR_FILE_AINFO   5
#define DATAMGR_FILE_SINFO   6
#define DATAMGR_FILE_LOCK    7

#define DATAMGR_TYPE_MEM  1
#define DATAMGR_TYPE_REC  2
#define DATAMGR_TYPE_RES  3
#define DATAMGR_TYPE_FILE 5

#define DATAMGR_MAGIC 'Hndl'

#define DBID_MASK 0x80000000
#define ATTR_MASK (dmRecAttrDelete | dmRecAttrSecret | dmRecAttrCategoryMask)

#define MAX_PATH 256

#define DM_MODULE data_module_t *module = (data_module_t *)pumpkin_get_local_storage(data_key)

#define DM_DIR_BEGIN(dir) \
  if (mutex_lock(module->dm->mutex) == 0) { \
    debug(DEBUG_TRACE, "DataMgr", "opendir [%s]", module->dm->path); \
    if ((dir = vfs_opendir(module->dm->session, module->dm->path)) != NULL) { \
      vfs_ent_t *ent; \
      DmOpenType db; \
      for (;;) { \
        ent = vfs_readdir(dir); \
        if (ent == NULL) break; \
        if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue; \
        if (sys_atoi(ent->name) == 0) continue; \
        debug(DEBUG_TRACE, "DataMgr", "entry [%s]", ent->name); \
        if (DataMgrReadHeader(module, 0, ent->name, &db) != 0) continue;

#define DM_DIR_END(dir) \
      } \
      vfs_closedir(dir); \
      debug(DEBUG_TRACE, "DataMgr", "closedir [%s]", module->dm->path); \
    } \
    mutex_unlock(module->dm->mutex); \
  }

#define DataMgrCheckErr(err) \
  pumpkin_set_lasterr(err); \
  if (module) module->lastErr = err; \
  if (err && err != dmErrResourceNotFound) debug(DEBUG_ERROR, "DataMgr", "%s: error 0x%04X (%s)", __FUNCTION__, err, pumpkin_error_msg(err));

#define CAP_SIZE 32

typedef struct {
  UInt32 uniqueID;
  UInt16 attr;
} DmRecIndex;

typedef struct DmOpenType {
  LocalID dbID;
  UInt16 mode, numRecs, protect;
  UInt32 ftype, readCount, writeCount, uniqueIDSeed;
  UInt32 creator, type, crDate, modDate, bckDate, modNum;
  UInt32 attributes, version, appInfoID, sortInfoID;
  char name[dmDBNameLength];
  DmRecIndex *idxRec;
  UInt32 capRecs;
  UInt32 numHandles, capHandles;
  struct DmHandle **h;
  struct DmOpenType *prev, *next;
} DmOpenType;

typedef struct data_infoid_t {
  LocalID dbID;
  LocalID appInfoID, sortInfoID;
  Boolean updated;
  struct data_infoid_t *next;
} data_infoid_t;

typedef struct {
  DataMgrType *dm;
  DmOpenType *db;
  data_infoid_t *id;
  uint8_t *heapBase;
  UInt32 heapSize;
  DmComparF *comparF;
  UInt32 comparF68K;
  Int16 other;
  MemHandle appInfoH;
  DmOpenType *tmpDb;
  Err lastErr;
} data_module_t;

struct DataMgrType {
  mutex_t *mutex;
  char path[MAX_PATH];
  vfs_session_t *session;
  LocalID nextDbID;
};

typedef struct DmHandle {
  UInt32 magic;
  UInt16 htype;
  UInt16 owner;
  UInt16 useCount;
  UInt16 lockCount;
  UInt32 size;
  LocalID dbID;
  union {
    struct {
      UInt32 uniqueID;
      UInt16 index;
      UInt16 attr;
    } rec;
    struct {
      UInt32 type;
      UInt16 id;
      UInt16 attr;
    } res;
  } d;
  uint8_t *buf;
} DmHandle;

static MemHandle DDmNewRecordEx(DmOpenRef dbP, UInt16 *atP, UInt32 size, void *p, UInt32 uniqueID, UInt16 attr, Boolean writeIndex);

static Boolean DataMgrValidName(UInt8 *buf) {
  Int32 i;

  for (i = 0; i < dmDBNameLength && buf[i]; i++) {
    if (buf[i] < 32) return false;
  }

  return true;
}

static Boolean DataMgrValidTypeCreator(UInt8 *buf) {
  Int32 i;

  for (i = 0; i < 4 && buf[i]; i++) {
    if (buf[i] < 32 || buf[i] > 126) return false;
  }

  return i > 0;
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
        //sys_snprintf(&buf[n], VFS_PATH-n-1, "/%08X.%02X", uniqueId, attr);
        sys_snprintf(&buf[n], VFS_PATH-n-1, "/%08X", uniqueId);
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

static int DataMgrReadHeader(data_module_t *module, LocalID dbID, char *name, DmOpenType *db) {
  char path[VFS_PATH];
  char namebuf[16];
  char *buf, *lf, *s, *value;
  vfs_file_t *f;
  int32_t size, i;
  int r = -1;

  if (name == NULL) {
    sys_snprintf(namebuf, sizeof(namebuf)-1, "%u", dbID & ~DBID_MASK);
    name = namebuf;
  }
  DataMgrName(module->dm->path, name, DATAMGR_FILE_HEADER, 0, 0, 0, 0, path);

  if ((f = vfs_open(module->dm->session, path, VFS_READ)) != NULL) {
    if ((size = vfs_seek(f, 0, 1)) > 0) {
      vfs_seek(f, 0, 0);
      if ((buf = sys_calloc(1, size + 2)) != NULL) {
        if (vfs_read(f, (uint8_t *)buf, size) == size) {
          db->dbID = sys_atoi(name);
          r = 0;
          if (buf[size - 1] != '\n') buf[size] = '\n';
          for (s = buf, i = 1; s[0];) {
            if ((lf = sys_strchr(s, '\n')) == NULL) break;
            *lf = 0;
            if ((value = sys_strchr(s, '=')) != NULL) {
              *value = 0;
              value++;
              if (value[0] == 0) {
                debug(DEBUG_ERROR, "DataMgr", "empty value for option \"%s\" at line %d in \"%s\"", s, i, path);
              } else {
                debug(DEBUG_TRACE, "DataMgr", "option \"%s\" value \"%s\" at line %d", s, value, i);
                if (!sys_strcmp(s, "name")) {
                  sys_strncpy(db->name, value, dmDBNameLength - 1);
                } else if (!sys_strcmp(s, "type")) {
                  pumpkin_s2id(&db->type, value);
                } else if (!sys_strcmp(s, "creator")) {
                  pumpkin_s2id(&db->creator, value);
                } else if (!sys_strcmp(s, "ftype")) {
                  db->ftype = sys_atoi(value);
                } else if (!sys_strcmp(s, "attributes")) {
                  db->attributes = sys_atoi(value);
                } else if (!sys_strcmp(s, "uniqueIDSeed")) {
                  db->uniqueIDSeed = sys_atoi(value);
                } else if (!sys_strcmp(s, "version")) {
                  db->version = sys_atoi(value);
                } else if (!sys_strcmp(s, "crDate")) {
                  db->crDate = sys_atoi(value);
                } else if (!sys_strcmp(s, "modDate")) {
                  db->modDate = sys_atoi(value);
                } else if (!sys_strcmp(s, "bckDate")) {
                  db->bckDate = sys_atoi(value);
                } else if (!sys_strcmp(s, "modNum")) {
                  db->modNum = sys_atoi(value);
                } else {
                  debug(DEBUG_ERROR, "DataMgr", "invalid option \"%s\" at line %d in \"%s\"", s, i, path);
                  r = -1;
                }
              }
            } else {
              debug(DEBUG_ERROR, "DataMgr", "missing '=' for option \"%s\" at line %d in \"%s\"", s, i, path);
              r = -1;
            }
            s = lf + 1;
            i++;
          }
        }
        sys_free(buf);
      }
    }
    vfs_close(f);
  }

  if (r != 0) {
    ErrFatalDisplayEx("write header failed", 1);
  }

  return r;
}

static int DataMgrWriteHeader(data_module_t *module, LocalID dbID, char *name, DmOpenType *db) {
  char buf[VFS_PATH];
  char stype[8], screator[8], namebuf[16];
  vfs_file_t *f;
  int n, r = -1;

  if (name == NULL) {
    sys_snprintf(namebuf, sizeof(namebuf)-1, "%u", dbID & ~DBID_MASK);
    name = namebuf;
  }
  DataMgrName(module->dm->path, name, DATAMGR_FILE_HEADER, 0, 0, 0, 0, buf);

  if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
    pumpkin_id2s(db->type, stype);
    pumpkin_id2s(db->creator, screator);
    sys_snprintf(buf, sizeof(buf)-1,
      "name=%s\nftype=%u\ntype=%4s\ncreator=%4s\nattributes=%u\nuniqueIDSeed=%u\nversion=%u\ncrDate=%u\nmodDate=%u\nbckDate=%u\nmodNum=%d\n",
      db->name, db->ftype, stype, screator, db->attributes, db->uniqueIDSeed, db->version, db->crDate, db->modDate, db->bckDate, db->modNum);
    n = sys_strlen(buf);
    if (vfs_write(f, (uint8_t *)buf, n) == n) {
      r = 0;
    }
    vfs_close(f);
  } else {
    ErrFatalDisplayEx("write header failed", 1);
  }

  return r;
}

static LocalID DataMgrReadInfo(data_module_t *module, LocalID dbID, UInt32 fileType) {
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  vfs_ent_t *ent;
  MemHandle h;
  Boolean ok;
  void *p;
  LocalID infoID = 0;

  sys_snprintf(name, sizeof(name)-1, "%u", dbID & ~DBID_MASK);
  DataMgrName(module->dm->path, name, fileType, 0, 0, 0, 0, buf);

  if (vfs_checktype(module->dm->session, buf) != -1) {
    if ((f = vfs_open(module->dm->session, buf, VFS_READ)) != NULL) {
      if ((ent = vfs_fstat(f)) != NULL) {
        if ((h = DMemHandleNew(ent->size)) != NULL) {
          if ((p = DMemHandleLock(h)) != NULL) {
            ok = vfs_read(f, p, ent->size) == ent->size;
            DMemHandleUnlock(h);
            if (ok) {
              infoID = DMemHandleToLocalID(h);
              debug(DEBUG_INFO, "DataMgr", "read LocalID %u from %s", infoID, buf);
            }
          }
        }
      }
      vfs_close(f);
    }
  }

  return infoID;
}

static void DataMgrReadAppInfo(data_module_t *module, LocalID dbID, LocalID *appInfoID, LocalID *sortInfoID) {
  data_infoid_t *id, *aux;

  for (id = module->id; id; id = id->next) {
    if (id->dbID == dbID) {
      if (!id->updated) {
        id->appInfoID = DataMgrReadInfo(module, dbID, DATAMGR_FILE_AINFO);
        id->sortInfoID = DataMgrReadInfo(module, dbID, DATAMGR_FILE_SINFO);
        id->updated = true;
      }
      if (appInfoID) *appInfoID = id->appInfoID;
      if (sortInfoID) *sortInfoID = id->sortInfoID;
      return;
    }
  }

  if ((id = sys_calloc(1, sizeof(data_infoid_t))) != NULL) {
    id->appInfoID = DataMgrReadInfo(module, dbID, DATAMGR_FILE_AINFO);
    id->sortInfoID = DataMgrReadInfo(module, dbID, DATAMGR_FILE_SINFO);
    id->updated = true;
    aux = module->id;
    module->id = id;
    id->next = aux;
    if (appInfoID) *appInfoID = id->appInfoID;
    if (sortInfoID) *sortInfoID = id->sortInfoID;
  }
}

static void DataMgrWriteInfo(data_module_t *module, LocalID dbID, UInt32 fileType, LocalID infoID) {
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  MemHandle h;
  UInt32 size;
  void *p;

  if ((h = DMemLocalIDToHandle(infoID)) != NULL) {
    if ((p = DMemHandleLock(h)) != NULL) {
      sys_snprintf(name, sizeof(name)-1, "%u", dbID & ~DBID_MASK);
      DataMgrName(module->dm->path, name, fileType, 0, 0, 0, 0, buf);

      if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
        size = DMemHandleSize(h);
        if (vfs_write(f, p, size) == size) {
          debug(DEBUG_INFO, "DataMgr", "wrote LocalID %u to %s", infoID, buf);
        }
        vfs_close(f);
      }
    }
  }
}

static void DataMgrWriteAppInfo(data_module_t *module, LocalID dbID, LocalID *appInfoID, LocalID *sortInfoID) {
  data_infoid_t *id, *aux;

  for (id = module->id; id; id = id->next) {
    if (id->dbID == dbID) {
      if (appInfoID) {
        DataMgrWriteInfo(module, dbID, DATAMGR_FILE_AINFO, *appInfoID);
        id->appInfoID = *appInfoID;
      }
      if (sortInfoID) {
        DataMgrWriteInfo(module, dbID, DATAMGR_FILE_SINFO, *sortInfoID);
        id->sortInfoID = *sortInfoID;
      }
      id->updated = true;
      return;
    }
  }

  if ((id = sys_calloc(1, sizeof(data_infoid_t))) != NULL) {
    if (appInfoID) {
      DataMgrWriteInfo(module, dbID, DATAMGR_FILE_AINFO, *appInfoID);
      id->appInfoID = *appInfoID;
    }
    if (sortInfoID) {
      DataMgrWriteInfo(module, dbID, DATAMGR_FILE_SINFO, *sortInfoID);
      id->sortInfoID = *sortInfoID;
    }
    id->updated = true;
    aux = module->id;
    module->id = id;
    id->next = aux;
  }
}

static void DataMgrInsertIntoIndex(DmOpenType *db, UInt16 index, UInt32 uniqueID, UInt16 attr) {
  UInt16 i;

  if (db->numRecs == db->capRecs) {
    db->capRecs += CAP_SIZE;
    db->idxRec = db->idxRec ?
      sys_realloc(db->idxRec, db->capRecs * sizeof(DmRecIndex)):
      sys_calloc(db->capRecs, sizeof(DmRecIndex));
  }
  db->numRecs++;

  if (index < db->numRecs - 1) {
    for (i = db->numRecs - 1; i > index; i--) {
      db->idxRec[i] = db->idxRec[i - 1];
    }
  }

  db->idxRec[index].uniqueID = uniqueID;
  db->idxRec[index].attr = attr;
}

static int DataMgrRemoveFromIndex(DmOpenType *db, UInt16 index, UInt32 *uniqueID, UInt16 *attr) {
  UInt16 i;
  int r = -1;

  if (db->numRecs > 0 && index < db->numRecs) {
    if (index < db->numRecs - 1) {
      *uniqueID = db->idxRec[index].uniqueID;
      *attr = db->idxRec[index].attr;
      for (i = index; i < db->numRecs - 1; i++) {
        db->idxRec[i] = db->idxRec[i + 1];
      }
    }
    db->numRecs--;
    r = 0;
  }

  return r;
}

static int DataMgrWriteIndex(DmOpenType *db) {
  DM_MODULE;
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  int i, r = -1;

  sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
  DataMgrName(module->dm->path, name, DATAMGR_FILE_INDEX, 0, 0, 0, 0, buf);

  if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
    debug(DEBUG_TRACE, "DataMgr", "write index \"%s\" begin", db->name);
    r = 0;
    for (i = 0; i < db->numRecs; i++) {
      sys_snprintf(buf, sizeof(buf)-1, "%08X.%02X\n", db->idxRec[i].uniqueID, db->idxRec[i].attr & ATTR_MASK);
      debug(DEBUG_TRACE, "DataMgr", "write index %4d: %s", i, buf);
      if (vfs_write(f, (uint8_t *)buf, 12) != 12) {
        r = -1;
        break;
      }
    }
    vfs_close(f);
    debug(DEBUG_TRACE, "DataMgr", "write index \"%s\" end", db->name);
  }

  return r;
}

static int DataMgrReadIndex(DmOpenType *db) {
  DM_MODULE;
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  UInt32 uniqueID, attr;
  int n, r = -1;

  sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
  DataMgrName(module->dm->path, name, DATAMGR_FILE_INDEX, 0, 0, 0, 0, buf);

  if ((f = vfs_open(module->dm->session, buf, VFS_READ)) != NULL) {
    r = 0;
    for (db->numRecs = 0;; db->numRecs++) {
      if ((n = vfs_read(f, (uint8_t *)buf, 12)) == 0) break;
      if (n != 12) {
        r = -1;
        break;
      }
      if (sys_sscanf(buf, "%08X.%02X\n", &uniqueID, &attr) != 2) {
        r = -1;
        break;
      }
      if (db->numRecs == db->capRecs) {
        db->capRecs += CAP_SIZE;
        db->idxRec = db->idxRec ?
          sys_realloc(db->idxRec, db->capRecs * sizeof(DmRecIndex)):
          sys_calloc(db->capRecs, sizeof(DmRecIndex));
      }
      db->idxRec[db->numRecs].uniqueID = uniqueID;
      db->idxRec[db->numRecs].attr = attr;
    }
    vfs_close(f);
  }

  return r;
}

static int DataMgrGetFileLocks(data_module_t *module, DmOpenType *db, int *read_locks, int *write_locks) {
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  int r = -1;

  *read_locks = 0;
  *write_locks = 0;

  sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
  DataMgrName(module->dm->path, name, DATAMGR_FILE_LOCK, 0, 0, 0, 0, buf);

  if (vfs_checktype(module->dm->session, buf) == VFS_FILE) {
    if ((f = vfs_open(module->dm->session, buf, VFS_READ)) != NULL) {
      sys_memset(buf, 0, sizeof(buf));
      if (vfs_read(f, (uint8_t *)buf, sizeof(buf)-1) > 0) {
        if (sys_sscanf(buf, "read=%u\nwrite=%u\n", read_locks, write_locks) == 2) {
          r = 0;
        } else {
          debug(DEBUG_ERROR, "DataMgr", "invalid lock \"%s\"", buf);
        }
      }
      vfs_close(f);
    }
  } else {
    // lock file does not exist
    r = 0;
  }

  return r;
}

static int DataMgrPutFileLocks(data_module_t *module, DmOpenType *db, int read_locks, int write_locks) {
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  int n, r = -1;

  sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
  DataMgrName(module->dm->path, name, DATAMGR_FILE_LOCK, 0, 0, 0, 0, buf);

  if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
    sys_snprintf(buf, sizeof(buf)-1, "read=%u\nwrite=%u\n", read_locks, write_locks);
    n = sys_strlen(buf);
    if (vfs_write(f, (uint8_t *)buf, n) == n) {
      r = 0;
    }
    vfs_close(f);
  } else {
    ErrFatalDisplayEx("create lock failed", 1);
  }

  return r;
}

static int DataMgrLockForWriting(data_module_t *module, DmOpenType *db) {
  int read_locks, write_locks;
  int r = -1;

  if (DataMgrGetFileLocks(module, db, &read_locks, &write_locks) == 0) {
    if (db->readCount < read_locks) {
      debug(DEBUG_ERROR, "DataMgr", "DataMgrLockForWriting file \"%s\" already locked for reading (other)", db->name);
    } else if (db->writeCount < write_locks) {
      debug(DEBUG_ERROR, "DataMgr", "DataMgrLockForWriting file \"%s\" already locked for writing (other)", db->name);
    } else {
      if (db->readCount > 0) {
        debug(DEBUG_INFO, "DataMgr", "DataMgrLockForWriting file \"%s\" already locked for reading (own)", db->name);
      }
      debug(DEBUG_TRACE, "DataMgr", "DataMgrLockForWriting \"%s\" writeCount %d -> %d", db->name, db->writeCount, db->writeCount+1);
      db->writeCount++;
      write_locks++;
      r = DataMgrPutFileLocks(module, db, read_locks, write_locks);
    }
  }

  return r;
}

static int DataMgrLockForReading(data_module_t *module, DmOpenType *db) {
  int read_locks, write_locks;
  int r = -1;

  if (DataMgrGetFileLocks(module, db, &read_locks, &write_locks) == 0) {
    if (db->writeCount < write_locks) {
      debug(DEBUG_ERROR, "DataMgr", "DataMgrLockForReading file \"%s\" already locked for writing (other)", db->name);
    } else {
      if (db->writeCount > 0) {
        debug(DEBUG_INFO, "DataMgr", "DataMgrLockForReading file \"%s\" already locked for writing (own)", db->name);
      }
      debug(DEBUG_TRACE, "DataMgr", "DataMgrLockForReading \"%s\" readCount %d -> %d", db->name, db->readCount, db->readCount+1);
      db->readCount++;
      read_locks++;
      r = DataMgrPutFileLocks(module, db, read_locks, write_locks);
    }
  }

  return r;
}

static int DataMgrUnlockForReading(data_module_t *module, DmOpenType *db) {
  int read_locks, write_locks;
  int r = -1;

  if (db->readCount > 0) {
    if (DataMgrGetFileLocks(module, db, &read_locks, &write_locks) == 0) {
      if (read_locks > 0) {
        debug(DEBUG_TRACE, "DataMgr", "DataMgrUnlockForReading \"%s\" readCount %d -> %d", db->name, db->readCount, db->readCount-1);
        db->readCount--;
        read_locks--;
        r = DataMgrPutFileLocks(module, db, read_locks, write_locks);
      } else {
        debug(DEBUG_ERROR, "DataMgr", "DataMgrUnlockForReading file \"%s\" not locked for reading", db->name);
      }
    }
  } else {
    debug(DEBUG_ERROR, "DataMgr", "DataMgrUnlockForReading file \"%s\" not open for reading", db->name);
  }

  return r;
}

static int DataMgrUnlockForWriting(data_module_t *module, DmOpenType *db) {
  int read_locks, write_locks;
  int r = -1;

  if (db->writeCount > 0) {
    if (DataMgrGetFileLocks(module, db, &read_locks, &write_locks) == 0) {
      if (write_locks > 0) {
        db->writeCount--;
        write_locks--;
        r = DataMgrPutFileLocks(module, db, read_locks, write_locks);
      } else {
        debug(DEBUG_ERROR, "DataMgr", "DataMgrUnlockForWriting file \"%s\" not locked for writing", db->name);
      }
    }
  } else {
    debug(DEBUG_ERROR, "DataMgr", "DataMgrUnlockForWriting file \"%s\" not open for writing", db->name);
  }

  return r;
}

DataMgrType *DataMgrInit(char *path) {
  DataMgrType *dm;
  data_module_t m, *module;
  LocalID dbID, maxID;
  vfs_dir_t *dir;

  if ((dm = sys_calloc(1, sizeof(DataMgrType))) != NULL) {
    debug(DEBUG_INFO, "DataMgr", "DataMgr init path \"%s\"", path);
    sys_strncpy(dm->path, path, MAX_PATH - 1);
    dm->mutex = mutex_create("DataMgr");
    dm->session = vfs_open_session();

    m.dm = dm;
    module = &m;

    maxID = 0;
    DM_DIR_BEGIN(dir) {
      dbID = sys_atoi(ent->name);
      if (dbID > maxID) maxID = dbID;
    } DM_DIR_END(dir);
    dm->nextDbID = (maxID + 1) | DBID_MASK;
    debug(DEBUG_INFO, "DataMgr", "next dbID is %u", dm->nextDbID & ~DBID_MASK);
  }

  return dm;
}

void DataMgrFinish(DataMgrType *dm) {
  if (dm) {
    debug(DEBUG_INFO, "DataMgr", "DataMgr finish");
    if (dm->session) vfs_close_session(dm->session);
    if (dm->mutex) mutex_destroy(dm->mutex);
    sys_free(dm);
  }
}

int DataMgrInitModule(DataMgrType *dm) {
  data_module_t *module;
  int r = -1;

  if ((module = sys_calloc(1, sizeof(data_module_t))) != NULL) {
    module->dm = dm;
    module->heapBase = (uint8_t *)pumpkin_heap_base();
    module->heapSize = pumpkin_heap_size();
    pumpkin_set_local_storage(data_key, module);
    r = 0;
  }

  return r;
}

void DataMgrFinishModule(void) {
  DM_MODULE;

  if (module) {
    sys_free(module);
  }
}

Err DDmInit(void) {
  return 0;
}

Err DDmCreateDatabaseEx(const Char *nameP, UInt32 creator, UInt32 type, UInt16 attr, UInt32 uniqueIDSeed, Boolean overwrite) {
  DM_MODULE;
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  LocalID dbID;
  DmOpenType db;
  SysNotifyParamType notify;
  SysNotifyDBCreatedType dbCreated;
  Err err = dmErrInvalidParam;

  if (nameP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      dbID = DDmFindDatabase(0, nameP);

      if (dbID) {
        db.dbID = dbID;
        if (overwrite) {
          debug(DEBUG_INFO, "DataMgr", "DmCreateDatabase overwriting database \"%s\"", nameP);
          if ((err = DDmDeleteDatabase(0, dbID)) != errNone) {
            mutex_unlock(module->dm->mutex);
            DataMgrCheckErr(err);
            return err;
          }
        } else {
          debug(DEBUG_ERROR, "DataMgr", "DmCreateDatabase database \"%s\" already exists", nameP);
          mutex_unlock(module->dm->mutex);
          DataMgrCheckErr(err);
          return err;
        }
      } else {
        db.dbID = module->dm->nextDbID;
        module->dm->nextDbID++;
        debug(DEBUG_INFO, "DataMgr", "DmCreateDatabase creating database \"%s\"", nameP);
        sys_snprintf(name, sizeof(name)-1, "%u", db.dbID & ~DBID_MASK);
        DataMgrName(module->dm->path, name, 0, 0, 0, 0, 0, buf);
        if (vfs_mkdir(module->dm->session, buf) == -1) {
          mutex_unlock(module->dm->mutex);
          DataMgrCheckErr(err);
          return err;
        }
      }

      sys_strncpy(db.name, nameP, dmDBNameLength-1);
      sys_snprintf(name, sizeof(name)-1, "%u", db.dbID & ~DBID_MASK);

      if (attr & dmHdrAttrResDB) {
        db.ftype = DATAMGR_TYPE_RES;
      } else if (attr & dmHdrAttrStream) {
        db.ftype = DATAMGR_TYPE_FILE;
        DataMgrName(module->dm->path, name, DATAMGR_FILE_DATA, 0, 0, 0, 0, buf);
        if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          vfs_close(f);
        }
      } else {
        db.ftype = DATAMGR_TYPE_REC;
        DataMgrName(module->dm->path, name, DATAMGR_FILE_INDEX, 0, 0, 0, 0, buf);
        if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          vfs_close(f);
        }
      }

      db.creator = creator;
      db.type = type;
      db.attributes = attr;
      db.uniqueIDSeed = uniqueIDSeed;
      db.crDate = TimGetSeconds();
      db.modDate = db.crDate;
      db.bckDate = db.crDate;

      if (DataMgrWriteHeader(module, db.dbID, NULL, &db) == 0) {
        err = errNone;
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  if (err == errNone) {
    sys_memset(&dbCreated, 0, sizeof(dbCreated));
    dbCreated.newDBID = db.dbID;
    dbCreated.creator = creator;
    dbCreated.type = type;
    dbCreated.resDB = attr & dmHdrAttrResDB;
    sys_strncpy(dbCreated.dbName, nameP, dmDBNameLength-1);

    sys_memset(&notify, 0, sizeof(notify));
    notify.notifyType = sysNotifyDBCreatedEvent;
    notify.broadcaster = 0;
    notify.notifyDetailsP = &dbCreated;
    SysNotifyBroadcast(&notify);
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmCreateDatabase(UInt16 cardNo, const Char *nameP, UInt32 creator, UInt32 type, Boolean resDB) {
  return DDmCreateDatabaseEx(nameP, creator, type, resDB ? dmHdrAttrResDB : 0, ((UInt32)SysRandom32(0)) & 0xFFFFFF, true);
}

Err DDmCreateDatabaseFromImage(MemPtr bufferP) {
  DM_MODULE;
  UInt32 creationDate, modificationDate, lastBackupDate, modificationNumber, appInfo, sortInfo;
  UInt32 uniqueIDSeed, dummy32, type, creator, size, i, j, k;
  UInt16 attr, version, numRecs, index, appInfoSize, sortInfoSize;
  UInt32 *offsets, *resTypes, *uniqueIDs, firstOffset;
  UInt16 *resIDs, *attrs;
  UInt8 *database, dummy8;
  LocalID dbID, appInfoID, sortInfoID;
  MemHandle appInfoH, sortInfoH;
  void *appInfoP, *sortInfoP;
  DmOpenType *db;
  char name[dmDBNameLength];
  char st[8];
  Err err = dmErrInvalidParam;

  if (bufferP) {
    database = (UInt8 *)bufferP;
    if (!DataMgrValidName(database)) {
      debug(DEBUG_ERROR, "DataMgr", "DmCreateDatabaseFromImage invalid name \"%.*s\"", dmDBNameLength, database);
      DataMgrCheckErr(err);
      return err;
    }

    sys_memset(name, 0, dmDBNameLength);
    sys_strncpy(name, (char *)database, dmDBNameLength - 1);
    i = 0;
    i += dmDBNameLength;
    i += get2b(&attr, database, i);
    i += get2b(&version, database, i);
    i += get4b(&creationDate, database, i);
    i += get4b(&modificationDate, database, i);
    i += get4b(&lastBackupDate, database, i);
    i += get4b(&modificationNumber, database, i);
    i += get4b(&appInfo, database, i);
    i += get4b(&sortInfo, database, i);
    pumpkin_s2id(&type, (char *)&database[i]);
    if (!DataMgrValidTypeCreator(&database[i])) {
      debug(DEBUG_ERROR, "DataMgr", "DmCreateDatabaseFromImage invalid type 0x%08X", type);
      DataMgrCheckErr(err);
      return err;
    }
    i += 4;
    pumpkin_s2id(&creator, (char *)&database[i]);
    if (!DataMgrValidTypeCreator(&database[i])) {
      debug(DEBUG_ERROR, "DataMgr", "DmCreateDatabaseFromImage invalid creator 0x%08X", creator);
      return err;
    }
    i += 4;
    i += get4b(&uniqueIDSeed, database, i);
    i += get4b(&dummy32, database, i);  // nextRecordListID
    i += get2b(&numRecs, database, i);  // numberOfRecords
    debug(DEBUG_INFO, "DataMgr", "DmCreateDatabaseFromImage \"%s\" with %d recs", name, numRecs);

    if (mutex_lock(module->dm->mutex) == 0) {
      if (DDmCreateDatabaseEx(name, creator, type, attr, uniqueIDSeed, true) == errNone) {
        if ((dbID = DDmFindDatabase(0, name)) != 0) {
          DDmSetDatabaseInfo(0, dbID, NULL, NULL, &version, &creationDate, &modificationDate, &lastBackupDate, NULL, NULL, NULL, NULL, NULL);
          if ((db = DDmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
            if (numRecs > 0) {
              offsets = sys_calloc(numRecs, sizeof(UInt32));
              resTypes = sys_calloc(numRecs, sizeof(UInt32));
              resIDs = sys_calloc(numRecs, sizeof(UInt16));
              uniqueIDs = sys_calloc(numRecs, sizeof(UInt32));
              attrs = sys_calloc(numRecs, sizeof(UInt16));

              if (attr & dmHdrAttrResDB) {
                for (j = 0; j < numRecs; j++) {
                  i += get4b(&resTypes[j], database, i);
                  i += get2b(&resIDs[j], database, i);
                  i += get4b(&offsets[j], database, i);
                }
                i = firstOffset = offsets[0];

                for (j = 0; j < numRecs; j++) {
                  size = (j < numRecs-1) ? offsets[j+1] - offsets[j] : DMemPtrSize(bufferP) - offsets[j];
                  pumpkin_id2s(resTypes[j], st);
                  debug(DEBUG_INFO, "DataMgr", "DmCreateDatabaseFromImage res %d type '%s' id %d size %u", j, st, resIDs[j], size);
                  DDmNewResourceEx(db, resTypes[j], resIDs[j], size, &database[offsets[j]]);
                }
                err = errNone;
              } else {
                for (j = 0, k = 0; j < numRecs; j++) {
                  debug_bytes(DEBUG_INFO, "DataMgr", database + i, 8);
                  i += get4b(&offsets[j], database, i);
                  i += get1(&dummy8, database, i);
                  attrs[j] = dummy8;
                  i += get1(&dummy8, database, i);
                  uniqueIDs[j] = ((UInt32)dummy8) << 16;
                  i += get1(&dummy8, database, i);
                  uniqueIDs[j] |= ((UInt32)dummy8) << 8;
                  i += get1(&dummy8, database, i);
                  uniqueIDs[j] |= ((UInt32)dummy8);
                  if (attrs[j] & dmRecAttrDelete) continue;
                  k++;
                }
                i = firstOffset = offsets[0];
                db->numRecs = k;
                db->capRecs = 0;

                if (db->numRecs > 0) {
                  db->capRecs = ((db->numRecs + CAP_SIZE - 1) / CAP_SIZE) * CAP_SIZE;
                  db->idxRec = sys_calloc(db->capRecs, sizeof(UInt32));

                  for (j = 0, k = 0; j < numRecs; j++) {
                    size = (j < numRecs-1) ? offsets[j+1] - offsets[j] : DMemPtrSize(bufferP) - offsets[j];
                    if (attrs[j] & dmRecAttrDelete) continue;
                    debug(DEBUG_INFO, "DataMgr", "DmCreateDatabaseFromImage rec %d size %u", k, size);
                    index = k;
                    DDmNewRecordEx(db, &index, size, &database[offsets[j]], uniqueIDs[j], attrs[j], false);
                    k++;
                  }
                }
                DataMgrWriteIndex(db);
                err = errNone;
              }

              sys_free(attrs);
              sys_free(uniqueIDs);
              sys_free(resTypes);
              sys_free(resIDs);
              sys_free(offsets);

            } else {
              err = errNone;
            }

            appInfoSize = 0;
            sortInfoSize = 0;
            if (firstOffset == 0) firstOffset = DMemPtrSize(bufferP);

            if (appInfo) {
              if (sortInfo) {
                appInfoSize = sortInfo - appInfo;
                sortInfoSize = firstOffset - sortInfo;
              } else {
                appInfoSize = firstOffset - appInfo;
              }
            } else if (sortInfo) {
              sortInfoSize = firstOffset - sortInfo;
            }

            appInfoID = 0;
            sortInfoID = 0;

            if (appInfoSize) {
              debug(DEBUG_INFO, "DataMgr", "DmCreateDatabaseFromImage appInfo %d bytes", appInfoSize);
              if ((appInfoH = DMemHandleNew(appInfoSize)) != NULL) {
                if ((appInfoP = DMemHandleLock(appInfoH)) != NULL) {
                  MemMove(appInfoP, &database[appInfo], appInfoSize);
                  DMemHandleUnlock(appInfoH);
                  appInfoID = DMemHandleToLocalID(appInfoH);
                }
              }
            }

            if (sortInfoSize) {
              debug(DEBUG_INFO, "DataMgr", "DmCreateDatabaseFromImage sortInfo %d bytes", sortInfoSize);
              if ((sortInfoH = DMemHandleNew(sortInfoSize)) != NULL) {
                if ((sortInfoP = DMemHandleLock(sortInfoH)) != NULL) {
                  MemMove(sortInfoP, &database[sortInfo], sortInfoSize);
                  DMemHandleUnlock(sortInfoH);
                  sortInfoID = DMemHandleToLocalID(sortInfoH);
                }
              }
            }

            if (appInfoID || sortInfoID) {
              DataMgrWriteAppInfo(module, dbID, appInfoID ? &appInfoID : NULL, sortInfoID ? &sortInfoID : NULL);
            }

            DDmCloseDatabase(db);
          }
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmDeleteDatabase(UInt16 cardNo, LocalID dbID) {
  DM_MODULE;
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  DmOpenType db;
  SysNotifyParamType notify;
  SysNotifyDBDeletedType dbDeleted;
  char buf[VFS_PATH], buf2[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      if (db.readCount > 0 || db.writeCount > 0) {
        debug(DEBUG_ERROR, "DataMgr", "DmDeleteDatabase database dbID %u name \"%s\" is open", dbID & ~DBID_MASK, db.name);
      } else {
        debug(DEBUG_INFO, "DataMgr", "DmDeleteDatabase database dbID %u name \"%s\"", dbID & ~DBID_MASK, db.name);
        DataMgrName(module->dm->path, db.name, 0, 0, 0, 0, 0, buf);
        if ((dir = vfs_opendir(module->dm->session, buf)) != NULL) {
          for (;;) {
            ent = vfs_readdir(dir);
            if (ent == NULL) break;
            if (ent->type != VFS_FILE) continue;
            if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue;
            sys_strncpy(buf2, buf, VFS_PATH-1);
            sys_strncat(buf2, "/", VFS_PATH-sys_strlen(buf2)-1);
            sys_strncat(buf2, ent->name, VFS_PATH-sys_strlen(buf2)-1);
            vfs_unlink(module->dm->session, buf2);
          }
          vfs_closedir(dir);
        }
        DataMgrName(module->dm->path, db.name, 0, 0, 0, 0, 0, buf);
        vfs_unlink(module->dm->session, buf);

        sys_memset(&dbDeleted, 0, sizeof(dbDeleted));
        dbDeleted.oldDBID = db.dbID;
        dbDeleted.creator = db.creator;
        dbDeleted.type = db.type;
        dbDeleted.attributes = db.attributes;
        sys_strncpy(dbDeleted.dbName, db.name, dmDBNameLength-1);

        sys_memset(&notify, 0, sizeof(notify));
        notify.notifyType = sysNotifyDBDeletedEvent;
        notify.broadcaster = 0;
        notify.notifyDetailsP = &dbDeleted;
        SysNotifyBroadcast(&notify);

        err = errNone;
      }
    }
    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return err;
}

UInt16 DDmNumDatabases(UInt16 cardNo) {
  DM_MODULE;
  vfs_dir_t *dir;
  UInt16 numDatabases = 0;

  DM_DIR_BEGIN(dir) {
    numDatabases++;
  } DM_DIR_END(dir);

  return numDatabases;
}

LocalID DDmGetDatabase(UInt16 cardNo, UInt16 index) {
  DM_MODULE;
  vfs_dir_t *dir;
  LocalID dbID = 0;
  UInt16 i = 0;
  Err err = dmErrCantFind;

  DM_DIR_BEGIN(dir) {
    if (i == index) {
      dbID = db.dbID;
      err = errNone;
      break;
    }
    i++;
  } DM_DIR_END(dir);

  DataMgrCheckErr(err);
  return dbID;
}

LocalID DDmFindDatabase(UInt16 cardNo, const Char *nameP) {
  DM_MODULE;
  vfs_dir_t *dir;
  LocalID dbID = 0;
  Err err = dmErrCantFind;

  DM_DIR_BEGIN(dir) {
    if (sys_strcmp(db.name, nameP) == 0) {
      dbID = db.dbID;
      err = errNone;
      break;
    }
  } DM_DIR_END(dir);

  DataMgrCheckErr(err);
  return dbID;
}

Err DDmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP, UInt32 type, UInt32 creator, Boolean onlyLatestVers, UInt16 *cardNoP, LocalID *dbIDP) {
  DM_MODULE;
  vfs_dir_t *dir;
  UInt16 index, i;
  Err err = dmErrCantFind;

  if (cardNoP) *cardNoP = 0;
  if (dbIDP) *dbIDP = 0;

  if (stateInfoP) {
    if (newSearch) {
      index = 0;
      stateInfoP->p = (void *)((uintptr_t)index);
    } else {
      index = (UInt32)((uintptr_t)stateInfoP->p);
    }
    i = 0;
    DM_DIR_BEGIN(dir) {
      if (i < index) {
        i++;
        continue;
      }
      if ((type != 0 && type != db.type) || (creator != 0 && creator != db.creator) || db.name[0] == 0) {
        index++;
        continue;
      }
      if (dbIDP) *dbIDP = db.dbID;
      err = errNone;
      break;
    } DM_DIR_END(dir);
    stateInfoP->p = (void *)((uintptr_t)index);
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmDatabaseInfo(UInt16 cardNo, LocalID dbID, Char *nameP, UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP, UInt32 *bckUpDateP, UInt32 *modNumP, LocalID *appInfoIDP, LocalID *sortInfoIDP, UInt32 *typeP, UInt32 *creatorP) {
  DM_MODULE;
  DmOpenType db;
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      if (nameP) sys_strncpy(nameP, db.name, dmDBNameLength - 1);
      if (attributesP) *attributesP = db.attributes;
      if (versionP) *versionP = db.version;
      if (crDateP) *crDateP = db.crDate;
      if (modDateP) *modDateP = db.modDate;
      if (bckUpDateP) *bckUpDateP = db.bckDate;
      if (modNumP) *modNumP = db.modNum;
      if (typeP) *typeP = db.type;
      if (creatorP) *creatorP = db.creator;

      if (appInfoIDP || sortInfoIDP) {
        DataMgrReadAppInfo(module, dbID, appInfoIDP, sortInfoIDP);
      }

      err = errNone;
    }
    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmSetDatabaseInfo(UInt16 cardNo, LocalID dbID, const Char *nameP, UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP, UInt32 *bckUpDateP, UInt32 *modNumP, LocalID *appInfoIDP, LocalID *sortInfoIDP, UInt32 *typeP, UInt32 *creatorP) {
  DM_MODULE;
  DmOpenType db;
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      if (attributesP) db.attributes = *attributesP;
      if (versionP) db.version = *versionP;
      if (crDateP) db.crDate = *crDateP;
      if (modDateP) db.modDate = *modDateP;
      if (bckUpDateP) db.bckDate = *bckUpDateP;
      if (modNumP) db.modNum = *modNumP;
      if (typeP) db.type = *typeP;
      if (creatorP) db.creator = *creatorP;
      if (nameP) sys_strncpy(db.name, nameP, dmDBNameLength - 1);

      if (DataMgrWriteHeader(module, db.dbID, NULL, &db) == 0) {
        if (appInfoIDP || sortInfoIDP) {
          DataMgrWriteAppInfo(module, dbID, appInfoIDP, sortInfoIDP);
        }
        err = errNone;
      }
    }
    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmDatabaseSize(UInt16 cardNo, LocalID dbID, UInt32 *numRecordsP, UInt32 *totalBytesP, UInt32 *dataBytesP) {
  DM_MODULE;
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  vfs_file_t *f;
  DmOpenType db;
  UInt32 size;
  char name[16], buf[VFS_PATH], buf2[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (numRecordsP) *numRecordsP = 0;
  if (totalBytesP) *totalBytesP = 0;
  if (dataBytesP) *dataBytesP = 0;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      sys_snprintf(name, sizeof(name)-1, "%u", db.dbID & ~DBID_MASK);
      DataMgrName(module->dm->path, name, 0, 0, 0, 0, 0, buf);
      if ((dir = vfs_opendir(module->dm->session, buf)) != NULL) {
        for (;;) {
          ent = vfs_readdir(dir);
          if (ent == NULL) break;
          if (ent->type != VFS_FILE) continue;
          if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue;
          if (ent->name[0] >= 'a' && ent->name[0] <= 'z') continue; // ignore header, index, lock, etc
          if (numRecordsP) *numRecordsP += 1;
          if (totalBytesP || dataBytesP) {
            sys_strncpy(buf2, buf, VFS_PATH-1);
            sys_strncat(buf2, "/", VFS_PATH-sys_strlen(buf2)-1);
            sys_strncat(buf2, ent->name, VFS_PATH-sys_strlen(buf2)-1);
            if ((f = vfs_open(module->dm->session, buf2, VFS_READ)) != NULL) {
              if ((size = vfs_seek(f, 0, 1)) != -1) {
                if (dataBytesP) *dataBytesP += size;
                if (totalBytesP) *totalBytesP += size + 84; // XXX fictional header size
              }
              vfs_close(f);
            }
          }
        }
        vfs_closedir(dir);
        err = errNone;
      }
    }
    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmDatabaseProtect(UInt16 cardNo, LocalID dbID, Boolean protect) {
  DM_MODULE;
  DmOpenType db;
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      if (protect) {
        if (db.protect < 32) {
          db.protect++;
          err = errNone;
        }
      } else {
        if (db.protect > 0) {
          db.protect--;
          err = errNone;
        } else {
          err = dmErrDatabaseNotProtected;
        }
      }
      if (err == errNone) {
        err = DataMgrWriteHeader(module, dbID, NULL, &db) == 0;
      }
    }
    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return err;
}

DmOpenRef DDmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  DM_MODULE;
  Boolean ok;
  DmOpenType *first, *db = NULL;
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if ((db = pumpkin_heap_alloc(sizeof(DmOpenType), "db")) != NULL) {
      if (DataMgrReadHeader(module, dbID, NULL, db) == 0) {
        if (mode & dmModeWrite) {
          ok = DataMgrLockForWriting(module, db) == 0;
        } else if (mode & dmModeReadOnly) {
          ok = DataMgrLockForReading(module, db) == 0;
        } else {
          debug(DEBUG_ERROR, "DataMgr", "DmOpenDatabase dbID %u name \"%s\" invalid mode 0x%04X", dbID & ~DBID_MASK, db->name, mode);
          ok = false;
        }
        if (ok) {
          db->dbID = dbID;
          db->mode = mode;
          DataMgrReadIndex(db);
          err = errNone;
        } else {
          pumpkin_heap_free(db, "db");
          db = NULL;
        }
      }
    }

    if (db) {
      first = module->db;
      if (first) {
        first->prev = db;
        db->next = first;
      }
      module->db = db;
    }

    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return db;
}

DmOpenRef DDmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode) {
  DM_MODULE;
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenType *db = NULL;
  Err err;

  err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, type, creator, false, &cardNo, &dbID);
  if (err == errNone) {
    db = DmOpenDatabase(cardNo, dbID, mode);
  }

  DataMgrCheckErr(err);
  return db;
}

DmOpenRef DDmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  return DmOpenDatabase(cardNo, dbID, mode);
}

Err DDmCloseDatabase(DmOpenRef dbP) {
  DM_MODULE;
  DmOpenType *db;
  Err err = dmErrInvalidParam;

  if (dbP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      db = (DmOpenType *)dbP;
      if (db->mode & dmModeWrite) {
        debug(DEBUG_TRACE, "DataMgr", "DmCloseDatabase \"%s\" writeCount %d -> %d", db->name, db->writeCount, db->writeCount-1);
        if (DataMgrUnlockForWriting(module, db) == 0) err = errNone;
      } else if (db->mode & dmModeReadOnly) {
        debug(DEBUG_TRACE, "DataMgr", "DmCloseDatabase \"%s\" readCount %d -> %d", db->name, db->readCount, db->readCount-1);
        if (DataMgrUnlockForReading(module, db) == 0) err = errNone;
      } else {
        debug(DEBUG_ERROR, "DataMgr", "DmCloseDatabase \"%s\" db invalid mode 0x%04X", db->name, db->mode);
      }

      if (db->h) sys_free(db->h);

      if (db->prev) {
        db->prev->next = db->next;
        if (db->next) db->next->prev = db->prev;
      } else {
        if (db->next) db->next->prev = db->prev;
        module->db = db->next;
      }
      pumpkin_heap_free(db, "db");
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

DmOpenRef DDmNextOpenDatabase(DmOpenRef currentP) {
  DM_MODULE;
  DmOpenType *db = NULL;

  if (currentP) {
    db = (DmOpenType *)currentP;
    db = db->next;
  } else {
    db = module->db;
  }

  return db;
}

Err DDmOpenDatabaseInfo(DmOpenRef dbP, LocalID *dbIDP, UInt16 *openCountP, UInt16 *modeP, UInt16 *cardNoP, Boolean *resDBP) {
  DM_MODULE;
  DmOpenType *db;
  Err err = dmErrInvalidParam;

  if (dbP) {
    db = (DmOpenType *)dbP;
    if (dbIDP) *dbIDP = db->dbID;
    if (openCountP) *openCountP = db->readCount + db->writeCount;
    if (modeP) *modeP = db->mode;
    if (cardNoP) *cardNoP = 0;
    if (resDBP) *resDBP = db->ftype == DATAMGR_TYPE_RES;
    err = errNone;
  }

  DataMgrCheckErr(err);
  return err;
}

LocalID DDmGetAppInfoID(DmOpenRef dbP) {
  DM_MODULE;
  DmOpenType *db;
  LocalID appInfo = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    db = (DmOpenType *)dbP;
    appInfo = db->appInfoID;
    err = errNone;
  }

  DataMgrCheckErr(err);
  return appInfo;
}

void DDmGetDatabaseLockState(DmOpenRef dbR, UInt8 *highest, UInt32 *count, UInt32 *busy) {
  debug(DEBUG_ERROR, "DataMgr", "DmGetDatabaseLockState not implemented");
}

Err DDmResetRecordStates(DmOpenRef dbP) {
  debug(DEBUG_ERROR, "DataMgr", "DmResetRecordStates not implemented");
  return dmErrInvalidParam;
}

Err DDmGetLastErr(void) {
  DM_MODULE;
  return module->lastErr;
}

UInt16 DDmNumRecords(DmOpenRef dbP) {
  DM_MODULE;
  DmOpenType *db;
  UInt32 numRecords = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    db = (DmOpenType *)dbP;
    err = DDmDatabaseSize(0, db->dbID, &numRecords, NULL, NULL);
  }

  DataMgrCheckErr(err);
  return numRecords;
}

UInt16 DDmNumRecordsInCategory(DmOpenRef dbP, UInt16 category) {
  DM_MODULE;
  DmOpenType *db;
  UInt16 i, num = 0;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    db = (DmOpenType *)dbP;
    if (mutex_lock(module->dm->mutex) == 0) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        for (i = 0; i < db->numRecs; i++) {
          if ((category == dmAllCategories || (db->idxRec[i].attr & dmRecAttrCategoryMask) == (category & dmRecAttrCategoryMask))) {
            if (!(db->idxRec[i].attr & dmRecAttrSecret) || (db->mode & dmModeShowSecret)) {
              num++;
            }
          }
        }
        err = errNone;
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return num;
}

Err DDmRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP, LocalID *chunkIDP) {
  DM_MODULE; 
  DmOpenType *db;
  MemHandle h;
  Err err = dmErrIndexOutOfRange;
    
  if (dbP) {
    db = (DmOpenType *)dbP;
    if (mutex_lock(module->dm->mutex) == 0) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        if (index < db->numRecs) {
          if (uniqueIDP) *uniqueIDP = db->idxRec[index].uniqueID;
          if (attrP) *attrP = db->idxRec[index].attr;
          if (chunkIDP) {
            h = DDmGetRecord(dbP, index);
            *chunkIDP = h ? DMemHandleToLocalID(h) : 0;
          }
          err = errNone;
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }
  
  DataMgrCheckErr(err);
  return err;
}

Err DDmSetRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP) {
  DM_MODULE;
  DmOpenType *db;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    db = (DmOpenType *)dbP;
    if (mutex_lock(module->dm->mutex) == 0) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        if (index < db->numRecs) {
          if (uniqueIDP) db->idxRec[index].uniqueID = *uniqueIDP;
          if (attrP) db->idxRec[index].attr = *attrP;
          err = errNone;
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP) {
  DM_MODULE;
  DmOpenType *db;
  Err err = dmErrInvalidParam;

  if (dbP && atP && newH) {
    if (mutex_lock(module->dm->mutex) == 0) {
      db = (DmOpenType *)dbP;
      if (db->mode & dmModeWrite) {
        if (db->ftype == DATAMGR_TYPE_REC) {
          if (*atP >= db->numRecs) *atP = db->numRecs;
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  DM_MODULE;
  DmOpenType *db;
  UInt32 uniqueID;
  UInt16 attr, i;
  char name[16], buf[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (dbP && oldHP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      db = (DmOpenType *)dbP;
      if (db->mode & dmModeWrite) {
        if (db->ftype == DATAMGR_TYPE_REC) {
          if (db->numRecs > 0 && index < db->numRecs) {
            if (DataMgrRemoveFromIndex(db, index, &uniqueID, &attr) == 0) {
              sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
              DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, 0x00, uniqueID, buf);
              vfs_unlink(module->dm->session, buf);

              for (i = 0; i < db->numHandles; i++) {
                if (db->h[i]->d.rec.index == index) {
                  db->h[i]->d.rec.index = 0;
                  db->h[i]->htype = DATAMGR_TYPE_MEM;
                  *oldHP = db->h[i];
                  for (; i < db->numHandles - 1; i++) {
                    db->h[i] = db->h[i + 1];
                  }
                  db->numHandles--;
                  break;
                }
              }

              db->modDate = TimGetSeconds();
              if (DataMgrWriteHeader(module, db->dbID, NULL, db) == 0) {
                err = errNone;
              }
              DataMgrWriteIndex(db);
            }
          }
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to) {
  return 0;
}

static MemHandle DDmNewRecordEx(DmOpenRef dbP, UInt16 *atP, UInt32 size, void *p, UInt32 uniqueID, UInt16 attr, Boolean writeIndex) {
  DM_MODULE;
  DmOpenType *db;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  DmHandle **q;
  Err err = dmErrInvalidParam;
  DmHandle *h = NULL;

  if (dbP && atP && size > 0) {
    if (mutex_lock(module->dm->mutex) == 0) {
      db = (DmOpenType *)dbP;
      if (db->mode & dmModeWrite) {
        if (db->ftype == DATAMGR_TYPE_REC) {
          if (*atP >= db->numRecs) *atP = db->numRecs;
          if ((h = pumpkin_heap_alloc(sizeof(DmHandle), "DmHandle")) != NULL) {
            h->magic = DATAMGR_MAGIC;
            h->htype = DATAMGR_TYPE_REC;
            h->owner = pumpkin_get_current();
            h->size = size;
            h->dbID = db->dbID;
            h->d.rec.index = *atP;
            if (writeIndex) {
              h->d.rec.uniqueID = ++db->uniqueIDSeed;
              h->d.rec.attr = dmRecAttrDirty | dmRecAttrBusy;
            } else {
              h->d.rec.uniqueID = uniqueID;
              h->d.rec.attr = attr;
            }
            q = pumpkin_heap_alloc(sizeof(DmHandle *) + size, "DmHandleBuf");
            q[0] = h;
            h->buf = (uint8_t *)&q[1];
            if (p) {
              sys_memcpy(h->buf, p, size);
            }

            sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
            DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, 0x00, h->d.rec.uniqueID, buf);
            if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
              vfs_write(f, h->buf, size);
              vfs_close(f);
            }

            db->modDate = TimGetSeconds();
            if (DataMgrWriteHeader(module, db->dbID, NULL, db) == 0) {
              err = errNone;
            }

            if (writeIndex) {
              DataMgrInsertIntoIndex(db, h->d.rec.index, h->d.rec.uniqueID, h->d.rec.attr);
              DataMgrWriteIndex(db);
            }
          }
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return h;
}

MemHandle DDmNewRecord(DmOpenRef dbP, UInt16 *atP, UInt32 size) {
  return DDmNewRecordEx(dbP, atP, size, NULL, 0, 0, true);
}

Err DDmRemoveRecord(DmOpenRef dbP, UInt16 index) {
  DM_MODULE;
  MemHandle h;
  DmHandle *hh;
  Err err = dmErrInvalidParam;
 
  if (mutex_lock(module->dm->mutex) == 0) {
    if ((err = DDmDetachRecord(dbP, index, &h)) == errNone && h != NULL) {
      hh = (DmHandle *)h;
      pumpkin_heap_free(hh->buf, "DmHandleBuf");
      pumpkin_heap_free(hh, "DmHandle");
    }
    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmDeleteRecord(DmOpenRef dbP, UInt16 index) {
  // XXX using DmRemoveRecord here
  return DDmRemoveRecord(dbP, index);
}

Err DDmArchiveRecord(DmOpenRef dbP, UInt16 index) {
  // XXX using DmRemoveRecord, the record is deleted, not left intact
  return DDmRemoveRecord(dbP, index);
}

MemHandle DDmNewHandle(DmOpenRef dbP, UInt32 size) {
  return MemHandleNew(size);
}

Err DDmRemoveSecretRecords(DmOpenRef dbP) {
  debug(DEBUG_ERROR, "DataMgr", "DmRemoveSecretRecords not implemented");
  return dmErrInvalidParam;
}

Err DDmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, UInt16 *indexP) {
  DM_MODULE;
  DmOpenType *db;
  UInt16 i;
  Err err = dmErrUniqueIDNotFound;

  if (dbP) {
    db = (DmOpenType *)dbP;
    if (mutex_lock(module->dm->mutex) == 0) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        for (i = 0; i < db->numRecs; i++) {
          if (db->idxRec[i].uniqueID == uniqueID) {
            *indexP = i;
            err = errNone;
            break;
          }
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

static MemHandle DDmQueryRecordEx(DmOpenRef dbP, UInt16 index, DmHandle *recH, Err (*callback)(DmHandle *h, void *data, Boolean loaded), void *data) {
  DM_MODULE;
  DmOpenType *db;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  UInt32 size, i;
  DmHandle *h = NULL;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    db = (DmOpenType *)dbP;
    if (index < db->numRecs) {
    if (mutex_lock(module->dm->mutex) == 0) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        for (i = 0; i < db->numHandles; i++) {
          if ((recH != NULL && recH == db->h[i]) || (db->h[i]->d.rec.index == index)) {
            err = callback(db->h[i], data, false);
            mutex_unlock(module->dm->mutex);
            DataMgrCheckErr(err);
            return db->h[i];
          }
        }

        if (recH) {
          mutex_unlock(module->dm->mutex);
          DataMgrCheckErr(err);
          return NULL;
        }

        sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
        DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, db->idxRec[index].attr & ATTR_MASK, db->idxRec[index].uniqueID, buf);

        if ((f = vfs_open(module->dm->session, buf, VFS_READ)) != NULL) {
          if ((size = vfs_seek(f, 0, 1)) > 0) {
            vfs_seek(f, 0, 0);
            if ((h = DMemHandleNew(size)) != NULL) {
              h->htype = DATAMGR_TYPE_REC;
              if (vfs_read(f, h->buf, h->size) == h->size) {
                vfs_close(f);
                h->d.rec.uniqueID = db->idxRec[index].uniqueID;
                h->d.rec.attr = db->idxRec[index].attr;
                h->d.rec.index = index;
                err = callback(h, data, true);
                if (db->numHandles == db->capHandles) {
                  db->capHandles += CAP_SIZE;
                  db->h = db->h ?
                    sys_realloc(db->h, db->capHandles * sizeof(DmHandle *)):
                    sys_calloc(db->capHandles, sizeof(DmHandle *));
                }
                db->h[db->numHandles++] = h;
              } else {
                vfs_close(f);
                err = dmErrMemError;
                DMemHandleFree(h);
                h = NULL;
              }
            } else {
              err = dmErrMemError;
              vfs_close(f);
            }
          } else {
            err = dmErrMemError;
            vfs_close(f);
          }
        } else {
          err = dmErrMemError;
        }
      }
      mutex_unlock(module->dm->mutex);
      }
    }
  }

  DataMgrCheckErr(err);
  return h;
}

static Err DDmQueryRecordCallback(DmHandle *h, void *data, Boolean loaded) {
  if (loaded) {
    h->d.rec.attr &= ~dmRecAttrDirty;
    h->useCount = 1;
    h->lockCount = 0;
  } else {
    h->useCount++;
  }

  return errNone;
}

MemHandle DDmQueryRecord(DmOpenRef dbP, UInt16 index) {
  return DDmQueryRecordEx(dbP, index, NULL, DDmQueryRecordCallback, NULL);
}

static Err DDmGetRecordCallback(DmHandle *h, void *data, Boolean loaded) {
  if (loaded) {
    h->d.rec.attr &= ~dmRecAttrDirty;
    h->useCount = 1;
    h->lockCount = 0;
  } else {
    h->useCount++;
  }
  h->d.rec.attr |= dmRecAttrBusy;

  return errNone;
}

MemHandle DDmGetRecord(DmOpenRef dbP, UInt16 index) {
  return DDmQueryRecordEx(dbP, index, NULL, DDmGetRecordCallback, NULL);
}

MemHandle DDmQueryNextInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 category) {
  return 0;
}

UInt16 DDmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category) {
  DM_MODULE;
  DmOpenType *db;
  UInt16 i, pos = 0;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    db = (DmOpenType *)dbP;
    if (mutex_lock(module->dm->mutex) == 0) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        for (i = 0; i < db->numRecs; i++) {
          if ((category == dmAllCategories || (db->idxRec[i].attr & dmRecAttrCategoryMask) == (category & dmRecAttrCategoryMask))) {
            if (!(db->idxRec[i].attr & dmRecAttrSecret) || (db->mode & dmModeShowSecret)) {
              if (i == index) break;
              pos++;
            }
          }
        }
        err = errNone;
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return pos;
}

Err DDmSeekRecordInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category) {
  DM_MODULE;
  DmOpenType *db;
  Err err = dmErrSeekFailed;

  if (dbP && indexP) {
    db = (DmOpenType *)dbP;
    debug(DEBUG_TRACE, "DataMgr", "DmSeekRecordInCategory n=%d i=%u o=%d d=%d c=%u", DmNumRecords(db), *indexP, offset, direction, category);
    if (db->numRecs > 0) {
      if (*indexP == 0xFFFF) *indexP = db->numRecs-1;
      if (*indexP < db->numRecs) {
        for (;;) {
          if ((category == dmAllCategories || (db->idxRec[*indexP].attr & dmRecAttrCategoryMask) == category)) {
            if (!(db->idxRec[*indexP].attr & dmRecAttrSecret) || (db->mode & dmModeShowSecret)) {
              if (offset == 0) {
                err = errNone;
                break;
              }
              offset--;
            }
          }
          if (direction == dmSeekForward) {
            // forward
            if (*indexP == db->numRecs - 1) {
              err = dmErrIndexOutOfRange;
              break;
             }
            *indexP = *indexP + 1;
          } else {
            // backward
            if (*indexP == 0) {
              err = dmErrSeekFailed;
              break;
            }
            *indexP = *indexP - 1;
          }
        }
      } else {
        err = dmErrIndexOutOfRange;
      }
    } else {
      err = (direction == dmSeekForward) ? dmErrIndexOutOfRange : dmErrSeekFailed;
    }
    debug(DEBUG_TRACE, "DataMgr", "DmSeekRecordInCategory i=%u err=%d", *indexP, err);
  }

  DataMgrCheckErr(err);
  return err;
}

static Err DDmResizeRecordCallback(DmHandle *h, void *data, Boolean loaded) {
  DM_MODULE;
  UInt32 newSize = *((UInt32 *)data);
  char name[16], buf[VFS_PATH];
  vfs_file_t *f;
  DmHandle **q;
  Err err = dmErrMemError;

  if (h->size != newSize) {
    if ((q = pumpkin_heap_realloc(h->buf, sizeof(DmHandle *) + newSize, "DmHandleBuf")) != NULL) {
      q[0] = h;
      h->buf = (uint8_t *)&q[1];
      h->size = newSize;

      sys_snprintf(name, sizeof(name)-1, "%u", h->dbID & ~DBID_MASK);
      DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);

      if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
        if (vfs_write(f, h->buf, h->size) == h->size) {
          err = errNone;
        }
        vfs_close(f);
      }
    }
  }

  return err;
}

MemHandle DDmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize) {
  return DDmQueryRecordEx(dbP, index, NULL, DDmResizeRecordCallback, &newSize);
}

Err DDmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty) {
  DM_MODULE;
  DmOpenType *db;
  DmHandle *h;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  UInt32 i;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      db = (DmOpenType *)dbP;
        if (db->ftype == DATAMGR_TYPE_REC) {
          for (i = 0; i < db->numHandles; i++) {
            if (db->h[i]->d.rec.index == index) {
              h = db->h[i];
              if (h->useCount > 0) {
                h->useCount--;
              } else {
                debug(DEBUG_ERROR, "DataMgr", "DmReleaseRecord database \"%s\" index %d useCount < 0", db->name, index);
              }
              if (dirty || (h->d.rec.attr & dmRecAttrDirty)) {
                if (db->mode & dmModeWrite) {
                  sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
                  DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, 0x00, h->d.rec.uniqueID, buf);
                  if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                    vfs_write(f, h->buf, h->size);
                    vfs_close(f);
                  }
                  h->d.rec.attr &= ~dmRecAttrDirty;

                  db->modDate = TimGetSeconds();
                  if (DataMgrWriteHeader(module, db->dbID, NULL, db) == 0) {
                    err = errNone;
                  }
                } else {
                  debug(DEBUG_ERROR, "DataMgr", "DmReleaseRecord database \"%s\" index %d dirty but read-only ", db->name, index);
                }
              } else {
                err = errNone;
              }
              if (h->useCount == 0) {
                if (db->numHandles > 1 && i < db->numHandles - 1) {
                  db->h[i] = db->h[db->numHandles - 1];
                  db->h[db->numHandles - 1] = NULL;
                } else {
                  db->h[i] = NULL;
                }
                db->numHandles--;
                pumpkin_heap_free(h->buf, "DmHandleBuf");
                pumpkin_heap_free(h, "DmHandle");
              }
              mutex_unlock(module->dm->mutex);
              DataMgrCheckErr(err);
              return err;
            }
          }
        }
    }
  }

  DataMgrCheckErr(err);
  return err;
}

static Err DDmSearchRecordCallback(DmHandle *h, void *data, Boolean loaded) {
  return errNone;
}

// Search all open record databases for a record with the handle passed.
UInt16 DDmSearchRecord(MemHandle recH, DmOpenRef *dbPP) {
  DM_MODULE;
  DmOpenType *db;
  DmHandle *h = NULL;

  *dbPP = NULL;
  for (db = module->db; db; db = db->next) {
    h = DDmQueryRecordEx(db, 0, recH, DDmSearchRecordCallback, dbPP);
    if (h) {
      *dbPP = db;
      break;
    }
  }

  return h ? h->d.rec.index : -1;
}

Err DDmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty) {
  debug(DEBUG_ERROR, "DataMgr", "DmMoveCategory not implemented");
  return dmErrInvalidParam;
}

Err DDmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum) {
  debug(DEBUG_ERROR, "DataMgr", "DmDeleteCategory not implemented");
  return dmErrInvalidParam;
}

Err DDmWriteCheck(void *recordP, UInt32 offset, UInt32 bytes) {
  return 0;
}

Err DDmWrite(void *recordP, UInt32 offset, const void *srcP, UInt32 bytes) {
  return 0;
}

Err DDmStrCopy(void *recordP, UInt32 offset, const Char *srcP) {
  return 0;
}

Err DDmSet(void *recordP, UInt32 offset, UInt32 bytes, UInt8 value) {
  return 0;
}

MemHandle DDmGetResource(DmResType type, DmResID resID) {
  return 0;
}

MemHandle DDmGet1Resource(DmResType type, DmResID resID) {
  return 0;
}

Err DDmReleaseResource(MemHandle resourceH) {
  return 0;
}

MemHandle DDmResizeResource(MemHandle resourceH, UInt32 newSize) {
  return 0;
}

DmOpenRef DDmNextOpenResDatabase(DmOpenRef dbP) {
  return 0;
}

UInt16 DDmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex) {
  return 0;
}

UInt16 DDmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH) {
  return 0;
}

UInt16 DDmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP) {
  return 0;
}

UInt16 DDmNumResources(DmOpenRef dbP) {
  return 0;
}

Err DDmResourceInfo(DmOpenRef dbP, UInt16 index, DmResType *resTypeP, DmResID *resIDP, LocalID *chunkLocalIDP) {
  return 0;
}

Err DDmSetResourceInfo(DmOpenRef dbP, UInt16 index, DmResType *resTypeP, DmResID *resIDP) {
  return 0;
}

Err DDmAttachResource(DmOpenRef dbP, MemHandle newH, DmResType resType, DmResID resID) {
  return 0;
}

Err DDmDetachResource(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  return 0;
}

MemHandle DDmNewResourceEx(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size, void *p) {
  DM_MODULE;
  DmOpenType *db;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  DmHandle **q;
  Err err = dmErrInvalidParam;
  DmHandle *h = NULL;

  if (dbP && size > 0) {
    if (mutex_lock(module->dm->mutex) == 0) {
      db = (DmOpenType *)dbP;
      if (db->mode & dmModeWrite) {
        if (db->ftype == DATAMGR_TYPE_RES) {
          if ((h = pumpkin_heap_alloc(sizeof(DmHandle), "DmHandle")) != NULL) {
            h->magic = DATAMGR_MAGIC;
            h->htype = DATAMGR_TYPE_RES;
            h->owner = pumpkin_get_current();
            h->size = size;
            h->dbID = db->dbID;
            h->d.res.type = resType;
            h->d.res.id = resID;
            h->d.res.attr = dmRecAttrDirty;
            q = pumpkin_heap_alloc(sizeof(DmHandle *) + size, "DmHandleBuf");
            q[0] = h;
            h->buf = (uint8_t *)&q[1];
            if (p) {
              sys_memcpy(h->buf, p, size);
            }

            sys_snprintf(name, sizeof(name)-1, "%u", db->dbID & ~DBID_MASK);
            DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, resID, resType, 0x00, 0, buf);
            if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
              vfs_write(f, h->buf, size);
              vfs_close(f);
            }

            db->modDate = TimGetSeconds();
            if (DataMgrWriteHeader(module, db->dbID, NULL, db) == 0) {
              err = errNone;
            }
          }
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return h;
}

MemHandle DDmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size) {
  return DDmNewResourceEx(dbP, resType, resID, size, NULL);
}

Err DDmRemoveResource(DmOpenRef dbP, UInt16 index) {
  return 0;
}

MemHandle DDmGetResourceIndex(DmOpenRef dbP, UInt16 index) {
  return 0;
}

static Err DataMgrSort(DmOpenRef dbP, DmComparF *comparF, UInt32 comparF68K, Int16 other) {
  DM_MODULE;
  DmOpenType *db;
  Err err = dmErrInvalidParam;

  if (dbP && (comparF || comparF68K)) {
    db = (DmOpenType *)dbP;
    if (db->mode & dmModeWrite) {
      if (db->ftype == DATAMGR_TYPE_REC) {
        if (db->numRecs > 1) {
          module->comparF = comparF;
          module->comparF68K = comparF68K;
          module->other = other;
          module->appInfoH = db->appInfoID ? DMemLocalIDToHandle(db->appInfoID) : NULL;
          module->tmpDb = db;
          debug(DEBUG_INFO, "DataMgr", "DataMgrSort sorting database \"%s\" with %d records (%s)", db->name, db->numRecs, comparF ? "native" : "68K");
          //sys_qsort(db->idxRec, db->numRecs, sizeof(DmRecIndex), DataMgrCompareHandle);
          module->tmpDb = NULL;
          module->comparF = NULL;
          module->comparF68K = 0;
          module->other = 0;
          module->appInfoH = NULL;
          //StoWriteIndex(sto, db);
          err = errNone;
        } else {
          err = errNone;
        }
      }
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DDmInsertionSort(DmOpenRef dbP, DmComparF *comparF, Int16 other) {
  return DataMgrSort(dbP, comparF, 0, other);
}

Err DDmInsertionSort68K(DmOpenRef dbP, UInt32 comparF, Int16 other) {
  return DataMgrSort(dbP, NULL, comparF, other);
}

Err DDmQuickSort(DmOpenRef dbP, DmComparF *comparF, Int16 other) {
  return DataMgrSort(dbP, comparF, 0, other);
}

Err DDmQuickSort68K(DmOpenRef dbP, UInt32 comparF, Int16 other) {
  return DataMgrSort(dbP, NULL, comparF, other);
}

UInt16 DDmFindSortPosition(DmOpenRef dbP, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other) {
  return 0;
}

UInt16 DDmFindSortPositionV10(DmOpenRef dbP, void *newRecord, DmComparF *compar, Int16 other) {
  return 0;
}

UInt32 DMemHandleSize(MemHandle h) {
  DM_MODULE;
  DmHandle *hh;
  UInt32 size = 0;
  Err err = memErrInvalidParam;

  if (h) {
    hh = (DmHandle *)h;
    size = hh->size;
    err = errNone;
  }

  DataMgrCheckErr(err);
  return size;
}

UInt32 DMemPtrSize(MemPtr p) {
  MemHandle h;
  UInt32 size = 0;

  if ((h = DMemPtrRecoverHandle(p)) != NULL) {
    size = DMemHandleSize(h);
  }

  return size;
}

MemHandle DMemPtrRecoverHandle(MemPtr p) {
  DM_MODULE;
  DmHandle *h = NULL;
  DmHandle **q;
  Err err = memErrInvalidParam;

  if (p) {
    q = (DmHandle **)p;
    h = q[-1];
    if (h->magic != DATAMGR_MAGIC) {
      debug(DEBUG_ERROR, "DataMgr", "MemPtrRecoverHandle invalid handle magic 0x%08X for handle %p pointer %p", h->magic, h, p);
      h = NULL;
    } else {
      err = errNone;
    }
  }

  DataMgrCheckErr(err);
  return h;
}

MemHandle DMemHandleNew(UInt32 size) {
  DM_MODULE;
  DmHandle *h = NULL;
  DmHandle **q;
  Err err = dmErrInvalidParam;

  if (size) {
    if ((h = pumpkin_heap_alloc(sizeof(DmHandle), "DmHandle")) != NULL) {
      h->magic = DATAMGR_MAGIC;
      h->htype = DATAMGR_TYPE_MEM;
      h->owner = pumpkin_get_current();
      q = pumpkin_heap_alloc(sizeof(DmHandle *) + size, "DmHandleBuf");
      q[0] = h;
      h->buf = (uint8_t *)&q[1];
      h->size = size;
      err = errNone;
    }
  }

  DataMgrCheckErr(err);
  return h;
}

Err DMemHandleFree(MemHandle h) {
  DM_MODULE;
  DmHandle *hh;
  Err err = dmErrInvalidParam;

  if (h) {
    hh = (DmHandle *)h;
    switch (hh->htype) {
      case DATAMGR_TYPE_MEM:
      case DATAMGR_TYPE_REC:
      case DATAMGR_TYPE_RES:
        if (hh->dbID == 0) {
          if (hh->buf) pumpkin_heap_free(hh->buf, "DmHandleBuf");
          pumpkin_heap_free(hh, "DmHandle");
          err = errNone;
        } else {
          debug(DEBUG_ERROR, "DataMgr", "MemHandleFree attempt to free handle assigned to dbID %u", hh->dbID & ~DBID_MASK);
        }
        break;
      default:
        debug(DEBUG_ERROR, "DataMgr", "MemHandleFree %p unexpected handle type %d", hh, hh->htype);
        break;
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DMemChunkFree(MemPtr chunkDataP) {
  DM_MODULE;
  DmHandle *h;
  Err err = memErrInvalidParam;

  debug(DEBUG_TRACE, "DataMgr", "MemChunkFree %p", chunkDataP);
  if (chunkDataP) {
    if ((h = DMemPtrRecoverHandle(chunkDataP)) != NULL) {
      err = MemHandleFree(h);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DMemHandleResize(MemHandle h, UInt32 newSize) {
  DM_MODULE;
  DmHandle *hh, **q;
  Err err = memErrInvalidParam;

  if (h && newSize > 0) {
    hh = (DmHandle *)h;
    switch (hh->htype) {
      case DATAMGR_TYPE_MEM:
      case DATAMGR_TYPE_REC:
      case DATAMGR_TYPE_RES:
        if (hh->dbID == 0) {
          if (hh->size != newSize) {
            if ((q = pumpkin_heap_realloc(hh->buf, sizeof(DmHandle *) + newSize, "DmHandleBuf")) != NULL) {
              q[0] = hh;
              hh->buf = (uint8_t *)&q[1];
              err = errNone;
            }
          } else {
            err = errNone;
          }
        } else {
          debug(DEBUG_ERROR, "DataMgr", "MemHandleResize attempt to resize handle assigned to dbID %u", hh->dbID & ~DBID_MASK);
        }
        break;
      default:
        debug(DEBUG_ERROR, "DataMgr", "MemHandleResize %p unexpected handle type %d", hh, hh->htype);
        break;
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DMemPtrResize(MemPtr p, UInt32 newSize) {
  DM_MODULE;
  MemHandle h;
  Err err = memErrInvalidParam;

  if ((h = DMemPtrRecoverHandle(p)) != NULL) {
    err = DMemHandleResize(h, newSize);
  }

  DataMgrCheckErr(err);
  return err;
}

MemPtr DMemHandleLock(MemHandle h) {
  DM_MODULE;
  DmHandle *hh;
  void *p = NULL;
  Err err = dmErrInvalidParam;

  if (h) {
    hh = (DmHandle *)h;
    if (hh->lockCount < 14) {
      switch (hh->htype) {
        case DATAMGR_TYPE_MEM:
        case DATAMGR_TYPE_REC:
        case DATAMGR_TYPE_RES:
          hh->lockCount++;
          p = hh->buf;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "DataMgr", "MemHandleLockEx %p unexpected handle type %d", hh, hh->htype);
          break;
      }
    } else {
      debug(DEBUG_ERROR, "DataMgr", "MemHandleLockEx %p lockCount %d", hh, hh->lockCount);
    }
  }

  DataMgrCheckErr(err);
  return p;
}

Err DMemHandleUnlock(MemHandle h) {
  DM_MODULE;
  DmHandle *hh;
  Err err = dmErrInvalidParam;

  if (h) {
    hh = (DmHandle *)h;
    if (hh->lockCount > 0) {
      switch (hh->htype) {
        case DATAMGR_TYPE_MEM:
        case DATAMGR_TYPE_REC:
        case DATAMGR_TYPE_RES:
          hh->lockCount--;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "DataMgr", "MemHandleUnlock %p unexpected handle type %d", hh, hh->htype);
          break;
      }
    } else {
      debug(DEBUG_ERROR, "DataMgr", "MemHandleUnlock %p is not locked", hh);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DMemHandleResetLock(MemHandle h) {
  DM_MODULE;
  DmHandle *hh;
  Err err = dmErrInvalidParam;

  if (h) {
    hh = (DmHandle *)h;
    if (hh->lockCount > 0) {
      switch (hh->htype) {
        case DATAMGR_TYPE_MEM:
        case DATAMGR_TYPE_REC:
        case DATAMGR_TYPE_RES:
          hh->lockCount = 0;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "DataMgr", "MemHandleResetLock %p unexpected handle type %d", hh, hh->htype);
          break;
      }
    } else {
      debug(DEBUG_ERROR, "DataMgr", "MemHandleResetLock %p is not locked", hh);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

Err DMemPtrResetLock(MemPtr p) {
  MemHandle h;
  Err err = memErrInvalidParam;

  if (p && (h = DMemPtrRecoverHandle(p)) != NULL) {
    err = DMemHandleResetLock(h);
  }

  return err;
}

UInt16 DMemHandleLockCount(MemHandle h) {
  DM_MODULE;
  DmHandle *handle;
  UInt16 count = 0;
  Err err = memErrInvalidParam;

  if (h) {
    handle = (DmHandle *)h;
    count = handle->lockCount;
    err = errNone;
  }

  DataMgrCheckErr(err);
  return count;
}

Err DMemPtrUnlock(MemPtr p) {
  DM_MODULE;
  MemHandle h;
  Err err = memErrInvalidParam;

  if (p && (h = DMemPtrRecoverHandle(p)) != NULL) {
    err = DMemHandleUnlock(h);
  }

  DataMgrCheckErr(err);
  return err;
}

UInt16 DMemHandleFlags(MemHandle h) {
  // XXX always 0
  return 0;
}

LocalID DMemHandleToLocalID(MemHandle h) {
  DM_MODULE;
  DmHandle *hh;
  LocalID id = 0;
  Err err = memErrInvalidParam;

  if (h) {
    hh = (DmHandle *)h;
    switch (hh->htype) {
      case DATAMGR_TYPE_MEM:
      case DATAMGR_TYPE_REC:
      case DATAMGR_TYPE_RES:
        id = (uint8_t *)hh - module->heapBase;
        err = errNone;
        break;
      default:
        debug(DEBUG_ERROR, "DataMgr", "MemHandleToLocalID %p unexpected handle type %d", hh, hh->htype);
        break;
    }
  }

  DataMgrCheckErr(err);
  return id;
}

MemHandle DMemLocalIDToHandle(LocalID local) {
  DM_MODULE;
  DmHandle *h = NULL;
  Err err = memErrInvalidParam;

  if (!(local & DBID_MASK) && local > 0 && local < module->heapSize) {
    h = (DmHandle *)(module->heapBase + local);
    if (h->magic == DATAMGR_MAGIC) {
      err = errNone;
    } else {
      h = NULL;
    }
  }

  DataMgrCheckErr(err);
  return h;
}

// This routine determines if the given local ID is to a nonmovable (memIDPtr) or movable (memIDHandle) chunk.
// XXX this distinction makes no sense in PumpkinOS,
// but some apps depends on it an even crash if the value is not the expected (example: Bird)
LocalIDKind DMemLocalIDKind(LocalID local) {
  return memIDHandle;
}

// XXX MemoPad application uses this code:
// return ((MemHandle) MemLocalIDToGlobal (appInfoID, cardNo));
// so it is getting a MemPtr and casting to MemHandle, which will obviously not work here.
// I am hoping that anyone that calls MemLocalIDToGlobal() assumes the returned value is a MemHandle.
MemPtr DMemLocalIDToGlobal(LocalID local, UInt16 cardNo) {
  return DMemLocalIDToHandle(local);
}

// New owner ID of the chunk. Specify 0 to set the owner to the operating system.
// Once you have granted ownership of a memory chunk to the system, do not attempt to free it yourself.
// The operating system will free it when the application invoked with SysUIAppSwitch() or SysAppLaunch() quits.
Err DMemHandleSetOwner(MemHandle h, UInt16 owner) {
  DmHandle *hh;
  Err err = dmErrInvalidParam;

  debug(DEBUG_TRACE, "DataMgr", "MemHandleSetOwner handle=%p owner=%d", h, owner);

  if (h) {
    hh = (DmHandle *)h;
    hh->owner = owner;
    err = errNone;
  }

  return err;
}

UInt16 DMemPtrOwner(MemPtr p) {
  DmHandle *h;
  UInt16 owner = 0;

  if (p) {
    if ((h = DMemPtrRecoverHandle(p)) != NULL) {
      owner = h->owner;
    }
  }

  debug(DEBUG_TRACE, "DataMgr", "MemPtrOwner p=%p owner=%d", p, owner);

  return owner;
}

UInt16 DMemHandleHeapID(MemHandle h) {
  return 0;
}

Boolean DMemHandleDataStorage(MemHandle h) {
  return false;
}

Err DMemSemaphoreReserve(Boolean writeAccess) {
  // system use only
  return dmErrInvalidParam;
}

Err DMemSemaphoreRelease(Boolean writeAccess) {
  // system use only
  return dmErrInvalidParam;
}

UInt16 DMemDebugMode(void) {
  // system use only
  return 0;
}

Err DMemSetDebugMode(UInt16 flags) {
  // system use only
  return dmErrInvalidParam;
}

Err DMemHeapScramble(UInt16 heapID) {
  return errNone;
}

Err DMemHeapCheck(UInt16 heapID) {
  return errNone;
}

Err DMemCardInfo(UInt16 cardNo, Char *cardNameP, Char *manufNameP, UInt16 *versionP, UInt32 *crDateP, UInt32 *romSizeP, UInt32 *ramSizeP, UInt32 *freeBytesP) {
  if (cardNameP) StrNCopy(cardNameP, "RAM", 32);
  if (manufNameP) StrNCopy(manufNameP, SYSTEM_NAME, 32);
  if (versionP) *versionP = sys_atoi(SYSTEM_VERSION);
  if (crDateP) *crDateP = pumpkin_dt() + CRDATE;
  if (romSizeP) *romSizeP = 8*1024*1024; // XXX
  if (ramSizeP) *ramSizeP = DMemHeapSize(1);
  if (freeBytesP) *freeBytesP = DMemHeapSize(1);

  return errNone;
}

Err DMemCardFormat(UInt16 cardNo, const Char *cardNameP, const Char *manufNameP, const Char *ramStoreNameP) {
  debug(DEBUG_ERROR, "DataMgr", "MemCardFormat not implemented");
  return dmErrInvalidParam;
}

Err DMemKernelInit(void) {
  return errNone;
}

Err DMemInitHeapTable(UInt16 cardNo) {
  return errNone;
}

UInt16 DMemNumCards(void) {
  return 1;
}

UInt16 DMemNumHeaps(UInt16 cardNo) {
  return 2;
}

UInt16 DMemNumRAMHeaps(UInt16 cardNo) {
  return 2;
}

UInt16 DMemHeapID(UInt16 cardNo, UInt16 heapIndex) {
  return heapIndex;
}

UInt16 DMemHandleCardNo(MemHandle h) {
  return 0;
}

Boolean DMemHeapDynamic(UInt16 heapID) {
  return heapID == 0;
}

Err DMemHeapFreeBytes(UInt16 heapID, UInt32 *freeP, UInt32 *maxP) {
  if (freeP) *freeP = DMemHeapSize(heapID);
  if (maxP) *maxP = DMemHeapSize(heapID);
  return errNone;
}

UInt32 DMemHeapSize(UInt16 heapID) {
  DM_MODULE;
  UInt32 size;
  size = heapID == 0 ? 2*1024*1024 : module->heapSize;
  return size;
}

UInt16 DMemHeapFlags(UInt16 heapID) {
  return 0;
}

Err DMemHeapCompact(UInt16 heapID) {
  return errNone;
}

Err DMemHeapInit(UInt16 heapID, Int16 numHandles, Boolean initContents) {
  return errNone;
}
