#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "util.h"
#include "pumpkin.h"
#include "ssfn_font.h"
#include "DDm.h"
#include "debug.h"
#include "storage.h"

#define MAX_STORAGE_PATH 256

#define STO_INFLATED 0x8000u

#define STO_TYPE_MEM  1
#define STO_TYPE_REC  2
#define STO_TYPE_RES  3
#define STO_TYPE_FILE 5

#define HANDLES_PER_PAGE 1024

#define STO_FILE_ELEMENT 1
#define STO_FILE_HEADER  2
#define STO_FILE_INDEX   3
#define STO_FILE_DATA    4
#define STO_FILE_AINFO   5
#define STO_FILE_SINFO   6
#define STO_FILE_LOCK    7

#define ATTR_MASK (dmRecAttrDelete | dmRecAttrSecret | dmRecAttrCategoryMask)

// fake attribute to indicate if a handle belongs to a database
// or if it is "loose" in the heap
#define dmRecAttached 0x8000

#define STO_MAGIC 'Hndl'

static const char *watchName = "tempData";

typedef struct storage_handle_t {
  uint32_t magic;
  uint16_t htype;
  uint16_t owner;
  uint16_t useCount;
  uint16_t lockCount;
  uint32_t size;
  union {
    struct {
      uint32_t uniqueID;
      uint16_t attr;
    } rec;
    struct {
      void *decoded;
      void (*destructor)(void *);
      void *(*encoder)(void *, uint32_t *size);
      uint32_t decodedSize;
      uint32_t type;
      uint16_t id;
      uint16_t attr;
      uint8_t writable;
    } res;
  } d;
  uint8_t *buf;
} storage_handle_t;

typedef struct storage_db_t {
  uint32_t ftype, readCount, writeCount, uniqueIDSeed;
  uint16_t mode, numRecs, protect;

  uint32_t creator, type, crDate, modDate, bckDate, modNum;
  uint32_t attributes, version, appInfoID, sortInfoID;
  char name[dmDBNameLength];
  vfs_file_t *f;

  storage_handle_t **elements;
  uint32_t totalElements;
  struct storage_db_t *next;
} storage_db_t;

typedef struct DmOpenType {
  LocalID dbID;
  UInt16 mode;
  MemHandle ovh;
  Boolean isOverlay;
  OmOverlaySpecType *ov;
  struct DmOpenType *overlayDb;
  struct DmOpenType *prev, *next;
} DmOpenType;

typedef struct {
  mutex_t *mutex;
  uint8_t *base;
  uint8_t *end;
  uint32_t size;
  vfs_session_t *session;
  uint32_t num_storage;
  char path[MAX_STORAGE_PATH];
  storage_db_t *list;
  DmOpenType *dbRef;
  Err lastErr;
  MemHandle appInfoH;
  UInt8 recInfo[8];
  DmComparF *comparF;
  UInt32 comparF68K;
  Int16 other;
  LocalID watchID;
  storage_db_t *tmpDb;
  UInt8 fontFamily;
  UInt8 fontStyle;
  UInt16 fontSize;
} storage_t;

static void StoDecodeResource(storage_handle_t *res, Boolean decoded);

static void *StoPtrNew(storage_handle_t *h, UInt32 size, UInt32 type, UInt16 id) {
  void **q;
  char st[8];
  UInt8 *p = NULL;

  if ((q = pumpkin_heap_alloc(sizeof(storage_handle_t *) + size, "HandlePtr")) != NULL) {
    if (type) {
      pumpkin_id2s(type, st);
      debug(DEBUG_TRACE, "Heap", "RSRC %p %p %s %d", h, q, st, id);
    }
    q[0] = h;
    p = (UInt8 *)&q[1];
  }

  return p;
}

static void *StoPtrRealloc(storage_handle_t *h, void *p, UInt32 size) {
  void **q;

  if (p) {
    q = (void **)p;
    p = &q[-1];

    if ((q = pumpkin_heap_realloc(p, sizeof(storage_handle_t *) + size, "HandlePtr")) != NULL) {
      q[0] = h;
      p = (UInt8 *)&q[1];
    }
  }

  return p;
}

static void StoPtrFree(void *p) {
  void **q;

  if (p) {
    q = (void **)p;
    p = &q[-1];
    pumpkin_heap_free(p, "HandlePtr");
  }
}

static storage_handle_t *StoPtrRecoverHandle(void *p) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h = NULL;
  void **q;

  if (p) {
    if ((uint8_t *)p >= sto->base && (uint8_t *)p < sto->end) {
      q = (void **)p;
      h = (storage_handle_t *)q[-1];
      if ((uint8_t *)h >= sto->base && (uint8_t *)h < sto->end) {
        if (h->magic != STO_MAGIC) {
          debug(DEBUG_ERROR, "STOR", "StoPtrRecoverHandle invalid handle magic 0x%08X for handle %p pointer %p", h->magic, h, p);
          h = NULL;
        }
      } else {
        debug(DEBUG_INFO, "STOR", "StoPtrRecoverHandle invalid handle %p for pointer %p", h, p);
        h = NULL;
      }
    } else {
      debug(DEBUG_ERROR, "STOR", "StoPtrRecoverHandle invalid pointer %p", p);
    }
  }

  return h;
}

static void StoEscapeName(uint8_t *src, char *dst, int n) {
  int i, j;

  for (i = 0, j = 0; src[i] && j < n-1; i++) {
    if ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z') || (src[i] >= '0' && src[i] <= '9')) {
      dst[j++] = src[i];
    } else {
      dst[j++] = '_';
    }
  }
  dst[j++] = '_';

  for (i = 0; src[i] && j < n-1; i++, j += 2) {
    sys_snprintf(&dst[j], n-j, "%02X", src[i]);
  }
  dst[j++] = 0;

  debug(DEBUG_TRACE, "STOR", "StoEscapeName \"%s\" -> \"%s\"", src, dst);
}

static void StoUnescapeName(char *src, char *dst, int n) {
  char buf[4];
  int i, j, c;

  for (j = 0, i = 0; src[j]; j++) {
    if (src[j] == '_') i = j;
  }

  for (j = 0, i++; src[i] && j < n-1; i += 2) {
    buf[0] = src[i];
    buf[1] = src[i+1];
    buf[2] = 0;
    sys_sscanf(buf, "%02X", &c);
    dst[j++] = c;
  }
  dst[j] = 0;

  debug(DEBUG_TRACE, "STOR", "StoUnescapeName \"%s\" -> \"%s\"", src, dst);
}

static vfs_file_t *StoVfsOpen(vfs_session_t *session, char *path, int mode) {
  return vfs_open(session, path, mode);
}

static int StoVfsMkdir(vfs_session_t *session, char *path) {
  return vfs_mkdir(session, path);
}

static int StoVfsRename(vfs_session_t *session, char *path1, char *path2) {
  return vfs_rename(session, path1, path2);
}

static int StoVfsUnlink(vfs_session_t *session, char *path) {
  return vfs_unlink(session, path);
}

static vfs_dir_t *StoVfsOpendir(vfs_session_t *session, char *path) {
  return vfs_opendir(session, path);
}

static vfs_ent_t *StoVfsReaddir(vfs_dir_t *dir) {
  return vfs_readdir(dir);
}

static int StoVfsChecktype(vfs_session_t *session, char *path) {
  return vfs_checktype(session, path);
}

static void *StoVfsLoadlib(vfs_session_t *session, char *path, int *first_load) {
  return vfs_loadlib(session, path, first_load);
}

static void storage_name(storage_t *sto, char *name, int file, int id, uint32_t type, uint8_t attr, uint32_t uniqueId, char *buf) {
  char escaped[4*dmDBNameLength], st[8];
  int n, i;

  StoEscapeName((uint8_t *)name, escaped, sizeof(escaped)-1);
  sys_snprintf(buf, VFS_PATH - 1, "%s%s", sto->path, escaped);
  n = sys_strlen(buf);

  switch (file) {
    case STO_FILE_HEADER:
      sys_strncat(buf, "/header", VFS_PATH-n-1);
      break;
    case STO_FILE_INDEX:
      sys_strncat(buf, "/index", VFS_PATH-n-1);
      break;
    case STO_FILE_DATA:
      sys_strncat(buf, "/data", VFS_PATH-n-1);
      break;
    case STO_FILE_AINFO:
      sys_strncat(buf, "/appInfo", VFS_PATH-n-1);
      break;
    case STO_FILE_SINFO:
      sys_strncat(buf, "/sortInfo", VFS_PATH-n-1);
      break;
    case STO_FILE_LOCK:
      sys_strncat(buf, "/lock", VFS_PATH-n-1);
      break;
    case STO_FILE_ELEMENT:
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
      debug(DEBUG_ERROR, "STOR", "storage_name \"%s\" invalid file type %d", name, file);
      ErrFatalDisplayEx("invalid file type", 1);
      break;
  }
}

static int StoInflateRec(storage_t *sto, storage_db_t *db, storage_handle_t *h) {
  char buf[VFS_PATH];
  vfs_file_t *f;
  int r = -1;

  if ((h->buf = StoPtrNew(h, h->size, 0, 0)) != NULL) {
    h->htype |= STO_INFLATED;
    h->useCount = 1;
    debug(DEBUG_TRACE, "STOR", "reading record at %p", h->buf);
    storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
    if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
      if (vfs_read(f, h->buf, h->size) > 0) {
        h->d.rec.attr &= ~dmRecAttrDirty;
        h->d.rec.attr |= dmRecAttrBusy;
        h->lockCount = 0;
        r = 0;
      }
      vfs_close(f);
    }
  }

  return r;
}

static int StoWriteIndex(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  vfs_file_t *f;
  storage_handle_t *h;
  uint32_t i;
  int r = -1;

  storage_name(sto, db->name, STO_FILE_INDEX, 0, 0, 0, 0, buf);
  if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
    for (i = 0; i < db->numRecs; i++) {
      h = db->elements[i];
      sys_snprintf(buf, sizeof(buf)-1, "%08X.%02X\n", h->d.rec.uniqueID, h->d.rec.attr & ATTR_MASK);
      if (vfs_write(f, (uint8_t *)buf, 12) != 12) break;
    }
    r = 0;
    vfs_close(f);
  } else {
    ErrFatalDisplayEx("create index failed", 1);
  }

  return r;
}

static int StoWriteHeader(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  char stype[8], screator[8];
  vfs_file_t *f;
  int n, r = -1;

  storage_name(sto, db->name, STO_FILE_HEADER, 0, 0, 0, 0, buf);
  if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
    pumpkin_id2s(db->type, stype);
    pumpkin_id2s(db->creator, screator);
    sys_snprintf(buf, sizeof(buf)-1, "ftype=%u\ntype='%4s'\ncreator='%4s'\nattributes=%u\nuniqueIDSeed=%u\nversion=%u\ncrDate=%u\nmodDate=%u\nbckDate=%u\nmodNum=%d\n",
      db->ftype, stype, screator, db->attributes, db->uniqueIDSeed, db->version, db->crDate, db->modDate, db->bckDate, db->modNum);
    n = sys_strlen(buf);
    if (vfs_write(f, (uint8_t *)buf, n) == n) {
      r = 0;
    }
    vfs_close(f);
  } else {
    ErrFatalDisplayEx("create header failed", 1);
  }

  return r;
}

static int StoReadHeader(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  char stype[8], screator[8];
  vfs_file_t *f;
  int r = -1;

  storage_name(sto, db->name, STO_FILE_HEADER, 0, 0, 0, 0, buf);
  if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
    sys_memset(buf, 0, sizeof(buf));
    if (vfs_read(f, (uint8_t *)buf, sizeof(buf)-1) > 0) {
      if (sys_sscanf(buf, "ftype=%u\ntype='%c%c%c%c'\ncreator='%c%c%c%c'\nattributes=%u\nuniqueIDSeed=%u\nversion=%u\ncrDate=%u\nmodDate=%u\nbckDate=%u\nmodNum=%d\n",
           &db->ftype, stype, stype+1, stype+2, stype+3, screator, screator+1, screator+2, screator+3,
           &db->attributes, &db->uniqueIDSeed, &db->version, &db->crDate, &db->modDate, &db->bckDate, &db->modNum) == 16) {
        stype[4] = 0;
        pumpkin_s2id(&db->type, stype);
        screator[4] = 0;
        pumpkin_s2id(&db->creator, screator);
        r = 0;
      } else {
        debug(DEBUG_ERROR, "STOR", "invalid header \"%s\"", buf);
      }
    }
    vfs_close(f);
  }

  return r;
}

static int StoGetFileLocks(storage_t *sto, storage_db_t *db, int *read_locks, int *write_locks) {
  char buf[VFS_PATH];
  vfs_file_t *f;
  int r = -1;

  *read_locks = 0;
  *write_locks = 0;

  storage_name(sto, db->name, STO_FILE_LOCK, 0, 0, 0, 0, buf);
  if (StoVfsChecktype(sto->session, buf) == VFS_FILE) {
    if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
      sys_memset(buf, 0, sizeof(buf));
      if (vfs_read(f, (uint8_t *)buf, sizeof(buf)-1) > 0) {
        if (sys_sscanf(buf, "read=%u\nwrite=%u\n", read_locks, write_locks) == 2) {
          r = 0;
        } else {
          debug(DEBUG_ERROR, "STOR", "invalid lock \"%s\"", buf);
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

static int StoPutFileLocks(storage_t *sto, storage_db_t *db, int read_locks, int write_locks) {
  char buf[VFS_PATH];
  vfs_file_t *f;
  int n, r = -1;

  storage_name(sto, db->name, STO_FILE_LOCK, 0, 0, 0, 0, buf);
  if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
    sys_snprintf(buf, sizeof(buf)-1, "read=%u\nwrite=%u\n", read_locks, write_locks);
    n = sys_strlen(buf);
    if (vfs_write(f, (uint8_t *)buf, n) == n) {
      r = 0;
    }
    vfs_close(f);
  } else {
    //ErrFatalDisplayEx("create lock failed", 1);
    debug(DEBUG_ERROR, "STOR", "create lock failed for \"%s\"", db->name);
  }

  return r;
}

static int StoLockForReading(storage_t *sto, storage_db_t *db) {
  int read_locks, write_locks;
  int r = -1;

  if (StoGetFileLocks(sto, db, &read_locks, &write_locks) == 0) {
    if (db->writeCount < write_locks) {
      debug(DEBUG_ERROR, "STOR", "StoLockForReading file \"%s\" already locked for writing (other)", db->name);
    } else {
      if (db->writeCount > 0) {
        debug(DEBUG_INFO, "STOR", "StoLockForReading file \"%s\" already locked for writing (own)", db->name);
      }
      debug(DEBUG_TRACE, "STOR", "StoLockForReading \"%s\" readCount %d -> %d", db->name, db->readCount, db->readCount+1);
      db->readCount++;
      read_locks++;
      r = StoPutFileLocks(sto, db, read_locks, write_locks);
    }
  }

  return r;
}

static int StoUnlockForReading(storage_t *sto, storage_db_t *db) {
  int read_locks, write_locks;
  int r = -1;

  if (db->readCount > 0) {
    if (StoGetFileLocks(sto, db, &read_locks, &write_locks) == 0) {
      if (read_locks > 0) {
        debug(DEBUG_TRACE, "STOR", "StoUnlockForReading \"%s\" readCount %d -> %d", db->name, db->readCount, db->readCount-1);
        db->readCount--;
        read_locks--;
        r = StoPutFileLocks(sto, db, read_locks, write_locks);
      } else {
        debug(DEBUG_ERROR, "STOR", "StoUnlockForReading file \"%s\" not locked for reading", db->name);
      }
    }
  } else {
    debug(DEBUG_ERROR, "STOR", "StoUnlockForReading file \"%s\" not open for reading", db->name);
  }

  return r;
}

static int StoLockForWriting(storage_t *sto, storage_db_t *db) {
  int read_locks, write_locks;
  int r = -1;

  if (StoGetFileLocks(sto, db, &read_locks, &write_locks) == 0) {
    if (db->readCount < read_locks) {
      debug(DEBUG_ERROR, "STOR", "StoLockForWriting file \"%s\" already locked for reading (other)", db->name);
    } else if (db->writeCount < write_locks) {
      debug(DEBUG_ERROR, "STOR", "StoLockForWriting file \"%s\" already locked for writing (other)", db->name);
    } else {
      if (db->readCount > 0) {
        debug(DEBUG_INFO, "STOR", "StoLockForWriting file \"%s\" already locked for reading (own)", db->name);
      }
      debug(DEBUG_TRACE, "STOR", "StoLockForWriting \"%s\" writeCount %d -> %d", db->name, db->writeCount, db->writeCount+1);
      db->writeCount++;
      write_locks++;
      r = StoPutFileLocks(sto, db, read_locks, write_locks);
    }
  }

  return r;
}

static int StoUnlockForWriting(storage_t *sto, storage_db_t *db) {
  int read_locks, write_locks;
  int r = -1;

  if (db->writeCount > 0) {
    if (StoGetFileLocks(sto, db, &read_locks, &write_locks) == 0) {
      if (write_locks > 0) {
        db->writeCount--;
        write_locks--;
        r = StoPutFileLocks(sto, db, read_locks, write_locks);
      } else {
        debug(DEBUG_ERROR, "STOR", "StoUnlockForWriting file \"%s\" not locked for writing", db->name);
      }
    }
  } else {
    debug(DEBUG_ERROR, "STOR", "StoUnlockForWriting file \"%s\" not open for writing", db->name);
  }

  return r;
}

static int StoReadAppInfo(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  vfs_ent_t *ent;
  vfs_file_t *f;
  MemHandle h;
  void *p;
  int r = -1;

  storage_name(sto, db->name, STO_FILE_AINFO, 0, 0, 0, 0, buf);
  if (StoVfsChecktype(sto->session, buf) != -1) {
    if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
      if ((ent = vfs_fstat(f)) != NULL) {
        if ((h = MemHandleNew(ent->size)) != NULL) {
          if ((p = MemHandleLock(h)) != NULL) {
            if (vfs_read(f, p, ent->size) == ent->size) {
              r = 0;
            }
            MemHandleUnlock(h);
            if (r == 0) {
              db->appInfoID = MemHandleToLocalID(h);
            }
          }
        }
      }
      vfs_close(f);
    }
  } else {
    r = 0;
  }

  return r;
}

static int StoWriteAppInfo(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  UInt32 size;
  vfs_file_t *f;
  MemHandle h;
  void *p;
  int r = -1;

  if (db->appInfoID) {
    if ((h = MemLocalIDToHandle(db->appInfoID)) != NULL) {
      if ((p = MemHandleLock(h)) != NULL) {
        storage_name(sto, db->name, STO_FILE_AINFO, 0, 0, 0, 0, buf);
        if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          size = MemHandleSize(h);
          if (vfs_write(f, p, size) == size) {
            r = 0;
          }
          vfs_close(f);
        } else {
          ErrFatalDisplayEx("create appInfo failed", 1);
        }
      }
    }
  }

  return r;
}

static int StoReadSortInfo(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  vfs_ent_t *ent;
  vfs_file_t *f;
  MemHandle h;
  void *p;
  int r = -1;

  storage_name(sto, db->name, STO_FILE_SINFO, 0, 0, 0, 0, buf);
  if (StoVfsChecktype(sto->session, buf) != -1) {
    if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
      if ((ent = vfs_fstat(f)) != NULL) {
        if ((h = MemHandleNew(ent->size)) != NULL) {
          if ((p = MemHandleLock(h)) != NULL) {
            if (vfs_read(f, p, ent->size) == ent->size) {
              r = 0;
            }
            MemHandleUnlock(h);
            if (r == 0) {
              db->sortInfoID = MemHandleToLocalID(h);
            }
          }
        }
      }
      vfs_close(f);
    }
  } else {
    r = 0;
  }

  return r;
}
static int StoWriteSortInfo(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  UInt32 size;
  vfs_file_t *f;
  MemHandle h;
  void *p;
  int r = -1;

  if (db->sortInfoID) {
    if ((h = MemLocalIDToHandle(db->sortInfoID)) != NULL) {
      if ((p = MemHandleLock(h)) != NULL) {
        storage_name(sto, db->name, STO_FILE_SINFO, 0, 0, 0, 0, buf);
        if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          size = MemHandleSize(h);
          if (vfs_write(f, p, size) == size) {
            r = 0;
          }
          vfs_close(f);
        } else {
          ErrFatalDisplayEx("create sortInfo failed", 1);
        }
      }
    }
  }

  return r;
}

static vfs_ent_t *StoReadEnt(vfs_dir_t *dir) {
  vfs_ent_t *ent;

  for (;;) {
    ent = StoVfsReaddir(dir);
    if (ent == NULL) break;
    debug(DEBUG_TRACE, "STOR", "entry \"%s\" type %d", ent->name, ent->type);
    if (ent->type != VFS_DIR) continue;
    if (ent->name[0] == '.') continue;
    break;
  }

  return ent;
}

int StoInit(char *path, mutex_t *mutex) {
  storage_t *sto;
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  storage_db_t *db;
  LocalID dbID;
  int r = -1;

  if ((sto = sys_calloc(1, sizeof(storage_t))) != NULL) {
    sto->mutex = mutex;
    sto->base = pumpkin_heap_base();
    sto->size = pumpkin_heap_size();
    sto->end = sto->base + sto->size;
    sys_strncpy(sto->path, path, MAX_STORAGE_PATH - 1);
    if ((sto->session = vfs_open_session()) != NULL) {
      if ((dir = StoVfsOpendir(sto->session, sto->path)) != NULL) {
        for (;;) {
          ent = StoReadEnt(dir);
          if (ent == NULL) break;
          if ((db = pumpkin_heap_alloc(sizeof(storage_db_t), "storage_db")) == NULL) {
            vfs_closedir(dir);
            vfs_close_session(sto->session);
            sys_free(sto);
            sto = NULL;
            break;
          }
          StoUnescapeName(ent->name, db->name, dmDBNameLength);
          db->next = sto->list;
          if (StoReadHeader(sto, db) != 0) {
            pumpkin_heap_free(db, "storage_db");
            continue;
          }
          dbID = (uint8_t *)db - sto->base;
          debug(DEBUG_TRACE, "STOR", "StoInit 0x%08X database \"%s\"", dbID, db->name);
          sto->list = db;
          sto->num_storage++;
        }
        vfs_closedir(dir);
        if (sto) {
          sto->fontSize = 12;
          sto->fontFamily = PUMPKIN_FONT_FAMILY_SANS;
          sto->fontStyle = PUMPKIN_FONT_STYLE_REGULAR;
          pumpkin_set_local_storage(sto_key, sto);
          r = 0;
        }
      } else {
        vfs_close_session(sto->session);
        sys_free(sto);
      }
    } else {
      sys_free(sto);
    }
  }

  return r;
}

int StoRefresh(void) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  storage_db_t *db, *old, *oldList;
  LocalID dbID;
  char name[dmDBNameLength];
  int found, r = -1;

  if (sto) {
    if (mutex_lock(sto->mutex) == 0) {
      if ((dir = StoVfsOpendir(sto->session, sto->path)) != NULL) {
        oldList = sto->list;
        for (;;) {
          ent = StoReadEnt(dir);
          if (ent == NULL) break;
          StoUnescapeName(ent->name, name, dmDBNameLength);
          for (old = oldList, found = 0; old && !found; old = old->next) {
            found = sys_strncmp(old->name, name, sys_strlen(name)) == 0;
          }
          if (!found) {
            if ((db = pumpkin_heap_alloc(sizeof(storage_db_t), "storage_db")) != NULL) {
              sys_strncpy(db->name, name, dmDBNameLength-1);
              if (StoReadHeader(sto, db) == 0) {
                dbID = (uint8_t *)db - sto->base;
                debug(DEBUG_INFO, "STOR", "StoRefresh 0x%08X database \"%s\"", dbID, db->name);
                db->next = sto->list;
                sto->list = db;
                sto->num_storage++;
              } else {
                pumpkin_heap_free(db, "storage_db");
              }
            }
          }
        }
        vfs_closedir(dir);
      }
      mutex_unlock(sto->mutex);
    }
  }

  return r;
}

int StoFinish(void) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  DmOpenType *dbRef;
  storage_db_t *db;
  int read_locks, write_locks, r = -1;

  if (sto) {
    for (dbRef = sto->dbRef; dbRef; dbRef = dbRef->next) {
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        debug(DEBUG_ERROR, "STOR", "StoFinish database \"%s\" was left open", db->name);
        if (mutex_lock(sto->mutex) == 0) {
          if (StoGetFileLocks(sto, db, &read_locks, &write_locks) == 0) {
            StoPutFileLocks(sto, db, read_locks - db->readCount, write_locks - db->writeCount);
          }
          mutex_unlock(sto->mutex);
        }
      }
    }
    vfs_close_session(sto->session);
    pumpkin_set_local_storage(sto_key, NULL);
    sys_free(sto);
    r = 0;
  }

  return r;
}

void StoHeapWalk(uint32_t *p, uint32_t size, uint32_t task) {
  storage_handle_t *h = (storage_handle_t *)p;
  char st[8];

  if (h && size >= sizeof(storage_handle_t) && h->magic == STO_MAGIC) {
    if (h->owner == task) {
      h->owner = 0;

      switch (h->htype & ~STO_INFLATED) {
        case STO_TYPE_MEM:
          debug(DEBUG_INFO, "STOR", "leak mem handle %p size %5u inflated %d use %d lock %d",
            p, h->size, h->htype & STO_INFLATED ? 1 : 0, h->useCount, h->lockCount);
          // free the handle regardless of useCount and lockCount
          MemHandleFree((MemHandle)h);
          break;
        case STO_TYPE_REC:
          debug(DEBUG_INFO, "STOR", "leak rec handle %p size %5u inflated %d use %d lock %d attached %d",
            p, h->size, h->htype & STO_INFLATED ? 1 : 0, h->useCount, h->lockCount, (h->d.rec.attr & dmRecAttached) ? 1 : 0);
          // free the handle regardless of useCount and lockCount, but only if dmRecAttached is not set
          if (!(h->d.rec.attr & dmRecAttached)) {
            MemHandleFree((MemHandle)h);
          }
          break;
        case STO_TYPE_RES:
          pumpkin_id2s(h->d.res.type, st);
          debug(DEBUG_INFO, "STOR", "leak res handle %p size %5u inflated %d use %d lock %d  attached %d decoded %d type %s id %d buf %p",
            p, h->size, h->htype & STO_INFLATED ? 1 : 0, h->useCount, h->lockCount, (h->d.res.attr & dmRecAttached) ? 1 : 0, h->d.res.decoded ? 1 : 0, st, h->d.res.id, h->buf);
          // free the handle regardless of useCount and lockCount, but only if dmRecAttached is not set
          if (!(h->d.res.attr & dmRecAttached)) {
            MemHandleFree((MemHandle)h);
          }
          break;
        default:
          debug(DEBUG_ERROR, "STOR", "leak type %d handle %p size %5u inflated %d use %d lock %d",
            h->htype & ~STO_INFLATED, p, h->size, h->htype & STO_INFLATED ? 1 : 0, h->useCount, h->lockCount);
          break;
      }
    }
  } else if (task == 0) {
    debug(DEBUG_INFO, "STOR", "leak %p %u (%u)", p, size, (uint32_t)sizeof(storage_handle_t));
  }
}

Err DmInit(void) {
  return errNone;
}

#define StoCheckErr(err) \
  pumpkin_set_lasterr(err); \
  if (sto) sto->lastErr = err; \
  if (err && err != dmErrResourceNotFound) debug(DEBUG_ERROR, "STOR", "%s: error 0x%04X (%s)", __FUNCTION__, err, pumpkin_error_msg(err));

static storage_db_t *getdb(storage_t *sto, DmOpenRef dbP) {
  storage_db_t *db = NULL;
  DmOpenType *dbRef;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      switch (db->ftype) {
        case STO_TYPE_REC:
        case STO_TYPE_RES:
        case STO_TYPE_FILE:
          break;
        default:
          db = NULL;
          break;
      }
    }
  }

  if (!db) {
    ErrFatalDisplayEx("invalid db", 1);
  }

  return db;
}

Err DmOpenDatabaseInfo(DmOpenRef dbP, LocalID *dbIDP, UInt16 *openCountP, UInt16 *modeP, UInt16 *cardNoP, Boolean *resDBP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;

  if ((db = getdb(sto, dbP)) != NULL) {
    dbRef = (DmOpenType *)dbP;
    if (dbIDP) *dbIDP = dbRef->dbID;
    if (openCountP) *openCountP = db->readCount + db->writeCount;
    if (modeP) *modeP = dbRef->mode;
    if (cardNoP) *cardNoP = 0;
    if (resDBP) *resDBP = db->ftype == STO_TYPE_RES;
    err = errNone;
  }

  StoCheckErr(err);
  return err;
}

Err DmDatabaseInfo(UInt16 cardNo, LocalID dbID, Char *nameP,
     UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
     UInt32 *modDateP, UInt32 *bckUpDateP,
     UInt32 *modNumP, LocalID *appInfoIDP,
     LocalID *sortInfoIDP, UInt32 *typeP,
     UInt32 *creatorP) {

  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  Err err = dmErrInvalidParam;

  if (dbID && dbID < (sto->size - sizeof(storage_db_t))) {
    db = (storage_db_t *)(sto->base + dbID);
    if (nameP) sys_strncpy(nameP, db->name, dmDBNameLength-1);
    if (attributesP) *attributesP = db->attributes;
    if (versionP) *versionP = db->version;
    if (crDateP) *crDateP = db->crDate;
    if (modDateP) *modDateP = db->modDate;
    if (bckUpDateP) *bckUpDateP = db->bckDate;
    if (modNumP) *modNumP = db->modNum;
    if (typeP) *typeP = db->type;
    if (creatorP) *creatorP = db->creator;
    if (appInfoIDP) *appInfoIDP = db->appInfoID;
    if (sortInfoIDP) *sortInfoIDP = db->sortInfoID;
    err = errNone;
  }

  StoCheckErr(err);
  return err;
}

Err DmSetDatabaseInfo(UInt16 cardNo, LocalID  dbID, const Char *nameP,
    UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
    UInt32 *modDateP, UInt32 *bckUpDateP,
    UInt32 *modNumP, LocalID *appInfoIDP,
    LocalID *sortInfoIDP, UInt32 *typeP,
    UInt32 *creatorP) {

  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  char buf1[VFS_PATH], buf2[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (mutex_lock(sto->mutex) == 0) {
    if (dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *) (sto->base + dbID);
      if (attributesP) db->attributes = *attributesP;
      if (versionP) db->version = *versionP;
      if (crDateP) db->crDate = *crDateP;
      if (modDateP) db->modDate = *modDateP;
      if (bckUpDateP) db->bckDate = *bckUpDateP;
      if (modNumP) db->modNum = *modNumP;
      if (typeP) db->type = *typeP;
      if (creatorP) db->creator = *creatorP;

      if (appInfoIDP) {
        db->appInfoID = *appInfoIDP;
        StoWriteAppInfo(sto, db);

      }
      if (sortInfoIDP) {
        db->sortInfoID = *sortInfoIDP;
        StoWriteSortInfo(sto, db);
      }
      if (StoWriteHeader(sto, db) == 0) {
        if (nameP && sys_strcmp(db->name, nameP)) {
          storage_name(sto, db->name, 0, 0, 0, 0, 0, buf1);
          storage_name(sto, (char *)nameP, 0, 0, 0, 0, 0, buf2);
          if (StoVfsRename(sto->session, buf1, buf2) == 0) {
            sys_strncpy(db->name, nameP, dmDBNameLength - 1);
            err = errNone;
          }
        } else {
          err = errNone;
        }
      }
    }
    mutex_unlock(sto->mutex);
  }

  StoCheckErr(err);
  return err;
}

Err DmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP, UInt32 type, UInt32 creator, Boolean onlyLatestVers, UInt16 *cardNoP, LocalID *dbIDP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  Err err = dmErrCantFind;

  if (dbIDP) *dbIDP = 0;

  if (stateInfoP) {
    if (newSearch) {
      stateInfoP->p = sto->list;
    } else {
    }
    for (db = (storage_db_t *)stateInfoP->p; db; db = db->next) {
      if ((type != 0 && type != db->type) || (creator != 0 && creator != db->creator) || db->name[0] == 0) {
        stateInfoP->p = db->next;
        continue;
      }
      stateInfoP->p = db->next;
      if (cardNoP) *cardNoP = 0;
      if (dbIDP) *dbIDP = (uint8_t *)db - sto->base;
      err = errNone;
      break;
    }
  }

  return err;
}

LocalID DmGetDatabase(UInt16 cardNo, UInt16 index) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  UInt16 i;
  storage_db_t *db;
  LocalID dbID = 0;
  Err err = dmErrCantFind;

  for (i = 0, db = sto->list; db; db = db->next) {
    if (db->name[0]) {
      if (i == index) {
        dbID = (uint8_t *)db - sto->base;
        err = errNone;
        break;
      }
      i++;
    }
  }

  StoCheckErr(err);
  return dbID;
}

LocalID DmFindDatabase(UInt16 cardNo, const Char *nameP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  LocalID dbID = 0;
  Err err = dmErrCantFind;

  if (nameP) {
    for (db = sto->list; db; db = db->next) {
      if (sys_strcmp(db->name, nameP) == 0) {
        dbID = (uint8_t *)db - sto->base;
        if (dbID == sto->watchID) {
          debug(DEBUG_INFO, "STOR", "WATCH DmFindDatabase(\"%s\"): 0x%08X", nameP, dbID);
        }
        err = errNone;
        break;
      }
    }
  }

  StoCheckErr(err);
  return dbID;
}

static MemHandle DmQueryRecordEx(DmOpenRef dbP, UInt16 index, Boolean setBusy) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  vfs_file_t *f;
  DmOpenType *dbRef;
  char buf[VFS_PATH];
  storage_handle_t *h = NULL;
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC && index < db->numRecs && db->elements[index]) {
          h = db->elements[index];
          if (!(h->htype & STO_INFLATED)) {
            if ((h->buf = StoPtrNew(h, h->size, 0, 0)) != NULL) {
              h->htype |= STO_INFLATED;
              h->useCount = 1;
              storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
              if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                if (vfs_read(f, h->buf, h->size) == h->size) {
                  h->d.rec.attr &= ~dmRecAttrDirty;
                  h->d.rec.attr |= dmRecAttrBusy;
                  h->lockCount = 0;
                  err = errNone;
                } else {
                  err = dmErrMemError;
                  h = NULL;
                }
                vfs_close(f);
              } else {
                err = dmErrMemError;
                h = NULL;
              }
            } else {
              err = dmErrMemError;
              h = NULL;
            }
          } else {
            h->useCount++;
            err = errNone;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return h;
}

MemHandle DmQueryRecord(DmOpenRef dbP, UInt16 index) {
  return DmQueryRecordEx(dbP, index, false);
}

MemHandle DmGetRecord(DmOpenRef dbP, UInt16 index) {
  return DmQueryRecordEx(dbP, index, true);
}

Err DmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  vfs_file_t *f;
  DmOpenType *dbRef;
  char buf[VFS_PATH];
  Err err = dmErrIndexOutOfRange;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC && index < db->numRecs && db->elements[index]) {
          h = db->elements[index];
          if (h->htype == (STO_TYPE_REC | STO_INFLATED)) {
            h->htype &= ~STO_INFLATED;
            if (h->useCount) {
              h->useCount--;
            } else {
              debug(DEBUG_ERROR, "STOR", "DmReleaseRecord database \"%s\" index %d useCount < 0", db->name, index);
            }
            if (dirty || (h->d.rec.attr & dmRecAttrDirty)) {
              storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
              if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                if (vfs_write(f, h->buf, h->size) != h->size) {
                }
                vfs_close(f);
              }
              h->d.rec.attr &= ~dmRecAttrDirty;
            }
            if (h->buf) StoPtrFree(h->buf);
            h->buf = NULL;
            h->d.rec.attr &= ~dmRecAttrBusy;
          }
          err = errNone;
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmDatabaseSize(UInt16 cardNo, LocalID dbID, UInt32 *numRecordsP, UInt32 *totalBytesP, UInt32 *dataBytesP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  uint32_t i, n;
  DmOpenRef dbRef;
  Err err = dmErrInvalidParam;

  if (dbID < (sto->size - sizeof(storage_db_t))) {
    // it is necessary to open the database, otherwise the records would not be mapped
    if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadOnly)) != NULL) {
      db = (storage_db_t *) (sto->base + dbID);
      if (numRecordsP) *numRecordsP = db->numRecs;
      if (dataBytesP || totalBytesP) {
        for (i = 0, n = 0; i < db->numRecs; i++) {
          h = db->elements[i];
          n += h->size;
        }
        if (dataBytesP) *dataBytesP = n;
        if (totalBytesP) *totalBytesP = n + 84; // XXX fictional header size
      }
      DmCloseDatabase(dbRef);
      err = errNone;
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmCreateDatabaseEx(const Char *nameP, UInt32 creator, UInt32 type, UInt16 attr, UInt32 uniqueIDSeed, Boolean overwrite) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db, *existing = NULL;
  SysNotifyParamType notify;
  SysNotifyDBCreatedType dbCreated;
  LocalID dbID = 0;
  vfs_file_t *f;
  char buf[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (nameP) {
    if (mutex_lock(sto->mutex) == 0) {
      for (db = sto->list; db; db = db->next) {
        if (sys_strcmp(nameP, db->name) == 0) {
          existing = db;
          break;
        }
      }
      if (existing) {
        if (overwrite) {
          debug(DEBUG_INFO, "STOR", "DmCreateDatabase overwriting database \"%s\"", nameP);
          dbID = (uint8_t *)db - sto->base;
          if ((err = DmDeleteDatabase(0, dbID)) != errNone) {
            mutex_unlock(sto->mutex);
            return err;
          }
          storage_name(sto, (char *)nameP, 0, 0, 0, 0, 0, buf);
          if (StoVfsMkdir(sto->session, buf) == -1) {
            pumpkin_heap_free(db, "storage_db");
            mutex_unlock(sto->mutex);
            return err;
          }
          db = existing;
        } else {
          debug(DEBUG_ERROR, "STOR", "DmCreateDatabase database \"%s\" already exists", nameP);
          mutex_unlock(sto->mutex);
          return err;
        }
      } else {
        debug(DEBUG_INFO, "STOR", "DmCreateDatabase creating database \"%s\"", nameP);
        if ((db = pumpkin_heap_alloc(sizeof(storage_db_t), "storage_db")) == NULL) {
          mutex_unlock(sto->mutex);
          return err;
        }
        storage_name(sto, (char *)nameP, 0, 0, 0, 0, 0, buf);
        if (StoVfsMkdir(sto->session, buf) == -1) {
          pumpkin_heap_free(db, "storage_db");
          mutex_unlock(sto->mutex);
          return err;
        }
        db->next = sto->list;
        sto->list = db;
        sto->num_storage++;

        if (watchName && !StrCompare(watchName, nameP)) {
          sto->watchID = (uint8_t *)db - sto->base;;
          debug(DEBUG_INFO, "STOR", "WATCH DmCreateDatabase(\"%s\", 0x%08X, 0x%08X, %d)", nameP, creator, type, attr & dmHdrAttrResDB ? 1 : 0);
        }
      }

      sys_strncpy(db->name, nameP, dmDBNameLength-1);
      if (attr & dmHdrAttrResDB) {
        db->ftype = STO_TYPE_RES;
      } else if (attr & dmHdrAttrStream) {
        db->ftype = STO_TYPE_FILE;
        storage_name(sto, db->name, STO_FILE_DATA, 0, 0, 0, 0, buf);
        if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          vfs_close(f);
        }
      } else {
        db->ftype = STO_TYPE_REC;
        storage_name(sto, db->name, STO_FILE_INDEX, 0, 0, 0, 0, buf);
        if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
          vfs_close(f);
        }
      }
      db->creator = creator;
      db->type = type;
      db->attributes = attr;
      db->uniqueIDSeed = uniqueIDSeed;
      db->crDate = TimGetSeconds();
      db->modDate = db->crDate;
      db->bckDate = db->crDate;

      if (StoWriteHeader(sto, db) == -1) {
        pumpkin_heap_free(db, "storage_db");
        if (existing == NULL) {
          sto->num_storage--;
        }
      } else {
        err = errNone;
      }

      mutex_unlock(sto->mutex);
    }
  }

  if (err == errNone) {
    MemSet(&dbCreated, sizeof(dbCreated), 0);
    dbCreated.newDBID = 0; // dbID's are not advertised, since they are local to the task
    dbCreated.creator = creator;
    dbCreated.type = type;
    dbCreated.resDB = attr & dmHdrAttrResDB;
    StrNCopy(dbCreated.dbName, nameP, dmDBNameLength-1);

    MemSet(&notify, sizeof(notify), 0);
    notify.notifyType = sysNotifyDBCreatedEvent;
    notify.broadcaster = 0;
    notify.notifyDetailsP = &dbCreated;
    SysNotifyBroadcast(&notify);
  }

  StoCheckErr(err);
  return err;
}

Err DmCreateDatabase(UInt16 cardNo, const Char *nameP, UInt32 creator, UInt32 type, Boolean resDB) {
  return DmCreateDatabaseEx(nameP, creator, type, resDB ? dmHdrAttrResDB : 0, ((UInt32)SysRandom32(0)) & 0xFFFFFF, true);
}

static int StoAddDatabaseHandle(storage_t *sto, storage_db_t *db, storage_handle_t *h) {
  int r = -1;

  if (db->numRecs == db->totalElements) {
    if (db->elements == NULL) {
      db->totalElements = HANDLES_PER_PAGE;
      db->elements = sys_calloc(db->totalElements, sizeof(storage_handle_t *));
    } else {
      db->totalElements += HANDLES_PER_PAGE;
      db->elements = sys_realloc(db->elements, db->totalElements * sizeof(storage_handle_t *));
    }
  }

  if (db->elements) {
    db->elements[db->numRecs++] = h;
    r = 0;
  } else {
    db->totalElements = 0;
  }

  return r;
}

static storage_handle_t *StoAddRec(storage_t *sto, storage_db_t *db, uint32_t uniqueID, uint8_t attr, uint32_t size) {
  storage_handle_t *h = NULL;

  if ((h = pumpkin_heap_alloc(sizeof(storage_handle_t), "Handle")) != NULL) {
    h->magic = STO_MAGIC;
    h->htype = STO_TYPE_REC;
    h->owner = 0;
    h->size = size;
    h->d.rec.uniqueID = uniqueID;
    h->d.rec.attr = attr;
    h->d.rec.attr |= dmRecAttached;
    if (StoAddDatabaseHandle(sto, db, h) == -1) {
      pumpkin_heap_free(h, "Handle");
      h = NULL;
    }
  }

  return h;
}

static storage_handle_t *StoAddRes(storage_t *sto, storage_db_t *db, uint32_t type, uint16_t id, uint32_t size) {
  storage_handle_t *h = NULL;

  if ((h = pumpkin_heap_alloc(sizeof(storage_handle_t), "Handle")) != NULL) {
    h->magic = STO_MAGIC;
    h->htype = STO_TYPE_RES;
    h->owner = 0;
    h->size = size;
    h->d.res.id = id;
    h->d.res.type = type;
    h->d.res.attr |= dmRecAttached;
    if (StoAddDatabaseHandle(sto, db, h) == -1) {
      pumpkin_heap_free(h, "Handle");
      h = NULL;
    }
  }

  return h;
}

static Int32 compare_handle(void *e1, void *e2, void *otherP) {
  storage_db_t *db;
  storage_handle_t *h1, *h2;
  Int32 r = 0;

  db = (storage_db_t *)otherP;
  if (db->ftype == STO_TYPE_RES) {
    h1 = *((storage_handle_t **)e1);
    h2 = *((storage_handle_t **)e2);

    if (h1->d.res.type < h2->d.res.type) {
      r = -1;
    } else if (h1->d.res.type > h2->d.res.type) {
      r = 1;
    } else {
      r = (int32_t)h1->d.res.id - (int32_t)h2->d.res.id;
    }
  }

  return r;
}

static void StoSortHandles(storage_db_t *db) {
  if (db->elements && db->numRecs) {
    SysQSortP(db->elements, db->numRecs, sizeof(storage_handle_t *), compare_handle, db);
  }
}

static int StoMapRecords(storage_t *sto, storage_db_t *db) {
  vfs_file_t *f, *f2;
  vfs_ent_t *ent;
  char buf[VFS_PATH];
  uint8_t rec[16];
  uint32_t attr, uniqueID, max;
  int r = -1;

  if (db->elements == NULL) {
    storage_name(sto, db->name, STO_FILE_INDEX, 0, 0, 0, 0, buf);
    if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
      for (max = 0; !thread_must_end();) {
        sys_memset(rec, 0, sizeof(rec));
        if (vfs_read(f, rec, 12) != 12) break;
        if (sys_sscanf((char *)rec, "%08X.%02X\n", &uniqueID, &attr) == 2) {
          if (uniqueID > max) max = uniqueID;
          storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, attr & ATTR_MASK, uniqueID, buf);
          if (attr & dmRecAttrDelete) {
            debug(DEBUG_INFO, "STOR", "removing deleted record %s", buf);
            StoVfsUnlink(sto->session, buf);
          } else {
            if ((f2 = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
              if ((ent = vfs_fstat(f2)) != NULL) {
                StoAddRec(sto, db, uniqueID, attr, ent->size);
              }
              vfs_close(f2);
            }
          }
        }
      }
      if (db->uniqueIDSeed < max) {
        db->uniqueIDSeed = max;
      }
      r = 0;
      vfs_close(f);
    }
  } else {
    r = 0;
  }

  return r;
}

static int StoMapResources(storage_t *sto, storage_db_t *db) {
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  char buf[VFS_PATH];
  char st[8];
  uint32_t type, id;
  int n, r = -1;

  if (db->elements == NULL) {
    storage_name(sto, db->name, 0, 0, 0, 0, 0, buf);
    if ((dir = StoVfsOpendir(sto->session, buf)) != NULL) {
      for (;;) {
        ent = StoVfsReaddir(dir);
        if (ent == NULL) break;
        if (ent->type != VFS_FILE) continue;
        if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue;
        st[4] = 0;
        if ((n = sys_sscanf(ent->name, "%c%c%c%c.%08X.%d", st, st+1, st+2, st+3, &type, &id)) == 6) {
          StoAddRes(sto, db, type, id, ent->size);
        }
      }
      vfs_closedir(dir);
      StoSortHandles(db);
    }
  } else {
    r = 0;
  }

  return r;
}

static int StoMapFile(storage_t *sto, storage_db_t *db) {
  char buf[VFS_PATH];
  int mode, r = -1;

  mode = 0;
  if (db->mode & dmModeReadOnly) mode |= VFS_READ;
  if (db->mode & dmModeWrite)    mode |= VFS_WRITE;

  storage_name(sto, db->name, STO_FILE_DATA, 0, 0, 0, 0, buf);
  if ((db->f = StoVfsOpen(sto->session, buf, mode)) != NULL) {
    r = 0;
  }

  return r;
}

static int StoMapContents(storage_t *sto, storage_db_t *db) {
  int r = -1;

  switch (db->ftype) {
    case STO_TYPE_REC:
      r = StoMapRecords(sto, db);
      break;
    case STO_TYPE_RES:
      r = StoMapResources(sto, db);
      break;
    case STO_TYPE_FILE:
      r = StoMapFile(sto, db);
      break;
  }

  if (r == 0) {
    r = StoReadAppInfo(sto, db);
  }
  if (r == 0) {
    r = StoReadSortInfo(sto, db);
  }

  return r;
}

static DmOpenRef DmOpenDatabaseOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode, Boolean setPath, Boolean searchOverlay) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  Boolean ok;
  LmLocaleType locale;
  DmOpenType *first, *dbRef = NULL;
  LocalID overlayID;
  Boolean resourceDB = false;
  char dbName[dmDBNameLength];
  char overlayName[dmDBNameLength];
  Err err = dmErrInvalidParam;

  if (mutex_lock(sto->mutex) == 0) {
    if (dbID && dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *) (sto->base + dbID);

      if (pumpkin_is_m68k() && (mode & dmModeWrite)) {
        // In PumpkinOS, the database formats for Date Book and To Do List are incompatible with the
        // corresponding formats in PalmOS. Because of this, if a 68K app opens one of these databases in write mode
        // with the intention of writing records, it will corrupt the database. So we deny access here.
        // In theory, even reading records from the database will cause problems for the app, but there are other ligitimate
        // uses of calling DmOpenDatabase() in read mode.
        switch (db->creator) {
          case sysFileCDatebook:
          case sysFileCToDo:
            mutex_unlock(sto->mutex);
            ErrFatalDisplayEx("68K app tried to open Date Book / To Do List database in write mode.", 1);
            return NULL;
        }
      }

      if ((dbRef = pumpkin_heap_alloc(sizeof(DmOpenType), "dbRef")) != NULL) {
        if (dbID == sto->watchID) {
          debug(DEBUG_INFO, "STOR", "WATCH DmOpenDatabase(0x%08X, 0x%04X): %p", dbID, mode, dbRef);
        }
        if (mode & dmModeWrite) {
          ok = StoLockForWriting(sto, db) == 0;
        } else if (mode & dmModeReadOnly) {
          ok = StoLockForReading(sto, db) == 0;
        } else {
          debug(DEBUG_ERROR, "STOR", "DmOpenDatabase \"%s\" invalid mode 0x%04X", db->name, mode);
          ok = false;
        }
        if (ok) {
          db->mode = mode;
          dbRef->dbID = dbID;
          dbRef->mode = mode;
          StoMapContents(sto, db);
          StrNCopy(dbName, db->name, dmDBNameLength-1);
          resourceDB = db->ftype == STO_TYPE_RES;
          err = errNone;
        } else {
          pumpkin_heap_free(dbRef, "dbRef");
          dbRef = NULL;
        }
      }
    } else {
      debug(DEBUG_ERROR, "STOR", "DmOpenDatabase invalid dbID 0x%08X", dbID);
    }
    mutex_unlock(sto->mutex);
  }

  if (dbRef) {
    if (setPath) {
      first = sto->dbRef;
      if (first) {
        first->prev = dbRef;
        dbRef->next = first;
      }
      sto->dbRef = dbRef;
    }

    if (searchOverlay && resourceDB && !(mode & dmModeWrite)) {
      if ((dbRef->ovh = DmGet1Resource(omOverlayRscType, omOverlayRscID)) != NULL) {
        if ((dbRef->ov = MemHandleLock(dbRef->ovh)) != NULL) {
          OmGetCurrentLocale(&locale);
          if (OmLocaleToOverlayDBName(dbName, &locale, overlayName) == errNone) {
            debug(DEBUG_INFO, "STOR", "DmOpenDatabase searching overlay \"%s\" for language %d country %d", overlayName, locale.language, locale.country);
            if ((overlayID = DmFindDatabase(cardNo, overlayName)) != 0) {
              dbRef->overlayDb = DmOpenDatabaseOverlay(cardNo, overlayID, mode, false, false);
              if (dbRef->overlayDb) {
                dbRef->overlayDb->isOverlay = true;
                dbRef->overlayDb->next = dbRef;
              }
            }
          }
          if (!dbRef->overlayDb) {
            MemHandleUnlock(dbRef->ovh);
            DmReleaseResource(dbRef->ovh);
            dbRef->ovh = NULL;
            dbRef->ov = NULL;
          }
        } else {
          DmReleaseResource(dbRef->ovh);
          dbRef->ovh = NULL;
        }
      }
    }
  }

  StoCheckErr(err);
  return dbRef;
}

DmOpenRef DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  return DmOpenDatabaseOverlay(cardNo, dbID, mode, true, true);
}

Err DmCloseDatabase(DmOpenRef dbP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  vfs_file_t *f;
  UInt32 i, size;
  void *encoded;
  char st[8], buf[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        if (dbRef->dbID == sto->watchID) {
          debug(DEBUG_INFO, "STOR", "WATCH DmCloseDatabase(%p)", dbRef);
        }
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (dbRef->mode & dmModeWrite) {
          debug(DEBUG_TRACE, "STOR", "DmCloseDatabase \"%s\" writeCount %d -> %d", db->name, db->writeCount, db->writeCount-1);
          if (StoUnlockForWriting(sto, db) == 0) err = errNone;
        } else if (dbRef->mode & dmModeReadOnly) {
          debug(DEBUG_TRACE, "STOR", "DmCloseDatabase \"%s\" readCount %d -> %d", db->name, db->readCount, db->readCount-1);
          if (StoUnlockForReading(sto, db) == 0) err = errNone;
        } else {
          debug(DEBUG_ERROR, "STOR", "DmCloseDatabase \"%s\" dbRef invalid mode 0x%04X", db->name, dbRef->mode);
        }

        if (err == errNone && db->readCount == 0 && db->writeCount == 0) {
          switch (db->ftype) {
            case STO_TYPE_REC:
              debug(DEBUG_TRACE, "STOR", "DmCloseDatabase \"%s\" flush %d records", db->name, db->numRecs);
              if (dbRef->mode & dmModeWrite) {
                for (i = 0; i < db->numRecs; i++) {
                  h = db->elements[i];
                  if ((h->htype & STO_INFLATED) && h->d.rec.attr & dmRecAttrDirty) {
                    debug(DEBUG_TRACE, "STOR", "DmCloseDatabase writing dirty record %d", i);
                    storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
                    if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                      vfs_write(f, h->buf, h->size);
                      vfs_close(f);
                    }
                    h->d.rec.attr &= ~dmRecAttrDirty;
                  }
                }
                StoWriteIndex(sto, db);
              }
              break;
            case STO_TYPE_RES:
              debug(DEBUG_TRACE, "STOR", "DmCloseDatabase \"%s\" flush %d resources", db->name, db->numRecs);
              if (dbRef->ovh) {
                if (dbRef->ov) {
                  MemHandleUnlock(dbRef->ovh);
                  dbRef->ov = NULL;
                }
                DmReleaseResource(dbRef->ovh);
                dbRef->ovh = NULL;
              }
              for (i = 0; i < db->numRecs; i++) {
                h = db->elements[i];
                pumpkin_id2s(h->d.res.type, st);
                debug(DEBUG_TRACE, "STOR", "DmCloseDatabase resource %s %5d inflated %d dirty %d use %d lock %d", st, h->d.res.id, h->htype & STO_INFLATED ? 1 : 0, h->d.res.attr & dmRecAttrDirty ? 1 : 0, h->useCount, h->lockCount);
                if (h->htype & STO_INFLATED) {
                  if (h->d.res.attr & dmRecAttrDirty) {
                    if (dbRef->mode & dmModeWrite) {
                      encoded = NULL;
                      if (h->d.res.encoder && h->d.res.decoded) {
                        encoded = h->d.res.encoder(h->d.res.decoded, &size);
                      }
                      debug(DEBUG_TRACE, "STOR", "DmCloseDatabase writing dirty resource %s %d", st, h->d.res.id);
                      storage_name(sto, db->name, STO_FILE_ELEMENT, h->d.res.id, h->d.res.type, 0, 0, buf);
                      if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                        if (encoded) {
                          vfs_write(f, encoded, size);
                          sys_free(encoded);
                        } else {
                          vfs_write(f, h->buf, h->size);
                        }
                        vfs_close(f);
                      }
                      h->d.res.attr &= ~dmRecAttrDirty;
                    } else {
                      debug(DEBUG_ERROR, "STOR", "DmCloseDatabase dirty resource %s %d but database is read only", st, h->d.res.id);
                    }
                  }
                  if (h->lockCount == 0) {
                    if (h->d.res.destructor && h->d.res.decoded) {
                      debug(DEBUG_TRACE, "STOR", "DmCloseDatabase calling destructor");
                      h->d.res.destructor(h->d.res.decoded);
                      h->d.res.decoded = NULL;
                      debug(DEBUG_TRACE, "STOR", "DmCloseDatabase destructor called");
                    }
                    debug(DEBUG_TRACE, "STOR", "DmCloseDatabase deflating resource %s %d ", st, h->d.res.id);
                    StoPtrFree(h->buf);
                    h->buf = NULL;
                    h->htype &= ~STO_INFLATED;
                  } else {
                    debug(DEBUG_ERROR, "STOR", "DmCloseDatabase \"%s\" resource %s %d could not be deflated (use=%d lock=%d)", db->name, st, h->d.res.id, h->useCount, h->lockCount);
                  }
                }
              }
              if (dbRef->overlayDb) {
                debug(DEBUG_INFO, "STOR", "DmCloseDatabase closing overlay");
                DmCloseDatabase(dbRef->overlayDb);
              }
              break;
            case STO_TYPE_FILE:
             if (db->f) {
                vfs_close(db->f);
                db->f = NULL;
              }
              break;
          }
          if (dbRef->mode & dmModeWrite) {
            StoWriteAppInfo(sto, db);
            StoWriteSortInfo(sto, db);
          }
          db->mode = 0;
          StoWriteHeader(sto, db);

/*
          if (db->elements) {
            sys_free(db->elements);
            db->elements = NULL;
            db->totalElements = 0;
            db->numRecs = 0;
          }
*/
        }

        if (dbRef->prev) {
          dbRef->prev->next = dbRef->next;
          if (dbRef->next) dbRef->next->prev = dbRef->prev;
        } else {
          if (dbRef->next) dbRef->next->prev = dbRef->prev;
          sto->dbRef = dbRef->next;
        }
        pumpkin_heap_free(dbRef, "dbRef");
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmDeleteDatabase(UInt16 cardNo, LocalID dbID) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  SysNotifyParamType notify;
  SysNotifyDBDeletedType dbDeleted;
  char buf[VFS_PATH], buf2[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (mutex_lock(sto->mutex) == 0) {
    if (dbID && dbID < (sto->size - sizeof(storage_db_t))) {
      if (dbID == sto->watchID) {
        debug(DEBUG_INFO, "STOR", "WATCH DmDeleteDatabase(0x%08X)", dbID);
        sto->watchID = 0;
      }
      db = (storage_db_t *) (sto->base + dbID);
      if (StoLockForWriting(sto, db) != 0) {
        debug(DEBUG_ERROR, "STOR", "DmDeleteDatabase database \"%s\" is open by another thread", db->name);
      } else {
        if (db->readCount > 0 || db->writeCount > 1) { // StoLockForWriting incremented writeCount, so it must be compared to 1
          debug(DEBUG_ERROR, "STOR", "DmDeleteDatabase database \"%s\" is open", db->name);
        } else {
          debug(DEBUG_INFO, "STOR", "DmDeleteDatabase database \"%s\"", db->name);

          storage_name(sto, db->name, 0, 0, 0, 0, 0, buf);
          if ((dir = StoVfsOpendir(sto->session, buf)) != NULL) {
            for (;;) {
              ent = StoVfsReaddir(dir);
              if (ent == NULL) break;
              if (ent->type != VFS_FILE) continue;
              if (!sys_strcmp(ent->name, ".") || !sys_strcmp(ent->name, "..")) continue;
              sys_strncpy(buf2, buf, VFS_PATH-1);
              sys_strncat(buf2, "/", VFS_PATH-sys_strlen(buf2)-1);
              sys_strncat(buf2, ent->name, VFS_PATH-sys_strlen(buf2)-1);
              StoVfsUnlink(sto->session, buf2);
            }
            vfs_closedir(dir);
          }

          storage_name(sto, db->name, 0, 0, 0, 0, 0, buf);
          StoVfsUnlink(sto->session, buf);

          MemSet(&dbDeleted, sizeof(dbDeleted), 0);
          dbDeleted.oldDBID = 0; // dbID's are not advertised, since they are local to the task
          dbDeleted.creator = db->creator;
          dbDeleted.type = db->type;
          dbDeleted.attributes = db->attributes;
          StrNCopy(dbDeleted.dbName, db->name, dmDBNameLength-1);

          db->ftype = 0;
          db->readCount = 0;
          db->writeCount = 0;
          db->mode = 0;
          db->numRecs = 0;
          db->protect = 0;
          db->creator = 0;
          db->type = 0;
          db->crDate = 0;
          db->modDate = 0;
          db->bckDate = 0;
          db->modNum = 0;
          db->attributes = 0;
          db->uniqueIDSeed = ((UInt32)SysRandom32(0)) & 0xFFFFFF;
          db->version = 0;
          db->appInfoID = 0;
          db->sortInfoID = 0;
          db->f = NULL;
          sys_memset(db->name, 0, dmDBNameLength);

          sto->num_storage--;
          err = errNone;
        }
      }
    }
    mutex_unlock(sto->mutex);
  }

  if (err == errNone) {
    MemSet(&notify, sizeof(notify), 0);
    notify.notifyType = sysNotifyDBDeletedEvent;
    notify.broadcaster = 0;
    notify.notifyDetailsP = &dbDeleted;
    SysNotifyBroadcast(&notify);
  }

  StoCheckErr(err);
  return err;
}

UInt16 DmNumDatabases(UInt16 cardNo) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  StoCheckErr(errNone);

  return sto->num_storage;
}

Err DmDatabaseProtect(UInt16 cardNo, LocalID dbID, Boolean protect) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  Err err = dmErrInvalidParam;

  if (dbID < (sto->size - sizeof(storage_db_t))) {
    db = (storage_db_t *)(sto->base + dbID);
    if (protect) {
      if (db->protect < 32) {
        db->protect++;
        err = errNone;
      }
    } else {
      if (db->protect > 0) {
        db->protect--;
        err = errNone;
      } else {
        err = dmErrDatabaseNotProtected;
      }
    }
  }

  StoCheckErr(err);
  return err;
}

UInt16 DmNumRecords(DmOpenRef dbP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  UInt16 numRecs = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC || db->ftype == STO_TYPE_RES) {
        numRecs = db->numRecs;
        err = errNone;
      } else {
        debug(DEBUG_ERROR, "STOR", "DmNumRecords database \"%s\" not a record database", db->name);
        err = dmErrNotRecordDB;
      }
    }
  }

  StoCheckErr(err);
  return numRecs;
}

Err DmRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP, LocalID *chunkIDP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (attrP) *attrP = 0;
      if (uniqueIDP) *uniqueIDP = 0;
      if (chunkIDP) *chunkIDP = 0;
      if (db->ftype == STO_TYPE_REC && index < db->numRecs) {
        h = db->elements[index];
        if (attrP) *attrP = h->d.rec.attr;
        if (uniqueIDP) *uniqueIDP = h->d.rec.uniqueID;
        if (chunkIDP) {
          if (h->htype & STO_INFLATED) {
            *chunkIDP = (uint8_t *)h - sto->base;
          } else {
            debug(DEBUG_ERROR, "STOR", "DmRecordInfo database \"%s\" index %d chunkID not inflated yet", db->name, index);
            *chunkIDP = 0;
          }
        }
        err = errNone;
      }
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmSetRecordInfo(DmOpenRef dbP, UInt16 index, UInt16 *attrP, UInt32 *uniqueIDP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  char oldName[VFS_PATH], newName[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC && index < db->numRecs) {
          h = db->elements[index];
          storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, oldName);
          if (attrP) h->d.rec.attr = *attrP;
          if (uniqueIDP) h->d.rec.uniqueID = *uniqueIDP;
          storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, newName);
          if (sys_strcmp(oldName, newName)) {
            if (StoVfsRename(sto->session, oldName, newName) == 0) {
              err = errNone;
            }
          } else {
            err = errNone;
          }
          db->modDate = TimGetSeconds();
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return err;
}

DmOpenRef DmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenType *dbRef = NULL;
  Err err;

  err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, type, creator, false, &cardNo, &dbID);
  if (err == errNone) {
    if (dbID == sto->watchID) {
      debug(DEBUG_INFO, "STOR", "WATCH DmOpenDatabaseByTypeCreator(0x%08X, 0x%08X, 0x%04X)", type, creator, mode);
    }
    dbRef = DmOpenDatabase(cardNo, dbID, mode);
  }

  return dbRef;
}

DmOpenRef DmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode) {
  return DmOpenDatabaseOverlay(cardNo, dbID, mode, true, false);
}

DmOpenRef DmNextOpenDatabase(DmOpenRef currentP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  DmOpenType *dbRef = NULL;

  if (currentP) {
    dbRef = (DmOpenType *)currentP;
    dbRef = dbRef->next;
  } else {
    dbRef = sto->dbRef;
  }

  return dbRef;
}

DmOpenRef DmNextOpenResDatabase(DmOpenRef dbP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef = NULL;

  dbRef = dbP ? (DmOpenType *)dbP : sto->dbRef;
  if (dbRef) {
    for (dbRef = dbRef->next; dbRef; dbRef = dbRef->next) {
      if (dbRef->dbID >= (sto->size - sizeof(storage_db_t))) continue;
      db = (storage_db_t *) (sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES) break;
    }
  }

  StoCheckErr(errNone);
  return dbRef;
}

static MemHandle DmGetResourceEx(DmOpenType *dbRef, DmResType type, DmResID resID, Boolean firstOnly, Boolean decoded) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  vfs_file_t *f;
  Boolean searchedOverlay;
  char buf[VFS_PATH], st[8];
  uint32_t i, found, load;
  storage_handle_t *h = NULL;
  Err err = dmErrResourceNotFound;

  if (mutex_lock(sto->mutex) == 0) {
    pumpkin_id2s(type, st);
    searchedOverlay = false;
    debug(DEBUG_TRACE, "STOR", "DmGetResourceEx searching resource %s %d first %d", st, resID, firstOnly);
    if (dbRef == NULL) dbRef = sto->dbRef;
    for (found = 0; dbRef && !found; dbRef = dbRef->next) {
      if (!searchedOverlay && dbRef->overlayDb) dbRef = dbRef->overlayDb;
      searchedOverlay = false;
      if (dbRef->dbID >= (sto->size - sizeof(storage_db_t))) continue;
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype != STO_TYPE_RES) continue;

      debug(DEBUG_TRACE, "STOR", "DmGetResourceEx checking database \"%s\" (%d resources)", db->name, db->numRecs);
      for (i = 0; i < db->numRecs; i++) {
        h = db->elements[i];
        pumpkin_id2s(h->d.res.type, st);
        debug(DEBUG_TRACE, "STOR", "DmGetResourceEx resource %d: %s %d", i, st, h->d.res.id);
        if (h->d.res.type == type && h->d.res.id == resID) {
          debug(DEBUG_TRACE, "STOR", "DmGetResourceEx found resource %s %d inflated %d on \"%s\"", st, resID, (h->htype & STO_INFLATED) ? 1 : 0, db->name);
          found = 1;
          load = 0;

          if (!(h->htype & STO_INFLATED)) {
            if ((h->buf = StoPtrNew(h, h->size, h->d.res.type, resID)) != NULL) {
              h->htype |= STO_INFLATED;
              h->useCount = 1;
              h->lockCount = 0;
              load = 1;
            }
          } else {
            load = 1;
            h->useCount++;
          }

          if (h->buf) {
            if (load) {
              debug(DEBUG_TRACE, "STOR", "reading %5d bytes from resource %s %d at %p", h->size, st, h->d.res.id, h->buf);
              storage_name(sto, db->name, STO_FILE_ELEMENT, resID, type, 0, 0, buf);
              if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                if (vfs_read(f, h->buf, h->size) > 0) {
                  err = errNone;
                }
                vfs_close(f);
              }
            } else {
              err = errNone;
            }
            if (err == errNone) {
              StoDecodeResource(h, decoded);
            }
          }
          break;
        }
      }
      if (dbRef->isOverlay && !found) {
        searchedOverlay = true;
        continue;
      }
      if (firstOnly) break;
    }

    if (!found) {
      pumpkin_id2s(type, st);
      debug(DEBUG_INFO, "STOR", "DmGetResourceEx resource %s %d not found", st, resID);
      err = errNone;
      h = NULL;
    }
    mutex_unlock(sto->mutex);
  }

  StoCheckErr(err);
  debug(DEBUG_TRACE, "STOR", "DmGetResourceEx %s %d: 0x%08X", st, resID, (uint32_t)(h ? (uint8_t *)h - sto->base : 0));
  return h;
}

MemHandle DmGetResourceDecoded(DmResType type, DmResID resID) {
  return DmGetResourceEx(NULL, type, resID, false, true);
}

MemHandle DmGetResource(DmResType type, DmResID resID) {
  return DmGetResourceEx(NULL, type, resID, false, false);
}

MemHandle DmGet1Resource(DmResType type, DmResID resID) {
  return DmGetResourceEx(NULL, type, resID, true, false);
}

// Search all open resource databases for a resource by type and ID, or by pointer if it is non-NULL.
UInt16 DmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  vfs_file_t *f;
  UInt16 index = 0xffff;
  Boolean found;
  char buf[VFS_PATH], st[8];
  uint32_t i;
  storage_handle_t *h = NULL;
  Err err = dmErrResourceNotFound;

  if (dbPP && mutex_lock(sto->mutex) == 0) {
    if (resH) {
      debug(DEBUG_TRACE, "STOR", "searching resource handle %p", resH);
    } else {
      pumpkin_id2s(resType, st);
      debug(DEBUG_TRACE, "STOR", "searching resource %s %d", st, resID);
    }

    for (dbRef = sto->dbRef, found = false; dbRef && !found; dbRef = dbRef->next) {
      if (dbRef->dbID >= (sto->size - sizeof(storage_db_t))) continue;
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype != STO_TYPE_RES) continue;

      debug(DEBUG_TRACE, "STOR", "checking database \"%s\" (%d resources)", db->name, db->numRecs);
      for (i = 0; i < db->numRecs; i++) {
        h = db->elements[i];
        found = resH ? (h == resH) : (h->d.res.type == resType && h->d.res.id == resID);
        if (found) {
          debug(DEBUG_TRACE, "STOR", "found resource at index %d", i);
          index = i;
          *dbPP = dbRef;

          if (!(h->htype & STO_INFLATED)) {
            if ((h->buf = StoPtrNew(h, h->size, resType, resID)) != NULL) {
              h->htype |= STO_INFLATED;
              h->useCount = 1;
              debug(DEBUG_TRACE, "STOR", "reading resource %s %d at %p", st, resID, h->buf);
              storage_name(sto, db->name, STO_FILE_ELEMENT, h->d.res.id, h->d.res.type, 0, 0, buf);
              if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                if (vfs_read(f, h->buf, h->size) > 0) {
                  h->lockCount = 0;
                  err = errNone;
                } else {
                  h = NULL;
                }
                vfs_close(f);
              } else {
                h = NULL;
              }
            } else {
              h = NULL;
            }
          } else {
            h->useCount++;
            err = errNone;
          }

          if (err == errNone) {
            StoDecodeResource(h, false);
          }
          break;
        }
      }
    }

    if (!found) {
      if (resH) {
        debug(DEBUG_INFO, "STOR", "resource handle %p not found", resH);
      } else {
        pumpkin_id2s(resType, st);
        debug(DEBUG_INFO, "STOR", "resource %s %d not found", st, resID);
      }
      err = errNone;
    }
    mutex_unlock(sto->mutex);
  }

  StoCheckErr(err);
  return index;
}

UInt16 DmSearchRecord(MemHandle recH, DmOpenRef *dbPP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  vfs_file_t *f;
  UInt16 index = 0xffff;
  Boolean found;
  char buf[VFS_PATH];
  uint32_t i;
  storage_handle_t *h = NULL;
  Err err = dmErrResourceNotFound;

  if (recH && dbPP && mutex_lock(sto->mutex) == 0) {
    debug(DEBUG_TRACE, "STOR", "searching record handle %p", recH);

    for (dbRef = sto->dbRef, found = false; dbRef && !found; dbRef = dbRef->next) {
      if (dbRef->dbID >= (sto->size - sizeof(storage_db_t))) continue;
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype != STO_TYPE_REC) continue;

      debug(DEBUG_TRACE, "STOR", "checking database \"%s\" (%d resources)", db->name, db->numRecs);
      for (i = 0; i < db->numRecs; i++) {
        h = db->elements[i];
        found = (h == recH);
        if (found) {
          debug(DEBUG_TRACE, "STOR", "found record at index %d", i);
          index = i;
          *dbPP = dbRef;

          if (!(h->htype & STO_INFLATED)) {
            if ((h->buf = StoPtrNew(h, h->size, 0, 0)) != NULL) {
              h->htype |= STO_INFLATED;
              h->useCount = 1;
              debug(DEBUG_TRACE, "STOR", "reading record %d at %p", i, h->buf);
              storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
              if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                if (vfs_read(f, h->buf, h->size) == h->size) {
                  h->d.rec.attr &= ~dmRecAttrDirty;
                  h->d.rec.attr |= dmRecAttrBusy;
                  h->lockCount = 0;
                  err = errNone;
                } else {
                  h = NULL;
                }
                vfs_close(f);
              } else {
                h = NULL;
              }
            } else {
              h = NULL;
            }
          } else {
            h->useCount++;
            err = errNone;
          }

          break;
        }
      }
    }

    mutex_unlock(sto->mutex);
  }

  StoCheckErr(err);
  return index;
}

MemHandle DmGetResourceIndex(DmOpenRef dbP, UInt16 index) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  vfs_file_t *f;
  storage_handle_t *h = NULL;
  char buf[VFS_PATH], st[8];
  Err err = dmErrResourceNotFound;

  if (mutex_lock(sto->mutex) == 0) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES && index < db->numRecs) {
        h = db->elements[index];
        if (!(h->htype & STO_INFLATED)) {
          if ((h->buf = StoPtrNew(h, h->size, h->d.res.type, h->d.res.id)) != NULL) {
            h->htype |= STO_INFLATED;
            h->useCount = 1;
            pumpkin_id2s(h->d.res.type, st);
            debug(DEBUG_TRACE, "STOR", "reading %5d bytes from resource %s %d at %p", h->size, st, h->d.res.id, h->buf);
            storage_name(sto, db->name, STO_FILE_ELEMENT, h->d.res.id, h->d.res.type, 0, 0, buf);
            if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
              if (vfs_read(f, h->buf, h->size) > 0) {
                if (db->mode & dmModeWrite) {
                  h->d.res.writable = 1;
                }
                h->lockCount = 0;
                err = errNone;
              } else {
                h = NULL;
              }
              vfs_close(f);
            } else {
              h = NULL;
            }
          } else {
            h = NULL;
          }
        } else {
          if (db->mode & dmModeWrite) {
            h->d.res.writable = 1;
          }
          h->useCount++;
          err = errNone;
        }

        if (err == errNone) {
          StoDecodeResource(h, false);
        }
      }
    }
    mutex_unlock(sto->mutex);
  }

  StoCheckErr(err);
  debug(DEBUG_TRACE, "STOR", "DmGetResourceIndex %p %u: 0x%08X", dbP, index, (uint32_t)(h ? (uint8_t *)h - sto->base : 0));
  return h;
}

Err DmResourceType(MemHandle resourceH, DmResType *resType, DmResID *resID) {
  storage_handle_t *h;
  Err err = dmErrIndexOutOfRange;

  *resType = 0;
  *resID = 0;

  if (resourceH) {
    h = (storage_handle_t *)resourceH;
    switch (h->htype & ~STO_INFLATED) {
      case STO_TYPE_RES:
        *resType = h->d.res.type;
        *resID = h->d.res.id;
        err = errNone;
        break;
      default:
        debug(DEBUG_INFO, "STOR", "DmResourceType %p handle type 0x%04X", resourceH, h->htype);
        break;
    }
  }

  return err;
}

Err DmReleaseResource(MemHandle resourceH) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  char st[8];
  Err err = dmErrIndexOutOfRange;

  if (resourceH) {
    h = (storage_handle_t *)resourceH;
    pumpkin_id2s(h->d.res.type, st);
    debug(DEBUG_TRACE, "STOR", "DmReleaseResource 0x%08X '%s' %d use %d lock %d", (uint32_t)((uint8_t *)h - sto->base), st, h->d.res.id, h->useCount, h->lockCount);
    switch (h->htype & ~STO_INFLATED) {
      case STO_TYPE_RES:
        debug(DEBUG_TRACE, "STOR", "DmReleaseResource '%s' %d inflated %d", st, h->d.res.id, (h->htype & STO_INFLATED) ? 1 : 0);
        if (h->htype & STO_INFLATED) {
          if (h->useCount) {
            h->useCount--;
          } else {
            debug(DEBUG_ERROR, "STOR", "DmReleaseResource '%s' %d useCount < 0", st, h->d.res.id);
          }
          if (h->useCount == 0) {
            if (h->d.res.type != 'BikL') {
            if (!(h->d.res.attr & dmRecAttrDirty)) {
              if (h->lockCount == 0) {
                if (h->d.res.destructor && h->d.res.decoded) {
                  debug(DEBUG_TRACE, "STOR", "DmReleaseResource calling destructor");
                  h->d.res.destructor(h->d.res.decoded);
                  h->d.res.decoded = NULL;
                  debug(DEBUG_TRACE, "STOR", "DmReleaseResource destructor called");
                }
                debug(DEBUG_TRACE, "STOR", "DmReleaseResource free buf '%s' %d %p", st, h->d.res.id, h->buf);
                StoPtrFree(h->buf);
                h->buf = NULL;
                h->htype &= ~STO_INFLATED;
              } else {
                debug(DEBUG_ERROR, "STOR", "DmReleaseResource resource is locked (%d)", h->lockCount);
              }
            } else {
              debug(DEBUG_TRACE, "STOR", "DmReleaseResource resource is dirty");
            }
            } else {
              debug(DEBUG_TRACE, "STOR", "DmReleaseResource free buf '%s' %d %p NOT DONE lock %d", st, h->d.res.id, h->buf, h->lockCount);
            }
          }
        } else {
          debug(DEBUG_ERROR, "STOR", "DmReleaseResource 0x%08X handle type %d is not inflated", (uint32_t)((uint8_t *)h - sto->base), h->htype);
        }
        break;
      default:
        debug(DEBUG_ERROR, "STOR", "DmReleaseResource 0x%08X unexpected handle type %d", (uint32_t)((uint8_t *)h - sto->base), h->htype & ~STO_INFLATED);
        break;
    }
    err = errNone;
  }

  StoCheckErr(err);
  return err;
}

MemHandle DmResizeResource(MemHandle resourceH, UInt32 newSize) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  storage_db_t *db;
  void *newBuf, *old;
  char buf[VFS_PATH];
  vfs_file_t *f;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;

  if (resourceH) {
    if (mutex_lock(sto->mutex) == 0) {
      h = (storage_handle_t *)resourceH;
      if (h->htype & STO_INFLATED) {
        switch (h->htype & ~STO_INFLATED) {
          case STO_TYPE_RES:
            if (DmSearchResource(0, 0, resourceH, (DmOpenRef *)&dbRef) != 0xffff) {
              if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
                db = (storage_db_t *) (sto->base + dbRef->dbID);
                old = h->buf;
                if ((newBuf = StoPtrNew(h, newSize, h->d.res.type, h->d.res.id)) != NULL) {
                  sys_memcpy(newBuf, old, newSize < h->size ? newSize : h->size);
                  StoPtrFree(old);
                  h->buf = newBuf;
                  h->size = newSize;

                  storage_name(sto, db->name, STO_FILE_ELEMENT, h->d.res.id, h->d.res.type, 0x00, 0, buf);
                  if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
                    vfs_write(f, (uint8_t *)h->buf, h->size);
                    vfs_close(f);
                  }

                  err = errNone;
                }
              }
            }
            break;
          default:
            debug(DEBUG_ERROR, "STOR", "DmResizeResource %p unexpected h type %d", h, h->htype & ~STO_INFLATED);
            break;
        }
      } else {
        h->size = newSize;
        err = errNone;
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return resourceH;
}

MemHandle DmNewResourceEx(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size, void *p) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  char buf[VFS_PATH];
  vfs_file_t *f;
  storage_handle_t *h = NULL;
  Err err = dmErrResourceNotFound;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_RES) {
          if ((h = pumpkin_heap_alloc(sizeof(storage_handle_t), "Handle")) != NULL) {
            h->magic = STO_MAGIC;
            h->htype = STO_TYPE_RES | STO_INFLATED;
            h->owner = pumpkin_get_current();
            h->d.res.attr |= dmRecAttrDirty;
            h->d.res.type = resType;
            h->d.res.id = resID;
            h->size = size;
            h->buf = StoPtrNew(h, h->size, resType, resID);
            StoAddDatabaseHandle(sto, db, h);
            db->modDate = TimGetSeconds();

            storage_name(sto, db->name, STO_FILE_ELEMENT, resID, resType, 0x00, 0, buf);
            if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
              if (p) sys_memcpy(h->buf, p, h->size);
              vfs_write(f, (uint8_t *)h->buf, h->size);
              vfs_close(f);
            }
            err = errNone;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return h;
}

MemHandle DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size) {
  return DmNewResourceEx(dbP, resType, resID, size, NULL);
}

Err DmAttachResource(DmOpenRef dbP, MemHandle newH, DmResType resType, DmResID resID) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  uint8_t *p;
  char buf[VFS_PATH];
  vfs_file_t *f;
  Err err = dmErrResourceNotFound;

  if (mutex_lock(sto->mutex) == 0) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES) {
        h = (storage_handle_t *)newH;
        h->owner = 0;
        if ((h->htype & ~STO_INFLATED) != STO_TYPE_RES) {
          h->htype = (h->htype & STO_INFLATED) | STO_TYPE_RES;
        }
        h->d.res.type = resType;
        h->d.res.id = resID;
        h->d.res.attr |= dmRecAttached;
        StoAddDatabaseHandle(sto, db, h);

        if (h->htype & STO_INFLATED) {
          storage_name(sto, db->name, STO_FILE_ELEMENT, resID, resType, 0x00, 0, buf);
          if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
            p = MemHandleLock(newH);
            vfs_write(f, p, MemHandleSize(newH));
            MemHandleUnlock(newH);
            vfs_close(f);
          }
        }
        db->modDate = TimGetSeconds();
      }
    }
    mutex_unlock(sto->mutex);
  }

  return err;
}

Err DmRemoveResource(DmOpenRef dbP, UInt16 index) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  char buf[VFS_PATH];
  uint32_t i;
  Err err = dmErrResourceNotFound;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_RES && db->numRecs > 0) {
          if (index >= db->numRecs) index = db->numRecs - 1;
          if (db->elements[index]->lockCount == 0) {
            h = db->elements[index];
            storage_name(sto, db->name, STO_FILE_ELEMENT, h->d.res.id, h->d.res.type, 0, 0, buf);
            StoVfsUnlink(sto->session, buf);
            if (h->buf) StoPtrFree(h->buf);
            pumpkin_heap_free(h, "Handle");
            db->numRecs--;
            for (i = index; i < db->numRecs; i++) {
              db->elements[i] = db->elements[i+1];
            }
            db->modDate = TimGetSeconds();
            err = errNone;
          } else {
            debug(DEBUG_ERROR, "STOR", "DmRemoveResource %p %u attempt to remove locked handle", dbP, index);
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return err;
}

LocalID DmGetAppInfoID(DmOpenRef dbP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  LocalID appInfo = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      appInfo = db->appInfoID;
      err = errNone;
    }
  }

  StoCheckErr(err);
  return appInfo;
}

UInt16 DmNumRecordsInCategory(DmOpenRef dbP, UInt16 category) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  UInt16 i, numRecs = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC) {
        for (i = 0; i < db->numRecs; i++) {
          h = db->elements[i];
          if (!(h->d.rec.attr & dmRecAttrDelete) && (category == dmAllCategories || (h->d.rec.attr & dmRecAttrCategoryMask) == (category & dmRecAttrCategoryMask))) {
            if (!(h->d.rec.attr & dmRecAttrSecret) || (dbRef->mode & dmModeShowSecret)) {
              numRecs++;
            }
          }
        }
        err = errNone;
      }
    }
  }

  StoCheckErr(err);
  return numRecs;
}

MemHandle DmQueryNextInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 category) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *ha, *h = NULL;
  DmOpenType *dbRef;
  vfs_file_t *f;
  char buf[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (dbP && indexP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC) {
          for (; *indexP < db->numRecs; (*indexP)++) {
            ha = db->elements[*indexP];
            if (!(ha->d.rec.attr & dmRecAttrDelete) && (category == dmAllCategories || (ha->d.rec.attr & dmRecAttrCategoryMask) == (category & dmRecAttrCategoryMask))) {
              if (!(ha->d.rec.attr & dmRecAttrSecret) || (dbRef->mode & dmModeShowSecret)) {
                h = ha;

                if (!(h->htype & STO_INFLATED)) {
                  if ((h->buf = StoPtrNew(h, h->size, 0, 0)) != NULL) {
                    h->htype |= STO_INFLATED;
                    h->useCount = 1;
                    storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
                    if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                      if (vfs_read(f, h->buf, h->size) == h->size) {
                        h->d.rec.attr &= ~dmRecAttrDirty;
                        //h->d.rec.attr |= dmRecAttrBusy; // XXX is it necessary ?
                        h->lockCount = 0;
                        err = errNone;
                      } else {
                        err = dmErrMemError;
                        h = NULL;
                      }
                      vfs_close(f);
                    } else {
                      err = dmErrMemError;
                      h = NULL;
                    }
                  } else {
                    err = dmErrMemError;
                    h = NULL;
                  }
                } else {
                  h->useCount++;
                  err = errNone;
                }

                break;
              }
            }
          }
          err = errNone;
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return h;
}

static UInt16 DmFindSortPositionBinary(storage_db_t *db, MemHandle appInfoH, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other, UInt16 start, UInt16 end, UInt16 level) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  SortRecordInfoType recInfo, *recInfoP;
  vfs_file_t *f;
  char buf[VFS_PATH];
  UInt16 pivot, pos;
  Int16 r;

//debug(1, "XXX", "DmFindSortPosition level %d begin", level);
  pivot = start + (end - start + 1) / 2;
//debug(1, "XXX", "DmFindSortPosition start=%d pivot=%d end=%d", start, pivot, end);
//debug(1, "check", "find level %d start %d pivot %d end %d", level, start, pivot, end);
  h = db->elements[pivot];
  if (newRecordInfo) {
    recInfo.attributes = h->d.rec.attr;
    recInfo.uniqueID[0] = (h->d.rec.uniqueID >> 16) & 0xFF;
    recInfo.uniqueID[1] = (h->d.rec.uniqueID >>  8) & 0xFF;
    recInfo.uniqueID[2] = (h->d.rec.uniqueID >>  0) & 0xFF;
    recInfoP = &recInfo;
  } else {
    recInfoP = NULL;
  }
//debug(1, "XXX", "DmFindSortPosition compare with rec %d (h=%p, buf=%p, len=%d, inflated=%d)", pivot, h, h->buf, h->size, h->htype & STO_INFLATED ? 1 : 0);

  if (!(h->htype & STO_INFLATED)) {
    h->htype |= STO_INFLATED;
    h->useCount = 1;
//debug(1, "XXX", "DmFindSortPosition inflate record");
    if ((h->buf = StoPtrNew(h, h->size, 0, 0)) != NULL) {
      storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
      if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
        vfs_read(f, h->buf, h->size);
        vfs_close(f);
      }
    }
  } else {
    h->useCount++;
  }

//debug(1, "XXX", "DmFindSortPosition compare %p with %p at position %d ...", newRecord, h->buf, pivot);
  r = compar(newRecord, h->buf, other, newRecordInfo, recInfoP, appInfoH);
//debug(1, "XXX", "DmFindSortPosition compare r=%d", r);
//debug(1, "check", "compare level %d pos %d: %d", level, pivot, r);
  if (r == 0) {
    pos = pivot;
//debug(1, "XXX", "DmFindSortPosition found");
  } else if (r < 0) {
    if (pivot > start) {
//debug(1, "XXX", "DmFindSortPosition less than, recursion");
      pos = DmFindSortPositionBinary(db, appInfoH, newRecord, newRecordInfo, compar, other, start, pivot-1, level+1);
    } else {
      pos = pivot;
//debug(1, "XXX", "DmFindSortPosition less than, pos %d", pos);
    }
  } else {
    if (pivot < end) {
//debug(1, "XXX", "DmFindSortPosition greater than, recursion");
      pos = DmFindSortPositionBinary(db, appInfoH, newRecord, newRecordInfo, compar, other, pivot+1, end, level+1);
    } else {
      pos = end+1;
//debug(1, "XXX", "DmFindSortPosition greater than, pos %d", pos);
    }
  }

//debug(1, "XXX", "DmFindSortPosition end pos %d", pos);
  return pos;
}

// Returns the position where the record should be inserted.
// The position should be viewed as between the record returned and the record before it.
// Note that the return value may be one greater Than the number of records.

UInt16 DmFindSortPosition(DmOpenRef dbP, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  MemHandle appInfoH;
  UInt16 start, end, pos = 0;
  Err err = dmErrInvalidParam;

  if (dbP && newRecord && compar) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC) {
          if (db->numRecs > 0) {
            appInfoH = db->appInfoID ? MemLocalIDToHandle(db->appInfoID) : NULL;
            start = 0;
            end = db->numRecs - 1;
            pos = DmFindSortPositionBinary(db, appInfoH, newRecord, newRecordInfo, compar, other, start, end, 0);
          } else {
            pos = 0;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return pos;
}

UInt16 DmFindSortPositionV10(DmOpenRef dbP, void *newRecord, DmComparF *compar, Int16 other) {
  return DmFindSortPosition(dbP, newRecord, NULL, compar, other);
}

UInt16 DmFindSortPosition68K(DmOpenRef dbP, UInt32 newRecord, UInt32 newRecordInfo, UInt32 compar, Int16 other) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  MemHandle appInfoH;
  UInt8 *recInfoP;
  UInt32 appInfo, recInfo;
  DmOpenType *dbRef;
  UInt16 i, pos = 0;
  Err err = dmErrInvalidParam;

  if (dbP && newRecord && compar) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC) {
        if (db->numRecs > 0) {
          pos = 0;
          appInfoH = db->appInfoID ? MemLocalIDToHandle(db->appInfoID) : NULL;
          appInfo = appInfoH ? (uint8_t *)appInfoH - sto->base : 0;
          recInfoP = pumpkin_heap_alloc(4, "recInfo");
          for (i = 0; i < db->numRecs; i++) {
            h = db->elements[i];
            if (StoInflateRec(sto, db, h) == -1) break;
            if (newRecordInfo) {
              recInfoP[0] = h->d.rec.attr;
              recInfoP[1] = (h->d.rec.uniqueID >> 16) & 0xFF;
              recInfoP[2] = (h->d.rec.uniqueID >>  8) & 0xFF;
              recInfoP[3] = (h->d.rec.uniqueID >>  0) & 0xFF;
              recInfo = recInfoP - sto->base;
            } else {
              recInfo = 0;
            }
            if (CallDmCompare(compar, newRecord, h->buf - sto->base, other, newRecordInfo, recInfo, appInfo) < 0) {
              pos = i;
              err = errNone;
              break;
            }
          }
          if (i == db->numRecs) {
            pos = i;
            err = errNone;
          }
          pumpkin_heap_free(recInfoP, "recInfo");
        } else {
          pos = 0;
          err = errNone;
        }
      }
    }
  }

  StoCheckErr(err);
  return pos;
}

UInt16 DmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  UInt16 i, pos = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC && index < db->numRecs) {
        for (i = 0; i < db->numRecs; i++) {
          h = db->elements[i];
          if (!(h->d.rec.attr & dmRecAttrDelete) && (category == dmAllCategories || (h->d.rec.attr & dmRecAttrCategoryMask) == (category & dmRecAttrCategoryMask))) {
            if (!(h->d.rec.attr & dmRecAttrSecret) || (dbRef->mode & dmModeShowSecret)) {
              if (i == index) break;
              pos++;
            }
          }
        }
        err = errNone;
      }
    }
  }

  StoCheckErr(err);
  return pos;
}

MemHandle DmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  void *p;
  vfs_file_t *f;
  char buf[VFS_PATH];
  storage_handle_t *h = NULL;
  Err err = dmErrInvalidParam;

  if (dbP && newSize) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *) (sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC && index < db->numRecs) {
        h = db->elements[index];
        if (h->htype & STO_INFLATED) {
          p = StoPtrNew(h, newSize, 0, 0);
          MemMove(p, h->buf, newSize < h->size ? newSize : h->size);
          StoPtrFree(h->buf);
          h->buf = p;
        } else {
          if ((h->buf = StoPtrNew(h, newSize, 0, 0)) != NULL) {
            h->htype |= STO_INFLATED;
            h->useCount = 1;
            storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
            if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
              vfs_read(f, h->buf, newSize < h->size ? newSize : h->size);
              vfs_close(f);
            }
            h->lockCount = 0;
          }
        }
        h->size = newSize;
        h->d.rec.attr |= dmRecAttrDirty;
        db->modDate = TimGetSeconds();
        err = errNone;
      }
    }
  }

  StoCheckErr(err);
  return h;
}

// Move a record from one index to another.
// Insert the record at the "to" index and move other records down. The
// "to" position should be viewed as an insertion position. This value
// may be one greater than the index of the last record in the database.
// In cases where "to" is greater than "from", the new index of the record
// becomes "to-1" after the move is complete.

Err DmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  storage_handle_t *h;
  UInt16 i;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC) {
        err = errNone;
        if (db->numRecs > 1 && from < db->numRecs) {
          if (to >= db->numRecs) {
//debug(1, "XXX", "DmMoveRecord %d to end", from);
            //     F           T
            // 0 1 2 3 4 5 6 7 8
            to = db->numRecs - 1;
            h = db->elements[from];
            for (i = from; i < to; i++) {
              db->elements[i] = db->elements[i + 1];
            }
            db->elements[to] = h;
          } else if (from > to) {
//debug(1, "XXX", "DmMoveRecord %d down to %d", from, to);
            //     T     F
            // 0 1 2 3 4 5 6 7 8
            h = db->elements[from];
            for (i = from; i > to; i--) {
              db->elements[i] = db->elements[i - 1];
            }
            db->elements[to] = h;
          } else if (from < to) {
//debug(1, "XXX", "DmMoveRecord %d up to %d", from, to);
            //     F     T
            // 0 1 2 3 4 5 6 7 8
            h = db->elements[from];
            to--;
            for (i = from; i < to; i++) {
              db->elements[i] = db->elements[i + 1];
            }
            db->elements[to] = h;
          }
          db->modDate = TimGetSeconds();
        }
      }
    }
  }

  StoCheckErr(err);
  return err;
}

// Delete a record’s chunk from a database but leave the record entry in the header and set the delete bit for the next sync.
// Marks the delete bit in the database header for the record and disposes of the record’s data chunk. Does not remove the record
// entry from the database header, but simply sets the localChunkID of the record entry to NULL.

Err DmDeleteRecord(DmOpenRef dbP, UInt16 index) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  char oldName[VFS_PATH];
  char newName[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *) (sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC && db->numRecs > 0 && index < db->numRecs) {
          if (db->elements[index]->lockCount == 0) {
//debug(1, "XXX", "DmDeleteRecord index %d", index);
            h = db->elements[index];
            if (!(h->d.rec.attr & dmRecAttrDelete)) {
              storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, oldName);
              h->d.rec.attr |= dmRecAttrDelete;
              storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, newName);
//debug(1, "XXX", "DmDeleteRecord rename [%s] to [%s]", oldName, newName);
              if (sys_strcmp(oldName, newName)) {
//debug(1, "XXX", "DmDeleteRecord renaming");
                if (StoVfsRename(sto->session, oldName, newName) == 0) {
//debug(1, "XXX", "DmDeleteRecord rename ok");
                  if (h->buf) {
//debug(1, "XXX", "DmDeleteRecord free buf");
                    StoPtrFree(h->buf);
                    h->buf = NULL;
                  }
//debug(1, "XXX", "DmDeleteRecord deflate");
                  h->htype &= ~STO_INFLATED;
                  if (h->useCount) {
                    h->useCount--;
                  } else {
                    debug(DEBUG_ERROR, "STOR", "DmDeleteRecord database \"%s\" index %d useCount < 0", db->name, index);
                  }
                  db->modDate = TimGetSeconds();
                  err = errNone;
                }
              } else {
//debug(1, "XXX", "DmDeleteRecord same name");
              }
            } else {
              debug(DEBUG_ERROR, "STOR", "DmDeleteRecord %p %u attempt to remove deleted record", dbP, index);
            }
          } else {
            debug(DEBUG_ERROR, "STOR", "DmDeleteRecord %p %u attempt to remove locked handle", dbP, index);
            err = memErrChunkLocked;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return err;
}

// Remove a record from a database and dispose of its data chunk.
// Disposes of the record’s data chunk and removes the record’s entry from the database header. DmRemoveRecord should only be
// used for newly-created records that have just been deleted or records that have never been sync’ed.

Err DmRemoveRecord(DmOpenRef dbP, UInt16 index) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  UInt16 i;
  char buf[VFS_PATH];
  Err err = dmErrInvalidParam;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *) (sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC && db->numRecs > 0 && index < db->numRecs) {
          if (db->elements[index]->lockCount == 0) {
            h = db->elements[index];
            storage_name(sto, db->name, STO_FILE_ELEMENT, index, 0, h->d.rec.attr & ATTR_MASK, h->d.rec.uniqueID, buf);
//debug(1, "XXX", "DmRemoveRecord remove file [%s]", buf);
            StoVfsUnlink(sto->session, buf);
            if (h->buf) StoPtrFree(h->buf);
            pumpkin_heap_free(h, "Handle");
            db->numRecs--;
            if (db->numRecs > 0) {
              for (i = index; i < db->numRecs; i++) {
                db->elements[i] = db->elements[i + 1];
              }
            }
            db->modDate = TimGetSeconds();
            err = errNone;
          } else {
            debug(DEBUG_ERROR, "STOR", "DmRemoveRecord %p %u attempt to remove locked handle", dbP, index);
            err = memErrChunkLocked;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return err;
}

MemHandle DmNewHandle(DmOpenRef dbP, UInt32 size) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  DmOpenType *dbRef;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID == sto->watchID) {
      debug(DEBUG_INFO, "STOR", "WATCH DmNewHandle(%p, %u)", dbRef, size);
    }
  }

  return MemHandleNew(size);
}

Err DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, UInt16 *indexP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  uint32_t i;
  Err err = dmErrInvalidParam;

  if (dbP && indexP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC) {
        err = dmErrUniqueIDNotFound;
        for (i = 0; i < db->numRecs; i++) {
          h = db->elements[i];
          if (h->d.rec.uniqueID == uniqueID) {
            *indexP = i;
            err = errNone;
            break;
          }
        }
        if (i == db->numRecs) {
          debug(DEBUG_ERROR, "STOR", "DmFindRecordByID 0x%08X not found", uniqueID);
        }
      }
    }
  }

  StoCheckErr(err);
  return err;
}

MemHandle DmNewRecordEx(DmOpenRef dbP, UInt16 *atP, UInt32 size, void *p, UInt32 uniqueID, UInt16 attr, Boolean setAttr) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h = NULL;
  DmOpenType *dbRef;
  vfs_file_t *f;
  uint32_t n;
  char buf[VFS_PATH];
  int j;
  Err err = dmErrInvalidParam;

  if (dbP && atP && size > 0) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        if (dbRef->dbID == sto->watchID) {
          debug(DEBUG_INFO, "STOR", "WATCH DmNewRecord(%p, %p [%d], %u)", dbRef, atP, atP ? *atP : 0, size);
        }
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        debug(DEBUG_TRACE, "STOR", "DmNewRecordEx database \"%s\" at %d size %u", db->name, *atP, size);
        if (db->ftype == STO_TYPE_REC) {
          if (*atP >= db->numRecs) *atP = db->numRecs;
          if ((h = pumpkin_heap_alloc(sizeof(storage_handle_t), "Handle")) != NULL) {
            h->magic = STO_MAGIC;
            h->htype = STO_TYPE_REC | STO_INFLATED;
            h->owner = pumpkin_get_current();
            if (setAttr) {
              h->d.rec.uniqueID = uniqueID;
              h->d.rec.attr = attr;
            } else {
              h->d.rec.uniqueID = db->uniqueIDSeed++;
              h->d.rec.attr = dmRecAttrDirty | dmRecAttrBusy;
            }
            h->buf = StoPtrNew(h, size, 0, 0);
            h->size = size;
            db->modDate = TimGetSeconds();

            if (*atP == db->numRecs) {
              StoAddDatabaseHandle(sto, db, h);
            } else {
              StoAddDatabaseHandle(sto, db, h);
              for (j = db->numRecs - 1; j > *atP; j--) {
                db->elements[j] = db->elements[j - 1];
              }
              db->elements[*atP] = h;
            }

            storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, h->d.rec.attr & 0x0F, h->d.rec.uniqueID, buf);
            if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
              if (p) {
                sys_memcpy(&h->buf[0], p, size);
                vfs_write(f, (uint8_t *)p, size);
              } else {
                sys_memset(buf, 0, sizeof(buf));
                for (n = 0; (n + sizeof(buf)) < size; n += sizeof(buf)) {
                  vfs_write(f, (uint8_t *)buf, sizeof(buf));
                }
                if (size > n) {
                  vfs_write(f, (uint8_t *)buf, size - n);
                }
              }
              vfs_close(f);
            }
            StoWriteIndex(sto, db);
            err = errNone;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return h;
}

MemHandle DmNewRecord(DmOpenRef dbP, UInt16 *atP, UInt32 size) {
  return DmNewRecordEx(dbP, atP, size, NULL, 0, 0, false);
}

// Attach an existing chunk ID handle to a database as a record.
// atP: pointer to the index where the new record should be placed. Specify the value dmMaxRecordIndex to add the record to the end of the database.
// Given the handle of an existing chunk, this routine makes that chunk a new record in a database and sets the dirty bit.
// If oldHP is NULL, the new record is inserted at index *atP and all record indices that follow are shifted down.
// If *atP is greater than the number of records currently in the database, the new record is appended to the end and its index is returned in *atP.
// If oldHP is not NULL, the new record replaces an existing record at index *atP and the handle of the old record is returned in *oldHP.

Err DmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h, *old;
  DmOpenType *dbRef;
  vfs_file_t *f;
  char buf[VFS_PATH];
  UInt16 i;
  Err err = dmErrIndexOutOfRange;

//debug(1, "XXX", "DmAttachRecord at %d", *atP);
  if (dbP && atP && newH) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC) {
//debug(1, "XXX", "DmAttachRecord numRecs %d", db->numRecs);
          if (*atP > db->numRecs) *atP = db->numRecs;
          h = (storage_handle_t *)newH;
          h->owner = 0;
          if ((h->htype & ~STO_INFLATED) != STO_TYPE_REC) {
//debug(1, "XXX", "DmAttachRecord fix type");
            h->htype = (h->htype & STO_INFLATED) | STO_TYPE_REC;
          }
          h->d.rec.uniqueID = db->uniqueIDSeed++;
          h->d.rec.attr = dmRecAttrDirty;
          h->d.rec.attr |= dmRecAttached;
//debug(1, "XXX", "DmAttachRecord uniqueID %d", h->d.rec.uniqueID);

          if (*atP == db->numRecs) {
//debug(1, "XXX", "DmAttachRecord add at end");
            StoAddDatabaseHandle(sto, db, h);
            if (oldHP) *oldHP = NULL;
          } else {
//debug(1, "XXX", "DmAttachRecord add in the begining/middle");
            old = db->elements[*atP];
            if (!(old->htype & STO_INFLATED)) {
//debug(1, "XXX", "DmAttachRecord old not inflated");
              old->htype |= STO_INFLATED;
              old->useCount = 1;
              if ((old->buf = StoPtrNew(old, old->size, 0, 0)) != NULL) {
//debug(1, "XXX", "DmAttachRecord old inflate old %d bytes", old->size);
                storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, old->d.rec.attr & ATTR_MASK, old->d.rec.uniqueID, buf);
                if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                  vfs_read(f, old->buf, old->size);
                  vfs_close(f);
                }
              }
            } else {
              old->useCount++;
            }

            if (oldHP) {
              // new record replaces old
//debug(1, "XXX", "DmAttachRecord replace element at %d", *atP);
              db->elements[*atP] = h;
              old->htype = (old->htype & STO_INFLATED) | STO_TYPE_MEM;
              *oldHP = old;
              storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, old->d.rec.attr & ATTR_MASK, old->d.rec.uniqueID, buf);
//debug(1, "XXX", "DmAttachRecord remove old file [%s]", buf);
              StoVfsUnlink(sto->session, buf);
            } else {
              // new record is inserted at position, records are shifted down
              StoAddDatabaseHandle(sto, db, h); // just to add space, h at last position will be overwritten below
              for (i = db->numRecs - 1; i > *atP; i--) {
//debug(1, "XXX", "DmAttachRecord shift element at %d to %d", i-1, i);
                db->elements[i] = db->elements[i - 1];
              }
//debug(1, "XXX", "DmAttachRecord set element at %d", *atP);
              db->elements[*atP] = h;
            }
          }

          if (h->htype & STO_INFLATED) {
            storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, 0x00, h->d.rec.uniqueID, buf);
//debug(1, "XXX", "DmAttachRecord write new file [%s]", buf);
            if ((f = StoVfsOpen(sto->session, buf, VFS_WRITE | VFS_TRUNC)) != NULL) {
//debug(1, "XXX", "DmAttachRecord write %d bytes", h->size);
              vfs_write(f, h->buf, h->size);
              vfs_close(f);
            }
          }
          db->modDate = TimGetSeconds();
          StoWriteIndex(sto, db);
          err = errNone;
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  return err;
}

// Detach and orphan a record from a database but don’t delete the record’s chunk.
// This routine detaches a record from a database by removing its entry from the database header and returns the handle of the record’s data chunk in *oldHP.
// Unlike DmDeleteRecord, this routine removes its entry in the database header but it does not delete the actual record.

Err DmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *old;
  DmOpenType *dbRef;
  vfs_file_t *f;
  char buf[VFS_PATH];
  UInt16 i;
  Err err = dmErrIndexOutOfRange;

//debug(1, "XXX", "DmDetachRecord at %d", *atP);
  if (dbP && oldHP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef && (dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_REC && index < db->numRecs) {
//debug(1, "XXX", "DmDetachRecord numRecs %d", db->numRecs);
          if (db->elements[index]->lockCount == 0) {
            old = db->elements[index];
            old->owner = pumpkin_get_current();
            storage_name(sto, db->name, STO_FILE_ELEMENT, 0, 0, old->d.rec.attr & ATTR_MASK, old->d.rec.uniqueID, buf);
            old->htype = (old->htype & STO_INFLATED) | STO_TYPE_MEM;
            old->d.rec.attr &= ~dmRecAttached;
            if (!(old->htype & STO_INFLATED)) {
              old->htype |= STO_INFLATED;
              old->useCount = 1;
//debug(1, "XXX", "DmDetachRecord old not inflated");
              if ((old->buf = StoPtrNew(old, old->size, 0, 0)) != NULL) {
//debug(1, "XXX", "DmDetachRecord old inflate old %d bytes", old->size);
                if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
                  vfs_read(f, old->buf, old->size);
                  vfs_close(f);
                }
              }
            } else {
              old->useCount++;
            }

//debug(1, "XXX", "DmDetachRecord remove old file [%s]", buf);
            StoVfsUnlink(sto->session, buf);
            *oldHP = old;
            for (i = index; i < db->numRecs-1; i++) {
//debug(1, "XXX", "DmDetachRecord shift element at %d to %d", i-1, i);
              db->elements[i] = db->elements[i + 1];
            }
            db->numRecs--;
            db->modDate = TimGetSeconds();
            err = errNone;
          } else {
            debug(DEBUG_ERROR, "STOR", "DmDetachRecord %p %u attempt to dettach locked handle", dbP, index);
            err = memErrChunkLocked;
          }
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  return err;
}

Err DmDetachResource(DmOpenRef dbP, UInt16 index, MemHandle *oldHP) {
  Err err = dmErrIndexOutOfRange;
  return err;
}

Err DmSet(void *recordP, UInt32 offset, UInt32 bytes, UInt8 value) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  uint16_t *attr;
  uint8_t *r, *b;
  Err err = dmErrNotValidRecord;

  if (recordP && bytes > 0) {
    if ((h = StoPtrRecoverHandle(recordP)) != NULL) {
        b = NULL;
        attr = NULL;
        switch (h->htype & ~STO_INFLATED) {
          case STO_TYPE_MEM:
            b = (uint8_t *)h->buf;
            break;
          case STO_TYPE_REC:
            b = (uint8_t *)h->buf;
            attr = &h->d.rec.attr;
            break;
          case STO_TYPE_RES:
            b = (uint8_t *)h->buf;
            attr = &h->d.res.attr;
            break;
          default:
            debug(DEBUG_ERROR, "STOR", "DmSet %p unexpected handle type %d", h, h->htype & ~STO_INFLATED);
            break;
        }

        if (b) {
          r = (uint8_t *)recordP;
          if ((r + offset) >= b && (r + offset + bytes) <= (b + h->size)) {
            sys_memset(r + offset, value, bytes);
            if (attr) *attr |= dmRecAttrDirty;
            err = errNone;
          }
        }
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmSetDirty(MemHandle handle) {
  storage_handle_t *h;
  Err err = dmErrIndexOutOfRange;

  if (handle) {
    h = (storage_handle_t *)handle;

    switch (h->htype & ~STO_INFLATED) {
      case STO_TYPE_REC:
        h->d.rec.attr |= dmRecAttrDirty;
        err = errNone;
        break;
      case STO_TYPE_RES:
        h->d.res.attr |= dmRecAttrDirty;
        err = errNone;
        break;
    }
  }

  return err;
}

Err DmWriteOrCheck(void *recordP, UInt32 offset, const void *srcP, UInt32 bytes, Boolean onlyCheck) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  uint16_t *attr;
  uint8_t *r, *b;
  Err err = dmErrNotValidRecord;

  if (recordP) {
    if (bytes == 0) {
      err = errNone;
    } else {
      if ((h = MemPtrRecoverHandle(recordP)) != NULL) {
        b = NULL;
        attr = NULL;
        switch (h->htype & ~STO_INFLATED) {
          case STO_TYPE_MEM:
            b = (uint8_t *)h->buf;
            break;
          case STO_TYPE_REC:
            b = (uint8_t *)h->buf;
            attr = &h->d.rec.attr;
            break;
          case STO_TYPE_RES:
            b = (uint8_t *)h->buf;
            attr = &h->d.res.attr;
            break;
          default:
            debug(DEBUG_ERROR, "STOR", "DmWriteOrCheck %p unexpected handle type %d", h, h->htype & ~STO_INFLATED);
            break;
        }

        if (b) {
          r = (uint8_t *)recordP;
          if ((r + offset) >= b && (r + offset + bytes) <= (b + h->size)) {
            if (onlyCheck) {
              err = errNone;
            } else if (srcP) {
//uint32_t size = MemHandleSize(h);
//debug_bytes(1, "XXX", r, size);
//debug_bytes(1, "XXX", (uint8_t *)srcP, bytes);
              sys_memcpy(r + offset, srcP, bytes);
//debug_bytes(1, "XXX", r, size);
              if (attr) *attr |= dmRecAttrDirty;
              err = errNone;
            }
          }
        }
      }
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmWriteCheck(void *recordP, UInt32 offset, UInt32 bytes) {
  return DmWriteOrCheck(recordP, offset, NULL, bytes, true);
}

Err DmWrite(void *recordP, UInt32 offset, const void *srcP, UInt32 bytes) {
  debug(DEBUG_TRACE, "STOR", "DmWrite offset %d size %d", offset, bytes);
  debug_bytes(DEBUG_TRACE, "STOR", (uint8_t *)srcP, bytes);
  return DmWriteOrCheck(recordP, offset, srcP, bytes, false);
}

Err DmStrCopy(void *recordP, UInt32 offset, const Char *srcP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  Err err = dmErrNotValidRecord;

  if (srcP) {
    err = DmWrite(recordP, offset, srcP, StrLen(srcP)+1);
  }

  StoCheckErr(err);
  return err;
}

Err DmResourceInfo(DmOpenRef dbP, UInt16 index, DmResType *resTypeP, DmResID *resIDP, LocalID *chunkLocalIDP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  Err err = dmErrResourceNotFound;

  dbRef = (DmOpenType *)dbP;
  if (dbRef) {
    db = (storage_db_t *)(sto->base + dbRef->dbID);
    if (db->ftype == STO_TYPE_RES && index < db->numRecs) {
      h = db->elements[index];
      if (resTypeP) *resTypeP = h->d.res.type;
      if (resIDP) *resIDP = h->d.res.id;
      if (chunkLocalIDP) *chunkLocalIDP = (uint8_t *)db->elements[index] - sto->base;
      err = errNone;
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmSetResourceInfo(DmOpenRef dbP, UInt16 index, DmResType *resTypeP, DmResID *resIDP) {
  return dmErrInvalidParam;
}

UInt16 DmNumResources(DmOpenRef dbP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  UInt16 numRecs = 0;
  Err err = dmErrInvalidParam;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES) {
        numRecs = db->numRecs;
        err = errNone;
      } else {
        err = dmErrNotResourceDB;
      }
    }
  }

  StoCheckErr(err);
  return numRecs;
}

void *DmResourceLoadLib(DmOpenRef dbP, DmResType resType, Boolean *firstLoad) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  UInt16 id;
  char buf[VFS_PATH];
  int first_load;
  void *lib = NULL;

  if (dbP && firstLoad) {
    if (mutex_lock(sto->mutex) == 0) {
      id = SYS_OS * 64 + SYS_CPU * 8 + SYS_SIZE;
      debug(DEBUG_INFO, "STOR", "searching for dlib id %d (os=%d cpu=%d size=%d)", id, SYS_OS, SYS_CPU, SYS_SIZE);

      if ((h = DmGetResourceEx((DmOpenType *)dbP, resType, id, true, false)) != NULL) {
        dbRef = (DmOpenType *)dbP;
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        storage_name(sto, db->name, STO_FILE_ELEMENT, id, resType, 0, 0, buf);
        lib = StoVfsLoadlib(sto->session, buf, &first_load);
        *firstLoad = lib != NULL && first_load == 1;
      }
      mutex_unlock(sto->mutex);
    }
  }

  return lib;
}

UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  uint32_t i;
  Err err = dmErrResourceNotFound;
  UInt16 index = 0xFFFF;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES) {
        if (resH) {
          // search by handle
          for (i = 0; i < db->numRecs; i++) {
            if (db->elements[i] == (storage_handle_t *)resH) {
              index = i;
              err = errNone;
              break;
            }
          }
        } else {
          // search by type and id
          for (i = 0; i < db->numRecs; i++) {
            h = db->elements[i];
            if (h->d.res.type == resType && h->d.res.id == resID) {
              index = i;
              err = errNone;
              break;
            }
          }
        }
      }
    }
  }

  StoCheckErr(err);
  return index;
}

UInt16 DmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  uint32_t i;
  Err err = dmErrResourceNotFound;
  UInt16 idx, index = 0xFFFF;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES) {
        for (i = 0, idx = 0; i < db->numRecs; i++) {
          h = db->elements[i];
          if (h->d.res.type == resType) {
            if (idx == typeIndex) {
              index = i;
              err = errNone;
              break;
            }
            idx++;
          }
        }
      }
    }
  }

  StoCheckErr(err);
  return index;
}

UInt16 DmFindResourceID(DmOpenRef dbP, UInt16 resID, UInt16 idIndex) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  uint32_t i;
  Err err = dmErrResourceNotFound;
  UInt16 idx, index = 0xFFFF;

  if (dbP) {
    dbRef = (DmOpenType *)dbP;
    if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_RES) {
        for (i = 0, idx = 0; i < db->numRecs; i++) {
          h = db->elements[i];
          if (h->d.res.id == resID) {
            if (idx == idIndex) {
              index = i;
              err = errNone;
              break;
            }
            idx++;
          }
        }
      }
    }
  }

  StoCheckErr(err);
  return index;
}

Err DmGetLastErr(void) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  return sto->lastErr;
}

// Mark a record as archived by leaving the record’s chunk intact and setting the delete bit for the next sync.
// When a record is archived, the deleted bit is set but the chunk is not freed and the local ID is preserved. This way, the next time the user
// synchronizes with the desktop system, the desktop can save the record data on the PC before it permanently removes the record
// entry and data from the device.
// Based on the assumption that a call to DmArchiveRecord indicates that you are finished with the record and aren’t going to refer to it
// again, this function sets the chunk’s lock count to zero.

Err DmArchiveRecord(DmOpenRef dbP, UInt16 index) {
  // XXX using DmDeleteRecord, the record is deleted, not left intact
  return DmDeleteRecord(dbP, index);
}

Err DmRemoveSecretRecords(DmOpenRef dbP) {
  debug(DEBUG_ERROR, "STOR", "DmRemoveSecretRecords not implemented");
  return dmErrInvalidParam;
}

Err DmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum) {
  debug(DEBUG_ERROR, "STOR", "DmDeleteCategory not implemented");
  return dmErrInvalidParam;
}

Err DmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty) {
  debug(DEBUG_ERROR, "STOR", "DmMoveCategory not implemented");
  return dmErrInvalidParam;
}

// numRecs=0, index=0, offset=0, direction=1 : dmErrIndexOutOfRange
// numRecs=0, index=0, offset=1, direction=1 : dmErrIndexOutOfRange
// numRecs=1, index=0, offset=0, direction=1 : errNone
// numRecs=1, index=0, offset=1, direction=1 : dmErrSeekFailed
// numRecs=1, index=1, offset=0, direction=1 : dmErrIndexOutOfRange

// numRecs=0, index=0, offset=0, direction=-1: dmErrSeekFailed
// numRecs=0, index=0, offset=1, direction=-1: dmErrSeekFailed
// numRecs=1, index=0, offset=1, direction=-1: dmErrSeekFailed

Err DmSeekRecordInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  storage_handle_t *h;
  DmOpenType *dbRef;
  Err err = dmErrSeekFailed;

//debug(1, "XXX", "DmSeekRecordInCategory index=%d (%p), offset=%d, direction=%d, category=%u", indexP ? *indexP : 0, indexP, offset, direction, category);
  if (dbP && indexP) {
//debug(1, "XXX", "seek begin offset=%d", offset);
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
         debug(DEBUG_TRACE, "STOR", "DmSeekRecordInCategory n=%d i=%u o=%d d=%d c=%u", DmNumRecords(dbRef), *indexP, offset, direction, category);
//debug(1, "XXX", "seek numRecs=%d", db->numRecs);
        if (db->numRecs > 0) {
          if (*indexP == 0xFFFF) *indexP = db->numRecs-1;
//debug(1, "XXX", "seek index=%d", *indexP);
          if (*indexP < db->numRecs) {
          for (;;) {
            h = db->elements[*indexP];
//debug(1, "XXX", "seek index=%d category rec=%d param=%d", *indexP, h->d.rec.attr & dmRecAttrCategoryMask, category);
            if (!(h->d.rec.attr & dmRecAttrDelete) && (category == dmAllCategories || (h->d.rec.attr & dmRecAttrCategoryMask) == category)) {
              if (!(h->d.rec.attr & dmRecAttrSecret) || (dbRef->mode & dmModeShowSecret)) {
//debug(1, "XXX", "seek match offset=%d", offset);
                if (offset == 0) {
//debug(1, "XXX", "seek success");
                  err = errNone;
                  break;
                }
                offset--;
//debug(1, "XXX", "seek decrement offset=%d", offset);
              }
            }
            if (direction == dmSeekForward) {
              // forward
//debug(1, "XXX", "seek forward index=%d", *indexP);
              if (*indexP == db->numRecs-1) {
//debug(1, "XXX", "seek forward abort");
                err = dmErrIndexOutOfRange;
                break;
               }
              *indexP = *indexP + 1;
            } else {
              // backward
//debug(1, "XXX", "seek backward index=%d", *indexP);
              if (*indexP == 0) {
//debug(1, "XXX", "seek backward abort");
                err = dmErrSeekFailed;
                break;
              }
              *indexP = *indexP - 1;
            }
          }
          } else {
//debug(1, "XXX", "seek index %d >= %d", *indexP, db->numRecs);
            err = dmErrIndexOutOfRange;
          }
        } else {
//debug(1, "XXX", "seek empty");
          err = (direction == dmSeekForward) ? dmErrIndexOutOfRange : dmErrSeekFailed;
        }
        debug(DEBUG_TRACE, "STOR", "DmSeekRecordInCategory i=%u err=%d", *indexP, err);
      }
  }

  StoCheckErr(err);
  return err;
}

void DmGetDatabaseLockState(DmOpenRef dbR, UInt8 *highest, UInt32 *count, UInt32 *busy) {
  debug(DEBUG_ERROR, "STOR", "DmGetDatabaseLockState not implemented");
}

Err DmResetRecordStates(DmOpenRef dbP) {
  debug(DEBUG_ERROR, "STOR", "DmResetRecordStates not implemented");
  return dmErrInvalidParam;
}

static Boolean StoValidName(UInt8 *buf) {
  Int32 i;

  for (i = 0; i < dmDBNameLength && buf[i]; i++) {
    if (buf[i] < 32) return false;
  }

  return true;
}

static Boolean StoValidTypeCreator(UInt8 *buf) {
  Int32 i;

  for (i = 0; i < 4 && buf[i]; i++) {
    if (buf[i] < 32 || buf[i] > 126) return false;
  }

  return i > 0;
}

Err VFSFileDBInfo(FileRef ref, Char *nameP,
          UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
          UInt32 *modDateP, UInt32 *bckUpDateP,
          UInt32 *modNumP, MemHandle *appInfoHP,
          MemHandle *sortInfoHP, UInt32 *typeP,
          UInt32 *creatorP, UInt16 *numRecordsP) {

  char name[dmDBNameLength];
  UInt8 database[256];
  UInt16 attr, version, numRecs;
  UInt32 i, nread, creationDate, modificationDate, lastBackupDate, modificationNumber, appInfo, sortInfo, type, creator, uniqueIDSeed, pos, dummy32;
  Err err;

  VFSFileTell(ref, &pos);

  if ((err = VFSFileRead(ref, sizeof(database), database, &nread)) == errNone) {
    VFSFileSeek(ref, vfsOriginBeginning, pos);
    err = dmErrInvalidParam;

    if (!StoValidName(database)) {
      debug(DEBUG_ERROR, "STOR", "VFSFileDBInfo invalid name \"%.*s\"", dmDBNameLength, database);
      pumpkin_set_lasterr(err);
      return err;
    }

    StrNCopy(name, (char *)database, dmDBNameLength - 1);
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
    if (!StoValidTypeCreator(&database[i])) {
      debug(DEBUG_ERROR, "STOR", "VFSFileDBInfo invalid type 0x%08X", type);
      pumpkin_set_lasterr(err);
      return err;
    }
    i += 4;
    pumpkin_s2id(&creator, (char *)&database[i]);
    if (!StoValidTypeCreator(&database[i])) {
      debug(DEBUG_ERROR, "STOR", "VFSFileDBInfo invalid creator 0x%08X", creator);
      pumpkin_set_lasterr(err);
      return err;
    }
    i += 4;
    i += get4b(&uniqueIDSeed, database, i);
    i += get4b(&dummy32, database, i);  // nextRecordListID
    i += get2b(&numRecs, database, i);  // numberOfRecords

    if (attributesP) *attributesP = attr;
    if (versionP) *versionP = version;
    if (crDateP) *crDateP = creationDate;
    if (modDateP) *modDateP = modificationDate;
    if (bckUpDateP) *bckUpDateP = lastBackupDate;
    if (modNumP) *modNumP = modificationNumber;
    if (appInfoHP) *appInfoHP = NULL; // XXX
    if (sortInfoHP) *sortInfoHP = NULL; // XXX
    if (typeP) *typeP = type;
    if (creatorP) *creatorP = creator;

    err = errNone;
  }

  pumpkin_set_lasterr(err);
  return err;
}

Err DmCreateDatabaseFromImage(MemPtr bufferP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  char name[dmDBNameLength], st[8];
  UInt16 j, attr, version, numRecs, index, appInfoSize, sortInfoSize;
  UInt32 i, creationDate, modificationDate, lastBackupDate, modificationNumber, appInfo, sortInfo, type, creator, uniqueIDSeed, size, dummy32;
  UInt32 *offsets, *resTypes, *uniqueIDs, firstOffset;
  UInt16 *resIDs, *attrs;
  UInt8 *database, dummy8;
  MemHandle appInfoH, sortInfoH;
  void *appInfoP, *sortInfoP;
  DmOpenType *dbRef;
  LocalID dbID;
  Err err = dmErrInvalidParam;

  if (bufferP) {
    database = (UInt8 *)bufferP;
    if (!StoValidName(database)) {
      debug(DEBUG_ERROR, "STOR", "DmCreateDatabaseFromImage invalid name \"%.*s\"", dmDBNameLength, database);
      return err;
    }
    MemSet(name, dmDBNameLength, 0);
    StrNCopy(name, (char *)database, dmDBNameLength - 1);
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
    if (!StoValidTypeCreator(&database[i])) {
      debug(DEBUG_ERROR, "STOR", "DmCreateDatabaseFromImage invalid type 0x%08X", type);
      return err;
    }
    i += 4;
    pumpkin_s2id(&creator, (char *)&database[i]);
    if (!StoValidTypeCreator(&database[i])) {
      debug(DEBUG_ERROR, "STOR", "DmCreateDatabaseFromImage invalid creator 0x%08X", creator);
      return err;
    }
    i += 4;
    i += get4b(&uniqueIDSeed, database, i);
    i += get4b(&dummy32, database, i);  // nextRecordListID
    i += get2b(&numRecs, database, i);  // numberOfRecords
    debug(DEBUG_INFO, "STOR", "DmCreateDatabaseFromImage \"%s\" with %d recs", name, numRecs);

    if (DmCreateDatabaseEx(name, creator, type, attr, uniqueIDSeed, true) == errNone) {
      if ((dbID = DmFindDatabase(0, name)) != 0) {
        DmSetDatabaseInfo(0, dbID, NULL, NULL, &version, &creationDate, &modificationDate, &lastBackupDate, NULL, NULL, NULL, NULL, NULL);
        if ((dbRef = DmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
          db = (storage_db_t *)(sto->base + dbID);
          if (numRecs > 0) {
            resTypes = MemPtrNew(numRecs * sizeof(UInt32));
            resIDs = MemPtrNew(numRecs * sizeof(UInt16));
            uniqueIDs = MemPtrNew(numRecs * sizeof(UInt32));
            attrs = MemPtrNew(numRecs * sizeof(UInt16));
            offsets = MemPtrNew(numRecs * sizeof(UInt32));

            if (attr & dmHdrAttrResDB) {
              for (j = 0; j < numRecs; j++) {
                i += get4b(&resTypes[j], database, i);
                i += get2b(&resIDs[j], database, i);
                i += get4b(&offsets[j], database, i);
              }
              i = firstOffset = offsets[0];
              for (j = 0; j < numRecs; j++) {
                size = (j < numRecs-1) ? offsets[j+1] - offsets[j] : MemPtrSize(bufferP) - offsets[j];
                pumpkin_id2s(resTypes[j], st);
                debug(DEBUG_INFO, "STOR", "DmCreateDatabaseFromImage res %d type '%s' id %d size %u", j, st, resIDs[j], size);
                DmNewResourceEx(dbRef, resTypes[j], resIDs[j], size, &database[offsets[j]]);
              }
              err = errNone;
            } else {
              for (j = 0; j < numRecs; j++) {
                i += get4b(&offsets[j], database, i);
                i += get1(&dummy8, database, i);
                attrs[j] = dummy8;
                i += get1(&dummy8, database, i);
                uniqueIDs[j] = ((UInt32)dummy8) << 16;
                i += get1(&dummy8, database, i);
                uniqueIDs[j] |= ((UInt32)dummy8) << 8;
                i += get1(&dummy8, database, i);
                uniqueIDs[j] |= ((UInt32)dummy8);
              }
              i = firstOffset = offsets[0];
              for (j = 0; j < numRecs; j++) {
                size = (j < numRecs-1) ? offsets[j+1] - offsets[j] : MemPtrSize(bufferP) - offsets[j];
                index = dmMaxRecordIndex;
                DmNewRecordEx(dbRef, &index, size, &database[offsets[j]], uniqueIDs[j], attrs[j], uniqueIDs[j] != 0);
              }
              if (StoWriteIndex(sto, db) == 0) {
                err = errNone;
              }
            }

            MemPtrFree(resTypes);
            MemPtrFree(resIDs);
            MemPtrFree(offsets);
          } else {
            firstOffset = 0;
            err = errNone;
          }

          appInfoSize = 0;
          sortInfoSize = 0;
          if (firstOffset == 0) firstOffset = MemPtrSize(bufferP);

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

          if (appInfoSize) {
            debug(DEBUG_INFO, "STOR", "DmCreateDatabaseFromImage appInfo %d bytes", appInfoSize);
            if ((appInfoH = MemHandleNew(appInfoSize)) != NULL) {
              if ((appInfoP = MemHandleLock(appInfoH)) != NULL) {
                MemMove(appInfoP, &database[appInfo], appInfoSize);
                MemHandleUnlock(appInfoH);
                db->appInfoID = MemHandleToLocalID(appInfoH);
                StoWriteAppInfo(sto, db);
              }
            }
          }
          if (sortInfoSize) {
            debug(DEBUG_INFO, "STOR", "DmCreateDatabaseFromImage sortInfo %d bytes", sortInfoSize);
            if ((sortInfoH = MemHandleNew(sortInfoSize)) != NULL) {
              if ((sortInfoP = MemHandleLock(sortInfoH)) != NULL) {
                MemMove(sortInfoP, &database[sortInfo], sortInfoSize);
                MemHandleUnlock(sortInfoH);
                db->sortInfoID = MemHandleToLocalID(sortInfoH);
                StoWriteSortInfo(sto, db);
              }
            }
          }

          DmCloseDatabase(dbRef);
        }
      }
    }
  }

  return err;
}

void *StoNewDecodedResource(void *h, UInt32 size, DmResType resType, DmResID resID) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h2, *handle = NULL;
  void *p = NULL;
  Err err = dmErrInvalidParam;

  if (size) {
    if (h == NULL) {
      // h will be NULL in case of FrmNewForm()
      if ((handle = pumpkin_heap_alloc(sizeof(storage_handle_t), "Handle")) != NULL) {
        handle->magic = STO_MAGIC;
        handle->htype = STO_TYPE_RES;
        handle->owner = pumpkin_get_current();
        handle->size = size;
      }
      h = handle;
    }
    if (h != NULL) {
      h2 = h;
      if ((h2->htype & ~STO_INFLATED) != STO_TYPE_RES && resType && resID) {
        h2->htype &= STO_INFLATED;
        h2->htype |= STO_TYPE_RES;
        h2->d.res.type = resType;
        h2->d.res.id = resID;
      }
      if (resType == bitmapRsc) {
        p = h2->buf;
      } else {
        p = StoPtrNew(h2, size, h2->d.res.type, h2->d.res.id);
        if (handle) handle->d.res.decoded = p;
      }
      err = errNone;
    }
  }

  StoCheckErr(err);
  return p;
}

static void StoDestroyConstant(void *p) {
  MemChunkFree(p);
}

static void StoDestroyWordList(void *p) {
  MemChunkFree(p);
}

static void StoDecodeResource(storage_handle_t *res, Boolean decoded) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  UInt8 *aux;
  uint32_t ftype32, dsize;
  uint16_t ftype, version;
  char st[8];
  void *p;
  int i;

  if (res && !res->d.res.decoded) {
    pumpkin_id2s(res->d.res.type, st);
    res->d.res.destructor = NULL;

    switch (res->d.res.type) {
      case alertRscType:
        debug(DEBUG_TRACE, "STOR", "decoding alert resource %s %d", st, res->d.res.id);
        if ((p = pumpkin_create_alert(res, res->buf, &dsize)) != NULL) {
          res->d.res.destructor = pumpkin_destroy_alert;
          res->d.res.decoded = p;
          res->d.res.decodedSize = dsize;
        }
        break;
      case MenuRscType:
        debug(DEBUG_TRACE, "STOR", "decoding menu resource %s %d", st, res->d.res.id);
        if ((p = pumpkin_create_menu(res, res->buf, &dsize)) != NULL) {
          res->d.res.destructor = pumpkin_destroy_menu;
          res->d.res.decoded = p;
          res->d.res.decodedSize = dsize;
        }
        break;
      case fontRscType:
        debug(DEBUG_TRACE, "STOR", "decoding font v1 resource %s %d", st, res->d.res.id);
        if ((p = pumpkin_create_font(res, res->buf, res->size, &dsize)) != NULL) {
          res->d.res.destructor = pumpkin_destroy_font;
          res->d.res.decoded = p;
          res->d.res.decodedSize = dsize;
        }
        break;
      case 'pFNT': // XXX SmallBasic defines a v1 font resource with type 'pFNT'
      case '_Fnt': // XXX HandyShop2 defines a v1 font resource with type '_Fnt'
        get2b(&ftype, res->buf, 0);
        if (ftype == 0x9000) {
          debug(DEBUG_TRACE, "STOR", "decoding font v1 resource %s %d", st, res->d.res.id);
          if ((p = pumpkin_create_font(res, res->buf, res->size, &dsize)) != NULL) {
            res->d.res.destructor = pumpkin_destroy_font;
            res->d.res.decoded = p;
            res->d.res.decodedSize = dsize;
          }
        }
        break;
      case fontExtRscType:
        debug(DEBUG_TRACE, "STOR", "decoding font v2 resource %s %d", st, res->d.res.id);
        if ((p = pumpkin_create_fontv2(res, res->buf, res->size, &dsize)) != NULL) {
          res->d.res.destructor = pumpkin_destroy_fontv2;
          res->d.res.decoded = p;
          res->d.res.decodedSize = dsize;
        }
        break;
      case 'font': // XXX TealPaint defines a v2 font resource with type 'font'
        get2b(&ftype, res->buf, 0);
        if (ftype == 0x9200) {
          debug(DEBUG_TRACE, "STOR", "decoding font v2 resource %s %d", st, res->d.res.id);
          if ((p = pumpkin_create_fontv2(res, res->buf, res->size, &dsize)) != NULL) {
            res->d.res.destructor = pumpkin_destroy_fontv2;
            res->d.res.decoded = p;
            res->d.res.decodedSize = dsize;
          }
        }
        break;
      case ssfnRscType:
        get4b(&ftype32, res->buf, 0);
        get2b(&ftype, res->buf, 0);
        if (ftype32 == 0x53464E32 || ftype == 0x1f8b) { // 'SFN2' ssfn font or gziped ssfn font
          debug(DEBUG_TRACE, "STOR", "decoding ssfn resource %s %d", st, res->d.res.id);
          if ((p = pumpkin_create_ssfn(res, res->buf, res->size, &dsize, sto->fontFamily, sto->fontStyle, sto->fontSize)) != NULL) {
            res->d.res.destructor = pumpkin_destroy_ssfn;
            res->d.res.decoded = p;
            res->d.res.decodedSize = dsize;
          }
        }
        break;
      case omOverlayRscType:
        get2b(&version, res->buf, 0);
        if (version == 4) {
          debug(DEBUG_TRACE, "STOR", "decoding overlay resource %s %d", st, res->d.res.id);
          if ((p = pumpkin_create_overlay(res, res->buf, res->size, &dsize)) != NULL) {
            res->d.res.destructor = pumpkin_destroy_overlay;
            res->d.res.decoded = p;
            res->d.res.decodedSize = dsize;
          }
        }
        break;
      case constantRscType:
        debug(DEBUG_TRACE, "STOR", "decoding constant resource %s %d", st, res->d.res.id);
        if (res->size == sizeof(UInt32) && (aux = StoNewDecodedResource(res, res->size, 0, 0)) != NULL) {
          aux[0] = res->buf[3];
          aux[1] = res->buf[2];
          aux[2] = res->buf[1];
          aux[3] = res->buf[0];
          res->d.res.destructor = StoDestroyConstant;
          res->d.res.decoded = aux;
          res->d.res.decodedSize = res->size;
        }
        break;
      case wrdListRscType:
        debug(DEBUG_TRACE, "STOR", "decoding wordList resource %s %d", st, res->d.res.id);
        if ((!pumpkin_is_m68k() || decoded) && (res->size & 1) == 0 && (aux = StoNewDecodedResource(res, res->size, 0, 0)) != NULL) {
          for (i = 0; i < res->size; i += 2) {
            aux[i] = res->buf[i+1];
            aux[i+1] = res->buf[i];
          }
          res->d.res.destructor = StoDestroyWordList;
          res->d.res.decoded = aux;
          res->d.res.decodedSize = res->size;
        }
        break;
    }
  }
}

Err MemInit(void) {
  return errNone;
}

MemHandle MemHandleNew(UInt32 size) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h = NULL;
  Err err = dmErrInvalidParam;

  if (size) {
    if ((h = pumpkin_heap_alloc(sizeof(storage_handle_t), "Handle")) != NULL) {
      h->magic = STO_MAGIC;
      h->htype = STO_TYPE_MEM | STO_INFLATED;
      h->owner = pumpkin_get_current();
      h->size = size;
      h->buf = StoPtrNew(h, size, 0, 0);
      err = errNone;
    }
  }

  StoCheckErr(err);
  debug(DEBUG_TRACE, "STOR", "MemHandleNew %u 0x%08X %p", size, (uint32_t)(h ? (uint8_t *)h - sto->base : 0), h);
  return h;
}

Err MemHandleFree(MemHandle hh) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  char st[8];
  Err err = dmErrInvalidParam;

  debug(DEBUG_TRACE, "STOR", "MemHandleFree 0x%08X %p", (uint32_t)(hh ? (uint8_t *)hh - sto->base : 0), hh);

  if (hh) {
    h = (storage_handle_t *)hh;
    switch (h->htype & ~STO_INFLATED) {
      case STO_TYPE_MEM:
        if (h->buf) {
          StoPtrFree(h->buf);
        }
        pumpkin_heap_free(h, "Handle");
        err = errNone;
        break;
      case STO_TYPE_REC:
        if (h->buf) {
          StoPtrFree(h->buf);
        }
        pumpkin_heap_free(h, "Handle");
        err = errNone;
        break;
      case STO_TYPE_RES:
        pumpkin_id2s(h->d.res.type, st);
        debug(DEBUG_TRACE, "STOR", "MemHandleFree handle=%p %s %d", h, st, h->d.res.id);
        if (h->d.res.destructor && h->d.res.decoded) {
          debug(DEBUG_TRACE, "STOR", "MemHandleFree calling destructor");
          h->d.res.destructor(h->d.res.decoded);
          h->d.res.decoded = NULL;
          debug(DEBUG_TRACE, "STOR", "MemHandleFree destructor called");
        }
        if (h->buf) {
          debug(DEBUG_TRACE, "STOR", "MemHandleFree free buf %s %d %p", st, h->d.res.id, h->buf);
          StoPtrFree(h->buf);
        }
        pumpkin_heap_free(h, "Handle");
        err = errNone;
        break;
    }
  }

  StoCheckErr(err);
  return err;
}

MemPtr MemHandleLockEx(MemHandle h, Boolean decoded) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  DmOpenRef dbRef;
  void *p = NULL;
  Err err = dmErrInvalidParam;

  if (h) {
    handle = (storage_handle_t *)h;
    if (handle->lockCount < 14) {
      switch (handle->htype & ~STO_INFLATED) {
        case STO_TYPE_MEM:
          handle->lockCount++;
          p = handle->buf;
          err = errNone;
          break;
        case STO_TYPE_REC:
          if (!(handle->htype & STO_INFLATED)) {
            DmSearchRecord(h, &dbRef);
          }
          handle->lockCount++;
          p = handle->buf;
          err = errNone;
          break;
        case STO_TYPE_RES:
          // Apparently some apps (ex: Filez) call MemHandleLock without calling DmGetResource first.
          // This should not happen (I believe), so test if this is the case here, and call DmSearchResource
          // to force the resource to be inflated. The same may apply to records (case above).
          if (!(handle->htype & STO_INFLATED)) {
            DmSearchResource(0, 0, h, &dbRef);
          }
          handle->lockCount++;
          if (pumpkin_is_m68k() && handle->d.res.writable) {
            decoded = false;
          }
          p = decoded && handle->d.res.decoded ? handle->d.res.decoded : handle->buf;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "STOR", "MemHandleLockEx %p unexpected handle type %d", handle, handle->htype & ~STO_INFLATED);
          break;
      }
    } else {
      debug(DEBUG_ERROR, "STOR", "MemHandleLockEx %p lockCount %d", handle, handle->lockCount);
    }
  }

  StoCheckErr(err);
  return p;
}

MemPtr MemHandleLock(MemHandle h) {
  return MemHandleLockEx(h, true);
}

Err MemHandleUnlock(MemHandle h) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  Err err = dmErrInvalidParam;

  if (h) {
    handle = (storage_handle_t *)h;
    if (handle->lockCount > 0) {
      switch (handle->htype & ~STO_INFLATED) {
        case STO_TYPE_MEM:
        case STO_TYPE_REC:
        case STO_TYPE_RES:
          if (!(handle->htype & STO_INFLATED)) {
            debug(DEBUG_ERROR, "STOR", "MemHandleUnlock %p type %d not inflated", handle, handle->htype);
          }
          handle->lockCount--;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "STOR", "MemHandleUnlock %p unexpected handle type %d", handle, handle->htype & ~STO_INFLATED);
          break;
      }
    } else {
      debug(DEBUG_ERROR, "STOR", "MemHandleUnlock 0x%08X handle is not locked", (uint32_t)((uint8_t *)h - sto->base));
    }
  }

  StoCheckErr(err);
  return err;
}

Err MemHandleUnlockEx(MemHandle h, UInt16 *lockCount) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  Err err = dmErrInvalidParam;

  if (h && lockCount) {
    if ((err = MemHandleUnlock(h)) == errNone) {
      handle = (storage_handle_t *)h;
      *lockCount = handle->lockCount;
    }
  }

  StoCheckErr(err);
  return err;
}

MemPtr MemPtrNew(UInt32 size) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  MemHandle h;
  MemPtr p = NULL;
  Err err = memErrInvalidParam;

  if (size) {
    h = MemHandleNew(size);
    p = h ? MemHandleLock(h) : NULL;
    err = errNone;
  }
  debug(DEBUG_TRACE, "STOR", "MemPtrNew %u %p", size, p);

  StoCheckErr(err);
  return p;
}

Err MemChunkFree(MemPtr chunkDataP) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  char st[8];
  Err err = memErrInvalidParam;

  debug(DEBUG_TRACE, "STOR", "MemChunkFree %p", chunkDataP);
  if (chunkDataP) {
    if ((h = MemPtrRecoverHandle(chunkDataP)) != NULL) {
      debug(DEBUG_TRACE, "STOR", "MemChunkFree handle 0x%08X", (uint32_t)((uint8_t *)h - sto->base));
      switch (h->htype & ~STO_INFLATED) {
        case STO_TYPE_MEM:
          debug(DEBUG_TRACE, "STOR", "MemChunkFree memory %p 0x%08X", chunkDataP, (uint32_t)((uint8_t *)h - sto->base));
          if (h->buf) StoPtrFree(h->buf);
          pumpkin_heap_free(h, "Handle");
          err = errNone;
          break;
        case STO_TYPE_RES:
          // freeing a resource means free the associated decoded chunk (if it exists)
          pumpkin_id2s(h->d.res.type, st);
          debug(DEBUG_TRACE, "STOR", "MemChunkFree resource %s %d %p", st, h->d.res.id, h);
          if (h->d.res.decoded) {
            debug(DEBUG_TRACE, "STOR", "MemChunkFree freeing decoded");
            StoPtrFree(h->d.res.decoded);
            h->d.res.decoded = NULL;
          }
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "STOR", "MemChunkFree unexpected handle type %d", h->htype & ~STO_INFLATED);
          break;
      }
    }
  }

  StoCheckErr(err);
  return err;
}

Err MemMove(void *dstP, const void *sP, Int32 numBytes) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  //if (dstP == NULL || sP == NULL) ErrFatalDisplayEx("MemMove NULL", 1);
  // XXX TealPaint calls MemMove with sp == NULL
  if (dstP && sP && numBytes > 0) sys_memmove(dstP, sP, numBytes);
  StoCheckErr(errNone);
  return errNone;
}

Err MemSet(void *dstP, Int32 numBytes, UInt8 value) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  if (dstP == NULL) ErrFatalDisplayEx("MemSet NULL", 1);
  if (numBytes > 0) {
    sys_memset(dstP, value, numBytes);
  }
  StoCheckErr(errNone);
  return errNone;
}

Int16 MemCmp(const void *s1, const void *s2, Int32 numBytes) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  Int16 r;
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("MemCmp NULL", 1);
  r = s1 && s2 ? sys_memcmp(s1, s2, numBytes) : -1;
  StoCheckErr(errNone);
  return r;
}

Err MemKernelInit(void) {
  return errNone;
}

Err MemInitHeapTable(UInt16 cardNo) {
  return errNone;
}

UInt16 MemNumCards(void) {
  return 1;
}

UInt16 MemNumHeaps(UInt16 cardNo) {
  return 2;
}

UInt16 MemNumRAMHeaps(UInt16 cardNo) {
  return 2;
}

UInt16 MemHeapID(UInt16 cardNo, UInt16 heapIndex) {
  return heapIndex;
}

Boolean MemHeapDynamic(UInt16 heapID) {
  return heapID == 0;
}

Err MemHeapFreeBytes(UInt16 heapID, UInt32 *freeP, UInt32 *maxP) {
  if (freeP) *freeP = MemHeapSize(heapID);
  if (maxP) *maxP = MemHeapSize(heapID);
  return errNone;
}

UInt32 MemHeapSize(UInt16 heapID) {
  UInt32 size;
  // XXX Cubis will not run if 8MB is returned here
  size = heapID == 0 ? 4*1024*1024 : pumpkin_heap_size();
  return size;
}

UInt16 MemHeapFlags(UInt16 heapID) {
  return 0;
}

Err MemHeapCompact(UInt16 heapID) {
  return errNone;
}

Err MemHeapInit(UInt16 heapID, Int16 numHandles, Boolean initContents) {
  return errNone;
}

Err MemHeapFreeByOwnerID(UInt16 heapID, UInt16 ownerID) {
  return errNone;
}

MemPtr MemChunkNew(UInt16 heapID, UInt32 size, UInt16 attr) {
  // XXX is it MemHandleNew or MemPtrNew ?
  // Bejeweled 2 assumes the returned value is a MemPtr ?
  return MemPtrNew(size);
}

// Recover the handle of a movable chunk, given a pointer to its data.
MemHandle MemPtrRecoverHandle(MemPtr p) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h = NULL;
  Err err = memErrInvalidParam;

  debug(DEBUG_TRACE, "STOR", "MemPtrRecoverHandle %p", p);
  if (p) {
    if ((h = StoPtrRecoverHandle(p)) != NULL) {
      err = errNone;
    }
  }

  StoCheckErr(err);
  return h;
}

UInt16 MemPtrFlags(MemPtr p) {
  // XXX always 0
  return 0;
}

UInt32 MemPtrSize(MemPtr p) {
  MemHandle h;
  UInt32 size = 0;

  if ((h = MemPtrRecoverHandle(p)) != NULL) {
    size = MemHandleSize(h);
  }

  return size;
}

UInt16 MemPtrOwner(MemPtr p) {
  storage_handle_t *h;
  UInt16 owner = 0;

  if (p) {
    if ((h = StoPtrRecoverHandle(p)) != NULL) {
      owner = h->owner;
    }
  }

  debug(DEBUG_TRACE, "STOR", "MemPtrOwner p=%p owner=%d", p, owner);

  return owner;
}

UInt16 MemPtrHeapID(MemPtr p) {
  return 0;
}

Boolean MemPtrDataStorage(MemPtr p) {
  return false;
}

UInt16 MemPtrCardNo(MemPtr p) {
  return 0;
}

LocalID MemPtrToLocalID(MemPtr p) {
  MemHandle h;
  LocalID id = 0;

  if ((h = MemPtrRecoverHandle(p)) != NULL) {
    id = MemHandleToLocalID(h);
  }

  return id;
}

Err MemPtrSetOwner(MemPtr p, UInt16 owner) {
  storage_handle_t *h;
  Err err = memErrInvalidParam;

  debug(DEBUG_TRACE, "STOR", "MemPtrSetOwner p=%p owner=%d", p, owner);

  if (p) {
    if ((h = StoPtrRecoverHandle(p)) != NULL) {
      h->owner = owner;
      err = errNone;
    }
  }

  return err;
}

static Err MemResize(MemHandle h, UInt32 newSize) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  Err err = memErrInvalidParam;

  debug(DEBUG_TRACE, "STOR", "MemResize handle=%p newSize=%u", h, newSize);
  handle = (storage_handle_t *)h;
  if (handle && newSize) {
    if (newSize != handle->size) {
      switch (handle->htype & ~STO_INFLATED) {
        case STO_TYPE_MEM:
          if (newSize < handle->size) {
            handle->size = newSize;
            err = errNone;
          } else {
            handle->buf = StoPtrRealloc(handle, handle->buf, newSize);
            handle->size = newSize;
            err = errNone;
          }
          break;
        case STO_TYPE_REC:
        case STO_TYPE_RES:
          if (handle->htype & STO_INFLATED) {
            handle->buf = StoPtrRealloc(handle, handle->buf, newSize);
            if ((handle->htype & ~STO_INFLATED) == STO_TYPE_REC) {
              handle->d.rec.attr |= dmRecAttrDirty;
            }
          }
          handle->size = newSize;
          err = errNone;
          break;
        default:
          debug(DEBUG_ERROR, "STOR", "MemResize %p unexpected handle type %d", handle, handle->htype & ~STO_INFLATED);
          break;
      }
    } else {
      err = errNone;
    }
  }

  StoCheckErr(err);
  return err;
}

Err MemPtrResize(MemPtr p, UInt32 newSize) {
  MemHandle h;
  storage_handle_t *handle;
  Err err = memErrInvalidParam;

  if ((h = MemPtrRecoverHandle(p)) != NULL) {
    if (h) {
      handle = h;
      if ((handle->htype & ~STO_INFLATED) == STO_TYPE_MEM && newSize > handle->size) {
        return memErrNotEnoughSpace;
      }
    }
    err = MemResize(h, newSize);
  }

  return err;
}

Err MemPtrResetLock(MemPtr p) {
  MemHandle h;
  Err err = memErrInvalidParam;

  if (p && (h = MemPtrRecoverHandle(p)) != NULL) {
    err = MemHandleResetLock(h);
  }

  return err;
}

Err MemPtrUnlock(MemPtr p) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  MemHandle h;
  Err err = memErrInvalidParam;

  if (p && (h = MemPtrRecoverHandle(p)) != NULL) {
    err = MemHandleUnlock(h);
  }

  StoCheckErr(err);
  return err;
}

UInt16 MemHandleFlags(MemHandle h) {
  // XXX always 0
  return 0;
}

UInt32 MemHandleSize(MemHandle h) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  UInt32 size = 0;
  Err err = memErrInvalidParam;

  if (h) {
    handle = (storage_handle_t *)h;
    size = handle->size;
    err = errNone;
  }

  StoCheckErr(err);
  return size;
}

UInt16 MemHandleOwner(MemHandle hh) {
  storage_handle_t *h;
  UInt16 owner = 0;

  if (hh) {
    h = (storage_handle_t *)hh;
    owner = h->owner;
  }

  debug(DEBUG_TRACE, "STOR", "MemHandleOwner handle=%p owner=%d", hh, owner);

  return owner;
}

UInt16 MemHandleLockCount(MemHandle h) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  UInt16 count = 0;
  Err err = memErrInvalidParam;

  if (h) {
    handle = (storage_handle_t *)h;
    count = handle->lockCount;
    err = errNone;
  }

  StoCheckErr(err);
  return count;
}

UInt16 MemHandleHeapID(MemHandle h) {
  return 0;
}

Boolean MemHandleDataStorage(MemHandle h) {
  return false;
}

UInt16 MemHandleCardNo(MemHandle h) {
  return 0;
}

LocalID MemHandleToLocalID(MemHandle h) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *handle;
  LocalID id = 0;
  Err err = memErrInvalidParam;

  if (h) {
    handle = (storage_handle_t *)h;
    switch (handle->htype & ~STO_INFLATED) {
      case STO_TYPE_MEM:
      case STO_TYPE_REC:
      case STO_TYPE_RES:
        id = (uint8_t *)handle - sto->base;
        err = errNone;
        break;
      default:
        debug(DEBUG_ERROR, "STOR", "MemHandleToLocalID %p unexpected handle type %d", handle, handle->htype & ~STO_INFLATED);
        break;
    }
  }

  StoCheckErr(err);
  return id;
}

// New owner ID of the chunk. Specify 0 to set the owner to the operating system.
// Once you have granted ownership of a memory chunk to the system, do not attempt to free it yourself.
// The operating system will free it when the application invoked with SysUIAppSwitch() or SysAppLaunch() quits.

Err MemHandleSetOwner(MemHandle hh,  UInt16 owner) {
  storage_handle_t *h;
  Err err = dmErrInvalidParam;

  debug(DEBUG_TRACE, "STOR", "MemHandleSetOwner handle=%p owner=%d", hh, owner);

  if (hh) {
    h = (storage_handle_t *)hh;
    h->owner = owner;
    err = errNone;
  }

  return err;
}

Err MemHandleResize(MemHandle h, UInt32 newSize) {
  return MemResize(h, newSize);
}

Err MemHandleResetLock(MemHandle h) {
  Err err = memErrInvalidParam;

  return err;
}

// XXX MemoPad application uses this code:
// return ((MemHandle) MemLocalIDToGlobal (appInfoID, cardNo));
// so it is getting a MemPtr and casting to MemHandle, which will obviously not work here.
// I am hoping that anyone that calls MemLocalIDToGlobal() assumes the returned value is a MemHandle.
MemPtr MemLocalIDToGlobal(LocalID local, UInt16 cardNo) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h = NULL;
  Err err = memErrInvalidParam;

  if (local > 0 && local < sto->size) {
    h = (storage_handle_t *)(sto->base + local);
    err = errNone;
  }

  StoCheckErr(err);
  return h;
}

// This routine determines if the given local ID is to a nonmovable (memIDPtr) or movable (memIDHandle) chunk.
// XXX this distinction makes no sense in PumpkinOS,
// but some apps depends on it an even crash if the value is not the expected (example: Bird)
LocalIDKind MemLocalIDKind(LocalID local) {
  return memIDHandle;
}

// If the local ID references a movable chunk and that chunk is not
// locked, this function returns 0 to indicate an error.
MemPtr MemLocalIDToPtr(LocalID local, UInt16 cardNo) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  uint8_t *p = NULL;
  Err err = memErrInvalidParam;

  if (local > 0 && local < sto->size) {
    h = (storage_handle_t *)(sto->base + local);
    if (h->htype == (STO_TYPE_MEM | STO_INFLATED) && h->lockCount > 0) {
      p = h->buf;
      err = errNone;
    } else if (h->htype == (STO_TYPE_RES | STO_INFLATED) && h->lockCount > 0) {
      p = h->buf;
      err = errNone;
    }
  }

  StoCheckErr(err);
  return p;
}

// If the local ID references a movable chunk handle, this routine
// automatically locks the chunk before returning.
MemPtr MemLocalIDToLockedPtr(LocalID local, UInt16 cardNo) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h;
  uint8_t *p = NULL;
  Err err = memErrInvalidParam;

  if (local < (sto->size - sizeof(storage_handle_t))) {
    h = (storage_handle_t *)(sto->base + local);
    if ((h->htype == (STO_TYPE_MEM | STO_INFLATED) || h->htype == (STO_TYPE_REC | STO_INFLATED)) && h->lockCount < 14) {
      h->lockCount++;
      p = h->buf;
      err = errNone;
    }
  }

  StoCheckErr(err);
  return p;
}

MemHandle MemLocalIDToHandle(LocalID local) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h = NULL;
  Err err = memErrInvalidParam;

  if (local > 0 && local < sto->size) {
    h = (storage_handle_t *)(sto->base + local);
    err = errNone;
  }

  StoCheckErr(err);
  return h;
}

Err MemSemaphoreReserve(Boolean writeAccess) {
  // system use only
  return dmErrInvalidParam;
}

Err MemSemaphoreRelease(Boolean writeAccess) {
  // system use only
  return dmErrInvalidParam;
}

UInt16 MemDebugMode(void) {
  // system use only
  return 0;
}

Err MemSetDebugMode(UInt16 flags) {
  // system use only
  return dmErrInvalidParam;
}

Err MemHeapScramble(UInt16 heapID) {
  return errNone;
}

Err MemHeapCheck(UInt16 heapID) {
  return errNone;
}

//#define CRDATE 1609108458

Err MemCardInfo(UInt16 cardNo, Char *cardNameP, Char *manufNameP, UInt16 *versionP, UInt32 *crDateP, UInt32 *romSizeP, UInt32 *ramSizeP, UInt32 *freeBytesP) {
  if (cardNameP) StrNCopy(cardNameP, "RAM", 32);
  if (manufNameP) StrNCopy(manufNameP, SYSTEM_NAME, 32);
  if (versionP) *versionP = sys_atoi(SYSTEM_VERSION);
  if (crDateP) *crDateP = pumpkin_dt() + CRDATE;
  if (romSizeP) *romSizeP = 8*1024*1024; // XXX
  if (ramSizeP) *ramSizeP = MemHeapSize(1);
  if (freeBytesP) *freeBytesP = MemHeapSize(1);

  return errNone;
}

Err MemCardFormat(UInt16 cardNo, const Char *cardNameP, const Char *manufNameP, const Char *ramStoreNameP) {
  debug(DEBUG_ERROR, "STOR", "MemCardFormat not implemented");
  return dmErrInvalidParam;
}

Int32 StoFileSeek(DmOpenRef dbP, UInt32 offset, Int32 whence) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;
  Int32 r = -1;

  if (dbP) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_FILE && db->f) {
          r = vfs_seek(db->f, offset, whence);
          if (r != -1) err = errNone;
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return r;
}

Int32 StoFileRead(DmOpenRef dbP, void *p, Int32 size) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;
  Int32 r = -1;

  if (dbP && p && size) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if (dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_FILE && db->f) {
          r = vfs_read(db->f, (uint8_t *)p, size);
          if (r != -1) err = errNone;
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return r;
}

Int32 StoFileWrite(DmOpenRef dbP, void *p, Int32 size) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  Err err = dmErrInvalidParam;
  Int32 r = -1;

  if (dbP && p && size) {
    if (mutex_lock(sto->mutex) == 0) {
      dbRef = (DmOpenType *)dbP;
      if ((dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
        db = (storage_db_t *)(sto->base + dbRef->dbID);
        if (db->ftype == STO_TYPE_FILE && db->f) {
          r = vfs_write(db->f, (uint8_t *)p, size);
          if (r != -1) err = errNone;
        }
      }
      mutex_unlock(sto->mutex);
    }
  }

  StoCheckErr(err);
  return r;
}

static int StoCompareHandle(const void *e1, const void *e2) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_handle_t *h1, *h2;
  SortRecordInfoType r1, r2;
  UInt8 *b1, *b2;
  Boolean free1, free2;
  vfs_file_t *f;
  char buf[VFS_PATH];
  UInt8 *b;
  UInt32 a;
  int r = 0;

  if (e1 && e2 && (sto->comparF || sto->comparF68K)) {
    h1 = *(storage_handle_t **)e1;
    h2 = *(storage_handle_t **)e2;

    b1 = h1->buf;
    free1 = false;
    if (b1 == NULL) {
      if ((b1 = pumpkin_heap_alloc(h1->size, "TmpHandleBuf")) != NULL) {
        storage_name(sto, sto->tmpDb->name, STO_FILE_ELEMENT, 0, 0, h1->d.rec.attr & ATTR_MASK, h1->d.rec.uniqueID, buf);
        if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
          vfs_read(f, b1, h1->size);
          vfs_close(f);
        }
        free1 = true;
      }
    }

    b2 = h2->buf;
    free2 = false;
    if (b2 == NULL) {
      if ((b2 = pumpkin_heap_alloc(h2->size, "TmpHandleBuf")) != NULL) {
        storage_name(sto, sto->tmpDb->name, STO_FILE_ELEMENT, 0, 0, h2->d.rec.attr & ATTR_MASK, h2->d.rec.uniqueID, buf);
        if ((f = StoVfsOpen(sto->session, buf, VFS_READ)) != NULL) {
          vfs_read(f, b2, h2->size);
          vfs_close(f);
        }
        free2 = true;
      }
    }

    if (sto->comparF) {
      r1.attributes = h1->d.rec.attr;
      r1.uniqueID[0] = (h1->d.rec.uniqueID >> 16) & 0xFF;
      r1.uniqueID[1] = (h1->d.rec.uniqueID >>  8) & 0xFF;
      r1.uniqueID[2] = (h1->d.rec.uniqueID >>  0) & 0xFF;

      r2.attributes = h2->d.rec.attr;
      r2.uniqueID[0] = (h2->d.rec.uniqueID >> 16) & 0xFF;
      r2.uniqueID[1] = (h2->d.rec.uniqueID >>  8) & 0xFF;
      r2.uniqueID[2] = (h2->d.rec.uniqueID >>  0) & 0xFF;

      r = sto->comparF(b1, b2, sto->other, &r1, &r2, sto->appInfoH);

    } else if (sto->comparF68K) {
      sto->recInfo[0] = h1->d.rec.attr;
      sto->recInfo[1] = (h1->d.rec.uniqueID >> 16) & 0xFF;
      sto->recInfo[2] = (h1->d.rec.uniqueID >>  8) & 0xFF;
      sto->recInfo[3] = (h1->d.rec.uniqueID >>  0) & 0xFF;

      sto->recInfo[4] = h2->d.rec.attr;
      sto->recInfo[5] = (h2->d.rec.uniqueID >> 16) & 0xFF;
      sto->recInfo[6] = (h2->d.rec.uniqueID >>  8) & 0xFF;
      sto->recInfo[7] = (h2->d.rec.uniqueID >>  0) & 0xFF;

      b = sto->base;
      a = sto->appInfoH ? (UInt8 *)sto->appInfoH - b : 0;
      r = CallDmCompare(sto->comparF68K, b1 ? b1 - b : 0, b2 ? b2 - b : 0, sto->other, &sto->recInfo[0] - b, &sto->recInfo[4] - b, a);
    }

    if (free1) pumpkin_heap_free(b1, "TmpHandleBuf");
    if (free2) pumpkin_heap_free(b2, "TmpHandleBuf");
  }

  return r;
}

static Err StoSort(DmOpenRef dbP, DmComparF *comparF, UInt32 comparF68K, Int16 other) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  DmOpenType *dbRef;
  UInt32 i;
  Boolean locked;
  Err err = dmErrInvalidParam;

  if (dbP && (comparF || comparF68K)) {
    dbRef = (DmOpenType *)dbP;
    if ((dbRef->mode & dmModeWrite) && dbRef->dbID < (sto->size - sizeof(storage_db_t))) {
      db = (storage_db_t *)(sto->base + dbRef->dbID);
      if (db->ftype == STO_TYPE_REC) {
        if (db->numRecs > 1) {
          for (i = 0, locked = false; i < db->numRecs && !locked; i++) {
            if (db->elements[i]->lockCount > 0) locked = true;
          }
          if (!locked) {
            sto->comparF = comparF;
            sto->comparF68K = comparF68K;
            sto->other = other;
            sto->appInfoH = db->appInfoID ? MemLocalIDToHandle(db->appInfoID) : NULL;
            sto->tmpDb = db;
            debug(DEBUG_INFO, "STOR", "StoSort sorting database \"%s\" with %d records (%s)", db->name, db->numRecs, comparF ? "native" : "68K");
            sys_qsort(db->elements, db->numRecs, sizeof(storage_handle_t *), StoCompareHandle);
            sto->tmpDb = NULL;
            sto->comparF = NULL;
            sto->comparF68K = 0;
            sto->other = 0;
            sto->appInfoH = NULL;
            StoWriteIndex(sto, db);
            err = errNone;
          } else {
            debug(DEBUG_ERROR, "STOR", "StoSort database \"%s\" at least one record locked", db->name);
          }
        } else {
          debug(DEBUG_INFO, "STOR", "StoSort database \"%s\" has %d record", db->name, db->numRecs);
          err = errNone;
        }
      }
    }
  }

  StoCheckErr(err);
  return err;
}

Err DmInsertionSort(DmOpenRef dbP, DmComparF *comparF, Int16 other) {
  return StoSort(dbP, comparF, 0, other);
}

Err DmInsertionSort68K(DmOpenRef dbP, UInt32 comparF, Int16 other) {
  return StoSort(dbP, NULL, comparF, other);
}

Err DmQuickSort(DmOpenRef dbP, DmComparF *comparF, Int16 other) {
  return StoSort(dbP, comparF, 0, other);
}

Err DmQuickSort68K(DmOpenRef dbP, UInt32 comparF, Int16 other) {
  return StoSort(dbP, NULL, comparF, other);
}

// Generate a list of databases found on the memory cards matching a
// specific type and return the result. If lookupName is true then a
// name in a tAIN resource is used instead of the database’s name and
// the list is sorted. Only the last version of a database is returned.
// Databases with multiple versions are listed only once.

/*
dbIDs: a pointer to a handle that gets allocated to contain the database list.
Upon return, this references an array of SysDBListItemType structures.

typedef struct {
  Char name[dmDBNameLength];
  UInt32 creator;
  UInt32 type;
  UInt16 version;
  LocalID dbID;
  UInt16 cardNo;
  BitmapPtr iconP;
} SysDBListItemType;
*/

static Boolean StoCreateDataBaseList(UInt32 type, UInt32 creator, UInt16 *dbCount, MemHandle *dbIDs, Boolean lookupName, Boolean m68k) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);
  storage_db_t *db;
  SysDBListItemType *list, *p;
  UInt16 size, count, sizeofSysDBListItemType;
  UInt32 offset;
  LocalID dbID;
  UInt8 *buf;
  char stype[8], screator[8];
  Boolean r;

  pumpkin_id2s(type, stype);
  pumpkin_id2s(creator, screator);
  debug(DEBUG_INFO, "STOR", "StoCreateDataBaseList type '%s' creator '%s' lookupName %d", type ? stype : "<any>", creator ? screator : "<any>", lookupName);

  sizeofSysDBListItemType = m68k ? (dmDBNameLength + 4*sizeof(UInt32) + 2*sizeof(UInt16)) : sizeof(SysDBListItemType);
  count = 0;
  size = 16;
  list = sys_calloc(size, sizeofSysDBListItemType);
  buf = (UInt8 *)list;
  offset = 0;

  for (db = sto->list; db; db = db->next) {
    if ((type != 0 && type != db->type) || (creator != 0 && creator != db->creator) || db->name[0] == 0) {
      continue;
    }
    if (count == size) {
      size += 16;
      list = sys_realloc(list, size * sizeofSysDBListItemType);
      buf = (UInt8 *)list;
    }
    dbID = (uint8_t *)db - sto->base;
    debug(DEBUG_INFO, "STOR", "found \"%s\" dbID 0x%08X", db->name, dbID);

    if (m68k) {
      StrNCopy((char *)&buf[offset], db->name, dmDBNameLength-1);
      offset += dmDBNameLength;
      offset += put4b(db->creator, buf, offset);
      offset += put4b(db->type, buf, offset);
      offset += put2b(db->version, buf, offset);
      offset += put4b(dbID, buf, offset);
      offset += put2b(0, buf, offset);
      offset += put4b(0, buf, offset);
    } else {
      // XXX lookupName is ignored: name is always taken from the database name, and the list is not sorted
      StrNCopy(list[count].name, db->name, dmDBNameLength-1);
      list[count].creator = db->creator;
      list[count].type = db->type;
      list[count].version = db->version;
      list[count].dbID = dbID;
      list[count].cardNo = 0;
      // XXX icon resource is not beeing returned
      list[count].iconP = NULL;
    }
    count++;
  }

  if (count) {
    if (dbIDs) {
      *dbIDs = MemHandleNew(count * sizeofSysDBListItemType);
      p = MemHandleLock(*dbIDs);
      MemMove(p, list, count * sizeofSysDBListItemType);
      MemHandleUnlock(*dbIDs);
    }
    r = true;
  } else {
    if (dbIDs) *dbIDs = NULL;
    r = false;
  }
  sys_free(list);
  if (dbCount) *dbCount = count;

  return r;
}

Boolean SysCreateDataBaseList(UInt32 type, UInt32 creator, UInt16 *dbCount, MemHandle *dbIDs, Boolean lookupName) {
  return StoCreateDataBaseList(type, creator, dbCount, dbIDs, lookupName, false);
}

Boolean SysCreateDataBaseList68K(UInt32 type, UInt32 creator, UInt16 *dbCount, MemHandle *dbIDs, Boolean lookupName) {
  return StoCreateDataBaseList(type, creator, dbCount, dbIDs, lookupName, true);
}

Err MemStoreInfo(UInt16 cardNo, UInt16 storeNumber, UInt16 *versionP, UInt16 *flagsP, Char *nameP, UInt32 *crDateP, UInt32 *bckUpDateP, UInt32 *heapListOffsetP, UInt32 *initCodeOffset1P, UInt32 *initCodeOffset2P, LocalID *databaseDirIDP) {
  debug(DEBUG_ERROR, "STOR", "MemStoreInfo not implemented");
  return dmErrInvalidParam;
}

Err MemStoreSetInfo(UInt16 cardNo, UInt16 storeNumber, UInt16 *versionP, UInt16 *flagsP, Char *nameP, UInt32 *crDateP, UInt32 *bckUpDateP, UInt32 *heapListOffsetP, UInt32 *initCodeOffset1P, UInt32 *initCodeOffset2P, LocalID *databaseDirIDP) {
  debug(DEBUG_ERROR, "STOR", "MemStoreSetInfo not implemented");
  return dmErrInvalidParam;
}

void FntSetAppearance(UInt8 family, UInt8 style, UInt16 size) {
  storage_t *sto = (storage_t *)pumpkin_get_local_storage(sto_key);

  if (sto) {
    sto->fontFamily = family;
    sto->fontStyle = style;
    sto->fontSize = size;
  }
}

void DmSync(void) {
}

Err DmSyncDatabase(DmOpenRef dbRef) {
  return errNone;
}
