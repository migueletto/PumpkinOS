#include <PalmOS.h>
#include <time.h>
#include <sys/time.h>

#include "AppRegistry.h"
#include "pumpkin.h"
#include "sys.h"
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

    for (i = 0;;) {
      if (i == ar->size) {
        ar->size += 1024;
        ar->registry = xrealloc(ar->registry, ar->size * sizeof(AppRegistryEntry));
      }
      if (sys_readdir(dir, name, sizeof(name)-1) == -1) break;
      screator[4] = 0;
      if (sscanf(name, "%c%c%c%c.%08X.%d.%d", screator, screator+1, screator+2, screator+3, &aux, &id, &seq) != 7) continue;
      if (id < appRegistryCompat || id >= appRegistryLast) continue;

      snprintf(path, sizeof(path)-1, "%s%s", regname, name);
      if ((fd = sys_open(path, SYS_READ)) == -1) continue;

      if ((size = sys_seek(fd, 0, SYS_SEEK_END)) != -1) {
        if (sys_seek(fd, 0, SYS_SEEK_SET) != -1) {
          if ((buf = xcalloc(1, size)) != NULL) {
            if (sys_read(fd, buf, size) == size) {
              debug(DEBUG_INFO, "AppReg", "reading registry creator '%s' id %d seq %d", screator, id, seq);
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
    ar->num = i;
  }

  return ar;
}

static int AppRegistrySave(AppRegistryType *ar, int i) {
  char path[64], screator[8];
  int fd, r = -1;

  pumpkin_id2s(ar->registry[i].creator, screator);
  snprintf(path, sizeof(path)-1, "%s%4s.%08X.%d.%d", ar->regname, screator, ar->registry[i].creator, ar->registry[i].id, ar->registry[i].seq);

  if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) != -1) {
    debug(DEBUG_INFO, "AppReg", "saving registry creator '%s' id %d seq %d", screator, ar->registry[i].id, ar->registry[i].seq);
    sys_write(fd, (uint8_t *)ar->registry[i].data, ar->registry[i].size);
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
        AppRegistrySave(ar, i);
        xfree(ar->registry[i].data);
      }
      xfree(ar->registry);
    }
    mutex_destroy(ar->mutex);
    xfree(ar);
  }
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
        ar->registry[i].size = size;
        ar->registry[i].data = size ? xcalloc(1, size) : NULL;
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
  char screator[8], stype[8];
  int i, n;

  n = e->size / sizeof(AppRegistryNotification);
  pumpkin_id2s(n2->appCreator, screator);
  pumpkin_id2s(n2->notifyType, stype);

  if (set) {
    for (i = 0; i < n; i++) {
      if (n1[i].appCreator == n2->appCreator && n1[i].notifyType == n2->notifyType) {
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
    if (i == n) {
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
      AppRegistryProcess(ar, creator, id, seq, AppRegistryNotificationCallback, p, 0, true);
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

void AppRegistryEnum(AppRegistryType *ar, void (*callback)(UInt32 creator, UInt16 index, UInt16 id, void *p, void *data), UInt32 creator, AppRegistryID id, void *data) {
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
          callback(ar->registry[i].creator, index, appRegistryCompat, ar->registry[i].data, data);
          break;
        case appRegistrySize:
          callback(ar->registry[i].creator, index, appRegistrySize, ar->registry[i].data, data);
          break;
        case appRegistryPosition:
          callback(ar->registry[i].creator, index, appRegistryPosition, ar->registry[i].data, data);
          break;
        case appRegistryNotification:
          num = ar->registry[i].size / sizeof(AppRegistryNotification);
          n = (AppRegistryNotification *)ar->registry[i].data;
          for (j = 0; j < num; j++) {
            callback(ar->registry[i].creator, index, appRegistryNotification, &n[j], data);
          }
          break;
        default:
          break;
      }
    }
    mutex_unlock(ar->mutex);
  }
}
