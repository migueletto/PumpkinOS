#define __USE_GNU
#include <pthread.h>
#undef __USE_GNU

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_QUEUE "thread_queue"
#define MAX_QUEUE 256

#define MAX_PS_THREADS 256

struct thread_key_t {
  pthread_key_t key;
};

typedef struct {
  int sender;
  unsigned int size;
  unsigned int len;
  unsigned char *buf;
} msg_t;

typedef struct {
  char *tag;
  msg_t msg[MAX_QUEUE];
  int n, in, out;
} queue_t;

typedef struct {
  char *name;
  int (*action)(void *arg);
  void *arg;
  int queue;
  int psi;
  uint64_t last_usage;
} thread_arg_t;

static thread_arg_t main_targ;
static thread_key_t *local;
static thread_key_t *tname;
static mutex_t *flags_mutex;
static int flags, status;

static mutex_t *mutex;
static unsigned int num_threads;
static thread_ps_t ps[MAX_PS_THREADS];

static void dummy_destructor(void *value) {
}

thread_key_t *thread_key(void) {
  struct thread_key_t *key = sys_malloc(sizeof(struct thread_key_t));
  if (key) {
    pthread_key_create(&key->key, dummy_destructor);
  }
  return key;
}

void thread_key_delete(thread_key_t *key) {
  if (key) {
    pthread_key_delete(key->key);
    sys_free(key);
  }
}

void thread_set(thread_key_t *key, void *value) {
  pthread_setspecific(key->key, value);
}

void *thread_get(thread_key_t *key) {
  return pthread_getspecific(key->key);
}

static int thread_create(void *(*action)(void *), void *arg) {
  pthread_t t;
  int err, r = 0;

  if ((err = pthread_create(&t, NULL, action, arg)) != 0) {
    debug(DEBUG_ERROR, "THREAD", "pthread_create: %d", err);
    r = -1;
  }
  debug(DEBUG_INFO, "THREAD", "pthread_create %p", t);

  return r;
}

static void thread_detach(void) {
  pthread_detach(pthread_self());
}

static void queue_destructor(void *p) {
  queue_t *q = (queue_t *)p;
  msg_t *msg;
  int i;

  if (q) {
    for (i = 0; i < MAX_QUEUE; i++) {
      msg = &q->msg[i];
      if (msg->buf) {
        sys_free(msg->buf);
      }
    }
    sys_free(q);
  }
}

static int queue_new(void) {
  queue_t *q;
  int queue = -1;

  if ((q = sys_calloc(1, sizeof(queue_t))) != NULL) {
    q->tag = TAG_QUEUE;
    queue = ptr_new(q, queue_destructor);
  }

  return queue;
}

static int queue_get(int queue, uint8_t **buf, uint32_t *len, int *sender) {
  msg_t *msg;
  queue_t *q;
  int r = -1;

  if ((q = ptr_lock(queue, TAG_QUEUE)) != NULL) {
    if (q->n > 0) {
      msg = &q->msg[q->out];
      if (sender) *sender = msg->sender;
      *len = msg->len;
      *buf = sys_calloc(1, msg->len);
      sys_memcpy(*buf, msg->buf, msg->len);
      q->out++;
      if (q->out == MAX_QUEUE) q->out = 0;
      q->n--;
      r = 1;
    } else {
      r = 0;
    }
    ptr_unlock(queue, TAG_QUEUE);
  }

  return r;
}

static int queue_put(int queue, uint8_t *buf, uint32_t len) {
  msg_t *msg;
  queue_t *q;
  int r = -1;

  if ((q = ptr_lock(queue, TAG_QUEUE)) != NULL) {
    if (q->n < MAX_QUEUE) {
      msg = &q->msg[q->in];
      msg->sender = thread_get_handle();
      msg->len = len;
      if (msg->buf == NULL) {
        msg->size = msg->len;
        msg->buf = sys_calloc(1, msg->size);
      } else if (msg->size < msg->len) {
        msg->size = msg->len;
        msg->buf = sys_realloc(msg->buf, msg->size);
      }
      sys_memcpy(msg->buf, buf, msg->len);
      q->in++;
      if (q->in == MAX_QUEUE) q->in = 0;
      q->n++;
      r = 0;
    } else {
      r = -1;
    }
    ptr_unlock(queue, TAG_QUEUE);
  }

  return r;
}

void thread_init(void) {
  local = thread_key();
  tname = thread_key();

  sys_memset(&main_targ, 0, sizeof(main_targ));
  main_targ.queue = queue_new();
  main_targ.psi = 0;

  thread_set(local, &main_targ);
  thread_set(tname, "MAIN");

  sys_memset(ps, 0, sizeof(ps));
  ps[0].handle = main_targ.queue;
  ps[0].name = "MAIN";

  flags_mutex = mutex_create("thread_flags");
  status = STATUS_SUCCESS;
  flags = 0;

  mutex = mutex_create("thread");
  num_threads = 0;
}

void thread_setmain(void) {
  ps[0].tid = sys_get_tid();
}

void thread_close(void) {
  if (main_targ.queue > 0) {
    ptr_free(main_targ.queue, TAG_QUEUE);
  }
  mutex_destroy(flags_mutex);
  mutex_destroy(mutex);
  thread_key_delete(tname);
  thread_key_delete(local);
}

void thread_set_name(char *name) {
  thread_set(tname, name);
  sys_set_thread_name(name);
}

void thread_get_name(char *name, int len) {
  char *s;
  int i;

  s = (char *)thread_get(tname);
  if (s == NULL) {
    s = "???";
  }

  for (i = 0; s[i] && i < len-1; i++) {
    name[i] = s[i];
  }
  name[i] = 0;
}

static int thread_get_queue(void) {
  thread_arg_t *targ;

  targ = (thread_arg_t *)thread_get(local);
  return targ ? targ->queue : -1;
}

int thread_get_handle(void) {
  thread_arg_t *targ;

  targ = (thread_arg_t *)thread_get(local);
  return targ ? targ->queue : -1;
}

int thread_must_end(void) {
  if (thread_get_flags(FLAG_FINISH)) {
    debug(DEBUG_INFO, "THREAD", "thread must end (flag)");
    return 1;
  }

  return 0;
}

void *thread_setup(char *name) {
  thread_arg_t *targ;
  int queue;

  targ = (thread_arg_t *)thread_get(local);
  if (targ == NULL) {
    if ((queue = queue_new()) != -1) {
      if ((targ = xcalloc(1, sizeof(thread_arg_t))) != NULL) {
        targ->name = name;
        targ->queue = queue;
        thread_set(local, targ);
        thread_set_name(name);
      }
    }
  }

  return targ;
}

int thread_unsetup(void *p) {
  thread_arg_t *targ;
  int r = -1;

  targ = (thread_arg_t *)p;
  if (targ) {
    if (targ->queue) ptr_free(targ->queue, TAG_QUEUE);
    xfree(targ);
    r = 0;
  }

  return r;
}

static void *thread_action(void *arg) {
  thread_arg_t *targ;
  uint32_t tid;
  int i;

  targ = (thread_arg_t *)arg;
  thread_detach();
  sys_block_signals();

  thread_set(local, targ);
  thread_set_name(targ->name);
  tid = sys_get_tid();

  if (mutex_lock(mutex) == 0) {
    num_threads++;
    for (i = 1; i < MAX_PS_THREADS; i++) {
      if (ps[i].tid == 0) {
        ps[i].tid = tid;
        ps[i].handle = targ->queue;
        ps[i].name = targ->name;
        ps[i].p = 0;
        targ->psi = i;
        break;
      }
    }
    mutex_unlock(mutex);
  }

  debug(DEBUG_INFO, "THREAD", "thread queue %d begin", targ->queue);
  targ->action(targ->arg);
  debug(DEBUG_INFO, "THREAD", "thread queue %d end", targ->queue);
  ptr_free(targ->queue, TAG_QUEUE);

  if (mutex_lock(mutex) == 0) {
    num_threads--;
    for (i = 1; i < MAX_PS_THREADS; i++) {
      if (ps[i].tid == tid && !sys_strcmp(ps[i].name, targ->name)) {
        ps[i].tid = 0;
        ps[i].handle = 0;
        ps[i].name = NULL;
        ps[i].p = 0;
        break;
      }
    }
    mutex_unlock(mutex);
  }

  thread_set(local, NULL);
  xfree(targ);
  thread_set(tname, NULL);

  return NULL;
}

thread_ps_t *thread_ps(void) {
  thread_ps_t *r = NULL;
  int i, j;

  if (mutex_lock(mutex) == 0) {
    if ((r = xcalloc(num_threads+2, sizeof(thread_ps_t))) != NULL) {
      r[0].tid = ps[0].tid;
      r[0].handle = ps[0].handle;
      r[0].name = xstrdup(ps[0].name);
      r[0].p = ps[0].p;

      for (i = 1, j = 0; i < MAX_PS_THREADS && j < num_threads; i++) {
        if (ps[i].tid) {
          r[j+1].tid = ps[i].tid;
          r[j+1].handle = ps[i].handle;
          r[j+1].name = xstrdup(ps[i].name);
          r[j+1].p = ps[i].p;
          j++;
        }
      }
    }
    mutex_unlock(mutex);
  }

  return r;
}

int thread_begin2(char *tag, int action(void *arg), void *arg) {
  thread_arg_t *targ;
  int queue;

  if ((queue = queue_new()) == -1) {
    return -1;
  }

  if ((targ = xcalloc(1, sizeof(thread_arg_t))) == NULL) {
    ptr_free(queue, TAG_QUEUE);
    return -1;
  }

  targ->name = tag;
  targ->action = action;
  targ->arg = arg;
  targ->queue = queue;
  debug(DEBUG_INFO, "THREAD", "thread queue %d", queue);

  thread_action(targ);

  ptr_free(queue, TAG_QUEUE);
  xfree(targ);

  return 0;
}

int thread_begin(char *tag, int action(void *arg), void *arg) {
  thread_arg_t *targ;
  int queue;

  if ((queue = queue_new()) == -1) {
    return -1;
  }

  if ((targ = xcalloc(1, sizeof(thread_arg_t))) == NULL) {
    ptr_free(queue, TAG_QUEUE);
    return -1;
  }

  targ->name = tag;
  targ->action = action;
  targ->arg = arg;
  targ->queue = queue;
  debug(DEBUG_INFO, "THREAD", "thread queue %d", queue);

  if (thread_create(thread_action, targ) == -1) {
    ptr_free(queue, TAG_QUEUE);
    xfree(targ);
    return -1;
  }

  return queue;
}

static int thread_write_queue(int queue, unsigned char *buf, unsigned int len) {
  int r = -1;

  r = queue_put(queue, buf, len);
  if (r == -1) {
    debug(DEBUG_ERROR, "THREAD", "write to queue %d failed", queue);
  }

  return r == 0 ? len : -1;
}

static int thread_read_queue(int queue, uint32_t usec, unsigned char **rbuf, unsigned int *len, int *client) {
  uint32_t dt;
  int i, n, r;

  *rbuf = NULL;
  *len = 0;

  if ((r = queue_get(queue, rbuf, len, client)) == -1) {
    debug(DEBUG_ERROR, "THREAD", "read from queue %d failed", queue);
    return -1;
  }

  if (r == 0) {
    if (usec == 0) return 0;

    if (usec >= 100) {
      n = 4;
    } else {
      n = 1;
    }
    dt = usec / n;

    for (i = 0; i < n; i++) {
      sys_usleep(dt);
      if ((r = queue_get(queue, rbuf, len, client)) == -1) {
        debug(DEBUG_ERROR, "THREAD", "read from queue %d failed", queue);
        return -1;
      }
      if (r) break;
    }

    if (r == 0) return 0;
  }

  if (*len == 1 && (*rbuf)[0] == 0) {
    debug(DEBUG_INFO, "THREAD", "received finish packet");
    return -1;
  }

  return 1;
}

// used by thread clients
int thread_client_write(int queue, unsigned char *buf, unsigned int len) {
  return thread_write_queue(queue, buf, len);
}

// used by thread clients
int thread_client_read_timeout(int handle, uint32_t usec, unsigned char **buf, unsigned int *len) {
  //return thread_read_queue(handle, usec, buf, len);
  return 0;
}

// used by thread clients
int thread_client_read(int handle, unsigned char **buf, unsigned int *len) {
  return thread_client_read_timeout(handle, 0, buf, len);
}

// used by thread action
int thread_server_write(unsigned char *buf, unsigned int len) {
  //return thread_write_queue(thread_get_handle(), buf, len);
  return 0;
}

// used by thread action
int thread_server_read_timeout_from(uint32_t usec, unsigned char **buf, unsigned int *len, int *client) {
  return thread_read_queue(thread_get_queue(), usec, buf, len, client);
}

// used by thread action
int thread_server_read_timeout(uint32_t usec, unsigned char **buf, unsigned int *len) {
  return thread_server_read_timeout_from(usec, buf, len, NULL);
}

// used by thread action
int thread_server_read(unsigned char **buf, unsigned int *len) {
  return thread_server_read_timeout(0, buf, len);
}

int thread_server_peek(void) {
  return sys_peek(thread_get_queue());
}

int thread_end(char *tag, int queue) {
  uint8_t packet;
  int r;

  debug(DEBUG_INFO, "THREAD", "closing thread with queue %d", queue);
  packet = 0;
  r = thread_client_write(queue, &packet, 1);

  return r;
}

void thread_wait_all(void) {
  int i, n;

  for (i = 0, n = -1; i < 30; i++) {
    if (mutex_lock(mutex) == 0) {
      n = num_threads;
      mutex_unlock(mutex);
    }
    if (n <= 0) break;
    debug(DEBUG_INFO, "THREAD", "waiting for %d thread(s)", n);
    sys_usleep(500000);
    n = -1;
  }

  if (n == 0) {
    debug(DEBUG_INFO, "THREAD", "all threads finished");
  } else if (n > 0) {
    debug(DEBUG_ERROR, "THREAD", "%d thread(s) did not finish", n);
  } else {
    debug(DEBUG_ERROR, "THREAD", "error waiting for threads");
  }
}

void thread_set_flags(unsigned int mask) {
  if (mutex_lock(flags_mutex) == 0) {
    flags |= mask;
    mutex_unlock(flags_mutex);
  }
}

void thread_reset_flags(unsigned int mask) {
  if (mutex_lock(flags_mutex) == 0) {
    flags &= ~mask;
    mutex_unlock(flags_mutex);
  }
}

void thread_set_status(int _status) {
  if (mutex_lock(flags_mutex) == 0) {
    status = _status;
    mutex_unlock(flags_mutex);
  }
}

int thread_get_status(void) {
  int _status = 0;

  if (mutex_lock(flags_mutex) == 0) {
    _status = status;
    mutex_unlock(flags_mutex);
  }

  return _status;
}

unsigned int thread_get_flags(unsigned int mask) {
  unsigned int r = 0;

  if (mutex_lock(flags_mutex) == 0) {
    r = flags & mask;
    mutex_unlock(flags_mutex);
  }

  return r;
}

void thread_yield(int waiting) {
}

void thread_resume(int handle) {
}

int thread_needs_run(void) {
  return 0;
}

void thread_run(void) {
}
