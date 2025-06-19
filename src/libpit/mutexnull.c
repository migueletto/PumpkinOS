#include "sys.h"
#include "mutex.h"
#include "debug.h"

struct mutex_t {
  int dummy;
};

struct cond_t {
  int dummy;
};

struct sema_t {
  int dummy;
};

mutex_t *mutex_create(char *name) {
  return sys_calloc(1, sizeof(mutex_t));
}

int mutex_destroy(mutex_t *m) {
  int r = -1;

  if (m) {
    sys_free(m);
    r = 0;
  }

  return r;
}

int mutex_lock_only(mutex_t *m) {
  return 0;
}

int mutex_unlock_only(mutex_t *m) {
  return 0;
}

int mutex_lock(mutex_t *m) {
  return 0;
}

int mutex_unlock(mutex_t *m) {
  return 0;
}

cond_t *cond_create(char *name) {
  return sys_calloc(1, sizeof(cond_t));
}

int cond_destroy(cond_t *c) {
  int r = -1;

  if (c) {
    sys_free(c);
    r = 0;
  }

  return r;
}

int cond_signal(cond_t *c) {
  return 0;
}

int cond_broadcast(cond_t *c) {
  return 0;
}

int cond_wait(cond_t *c, mutex_t *m) {
  return 0;
}

int cond_timedwait(cond_t *c, mutex_t *m, int us) {
  return 0;
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
