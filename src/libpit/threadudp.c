#define __USE_GNU
#include <pthread.h>
#undef __USE_GNU

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "debug.h"
#include "xalloc.h"

#define LOCALHOST "127.0.0.1"

#define MAX_PS_THREADS 256

struct thread_key_t {
  pthread_key_t key;
};

typedef struct {
  char *name;
  int (*action)(void *arg);
  void *arg;
  int sock;
  int port;
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

static double thread_usage(void) {
  int64_t tt, pt;
  double p;

  tt = sys_get_thread_time();
  pt = sys_get_process_time();
  if (pt != 0) {
    p = (double)tt / (double)(pt);
  } else {
    p = 0;
  }

  return p;
}

static void dummy_destructor(void *value) {
}

thread_key_t *thread_key(void) {
  struct thread_key_t *key = sys_malloc(sizeof(struct thread_key_t));
  if (key) {
    pthread_key_create(&key->key, dummy_destructor);
  }
  return key;
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

  return r;
}

static void thread_detach(void) {
  pthread_detach(pthread_self());
}

void thread_init(void) {
  int port;

  local = thread_key();
  tname = thread_key();

  sys_memset(&main_targ, 0, sizeof(main_targ));
  port = 0;
  main_targ.sock = sys_socket_bind(LOCALHOST, &port, IP_DGRAM);
  main_targ.port = port;
  main_targ.psi = 0;

  thread_set(local, &main_targ);
  thread_set(tname, "MAIN");

  sys_memset(ps, 0, sizeof(ps));
  ps[0].handle = port;
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
  if (main_targ.sock > 0) {
    sys_close(main_targ.sock);
  }
  mutex_destroy(flags_mutex);
  mutex_destroy(mutex);
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

static int thread_get_sock(void) {
  thread_arg_t *targ;

  targ = (thread_arg_t *)thread_get(local);
  return targ ? targ->sock : -1;
}

int thread_get_handle(void) {
  thread_arg_t *targ;

  targ = (thread_arg_t *)thread_get(local);
  return targ ? targ->port : -1;
}

int thread_must_end(void) {
  thread_arg_t *targ;
  double p;
  uint64_t t;

  if (thread_get_flags(FLAG_FINISH)) {
    debug(DEBUG_INFO, "THREAD", "thread must end (flag)");
    return 1;
  }

  targ = (thread_arg_t *)thread_get(local);

  if (targ == NULL || targ->sock <= 0 || sys_peek(targ->sock) != -1) {
    t = sys_time();
    if ((t - targ->last_usage) >= 15) {
      p = thread_usage();
      targ->last_usage = t;
      if (mutex_lock(mutex) == 0) {
        ps[targ->psi].p = p;
        mutex_unlock(mutex);
      }
    }

    return 0;
  }

  debug(DEBUG_INFO, "THREAD", "thread port %d must end", targ->port);
  return 1;
}

void *thread_setup(char *name) {
  thread_arg_t *targ;
  int sock, port;

  targ = (thread_arg_t *)thread_get(local);
  if (targ == NULL) {
    port = 0;
    if ((sock = sys_socket_bind(LOCALHOST, &port, IP_DGRAM)) != -1) {
      if ((targ = xcalloc(1, sizeof(thread_arg_t))) != NULL) {
        targ->name = name;
        targ->sock = sock;
        targ->port = port;
        thread_set(local, targ);
        thread_set_name(name);
      }
    } else {
      sys_close(sock);
    }
  }

  return targ;
}

int thread_unsetup(void *p) {
  thread_arg_t *targ;
  int r = -1;

  targ = (thread_arg_t *)p;
  if (targ) {
    if (targ->sock) sys_close(targ->sock);
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
        ps[i].handle = targ->port;
        ps[i].name = targ->name;
        ps[i].p = 0;
        targ->psi = i;
        break;
      }
    }
    mutex_unlock(mutex);
  }

  debug(DEBUG_INFO, "THREAD", "thread port %d begin", targ->port);
  targ->action(targ->arg);
  debug(DEBUG_INFO, "THREAD", "thread port %d end", targ->port);
  sys_close(targ->sock);

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
  int sock, port;

  port = 0;
  if ((sock = sys_socket_bind(LOCALHOST, &port, IP_DGRAM)) == -1) {
    return -1;
  }

  if ((targ = xcalloc(1, sizeof(thread_arg_t))) == NULL) {
    sys_close(sock);
    return -1;
  }

  targ->name = tag;
  targ->action = action;
  targ->arg = arg;
  targ->sock = sock;
  targ->port = port;
  debug(DEBUG_INFO, "THREAD", "thread sock %d bound to port %d", sock, port);

  thread_action(targ);

  sys_close(sock);
  xfree(targ);

  return 0;
}

int thread_begin(char *tag, int action(void *arg), void *arg) {
  thread_arg_t *targ;
  int sock, port;

  port = 0;
  if ((sock = sys_socket_bind(LOCALHOST, &port, IP_DGRAM)) == -1) {
    return -1;
  }

  if ((targ = xcalloc(1, sizeof(thread_arg_t))) == NULL) {
    sys_close(sock);
    return -1;
  }

  targ->name = tag;
  targ->action = action;
  targ->arg = arg;
  targ->sock = sock;
  targ->port = port;
  debug(DEBUG_INFO, "THREAD", "thread sock %d bound to port %d", sock, port);

  if (thread_create(thread_action, targ) == -1) {
    sys_close(sock);
    xfree(targ);
    return -1;
  }

  return port;
}

static int thread_write_port(int port, unsigned char *buf, unsigned int len) {
  int sock, r;

  sock = thread_get_sock();
  r = sys_socket_sendto(sock, LOCALHOST, port, buf, len);
  if (r == -1) {
    debug(DEBUG_ERROR, "THREAD", "write to port %d from sock %d failed", port, sock);
  }

  return r == len ? len : -1;
}

static int thread_read_sock(int sock, uint32_t usec, unsigned char **rbuf, unsigned int *len, int *client) {
  sys_timeval_t tv;
  uint8_t *buf;
  char host[32];
  int port, n;

  *rbuf = NULL;
  *len = 0;

  if ((buf = xcalloc(1, 65536)) == NULL) {
    return -1;
  }

  tv.tv_sec = 0;
  tv.tv_usec = usec;
  n = sys_socket_recvfrom(sock, host, sizeof(host)-1, &port, buf, 65536, usec == ((uint32_t)-1) ? NULL : &tv);

  if (n < 0) {
    debug(DEBUG_ERROR, "THREAD", "read from sock %d failed", sock);
    xfree(buf);
    return -1;
  }

  if (n == 0) {
    xfree(buf);
    return 0;
  }

  if (n == 1 && buf[0] == 0) {
    debug(DEBUG_INFO, "THREAD", "received finish packet");
    xfree(buf);
    return -1;
  }


  *rbuf = buf;
  *len = n;
  if (client) *client = port;

  return 1;
}

// used by thread clients
int thread_client_write(int port, unsigned char *buf, unsigned int len) {
  return thread_write_port(port, buf, len);
}

// used by thread clients
int thread_client_read_timeout(int handle, uint32_t usec, unsigned char **buf, unsigned int *len) {
  //return thread_read_sock(handle, usec, buf, len);
  return 0;
}

// used by thread clients
int thread_client_read(int handle, unsigned char **buf, unsigned int *len) {
  return thread_client_read_timeout(handle, 0, buf, len);
}

// used by thread action
int thread_server_write(unsigned char *buf, unsigned int len) {
  //return thread_write_port(thread_get_handle(), buf, len);
  return 0;
}

// used by thread action
int thread_server_read_timeout_from(uint32_t usec, unsigned char **buf, unsigned int *len, int *client) {
  return thread_read_sock(thread_get_sock(), usec, buf, len, client);
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
  return sys_peek(thread_get_sock());
}

int thread_end(char *tag, int handle) {
  uint8_t packet;
  int r;

  debug(DEBUG_INFO, "THREAD", "closing thread with port %d", handle);
  packet = 0;
  r = thread_client_write(handle, &packet, 1);

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
