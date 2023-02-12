#ifndef PIT_THREAD_H
#define PIT_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLAG_FINISH  1

#define STATUS_SUCCESS 0
#define STATUS_ERROR   1
#define STATUS_FAULT   2

typedef struct thread_key_t thread_key_t;

typedef struct {
  uint32_t tid;
  int handle;
  char *name;
  double p;
} thread_ps_t;

void thread_init(void);

void thread_close(void);

thread_key_t *thread_key(void);

void thread_set(thread_key_t *key, void *value);

void *thread_get(thread_key_t *key);

void thread_setmain(void);

int thread_must_end(void);

int thread_get_handle(void);

int thread_get_fd(void);

void *thread_setup(char *name);

int thread_unsetup(void *p);

int thread_begin(char *tag, int action(void *arg), void *arg);

int thread_begin2(char *tag, int action(void *arg), void *arg);

int thread_open_channel(int port);

int thread_close_channel(int fd);

int thread_write_fd(int fd, unsigned char *buf, unsigned int len);

int thread_read_fd(int fd, uint32_t usec, unsigned char **rbuf, unsigned int *len);

int thread_client_write(int handle, unsigned char *buf, unsigned int len);

int thread_client_read(int handle, unsigned char **buf, unsigned int *len);

int thread_client_read_timeout(int handle, uint32_t usec, unsigned char **buf, unsigned int *len);

int thread_server_write(unsigned char *buf, unsigned int len);

int thread_server_read(unsigned char **buf, unsigned int *len);

int thread_server_read_timeout(uint32_t usec, unsigned char **buf, unsigned int *len);

int thread_server_read_timeout_from(uint32_t usec, unsigned char **buf, unsigned int *len, int *client);

int thread_server_peek(void);

int thread_end(char *tag, int handle);

void thread_block_signals(void);

void thread_unblock_signals(void);

void thread_set_name(char *name);

void thread_get_name(char *name, int len);

void thread_wait_all(void);

void thread_set_flags(unsigned int mask);

void thread_reset_flags(unsigned int mask);

void thread_set_status(int status);

int thread_get_status(void);

unsigned int thread_get_flags(unsigned int mask);

thread_ps_t *thread_ps(void);

#ifdef __cplusplus
}
#endif

#endif
