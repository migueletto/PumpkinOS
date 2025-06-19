#include "sys.h"
#include "thread.h"
#include "debug.h"

#define MAX_KEYS     256
#define MAX_THREADS  256
#define MAX_MESSAGES 256
#define STACK_SIZE   (1024 * 1024)

struct thread_key_t {
  int id, inuse;
};

typedef struct {
  int client;
  uint32_t len;
  void *buf;
} msg_t;

typedef struct {
  int inuse;
  int id;
  char *name;
  int (*action)(void *arg);
  void *arg;
  void *values[MAX_KEYS];
  msg_t messages[MAX_MESSAGES];
  uint32_t nmsg, imsg, omsg;
  void *stack_bottom;
  void *stack_top;
  uint32_t stack_size;
  jmp_buf jbuf;
} thread_arg_t;

static thread_arg_t tasks[MAX_THREADS];
static thread_key_t *local;
static thread_key_t *tname;
static int flags, status;

static int current;

static uint32_t num_threads;
static thread_ps_t ps[MAX_THREADS];

static uint32_t num_keys;
static thread_key_t keys[MAX_KEYS];

void thread_init(void) {
  sys_memset(tasks, 0, sizeof(tasks));
  sys_memset(keys, 0, sizeof(keys));
  sys_memset(ps, 0, sizeof(ps));

  local = thread_key();
  tname = thread_key();

  status = STATUS_SUCCESS;
  flags = 0;

  num_threads = 0;
  num_keys = 0;
  current = 0;
}

void thread_close(void) {
  thread_key_delete(tname);
  thread_key_delete(local);
}

void thread_yield(int waiting) {
}

void thread_resume(int id) {
}

int thread_needs_run(void) {
  return 0;
}

void thread_run(void) {
}

thread_key_t *thread_key(void) {
  thread_key_t *key = NULL;
  int i;

  for (i = 0; i < MAX_KEYS; i++) {
    if (keys[i].inuse == 0) {
      key = &keys[i];
      key->id = i+1;
      key->inuse = 1;
      num_keys++;
      break;
    }
  }

  return key;
}

void thread_key_delete(thread_key_t *key) {
  if (key && key->inuse) {
    key->id = 0;
    key->inuse = 0;
    num_keys--;
  }
}

void thread_set(thread_key_t *key, void *value) {
  thread_arg_t *targ;

  if (key && key->inuse) {
    targ = &tasks[current];
    targ->values[key->id - 1] = value;
  }
}

void *thread_get(thread_key_t *key) {
  void *value = NULL;
  thread_arg_t *targ;

  if (key && key->inuse) {
    targ = &tasks[current];
    value = targ->values[key->id - 1];
  }

  return value;
}

void thread_setmain(void) {
}

int thread_must_end(void) {
  if (thread_get_flags(FLAG_FINISH)) {
    debug(DEBUG_INFO, "THREAD", "thread must end (flag)");
    return 1;
  }

  return 0;
}

int thread_get_handle(void) {
  thread_arg_t *targ;

  targ = (thread_arg_t *)thread_get(local);
  return targ ? targ->id : -1;
}

int thread_begin(char *tag, int action(void *arg), void *arg) {
  thread_arg_t *targ = NULL;
  int i;

  for (i = 0; i < MAX_THREADS; i++) {
    if (tasks[i].inuse == 0) {
      targ = &tasks[i];
      break;
    }
  }

  if (targ == NULL) {
    return -1;
  }

  targ->inuse = 1;
  targ->id = i;
  targ->name = tag;
  targ->action = action;
  targ->arg = arg;

  targ->stack_size = STACK_SIZE;
  targ->stack_bottom = sys_malloc(targ->stack_size);
  targ->stack_top = targ->stack_bottom + targ->stack_size;

  num_threads++;

  return i;
}

int thread_client_write(int id, unsigned char *buf, unsigned int len) {
  thread_arg_t *targ;
  int r = -1;

  if (buf && len) {
    if (id >= 0 && id < MAX_THREADS) {
      targ = &tasks[id];

      if (targ->inuse) {
        if (targ->nmsg < MAX_MESSAGES) {
          targ->messages[targ->imsg].client = thread_get_handle();
          targ->messages[targ->imsg].len = len;
          targ->messages[targ->imsg].buf = sys_malloc(len);
          sys_memcpy(targ->messages[targ->imsg].buf, buf, len);
          targ->imsg++;
          if (targ->imsg == MAX_MESSAGES) targ->imsg = 0;
          targ->nmsg++;
          r = len;
        } else {
          debug(DEBUG_ERROR, "THREAD", "thread_client_write id %d queue overflow", id);
        }
      } else {
        debug(DEBUG_ERROR, "THREAD", "thread_client_write id %d not in use", id);
      }
    } else {
      debug(DEBUG_ERROR, "THREAD", "thread_client_write invalid id %d", id);
    }
  }

  return r;
}

int thread_server_read(unsigned char **buf, unsigned int *len) {
  return thread_server_read_timeout(0, buf, len);
}

int thread_server_read_timeout(uint32_t usec, unsigned char **buf, unsigned int *len) {
  return thread_server_read_timeout_from(usec, buf, len, NULL);
}

int thread_server_read_timeout_from(uint32_t usec, unsigned char **buf, unsigned int *len, int *client) {
  thread_arg_t *targ;
  int r = 0;
  
  if (buf && len) {
    targ = &tasks[current];
    if (targ->nmsg == 0) return 0;

    if (client) *client = targ->messages[targ->omsg].client;
    *len = targ->messages[targ->omsg].len;
    *buf = targ->messages[targ->omsg].buf;
    targ->messages[targ->omsg].buf = NULL;
    targ->messages[targ->omsg].len = 999;
    targ->messages[targ->omsg].client = 0;
    targ->omsg++;
    if (targ->omsg == MAX_MESSAGES) targ->omsg = 0;
    targ->nmsg--;
    r = 1;

    if (*len == 1 && (*buf)[0] == 0) {
      debug(DEBUG_INFO, "THREAD", "received finish packet");
      r = -1;
    }
  }

  return r;
}

int thread_server_peek(void) {
  thread_arg_t *targ;
  
  targ = &tasks[current];

  return targ->nmsg > 0;
}

int thread_end(char *tag, int id) {
  uint8_t packet;
  int r;

  debug(DEBUG_INFO, "THREAD", "closing thread with id %d", id);
  packet = 0;
  r = thread_client_write(id, &packet, 1);

  return r;
}

void thread_set_name(char *name) {
  thread_set(tname, name);
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

void thread_wait_all(void) {
  // XXX
}

void thread_set_flags(unsigned int mask) {
  flags |= mask;
}

void thread_reset_flags(unsigned int mask) {
  flags &= ~mask;
}

void thread_set_status(int _status) {
  status = _status;
}

int thread_get_status(void) {
  return status;
}

unsigned int thread_get_flags(unsigned int mask) {
  return flags & mask;
}

thread_ps_t *thread_ps(void) {
  thread_ps_t *r = NULL;
  int i, j;

  if ((r = sys_calloc(num_threads + 2, sizeof(thread_ps_t))) != NULL) {
    r[0].tid = 0;
    r[0].handle = 0;
    r[0].name = sys_strdup("MAIN");
    r[0].p = 0;

    for (i = 1, j = 0; i < MAX_THREADS && j < num_threads; i++) {
      if (tasks[i].inuse) {
        r[j+1].tid = i;
        r[j+1].handle = tasks[i].id;
        r[j+1].name = sys_strdup(tasks[i].name);
        r[j+1].p = 0;
        j++;
      }
    }
  }

  return r;
}
