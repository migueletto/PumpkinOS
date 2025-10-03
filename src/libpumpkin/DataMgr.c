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
      data_db_t db; \
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

#define CAP_HANDLE 32

typedef struct DmOpenType {
  LocalID dbID;
  UInt16 mode;
  struct DmHandle **h;
  UInt32 numHandles, capHandles;
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
  DmOpenType *dbRef;
  data_infoid_t *id;
  Err lastErr;
} data_module_t;

struct DataMgrType {
  mutex_t *mutex;
  char path[MAX_PATH];
  vfs_session_t *session;
  LocalID nextDbID;
};

typedef struct data_db_t {
  LocalID dbID;
  uint32_t ftype, readCount, writeCount, uniqueIDSeed;
  uint16_t mode, numRecs, protect;
  uint32_t creator, type, crDate, modDate, bckDate, modNum;
  uint32_t attributes, version, appInfoID, sortInfoID;
  char name[dmDBNameLength];
} data_db_t;

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
  uint8_t buf[0];
} DmHandle;

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

static int DataMgrReadHeader(data_module_t *module, LocalID dbID, char *name, data_db_t *db) {
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
          sys_memset(db, 0, sizeof(data_db_t));
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

static int DataMgrWriteHeader(data_module_t *module, LocalID dbID, char *name, data_db_t *db) {
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

static vfs_file_t *DataMgrOpenIndex(data_module_t *module, LocalID dbID) {
  char name[16], buf[VFS_PATH];

  sys_snprintf(name, sizeof(name)-1, "%u", dbID & ~DBID_MASK);
  DataMgrName(module->dm->path, name, DATAMGR_FILE_INDEX, 0, 0, 0, 0, buf);

  return vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC);
}

static int DataMgrWriteIndex(vfs_file_t *f, UInt32 uniqueID, UInt8 attr) {
  char buf[32];
  int r = -1;

  if (f) {
    sys_snprintf(buf, sizeof(buf)-1, "%08X.%02X\n", uniqueID, attr & ATTR_MASK);
    r = vfs_write(f, (uint8_t *)buf, 12) == 12 ? 0 : -1;
  }

  return r;
}

static int DataMgrReadIndex(vfs_file_t *f, UInt32 *uniqueID, UInt8 *attr) {
  UInt32 id, a;
  char buf[16];
  int n;

  if (f) {
    if ((n = vfs_read(f, (uint8_t *)buf, 1)) == 0) return 0;
    if (n != 12) return -1;
    if (sys_sscanf(buf, "%08X.%02X\n", &id, &a) != 2) return -1;
    *uniqueID = id;
    *attr = a;
  }

  return 0;
}

static int DataMgrGetFileLocks(data_module_t *module, data_db_t *db, int *read_locks, int *write_locks) {
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

static int DataMgrPutFileLocks(data_module_t *module, data_db_t *db, int read_locks, int write_locks) {
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

static int DataMgrLockForWriting(data_module_t *module, data_db_t *db) {
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

static int DataMgrLockForReading(data_module_t *module, data_db_t *db) {
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

static int DataMgrUnlockForReading(data_module_t *module, data_db_t *db) {
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

static int DataMgrUnlockForWriting(data_module_t *module, data_db_t *db) {
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
  data_db_t db;
  vfs_file_t *f;
  LocalID dbID;
  SysNotifyParamType notify;
  SysNotifyDBCreatedType dbCreated;
  Err err = dmErrInvalidParam;

  if (nameP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      sys_memset(&db, 0, sizeof(data_db_t));
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
      if (attr & dmHdrAttrResDB) {
        db.ftype = DATAMGR_TYPE_RES;
      } else if (attr & dmHdrAttrStream) {
        db.ftype = DATAMGR_TYPE_FILE;
        DataMgrName(module->dm->path, db.name, DATAMGR_FILE_DATA, 0, 0, 0, 0, buf);
        if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          vfs_close(f);
        }
      } else {
        db.ftype = DATAMGR_TYPE_REC;
        DataMgrName(module->dm->path, db.name, DATAMGR_FILE_INDEX, 0, 0, 0, 0, buf);
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
  UInt32 uniqueIDSeed, dummy32, type, creator, size, i, j;
  UInt16 attr, version, numRecs, index, appInfoSize, sortInfoSize;
  UInt32 *offsets, *resTypes, *uniqueIDs, firstOffset;
  UInt16 *resIDs, *recAttrs;
  UInt8 *database, dummy8;
  LocalID dbID, appInfoID, sortInfoID;
  MemHandle appInfoH, sortInfoH;
  vfs_file_t *indexFile;
  void *appInfoP, *sortInfoP;
  DmOpenType *dbRef;
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
          if ((dbRef = DDmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
            if (numRecs > 0) {
              resTypes = sys_calloc(numRecs, sizeof(UInt32));
              resIDs = sys_calloc(numRecs, sizeof(UInt16));
              recAttrs = sys_calloc(numRecs, sizeof(UInt16));
              uniqueIDs = sys_calloc(numRecs, sizeof(UInt32));
              offsets = sys_calloc(numRecs, sizeof(UInt32));

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
                  DDmNewResourceEx(dbRef, resTypes[j], resIDs[j], size, &database[offsets[j]]);
                }
                err = errNone;
              } else {
                for (j = 0; j < numRecs; j++) {
                  i += get4b(&offsets[j], database, i);
                  i += get1(&dummy8, database, i);
                  recAttrs[j] = dummy8;
                  i += get1(&dummy8, database, i);
                  uniqueIDs[i] = ((UInt32)dummy8) << 16;
                  i += get1(&dummy8, database, i);
                  uniqueIDs[i] |= ((UInt32)dummy8) << 8;
                  i += get1(&dummy8, database, i);
                  uniqueIDs[i] |= ((UInt32)dummy8);
                }
                i = firstOffset = offsets[0];

                indexFile = DataMgrOpenIndex(module, dbID);

                for (j = 0; j < numRecs; j++) {
                  size = (j < numRecs-1) ? offsets[j+1] - offsets[j] : DMemPtrSize(bufferP) - offsets[j];
                  index = dmMaxRecordIndex;
                  DDmNewRecordEx(dbRef, &index, size, &database[offsets[j]]);
                  DDmSetRecordInfo(dbRef, index, &recAttrs[j], &uniqueIDs[j]);
                  DataMgrWriteIndex(indexFile, uniqueIDs[j], recAttrs[j]);
                }

                vfs_close(indexFile);
              }

              sys_free(resTypes);
              sys_free(resIDs);
              sys_free(recAttrs);
              sys_free(uniqueIDs);
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

            DDmCloseDatabase(dbRef);
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
  data_db_t db;
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
  data_db_t db;
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
  data_db_t db;
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
  data_db_t db;
  UInt32 size;
  char buf[VFS_PATH], buf2[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      if (numRecordsP) *numRecordsP = db.numRecs;
      DataMgrName(module->dm->path, db.name, 0, 0, 0, 0, 0, buf);
      if ((dir = vfs_opendir(module->dm->session, buf)) != NULL) {
        if (numRecordsP) *numRecordsP = 0;
        if (totalBytesP) *totalBytesP = 0;
        if (dataBytesP) *dataBytesP = 0;
        for (;;) {
          ent = vfs_readdir(dir);
          if (ent == NULL) break;
          if (ent->type != VFS_FILE) continue;
          if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue;
          if (!sys_strchr(ent->name, '.')) continue; // ignore header, index, lock, etc
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
  data_db_t db;
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
  data_db_t db;
  Boolean ok;
  DmOpenType *first, *dbRef = NULL;
  Err err = dmErrInvalidParam;

  if (mutex_lock(module->dm->mutex) == 0) {
    if (DataMgrReadHeader(module, dbID, NULL, &db) == 0) {
      if ((dbRef = pumpkin_heap_alloc(sizeof(DmOpenType), "dbRef")) != NULL) {
        if (mode & dmModeWrite) {
          ok = DataMgrLockForWriting(module, &db) == 0;
        } else if (mode & dmModeReadOnly) {
          ok = DataMgrLockForReading(module, &db) == 0;
        } else {
          debug(DEBUG_ERROR, "DataMgr", "DmOpenDatabase dbID %u name \"%s\" invalid mode 0x%04X", dbID & ~DBID_MASK, db.name, mode);
          ok = false;
        }
        if (ok) {
          dbRef->dbID = dbID;
          dbRef->mode = mode;
          err = errNone;
        } else {
          pumpkin_heap_free(dbRef, "dbRef");
          dbRef = NULL;
        }
      }
    }

    if (dbRef) {
      first = module->dbRef;
      if (first) {
        first->prev = dbRef;
        dbRef->next = first;
      }
      module->dbRef = dbRef;
    }

    mutex_unlock(module->dm->mutex);
  }

  DataMgrCheckErr(err);
  return dbRef;
}

DmOpenRef DDmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode) {
  DM_MODULE;
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenType *dbRef = NULL;
  Err err;

  err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, type, creator, false, &cardNo, &dbID);
  if (err == errNone) {
    dbRef = DmOpenDatabase(cardNo, dbID, mode);
  }

  DataMgrCheckErr(err);
  return dbRef;
}

DmOpenRef DDmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  return DmOpenDatabase(cardNo, dbID, mode);
}

Err DDmCloseDatabase(DmOpenRef dbP) {
  DM_MODULE;
  DmOpenType *dbRef;
  data_db_t db;
  Err err = dmErrInvalidParam;

  if (dbP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (DataMgrReadHeader(module, dbRef->dbID, NULL, &db) == 0) {
        if (dbRef->mode & dmModeWrite) {
          debug(DEBUG_TRACE, "DataMgr", "DmCloseDatabase \"%s\" writeCount %d -> %d", db.name, db.writeCount, db.writeCount-1);
          if (DataMgrUnlockForWriting(module, &db) == 0) err = errNone;
        } else if (dbRef->mode & dmModeReadOnly) {
          debug(DEBUG_TRACE, "DataMgr", "DmCloseDatabase \"%s\" readCount %d -> %d", db.name, db.readCount, db.readCount-1);
          if (DataMgrUnlockForReading(module, &db) == 0) err = errNone;
        } else {
          debug(DEBUG_ERROR, "DataMgr", "DmCloseDatabase \"%s\" dbRef invalid mode 0x%04X", db.name, dbRef->mode);
        }

        if (dbRef->h) sys_free(dbRef->h);

        if (dbRef->prev) {
          dbRef->prev->next = dbRef->next;
          if (dbRef->next) dbRef->next->prev = dbRef->prev;
        } else {
          if (dbRef->next) dbRef->next->prev = dbRef->prev;
          module->dbRef = dbRef->next;
        }
        pumpkin_heap_free(dbRef, "dbRef");
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return err;
}

DmOpenRef DDmNextOpenDatabase(DmOpenRef currentP) {
  DM_MODULE;
  DmOpenType *dbRef = NULL;

  if (currentP) {
    dbRef = (DmOpenType *)currentP;
    dbRef = dbRef->next;
  } else {
    dbRef = module->dbRef;
  }

  return dbRef;
}

Err DDmOpenDatabaseInfo(DmOpenRef dbP, LocalID *dbIDP, UInt16 *openCountP, UInt16 *modeP, UInt16 *cardNoP, Boolean *resDBP) {
  DM_MODULE;
  data_db_t db;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (DataMgrReadHeader(module, dbRef->dbID, NULL, &db) == 0) {
      if (dbIDP) *dbIDP = dbRef->dbID;
      if (openCountP) *openCountP = db.readCount + db.writeCount;
      if (modeP) *modeP = dbRef->mode;
      if (cardNoP) *cardNoP = 0;
      if (resDBP) *resDBP = db.ftype == DATAMGR_TYPE_RES;
      err = errNone;
    }
  }

  DataMgrCheckErr(err);
  return err;
}

LocalID DDmGetAppInfoID(DmOpenRef dbP) {
  DM_MODULE;
  data_db_t db;
  DmOpenType *dbRef;
  LocalID appInfo = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (DataMgrReadHeader(module, dbRef->dbID, NULL, &db) == 0) {
      appInfo = db.appInfoID;
      err = errNone;
    }
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
  DmOpenType *dbRef;
  UInt32 numRecords = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    err = DDmDatabaseSize(0, dbRef->dbID, &numRecords, NULL, NULL);
  }

  DataMgrCheckErr(err);
  return numRecords;
}

UInt16 DDmNumRecordsInCategory(DmOpenRef dbP, UInt16 category) {
  return 0;
}

Err DDmRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP, LocalID *chunkIDP) {
  return 0;
}

Err DDmSetRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP) {
  return 0;
}

Err DDmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP) {
  return 0;
}

Err DDmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  return 0;
}

Err DDmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to) {
  return 0;
}

MemHandle DDmNewRecordEx(DmOpenRef dbP, UInt16 *atP, UInt32 size, void *p) {
  DM_MODULE;
  DmOpenType *dbRef;
  data_db_t db;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  Err err = dmErrInvalidParam;
  DmHandle *h = NULL;

  if (dbP && atP && size > 0) {
    if (mutex_lock(module->dm->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->mode & dmModeWrite) {
        if (DataMgrReadHeader(module, dbRef->dbID, NULL, &db) == 0) {
          if (db.ftype == DATAMGR_TYPE_REC) {
            if (*atP >= db.numRecs) *atP = db.numRecs;
            if ((h = pumpkin_heap_alloc(sizeof(DmHandle) + size, "DmHandle")) != NULL) {
              h->magic = DATAMGR_MAGIC;
              h->htype = DATAMGR_TYPE_REC;
              h->owner = pumpkin_get_current();
              h->size = size;
              h->dbID = db.dbID;
              h->d.rec.uniqueID = db.uniqueIDSeed++;
              h->d.rec.index = *atP;
              h->d.rec.attr = dmRecAttrDirty | dmRecAttrBusy;
              if (p) {
                sys_memcpy(h->buf, p, size);
              }

              sys_snprintf(name, sizeof(name)-1, "%u", db.dbID);
              DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, 0x00, h->d.rec.uniqueID, buf);
              if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                vfs_write(f, h->buf, size);
                vfs_close(f);
              }

              db.modDate = TimGetSeconds();
              if (DataMgrWriteHeader(module, db.dbID, NULL, &db) == 0) {
                err = errNone;
              }
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
  return DDmNewRecordEx(dbP, atP, size, NULL);
}

Err DDmRemoveRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

Err DDmDeleteRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

Err DDmArchiveRecord(DmOpenRef dbP, UInt16 index) {
  return 0;
}

MemHandle DDmNewHandle(DmOpenRef dbP, UInt32 size) {
  return 0;
}

Err DDmRemoveSecretRecords(DmOpenRef dbP) {
  return 0;
}

Err DDmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, UInt16 *indexP) {
  return 0;
}

static MemHandle DDmQueryRecordEx(DmOpenRef dbP, UInt16 index, Boolean setBusy) {
  DM_MODULE;
  DmOpenType *dbRef;
  data_db_t db;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  UInt32 id, uniqueID, size, i;
  UInt8 a, attr;
  Boolean found;
  DmHandle *h = NULL;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    if (mutex_lock(module->dm->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (DataMgrReadHeader(module, dbRef->dbID, NULL, &db) == 0) {
        if (db.ftype == DATAMGR_TYPE_REC) {
          for (i = 0; i < dbRef->numHandles; i++) { 
            if (dbRef->h[i]->d.rec.index == index) {
              dbRef->h[i]->useCount++;
              err = errNone;
              mutex_unlock(module->dm->mutex);
              DataMgrCheckErr(err);
              return dbRef->h[i];
            }
          }

          db.numRecs = 0;
          found = false;
          if ((f = DataMgrOpenIndex(module, dbRef->dbID)) != NULL) {
            for (;;) {
              if (DataMgrReadIndex(f, &id, &a) != 0) break;
              if (index == db.numRecs) {
                uniqueID = id;
                attr = a;
                found = false;
              }
              db.numRecs++;
            }
            vfs_close(f);
          }

          if (!found) {
            mutex_unlock(module->dm->mutex);
            DataMgrCheckErr(err);
            return NULL;
          }

          sys_snprintf(name, sizeof(name)-1, "%u", db.dbID);
          DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, 0, 0, attr & ATTR_MASK, uniqueID, buf);

          if ((f = vfs_open(module->dm->session, buf, VFS_READ)) != NULL) {
            if ((size = vfs_seek(f, 0, 1)) > 0) {
              vfs_seek(f, 0, 0);
              if ((h = DMemHandleNew(size)) != NULL) {
                h->htype = DATAMGR_TYPE_REC;
                if (vfs_read(f, h->buf, h->size) == h->size) {
                  h->d.rec.uniqueID = uniqueID;
                  h->d.rec.index = index;
                  h->d.rec.attr = attr & ~dmRecAttrDirty;
                  h->d.rec.attr |= dmRecAttrBusy;
                  h->useCount = 1;
                  h->lockCount = 0;
                  if (dbRef->numHandles == dbRef->capHandles) {
                    dbRef->capHandles += CAP_HANDLE;
                    dbRef->h = dbRef->capHandles ?
                      sys_realloc(dbRef->h, dbRef->capHandles * sizeof(DmHandle *)):
                      sys_calloc(dbRef->capHandles, sizeof(DmHandle *));
                  }
                  dbRef->h[dbRef->numHandles++] = h;
                  err = errNone;
                } else {
                  err = dmErrMemError;
                  DMemHandleFree(h);
                  h = NULL;
                }
              }
            }
            vfs_close(f);
          } else {
            err = dmErrMemError;
          }
        }
      }
      mutex_unlock(module->dm->mutex);
    }
  }

  DataMgrCheckErr(err);
  return h;
}

MemHandle DDmQueryRecord(DmOpenRef dbP, UInt16 index) {
  return DDmQueryRecordEx(dbP, index, false);
}

MemHandle DDmGetRecord(DmOpenRef dbP, UInt16 index) {
  return DDmQueryRecordEx(dbP, index, true);
}

MemHandle DDmQueryNextInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 category) {
  return 0;
}

UInt16 DDmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category) {
  return 0;
}

Err DDmSeekRecordInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category) {
  return 0;
}

MemHandle DDmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize) {
  return 0;
}

Err DDmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty) {
  return 0;
}

UInt16 DDmSearchRecord(MemHandle recH, DmOpenRef *dbPP) {
  return 0;
}

Err DDmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty) {
  return 0;
}

Err DDmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum) {
  return 0;
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
  DmOpenType *dbRef;
  data_db_t db;
  vfs_file_t *f;
  char name[16], buf[VFS_PATH];
  Err err = dmErrInvalidParam;
  DmHandle *h = NULL;
  
  if (dbP && size > 0) {
    if (mutex_lock(module->dm->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->mode & dmModeWrite) {
        if (DataMgrReadHeader(module, dbRef->dbID, NULL, &db) == 0) {
          if (db.ftype == DATAMGR_TYPE_RES) {
            if ((h = pumpkin_heap_alloc(sizeof(DmHandle) + size, "DmHandle")) != NULL) {
              h->magic = DATAMGR_MAGIC;
              h->htype = DATAMGR_TYPE_RES;
              h->owner = pumpkin_get_current();
              h->size = size;
              h->dbID = db.dbID;
              h->d.res.type = resType;
              h->d.res.id = resID;
              h->d.res.attr = dmRecAttrDirty;
              if (p) {
                sys_memcpy(h->buf, p, size);
              }

              sys_snprintf(name, sizeof(name)-1, "%u", db.dbID);
              DataMgrName(module->dm->path, name, DATAMGR_FILE_ELEMENT, resID, resType, 0x00, 0, buf);
              if ((f = vfs_open(module->dm->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                vfs_write(f, h->buf, size);
                vfs_close(f);
              }

              db.modDate = TimGetSeconds();
              if (DataMgrWriteHeader(module, db.dbID, NULL, &db) == 0) {
                err = errNone;
              }
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

Err DDmQuickSort(DmOpenRef dbP, DmComparF *compar, Int16 other) {
  return 0;
}

Err DDmInsertionSort(DmOpenRef dbR, DmComparF *compar, Int16 other) {
  return 0;
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
  uint8_t *q;
  Err err = memErrInvalidParam;

  if (p) {
    q = (uint8_t *)p;
    q -= sizeof(DmHandle);
    h = (DmHandle *)q;
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
  Err err = dmErrInvalidParam;

  if (size) {
    if ((h = pumpkin_heap_alloc(sizeof(DmHandle) + size, "DmHandle")) != NULL) {
      h->magic = DATAMGR_MAGIC;
      h->htype = DATAMGR_TYPE_MEM;
      h->owner = pumpkin_get_current();
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
        pumpkin_heap_free(hh, "DmHandle");
        break;
      default:
        debug(DEBUG_ERROR, "DataMgr", "MemHandleFree %p unexpected handle type %d", hh, hh->htype);
        break;
    }
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
          debug(DEBUG_ERROR, "STOR", "MemHandleUnlock %p unexpected handle type %d", hh, hh->htype);
          break;
      }
    } else {
      debug(DEBUG_ERROR, "STOR", "MemHandleUnlock %p is not locked", hh);
    }
  }

  DataMgrCheckErr(err);
  return err;
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
        id = (uint8_t *)hh - (uint8_t *)pumpkin_heap_base();
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

  if (local > 0 && !(local & DBID_MASK) && local < pumpkin_heap_size()) {
    h = (DmHandle *)((uint8_t *)pumpkin_heap_base() + local);
    if (h->magic == DATAMGR_MAGIC) {
      err = errNone;
    } else {
      h = NULL;
    }
  }

  DataMgrCheckErr(err);
  return h;
}
