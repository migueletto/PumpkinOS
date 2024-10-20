#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "debug.h"

#define MAX_QUEUE 8

struct mutex_t {
  char name[16];
  int owner;
  int count;
  int waiting;
  int queue[MAX_QUEUE];
};

mutex_t *mutex_create(char *name) {
  mutex_t *m;

  if ((m = sys_calloc(1, sizeof(mutex_t))) != NULL) {
    sys_strncpy(m->name, name, sizeof(m->name)-1);
    m->owner = -1;
    debug(DEBUG_TRACE, "MUTEX", "created mutex %s (%p)", m->name, m);
  }

  return m;
}

int mutex_destroy(mutex_t *m) {
  int r = -1;

  if (m) {
    debug(DEBUG_TRACE, "MUTEX", "destroyed mutex %s (%p)", m->name, m);
    sys_free(m);
  }

  return r;
}

int mutex_lock_only(mutex_t *m) {
  int handle, i, r = -1;

  if (m) {
    handle = thread_get_handle();

    for (;;) {
      if (m->owner == -1) {
        m->owner = handle;
        m->count = 1;
        r = 0;
        break;
      }

      if (m->owner == handle) {
        m->count++;
        r = 0;
        break;
      }

      for (i = 0; i < m->waiting; i++) {
        if (m->queue[i] == handle) break;
      }

      if (i == m->waiting) {
        if (i == MAX_QUEUE) {
          break;
        }
        m->queue[m->waiting++] = handle;
      }

      thread_yield(1);
    }
  }

  return r;
}

int mutex_unlock_only(mutex_t *m) {
  int i, r = -1;

  if (m) {
    if (m->owner == thread_get_handle()) {
      m->count--;

      if (m->count == 0) {
        if (m->waiting) {
          thread_resume(m->queue[0]);
          for (i = 0; i < m->waiting-1; i++) {
            m->queue[i] = m->queue[i+1];
					}
          m->waiting--;
        }
        m->owner = -1;
      }
      r = 0;
    }
  }

  return r;
}

int mutex_lock(mutex_t *m) {
  int r = -1;

  if (m) {
    debug(DEBUG_TRACE, "MUTEX", "locking mutex %s (%p)", m->name, m);
    if ((r = mutex_lock_only(m)) == -1) {
      debug(DEBUG_ERROR, "MUTEX", "error locking mutex %s (%p)", m->name, m);
    }
  }

  return r;
}

int mutex_unlock(mutex_t *m) {
  int r = -1;

  if (m) {
    if ((r = mutex_unlock_only(m)) == 0) {
      debug(DEBUG_TRACE, "MUTEX", "unlocked mutex %s (%p)", m->name, m);
    } else {
      debug(DEBUG_ERROR, "MUTEX", "error unlocking mutex %s (%p)", m->name, m);
    }
  }

  return r;
}

cond_t *cond_create(char *name) {
  return NULL;
}

int cond_destroy(cond_t *c) {
  return -1;
}

int cond_signal(cond_t *c) {
  return -1;
}

int cond_broadcast(cond_t *c) {
  return -1;
}

int cond_wait(cond_t *c, mutex_t *m) {
  return -1;
}

int cond_timedwait(cond_t *c, mutex_t *m, int us) {
  return -1;
}

sema_t *semaphore_create_named(char *name, int count) {
  return NULL;
}

int semaphore_remove_named(char *name) {
  return -1;
}

sema_t *semaphore_create(int count) {
  return NULL;
}

int semaphore_destroy(sema_t *sem) {
  return -1;
}

int semaphore_post(sema_t *sem) {
  return -1;
}

int semaphore_wait(sema_t *sem, int block) {
  return -1;
}

int semaphore_timedwait(sema_t *sem, int us) {
  return -1;
}
