#ifndef PIT_MUTEX_H
#define PIT_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mutex_t mutex_t;
typedef struct cond_t cond_t;
typedef struct sema_t sema_t;

mutex_t *mutex_create(char *name);

int mutex_destroy(mutex_t *m);

int mutex_lock(mutex_t *m);

int mutex_unlock(mutex_t *m);

cond_t *cond_create(char *name);

int cond_destroy(cond_t *c);

int cond_signal(cond_t *c);

int cond_broadcast(cond_t *c);

int cond_wait(cond_t *c, mutex_t *m);

int cond_timedwait(cond_t *c, mutex_t *m, int us);

sema_t *semaphore_create_named(char *name, int count);

int semaphore_remove_named(char *name);

sema_t *semaphore_create(int count);

int semaphore_destroy(sema_t *sem);

int semaphore_post(sema_t *sem);

int semaphore_wait(sema_t *sem, int block);

int semaphore_timedwait(sema_t *sem, int us);

#ifdef __cplusplus
}
#endif

#endif
