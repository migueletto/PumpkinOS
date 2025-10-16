#include <PalmOS.h>

#include "sys.h"
#include "AppRegistry.h"
#include "pumpkin.h"
#include "bytes.h"
#include "mutex.h"
#include "xalloc.h"
#include "debug.h"

typedef struct {
  DmResType creator;
  AppRegistryID id;
  UInt16 seq;
  UInt16 size;
  void *data;
  Boolean deleted;
} AppRegistryEntry;

struct AppRegistryType {
  mutex_t *mutex;
  char *regname;
  AppRegistryEntry *registry;
  UInt32 num, size;
  Boolean ordered;
};

AppRegistryType *AppRegistryInit(char *regname) {
  AppRegistryType *ar;
  sys_dir_t *dir;
  char path[64], name[32], screator[8];
  uint8_t *buf;
  uint32_t creator;
  int64_t size;
  int i, id, seq, aux, fd;

  if ((ar = xcalloc(1, (sizeof(AppRegistryType)))) != NULL) {
    ar->mutex = mutex_create("registry");
    ar->regname = xstrdup(regname);
    ar->size = 1024;
    ar->registry = xcalloc(ar->size, sizeof(AppRegistryEntry));

    if ((dir = sys_opendir(regname)) == NULL) {
      sys_mkdir(regname);
      dir = sys_opendir(regname);
    }
    i = 0;

    if (dir) {
      for (;;) {
        if (i == ar->size) {
          ar->size += 1024;
          ar->registry = xrealloc(ar->registry, ar->size * sizeof(AppRegistryEntry));
        }
        if (sys_readdir(dir, name, sizeof(name) - 1) == -1) break;
        screator[4] = 0;
        if (sys_sscanf(name, "%c%c%c%c.%08X.%d.%d", screator, screator + 1, screator + 2,
                       screator + 3, &aux, &id, &seq) != 7)
          continue;
        if (id < appRegistryCompat || id >= appRegistryLast) continue;

        sys_snprintf(path, sizeof(path) - 1, "%s%s", regname, name);
        if ((fd = sys_open(path, SYS_READ)) == -1) continue;

        if ((size = sys_seek(fd, 0, SYS_SEEK_END)) != -1) {
          if (sys_seek(fd, 0, SYS_SEEK_SET) != -1) {
            if ((buf = xcalloc(1, size)) != NULL) {
              if (sys_read(fd, buf, size) == size) {
                debug(DEBUG_TRACE, "AppReg", "reading registry creator '%s' id %d seq %d", screator,
                      id, seq);
                pumpkin_s2id(&creator, screator);
                ar->registry[i].creator = creator;
                ar->registry[i].id = id;
                ar->registry[i].seq = seq;
                ar->registry[i].size = size;
                ar->registry[i].data = buf;
                i++;
              } else {
                xfree(buf);
              }
            }
          }
        }
        sys_close(fd);
      }

      sys_closedir(dir);
    }
    ar->num = i;
  }

  return ar;
}

static int AppRegistrySave(AppRegistryType *ar, int i) {
  char path[64], screator[8];
  int fd, r = -1;

  pumpkin_id2s(ar->registry[i].creator, screator);
  sys_snprintf(path, sizeof(path)-1, "%s%4s.%08X.%d.%d", ar->regname, screator, ar->registry[i].creator, ar->registry[i].id, ar->registry[i].seq);

  if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) != -1) {
    debug(DEBUG_TRACE, "AppReg", "saving registry creator '%s' id %d seq %d size %d", screator, ar->registry[i].id, ar->registry[i].seq, ar->registry[i].size);
    if (ar->registry[i].data && ar->registry[i].size) sys_write(fd, (uint8_t *)ar->registry[i].data, ar->registry[i].size);
    sys_close(fd);
    r = 0;
  }

  return r;
}

void AppRegistryFinish(AppRegistryType *ar) {
  int i;

  if (ar) {
    if (ar->registry) {
      for (i = 0; i < ar->num; i++) {
        if (!ar->registry[i].deleted && ar->registry[i].id != appRegistrySavedPref && ar->registry[i].id != appRegistryUnsavedPref) {
          AppRegistrySave(ar, i);
        }
        if (ar->registry[i].data) xfree(ar->registry[i].data);
      }
      xfree(ar->registry);
    }
    if (ar->regname) xfree(ar->regname);
    mutex_destroy(ar->mutex);
    xfree(ar);
  }
}

void AppRegistryDelete(char *regname, UInt16 id) {
  sys_dir_t *dir;
  char path[64], name[32], screator[8];
  int id1, seq, aux;

  if ((dir = sys_opendir(regname)) != NULL) {
    for (;;) {
      if (sys_readdir(dir, name, sizeof(name) - 1) == -1) break;
      screator[4] = 0;
      if (sys_sscanf(name, "%c%c%c%c.%08X.%d.%d", screator, screator + 1, screator + 2,
                     screator + 3, &aux, &id1, &seq) != 7)
        continue;
      if (id1 != id) continue;

      debug(DEBUG_INFO, "AppReg", "deleting registry creator '%s' id %d seq %d", screator, id, seq);
      sys_snprintf(path, sizeof(path) - 1, "%s%s", regname, name);
      sys_unlink(path);
    }
    sys_closedir(dir);
  }
}

int AppRegistryDeleteByCreator(AppRegistryType *ar, UInt32 creator) {
  sys_dir_t *dir;
  UInt32 rcreator;
  char path[64], name[32], screator[8];
  int i, id, seq, aux, r = -1;

  if (ar && mutex_lock(ar->mutex) == 0) {
    if ((dir = sys_opendir(ar->regname)) != NULL) {
      for (;;) {
        if (sys_readdir(dir, name, sizeof(name) - 1) == -1) break;
        screator[4] = 0;
        if (sys_sscanf(name, "%c%c%c%c.%08X.%d.%d", screator, screator + 1, screator + 2,
                       screator + 3, &aux, &id, &seq) != 7)
          continue;
        pumpkin_s2id(&rcreator, screator);
        if (rcreator != creator) continue;
  
        debug(DEBUG_INFO, "AppReg", "deleting registry creator '%s' id %d seq %d", screator, id, seq);
        sys_snprintf(path, sizeof(path) - 1, "%s%s", ar->regname, name);
        sys_unlink(path);
      }
      sys_closedir(dir);
      r = 0;
    }

    for (i = 0; i < ar->num; i++) {
      if (ar->registry[i].creator == creator) {
        ar->registry[i].deleted = true;
      }
    }

    mutex_unlock(ar->mutex);
  }

  return r;
}

static UInt16 AppRegistryProcess(AppRegistryType *ar, UInt32 creator, UInt16 id, UInt16 seq, UInt16 (*callback)(AppRegistryEntry *e, void *d, UInt16 size, Boolean set), void *d, UInt16 size, Boolean set) {
  char screator[8];
  int i;
  UInt16 r = 0;

  if (ar && mutex_lock(ar->mutex) == 0) {
    for (i = 0; i < ar->num; i++) {
      if (ar->registry[i].creator == creator && ar->registry[i].id == id && ar->registry[i].seq == seq) {
        break;
      }
    }

    if (set) {
      if (i == ar->num) {
        ar->num++;
        if (ar->num == ar->size) {
          ar->size += 1024;
          ar->registry = xrealloc(ar->registry, ar->size * sizeof(AppRegistryEntry));
        }
        ar->registry[i].creator = creator;
        ar->registry[i].id = id;
        ar->registry[i].seq = seq;
        if (id != appRegistryNotification) {
          ar->registry[i].size = size;
          ar->registry[i].data = size ? xcalloc(1, size) : NULL;
        }
        ar->registry[i].deleted = false;
        ar->ordered = false;
        pumpkin_id2s(creator, screator);
        debug(DEBUG_INFO, "AppReg", "creating registry creator '%s' id %d seq %d", screator, id, seq);
      }

      r = callback(&ar->registry[i], d, size, true);
      AppRegistrySave(ar, i);

    } else {
      if (i < ar->num) {
        r = callback(&ar->registry[i], d, size, false);
      }
    }
    mutex_unlock(ar->mutex);
  }

  return r;
}

static UInt16 AppRegistrySizeCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistrySize *s1 = (AppRegistrySize *)e->data;
  AppRegistrySize *s2 = (AppRegistrySize *)d;
  char st[8];

  if (set) {
    pumpkin_id2s(e->creator, st);
    debug(DEBUG_INFO, "AppReg", "updating size %dx%d for '%s'", s2->width, s2->height, st);
    s1->width = s2->width;
    s1->height = s2->height;
  } else {
    s2->width = s1->width;
    s2->height = s1->height;
  }

  return sizeof(AppRegistrySize);
}

static UInt16 AppRegistryPositionCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistryPosition *p1 = (AppRegistryPosition *)e->data;
  AppRegistryPosition *p2 = (AppRegistryPosition *)d;
  char st[8];

  if (set) {
    pumpkin_id2s(e->creator, st);
    debug(DEBUG_INFO, "AppReg", "updating position %d,%d for '%s'", p2->x, p2->y, st);
    p1->x = p2->x;
    p1->y = p2->y;
  } else {
    p2->x = p1->x;
    p2->y = p1->y;
  }

  return sizeof(AppRegistryPosition);
}

static UInt16 AppRegistryOSVersionCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistryOSVersion *p1 = (AppRegistryOSVersion *)e->data;
  AppRegistryOSVersion *p2 = (AppRegistryOSVersion *)d;
  char st[8];

  if (set) {
    pumpkin_id2s(e->creator, st);
    debug(DEBUG_INFO, "AppReg", "updating os version %d for '%s'", p2->osversion, st);
    p1->osversion = p2->osversion;
  } else {
    p2->osversion = p1->osversion;
  }

  return sizeof(AppRegistryOSVersion);
}

static UInt16 AppRegistryDepthCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistryDepth *p1 = (AppRegistryDepth *)e->data;
  AppRegistryDepth *p2 = (AppRegistryDepth *)d;
  char st[8];

  if (set) {
    pumpkin_id2s(e->creator, st);
    debug(DEBUG_INFO, "AppReg", "updating depth %d for '%s'", p2->depth, st);
    p1->depth = p2->depth;
  } else {
    p2->depth = p1->depth;
  }

  return sizeof(AppRegistryDepth);
}

static UInt16 AppRegistryFlagsCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistryFlags *p1 = (AppRegistryFlags *)e->data;
  AppRegistryFlags *p2 = (AppRegistryFlags *)d;
  char st[8];

  if (set) {
    pumpkin_id2s(e->creator, st);
    debug(DEBUG_INFO, "AppReg", "updating flags 0x%08X for '%s'", p2->flags, st);
    p1->flags = p2->flags;
  } else {
    p2->flags = p1->flags;
  }

  return sizeof(AppRegistryFlags);
}

static UInt16 AppRegistryCompatCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistryCompat *c1 = (AppRegistryCompat *)e->data;
  AppRegistryCompat *c2 = (AppRegistryCompat *)d;
  char st[8];

  if (set) {
    if (c2->compat == appCompatUnknown || c2->compat >= c1->compat) {
      pumpkin_id2s(e->creator, st);
      debug(DEBUG_INFO, "AppReg", "updating compatibility %d (%d) for '%s'", c2->compat, c2->code, st);
      c1->compat = c2->compat;
      c1->code = c2->code;
    }
  } else {
    c2->compat = c1->compat;
    c2->code = c1->code;
  }

  return sizeof(AppRegistryCompat);
}

static UInt16 AppRegistryNotificationCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  AppRegistryNotification *n1 = (AppRegistryNotification *)e->data;
  AppRegistryNotification *n2 = (AppRegistryNotification *)d;
  Boolean found;
  char screator[8], stype[8];
  int i, n;

  n = e->size / sizeof(AppRegistryNotification);
  pumpkin_id2s(n2->appCreator, screator);
  pumpkin_id2s(n2->notifyType, stype);

  if (set) {
    found = false;
    for (i = 0; i < n; i++) {
      if (n1[i].appCreator == n2->appCreator && n1[i].notifyType == n2->notifyType) {
        found = true;
        if (n2->priority > 0xFF) {
          // delete
          debug(DEBUG_INFO, "AppReg", "deleting notification %d: '%s' '%s' %d", i, screator, stype, n1[i].priority);
          if (n == 1) {
            // delete last one
            debug(DEBUG_INFO, "AppReg", "no more notifications");
            xfree(e->data);
            e->size = 0;
            e->data = NULL;
          } else {
            if (i < n-1) {
              // shift remaining entries to the left
              debug(DEBUG_INFO, "AppReg", "shifting notifications");
              MemMove(&n1[i], &n1[i+1], (n-i-1) * sizeof(AppRegistryNotification));
            } else {
              debug(DEBUG_INFO, "AppReg", "last notification");
            }
            n--;
            e->size = n * sizeof(AppRegistryNotification);
            e->data = xrealloc(e->data, e->size);
          }
        } else {
          // update
          debug(DEBUG_INFO, "AppReg", "updating notification %d: '%s' '%s' %d to %d", i, screator, stype, n1[i].priority, n2->priority);
          n1[i].priority = n2->priority;
        }
        break;
      }
    }
    if (!found && i == n) {
      // add
      debug(DEBUG_INFO, "AppReg", "inserting notification %d: '%s' '%s' %d", n, screator, stype, n2->priority);
      n++;
      e->size = n * sizeof(AppRegistryNotification);
      e->data = (n == 1) ? xcalloc(1, e->size) : xrealloc(e->data, e->size);
      n1 = (AppRegistryNotification *)e->data;
      n1[n-1].appCreator = n2->appCreator;
      n1[n-1].notifyType = n2->notifyType;
      n1[n-1].priority = n2->priority;
    }
  }

  return sizeof(AppRegistryNotification);
}

static UInt16 AppRegistryPreferenceCallback(AppRegistryEntry *e, void *d, UInt16 size, Boolean set) {
  char st[8];

  if (set) {
    pumpkin_id2s(e->creator, st);
    debug(DEBUG_INFO, "AppReg", "updating preference %d for '%s'", e->seq, st);
    MemMove(e->data, d, e->size);
  } else {
    if (d) {
      if (size == 0) size = e->size;
      MemMove(d, e->data, size <= e->size ? size : e->size);
    }
  }

  return e->size;
}

void AppRegistrySet(AppRegistryType *ar, UInt32 creator, AppRegistryID id, UInt16 seq, void *p) {
  switch (id) {
    case appRegistryCompat:
      AppRegistryProcess(ar, creator, id, seq, AppRegistryCompatCallback, p, sizeof(AppRegistryCompat), true);
      break;
    case appRegistrySize:
      AppRegistryProcess(ar, creator, id, seq, AppRegistrySizeCallback, p, sizeof(AppRegistrySize), true);
      break;
    case appRegistryPosition:
      AppRegistryProcess(ar, creator, id, seq, AppRegistryPositionCallback, p, sizeof(AppRegistryPosition), true);
      break;
    case appRegistryNotification:
      AppRegistryProcess(ar, creator, id, seq, AppRegistryNotificationCallback, p, sizeof(AppRegistryNotification), true);
      break;
    case appRegistryOSVersion:
      AppRegistryProcess(ar, creator, id, seq, AppRegistryOSVersionCallback, p, sizeof(AppRegistryOSVersion), true);
      break;
    case appRegistryDepth:
      AppRegistryProcess(ar, creator, id, seq, AppRegistryDepthCallback, p, sizeof(AppRegistryDepth), true);
      break;
    case appRegistryFlags:
      AppRegistryProcess(ar, creator, id, seq, AppRegistryFlagsCallback, p, sizeof(AppRegistryFlags), true);
      break;
    default:
      break;
  }
}

void AppRegistrySetPreference(AppRegistryType *ar, UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved) {
  AppRegistryProcess(ar, creator, saved ? appRegistrySavedPref : appRegistryUnsavedPref, seq, AppRegistryPreferenceCallback, p, size, true);
}

Boolean AppRegistryGet(AppRegistryType *ar, UInt32 creator, AppRegistryID id, UInt16 seq, void *p) {
  UInt16 r = 0;

  switch (id) {
    case appRegistryCompat:
      r = AppRegistryProcess(ar, creator, id, seq, AppRegistryCompatCallback, p, sizeof(AppRegistryCompat), false);
      break;
    case appRegistrySize:
      r = AppRegistryProcess(ar, creator, id, seq, AppRegistrySizeCallback, p, sizeof(AppRegistrySize), false);
      break;
    case appRegistryPosition:
      r = AppRegistryProcess(ar, creator, id, seq, AppRegistryPositionCallback, p, sizeof(AppRegistryPosition), false);
      break;
    case appRegistryOSVersion:
      r = AppRegistryProcess(ar, creator, id, seq, AppRegistryOSVersionCallback, p, sizeof(AppRegistryOSVersion), false);
      break;
    case appRegistryDepth:
      r = AppRegistryProcess(ar, creator, id, seq, AppRegistryDepthCallback, p, sizeof(AppRegistryDepth), false);
      break;
    case appRegistryFlags:
      r = AppRegistryProcess(ar, creator, id, seq, AppRegistryFlagsCallback, p, sizeof(AppRegistryFlags), false);
      break;
    default:
      break;
  }

  return r > 0;
}

UInt16 AppRegistryGetPreference(AppRegistryType *ar, UInt32 creator, UInt16 seq, void *p, UInt16 size, Boolean saved) {
  return AppRegistryProcess(ar, creator, saved ? appRegistrySavedPref : appRegistryUnsavedPref, seq, AppRegistryPreferenceCallback, p, size, false);
}

static Int32 compare_entry(void *e1, void *e2, void *otherP) {
  AppRegistryEntry *r1, *r2;
  Int32 r;

  r1 = (AppRegistryEntry *)e1;
  r2 = (AppRegistryEntry *)e2;

  if (r1->creator < r2->creator) {
    r = -1;
  } else if (r1->creator > r2->creator) {
    r = 1;
  } else {
    r = (int)r1->id - (int)r2->id;
  }

  return r;
}

void AppRegistryEnum(AppRegistryType *ar, void (*callback)(UInt32 creator, UInt16 seq, UInt16 index, UInt16 id, void *p, UInt16 size, void *data), UInt32 creator, AppRegistryID id, void *data) {
  AppRegistryNotification *n;
  int i, j, num, index;

  if (ar && callback && mutex_lock(ar->mutex) == 0) {
    if (!ar->ordered) {
      SysQSortP(ar->registry, ar->num, sizeof(AppRegistryEntry), compare_entry, NULL);
      ar->ordered = true;
    }

    for (i = 0, index = 0; i < ar->num; i++) {
      if (i > 0 && ar->registry[i].creator != ar->registry[i-1].creator) index++;

      if (creator && ar->registry[i].creator != creator) continue;
      if (id && ar->registry[i].id != id) continue;

      switch (ar->registry[i].id) {
        case appRegistryCompat:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistryCompat, ar->registry[i].data, 0, data);
          break;
        case appRegistrySize:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistrySize, ar->registry[i].data, 0, data);
          break;
        case appRegistryPosition:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistryPosition, ar->registry[i].data, 0, data);
          break;
        case appRegistryNotification:
          num = ar->registry[i].size / sizeof(AppRegistryNotification);
          n = (AppRegistryNotification *)ar->registry[i].data;
          for (j = 0; j < num; j++) {
            callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistryNotification, &n[j], 0, data);
          }
          break;
        case appRegistrySavedPref:
        case appRegistryUnsavedPref:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, ar->registry[i].id, ar->registry[i].data, ar->registry[i].size, data);
          break;
        case appRegistryOSVersion:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistryOSVersion, ar->registry[i].data, 0, data);
          break;
        case appRegistryDepth:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistryDepth, ar->registry[i].data, 0, data);
          break;
        case appRegistryFlags:
          callback(ar->registry[i].creator, ar->registry[i].seq, index, appRegistryFlags, ar->registry[i].data, 0, data);
          break;
        default:
          break;
      }
    }
    mutex_unlock(ar->mutex);
  }
}
