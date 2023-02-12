#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "ptr.h"
#include "mutex.h"
#include "sys.h"
#include "debug.h"

#define MAX_PTRS 1024

#define OP_LOCK   1
#define OP_UNLOCK 2
#define OP_WAIT   3
#define OP_SIGNAL 4
#define OP_FREE   5

typedef struct {
  unsigned int id;
  int used, locking, delete;
  int64_t t;
  void (*destructor)(void *p);
  mutex_t *mutex;
  cond_t *cond;
  void *p;
} ptr_t;

typedef struct {
  char *tag;
} generic_t;

static ptr_t table[MAX_PTRS];
static unsigned int next_id;
static mutex_t *mutex;
static char *op_name[] = { "", "lock", "unlock", "wait", "signal", "free" };

int ptr_init(void) {
  int i;

  if ((mutex = mutex_create("ptr")) == NULL) {
    return -1;
  }

  for (i = 0; i < MAX_PTRS; i++) {
    table[i].id = 0;
    table[i].used = 0;
    table[i].locking = 0;
    table[i].delete = 0;
    table[i].destructor = NULL;
    table[i].mutex = NULL;
    table[i].p = NULL;
  }

  table[0].used = 1;
  next_id = 1;

  return 0;
}

int ptr_close(void) {
  //generic_t *gp;
  int i;

  for (i = 1; i < MAX_PTRS; i++) {
/*
    gp = (generic_t *)table[i].p;
    if (table[i].used) {
      debug(DEBUG_INFO, "PTR", "handle %d (%d) (%s) was not closed", table[i].id, i, gp->tag);
    }
*/
    table[i].used = 0;
    table[i].locking = 0;
    table[i].delete = 0;
    table[i].destructor = NULL;
    if (table[i].mutex) mutex_destroy(table[i].mutex);
    if (table[i].cond) cond_destroy(table[i].cond);
    table[i].mutex = NULL;
    table[i].cond = NULL;
    table[i].p = NULL;
  }

  mutex_destroy(mutex);

  return 0;
}

static int ptr_new_aux(void *p, void (*destructor)(void *p), int c) {
  generic_t *gp;
  char buf[16];
  int id, i, index;

  id = -1;

  if (mutex_lock(mutex) == 0) {
    for (i = 0; i < MAX_PTRS; i++) {
      id = next_id + i;
      index = id % MAX_PTRS;

      if (!table[index].used) {
        snprintf(buf, sizeof(buf)-1, "ptr%d", id);
        table[index].mutex = mutex_create(buf);

        if (table[index].mutex == NULL) {
          id = -1;
        } else {
          table[index].id = id;
          table[index].used = 1;
          table[index].locking = 0;
          table[index].delete = 0;
          table[index].destructor = destructor;
          table[index].p = p;
          if (c) table[index].cond = cond_create(buf);
          gp = (generic_t *)table[index].p;
          debug(DEBUG_TRACE, "PTR", "new handle %d (%d) (%s) (%08x)", id, index, gp->tag, p);
        }
        break;
      }
    }

    if (i == MAX_PTRS) {
      debug(DEBUG_ERROR, "PTR", "max pointers reached");
      id = -1;
    } else {
      next_id = id + 1;
    }

    mutex_unlock(mutex);
  }

  return id;
}

int ptr_new(void *p, void (*destructor)(void *p)) {
  return ptr_new_aux(p, destructor, 0);
}

int ptr_new_c(void *p, void (*destructor)(void *p)) {
  return ptr_new_aux(p, destructor, 1);
}

static void *ptr_access(const char *file, const char *func, int line, int id, char *tag, int op, uint32_t arg) {
  int index, delete, locking, ok;
  void (*destructor)(void *p);
  mutex_t *ptr_mutex;
  cond_t *ptr_cond;
  generic_t *p;
  int64_t t;

  index = id % MAX_PTRS;
  destructor = NULL;
  ptr_mutex = NULL;
  p = NULL;
  ok = 0;

  if (mutex_lock(mutex) == 0) {
    if (index > 0 && index < MAX_PTRS) {
      if (table[index].used) {
        if (!table[index].delete) {
          if (table[index].id == id) {
            p = (generic_t *)table[index].p;

            if (!strcmp(p->tag, tag)) {
              switch (op) {
                case OP_LOCK:
                  table[index].locking++;
                  ok = 1;
                  break;
                case OP_UNLOCK:
                  if (table[index].locking) {
                    table[index].locking--;
                    ok = 1;
                  }
                  break;
                case OP_WAIT:
                  if (table[index].locking) {
                    ok = 1;
                  }
                  break;
                case OP_SIGNAL:
                  if (table[index].locking) {
                    ok = 1;
                  }
                  break;
                case OP_FREE:
                  table[index].delete = 1;
                  ok = 1;
                  break;
              }

              delete = table[index].delete;
              locking = table[index].locking;
              ptr_mutex = table[index].mutex;
              ptr_cond = table[index].cond;
              destructor = table[index].destructor;

            } else {
              debug(DEBUG_ERROR, "PTR", "attempt to %s handle %d with tag %s != %s", op_name[op], id, p->tag, tag);
            }
          } else {
            debug(DEBUG_ERROR, "PTR", "attempt to %s wrong handle %d != %d (%d)", op_name[op], id, table[index].id, index);
          }
        } else {
          debug(DEBUG_ERROR, "PTR", "attempt to %s deleted handle %d (%d)", op_name[op], id, index);
        }
      } else {
        debug(DEBUG_ERROR, "PTR", "attempt to %s unused handle %d (%d)", op_name[op], id, index);
      }
    } else {
      debug(DEBUG_ERROR, "PTR", "attempt to %s invalid handle %d (%d)", op_name[op], id, index);
    }
    mutex_unlock(mutex);
  }

  if (ok) {
    switch (op) {
      case OP_LOCK:
        debug_full(file, func, line, DEBUG_TRACE, "PTR", "locking handle %d (%d) (%s) locking=%d", id, index, tag, locking);
        t = sys_get_clock();
        if (mutex_lock(ptr_mutex) != 0) {
          p = NULL;
        } else {
          debug_full(file, func, line, DEBUG_TRACE, "PTR", "locked handle %d (%d) (%s) locking=%d", id, index, tag, locking);
          t = sys_get_clock() - t;
          if (t >= 5000) {
            debug_full(file, func, line, DEBUG_INFO, "PTR", "lock handle %d (%d) (%s) wait %lld us", id, index, tag, t);
          }
        }
        break;
      case OP_UNLOCK:
        mutex_unlock(ptr_mutex);
        debug_full(file, func, line, DEBUG_TRACE, "PTR", "unlocked handle %d (%d) (%s) locking=%d", id, index, tag, locking);
        break;
      case OP_WAIT:
        debug_full(file, func, line, DEBUG_TRACE, "PTR", "waiting handle %d (%d) (%s) locking=%d us=%d", id, index, tag, locking, arg);
        if (cond_timedwait(ptr_cond, ptr_mutex, arg) != 0) {
          p = NULL;
        } else {
          debug_full(file, func, line, DEBUG_TRACE, "PTR", "waited handle %d (%d) (%s) locking=%d us=%d", id, index, tag, locking, arg);
        }
        break;
      case OP_SIGNAL:
        debug_full(file, func, line, DEBUG_TRACE, "PTR", "signaling handle %d (%d) (%s) locking=%d", id, index, tag, locking);
        if (cond_signal(ptr_cond) != 0) {
          p = NULL;
        } else {
          debug_full(file, func, line, DEBUG_TRACE, "PTR", "signaled handle %d (%d) (%s) locking=%d", id, index, tag, locking);
        }
        break;
    }

    if (delete && locking == 0) {
      if (mutex_lock(mutex) == 0) {
        debug(DEBUG_TRACE, "PTR", "free handle %d (%d) (%s)", id, index, tag);
        table[index].used = 0;
        table[index].locking = 0;
        table[index].delete = 0;
        table[index].destructor = NULL;
        if (table[index].mutex) mutex_destroy(table[index].mutex);
        if (table[index].cond) cond_destroy(table[index].cond);
        table[index].mutex = NULL;
        table[index].cond = NULL;
        table[index].p = NULL;
        mutex_unlock(mutex);
        if (destructor) destructor(p);
      } else {
        debug(DEBUG_ERROR, "PTR", "free handle %d (%d) (%s), lock failed", id, index, tag);
      }
    }
  } else {
    p = NULL;
  }

  return p;
}

void *ptr_lock_full(const char *file, const char *func, int line, int id, char *tag) {
  return ptr_access(file, func, line, id, tag, OP_LOCK, 0);
}

void ptr_unlock_full(const char *file, const char *func, int line, int id, char *tag) {
  ptr_access(file, func, line, id, tag, OP_UNLOCK, 0);
}

int ptr_wait_full(const char *file, const char *func, int line, int id, int us, char *tag) {
  return ptr_access(file, func, line, id, tag, OP_WAIT, us) ? 0 : -1;
}

int ptr_signal_full(const char *file, const char *func, int line, int id, char *tag) {
  return ptr_access(file, func, line, id, tag, OP_SIGNAL, 0) ? 0 : -1;
}

int ptr_free_full(const char *file, const char *func, int line, int id, char *tag) {
  return ptr_access(file, func, line, id, tag, OP_FREE, 0) ? 0 : -1;
}

int ptr_check_tag(char *received, char *expected) {
  int r = 0;

  if (received) {
    if (strcmp(received, expected) == 0) {
      r = 1;
    } else {
      debug(DEBUG_ERROR, "PTR", "expecting tag '%s' but received tag '%s'", expected, received);
    }
  } else {
    debug(DEBUG_ERROR, "PTR", "expecting tag '%s' but received null tag", expected);
  }

  return r;
}
