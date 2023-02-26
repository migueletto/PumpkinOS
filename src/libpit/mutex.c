#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "mutex.h"
#include "sys.h"
#include "debug.h"
#include "xalloc.h"

struct mutex_t {
  pthread_mutex_t mutex;
  char name[16];
  int64_t t;
  int count;
};

struct cond_t {
  pthread_cond_t cond;
  char name[16];
};

struct sema_t {
  sem_t sem;
  sem_t *s;
  int named;
};

mutex_t *mutex_create(char *name) {
  mutex_t *m;
  pthread_mutexattr_t attr;

  if ((m = xcalloc(1, sizeof(mutex_t))) != NULL) {
    strncpy(m->name, name, sizeof(m->name)-1);

    if (pthread_mutexattr_init(&attr) == 0) {
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

      if (pthread_mutex_init(&m->mutex, &attr) != 0) {
        debug_errno("MUTEX", "pthread_mutex_init");
        xfree(m);
        m = NULL;
      }

      pthread_mutexattr_destroy(&attr);

    } else {
      debug_errno("MUTEX", "pthread_mutexattr_init");
      xfree(m);
      m = NULL;
    }
  }

  if (m) {
    debug(DEBUG_TRACE, "MUTEX", "created mutex %s (%08x)", m->name, m);
  }

  return m;
}

int mutex_destroy(mutex_t *m) {
  int r = -1;

  if (m) {
    r = pthread_mutex_destroy(&m->mutex);
    if (r != 0) {
      debug_errno("MUTEX", "pthread_mutex_destroy (%s)", m->name);
    }
    debug(DEBUG_TRACE, "MUTEX", "destroyed mutex %s (%08x)", m->name, m);
    xfree(m);
  }

  return r;
}

int mutex_lock(mutex_t *m) {
  int r = -1;

  if (m) {
    debug(DEBUG_TRACE, "MUTEX", "locking mutex %s (%08x)", m->name, m);
    r = pthread_mutex_lock(&m->mutex);
    if (r != 0) {
      debug_errno("MUTEX", "pthread_mutex_lock");
    } else {
      if (m->count == 0) {
        m->t = sys_get_clock();
      }
      m->count++;
     debug(DEBUG_TRACE, "MUTEX", "locked mutex %s (%08x) count %d", m->name, m, m->count);
    }
  }

  return r;
}

int mutex_unlock(mutex_t *m) {
  int64_t dt;
  int r = -1;

  if (m) {
    debug(DEBUG_TRACE, "MUTEX", "unlocking mutex %s (%08x) count %d", m->name, m, m->count);
    m->count--;
    if (m->count == 0) {
      dt = sys_get_clock() - m->t;
      if (dt >= 300000) {
        debug(DEBUG_INFO, "MUTEX", "mutex %s (%08x) locked for %lld us", m->name, m, dt);
      }
    }
    r = pthread_mutex_unlock(&m->mutex);
    debug(DEBUG_TRACE, "MUTEX", "unlocked mutex %s (%08x)", m->name, m);
    if (r != 0) {
      debug_errno("MUTEX", "pthread_mutex_unlock");
    }
  }

  return r;
}

cond_t *cond_create(char *name) {
  cond_t *c;
  int r;

  if ((c = xcalloc(1, sizeof(cond_t))) != NULL) {
    strncpy(c->name, name, sizeof(c->name)-1);

    r = pthread_cond_init(&c->cond, NULL);
    if (r != 0) {
      errno = r;
      debug_errno("MUTEX", "pthread_cond_init");
      xfree(c);
      c = NULL;
    }
  }

  if (c) {
    debug(DEBUG_TRACE, "MUTEX", "created cond %s (%08x)", c->name, c);
  }

  return c;
}

int cond_destroy(cond_t *c) {
  int r = -1;

  if (c) {
    r = pthread_cond_destroy(&c->cond);
    if (r != 0) {
      errno = r;
      debug_errno("MUTEX", "pthread_cond_destroy");
      r = -1;
    }
    debug(DEBUG_TRACE, "MUTEX", "destroyed cond %s (%08x)", c->name, c);
    xfree(c);
  }

  return r;
}

int cond_signal(cond_t *c) {
  int r = -1;

  if (c) {
    r = pthread_cond_signal(&c->cond);
    if (r != 0) {
      errno = r;
      debug_errno("MUTEX", "pthread_cond_signal \"%s\"", c->name);
      r = -1;
    }
  }

  return r;
}

int cond_broadcast(cond_t *c) {
  int r = -1;

  if (c) {
    r = pthread_cond_broadcast(&c->cond);
    if (r != 0) {
      errno = r;
      debug_errno("MUTEX", "pthread_cond_broadcast \"%s\"", c->name);
      r = -1;
    }
  }

  return r;
}

int cond_wait(cond_t *c, mutex_t *m) {
  int r = -1;

  if (c && m) {
    r = pthread_cond_wait(&c->cond, &m->mutex);
    if (r != 0) {
      errno = r;
      debug_errno("MUTEX", "pthread_cond_wait \"%s\"", c->name);
      r = -1;
    }
  }

  return r;
}

int cond_timedwait(cond_t *c, mutex_t *m, int us) {
  sys_timespec_t ts;
  struct timespec t;
  int r = -1;

  if (c && m) {
    sys_get_clock_ts(&ts);
    ts.tv_nsec += us*1000;
    if (ts.tv_nsec > 1000000000) {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
    }

    t.tv_sec = ts.tv_sec;
    t.tv_nsec = ts.tv_nsec;
    r = pthread_cond_timedwait(&c->cond, &m->mutex, &t);
    if (r != 0) {
      if (r != ETIMEDOUT) {
        errno = r;
        debug_errno("MUTEX", "pthread_cond_timedwait \"%s\"", c->name);
      }
      r = -1;
    }
  }

  return r;
}

#ifndef O_CREAT 
#define O_CREAT 0
#endif
#ifndef O_EXCL 
#define O_EXCL 0
#endif
#ifndef S_IWUSR 
#define S_IWUSR 0
#endif
#ifndef S_IRUSR
#define S_IRUSR 0
#endif

sema_t *semaphore_create_named(char *name, int count) {
  sema_t *sem;

  if ((sem = xcalloc(1, sizeof(sema_t))) != NULL) {
    if ((sem->s = sem_open(name, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, count)) == (sem_t *)SEM_FAILED) {
      debug_errno("MUTEX", "sem_open \"%s\"", name);
      xfree(sem);
      sem = NULL;
    } else {
      debug(DEBUG_TRACE, "MUTEX", "created named semaphore \"%s\" (%08x)", name, sem);
      sem->named = 1;
    }
  }

  return sem;
}

sema_t *semaphore_create(int count) {
  uint32_t n1, n2, n3;
  uint64_t t;
  sema_t *sem;
  char name[64];

  name[0] = 0;

  if ((sem = xcalloc(1, sizeof(sema_t))) != NULL) {
    if (sem_init(&sem->sem, 0, count) == 0) {
      sem->s = &sem->sem;
      debug(DEBUG_TRACE, "MUTEX", "created semaphore (%08x)", sem);
    } else {
      if (errno == ENOSYS) {
        n1 = sys_get_tid();
        t = sys_get_clock();
        n2 = t >> 32;
        n3 = t & 0xFFFFFFFF;
        snprintf(name, sizeof(name)-1, "/pit_%u%u%u.sem", n1, n2, n3);
        sem_unlink(name);
        if ((sem->s = sem_open(name, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, count)) == (sem_t *)SEM_FAILED) {
          debug_errno("MUTEX", "sem_open \"%s\"", name);
          xfree(sem);
          sem = NULL;
        } else {
          debug(DEBUG_TRACE, "MUTEX", "created named semaphore \"%s\" (%08x)", name, sem);
          sem->named = 1;
        }
      } else {
        debug_errno("MUTEX", "sem_init");
        xfree(sem);
        sem = NULL;
      }
    }
  }

  return sem;
}

int semaphore_post(sema_t *sem) {
  int r = -1;

  if (sem) {
    r = sem_post(sem->s);
    if (r != 0) {
      debug_errno("MUTEX", "sem_post");
    }
  }

  return r;
}

int semaphore_wait(sema_t *sem, int block) {
  int r = -1;

  if (sem) {
    if (block) {
      r = sem_wait(sem->s);
      if (r != 0) {
        debug_errno("MUTEX", "sem_wait");
      }
    } else {
      r = sem_trywait(sem->s);
      if (r != 0 && errno != EAGAIN) {
        debug_errno("MUTEX", "sem_trywait");
      }
    }
  }

  return r;
}

int semaphore_timedwait(sema_t *sem, int us) {
  struct timespec ts;
  int64_t t;
  int r = -1;

  if (sem) {
    t = sys_get_clock();
    t += us;
    ts.tv_sec = t / 1000000;
    t -= ts.tv_sec * 1000000;
    ts.tv_nsec = t * 1000;

    r = sem_timedwait(sem->s, &ts);

    if (r != 0 && errno != ETIMEDOUT) {
      debug_errno("MUTEX", "sem_timedwait");
    }
  }

  return r;
}

int semaphore_destroy(sema_t *sem) {
  int r = -1;

  if (sem) {
    if (sem->named) {
      if ((r = sem_close(sem->s)) != 0) {
        debug_errno("MUTEX", "sem_close");
      }
    } else {
      if ((r = sem_destroy(sem->s)) != 0) {
        debug_errno("MUTEX", "sem_destroy");
      }
    }
    
    if (r == 0) {
      debug(DEBUG_TRACE, "MUTEX", "destroyed semaphore (%08x)", sem);
    }
    xfree(sem);
  }

  return r;
}

int semaphore_remove_named(char *name) {
  int r = -1;

  if (name) {
    if ((r = sem_unlink(name)) != 0) {
      debug_errno("MUTEX", "sem_unlink");
    } else {
      debug(DEBUG_TRACE, "MUTEX", "removed named semaphore \"%s\"", name);
    }
  }

  return r;
}
